/*=========================================================================
This file is part of CustusX, an Image Guided Therapy Application.

Copyright (c) SINTEF Department of Medical Technology.
All rights reserved.

CustusX is released under a BSD 3-Clause license.

See Lisence.txt (https://github.com/SINTEFMedtek/CustusX/blob/master/License.txt) for details.
=========================================================================*/

#include "cxAirwaysFromCenterline.h"
#include <boost/math/special_functions/round.hpp>
#include <vtkPolyData.h>
#include "cxBranchList.h"
#include "cxBranch.h"
#include "vtkCardinalSpline.h"
#include <cxImage.h>
#include "cxContourFilter.h"
#include <vtkImageData.h>
#include <vtkPointData.h>
#include "cxVolumeHelpers.h"


namespace cx
{

AirwaysFromCenterline::AirwaysFromCenterline():
    mBranchListPtr(new BranchList),
    mAirwaysVolumeBoundaryExtention(10),
    mAirwaysVolumeBoundaryExtentionTracheaStart(2),
    mAirwaysVolumeSpacing(0.5)
{
}

AirwaysFromCenterline::~AirwaysFromCenterline()
{
}

void AirwaysFromCenterline::setTypeToBloodVessel(bool bloodVessel)
{
	mBloodVessel = bloodVessel;
}

Eigen::MatrixXd AirwaysFromCenterline::getCenterlinePositions(vtkPolyDataPtr centerline_r)
{

    int N = centerline_r->GetNumberOfPoints();
    Eigen::MatrixXd CLpoints(3,N);
    for(vtkIdType i = 0; i < N; i++)
        {
        double p[3];
        centerline_r->GetPoint(i,p);
        Eigen::Vector3d position;
		position(0) = p[0]; position(1) = p[1]; position(2) = p[2];
		CLpoints.block(0 , i , 3 , 1) = position;
		}
	return CLpoints;
}

void AirwaysFromCenterline::setBranches(BranchListPtr branches)
{
	mBranchListPtr = branches;
}

void AirwaysFromCenterline::processCenterline(vtkPolyDataPtr centerline_r)
{
	if (mBranchListPtr)
		mBranchListPtr->deleteAllBranches();

    Eigen::MatrixXd CLpoints_r = getCenterlinePositions(centerline_r);

    mBranchListPtr->findBranchesInCenterline(CLpoints_r);

    mBranchListPtr->smoothBranchPositions(40);
    mBranchListPtr->interpolateBranchPositions(5);
}

//Not in use? Delete?
void AirwaysFromCenterline::processCenterline(Eigen::MatrixXd CLpoints_r)
{
	if (mBranchListPtr)
		mBranchListPtr->deleteAllBranches();

    mBranchListPtr->findBranchesInCenterline(CLpoints_r);

    mBranchListPtr->smoothBranchPositions(40);
    mBranchListPtr->interpolateBranchPositions(5);
}

/*
    AirwaysFromCenterline::generateTubes makes artificial airway tubes around the input centerline. The radius
    of the tubes is decided by the generation number, based on Weibel's model of airways. In contradiction to the model,
    it is set a lower boundary for the tube radius (2 mm) making the peripheral airways larger than in reality,
    which makes it possible to virtually navigate inside the tubes. The airways are generated by adding a sphere to
    a volume (image) at each point along every branch. The output is a surface model generated from the volume.
*/
vtkPolyDataPtr AirwaysFromCenterline::generateTubes(double staticRadius) // if staticRadius == 0, radius is retrieved from branch generation number
{
    vtkImageDataPtr airwaysVolumePtr = this->initializeAirwaysVolume();

    airwaysVolumePtr = addSpheresAlongCenterlines(airwaysVolumePtr, staticRadius);

    //create contour from image
    vtkPolyDataPtr rawContour = ContourFilter::execute(
                airwaysVolumePtr,
            1, //treshold
            false, // reduce resolution
            true, // smoothing
            true, // keep topology
            0, // target decimation
            30, // number of iterations smoothing
            0.10 // band pass smoothing
    );

    return rawContour;
}

vtkImageDataPtr AirwaysFromCenterline::initializeAirwaysVolume()
{
    std::vector<BranchPtr> branches = mBranchListPtr->getBranches();
    vtkPointsPtr pointsPtr = vtkPointsPtr::New();

    int numberOfPoints = 0;
    for (int i = 0; i < branches.size(); i++)
        numberOfPoints += branches[i]->getPositions().cols();

    pointsPtr->SetNumberOfPoints(numberOfPoints);

    int pointIndex = 0;
    for (int i = 0; i < branches.size(); i++)
    {
        Eigen::MatrixXd positions = branches[i]->getPositions();
        for (int j = 0; j < positions.cols(); j++)
        {
            pointsPtr->SetPoint(pointIndex, positions(0,j), positions(1,j), positions(2,j));
            pointIndex += 1;
        }
    }

    pointsPtr->GetBounds(mBounds);

    //Extend bounds to make room for surface model extended from centerline
    mBounds[0] -= mAirwaysVolumeBoundaryExtention;
    mBounds[1] += mAirwaysVolumeBoundaryExtention;
    mBounds[2] -= mAirwaysVolumeBoundaryExtention;
    mBounds[3] += mAirwaysVolumeBoundaryExtention;
    mBounds[4] -= mAirwaysVolumeBoundaryExtention;
    mBounds[5] -= mAirwaysVolumeBoundaryExtentionTracheaStart; // to make top of trachea open
    if (mBloodVessel)
    	mBounds[5] += mAirwaysVolumeBoundaryExtention;

    mSpacing[0] = mAirwaysVolumeSpacing;  //Smaller spacing improves resolution but increases run-time
    mSpacing[1] = mAirwaysVolumeSpacing;
    mSpacing[2] = mAirwaysVolumeSpacing;

    // compute dimensions
    for (int i = 0; i < 3; i++)
        mDim[i] = static_cast<int>(std::ceil((mBounds[i * 2 + 1] - mBounds[i * 2]) / mSpacing[i]));

    mOrigin[0] = mBounds[0] + mSpacing[0] / 2;
    mOrigin[1] = mBounds[2] + mSpacing[1] / 2;
    mOrigin[2] = mBounds[4] + mSpacing[2] / 2;

    vtkImageDataPtr airwaysVolumePtr = generateVtkImageData(mDim, mSpacing, 0);
    airwaysVolumePtr->SetOrigin(mOrigin);

    return airwaysVolumePtr;
}


vtkImageDataPtr AirwaysFromCenterline::addSpheresAlongCenterlines(vtkImageDataPtr airwaysVolumePtr, double staticRadius)
{
    std::vector<BranchPtr> branches = mBranchListPtr->getBranches();

    for (int i = 0; i < branches.size(); i++)
    {
        Eigen::MatrixXd positions = branches[i]->getPositions();
        vtkPointsPtr pointsPtr = vtkPointsPtr::New();
        int numberOfPositionsInBranch = positions.cols();
        pointsPtr->SetNumberOfPoints(numberOfPositionsInBranch);

        double radius = staticRadius;
        if (staticRadius == 0)
        	radius = branches[i]->findBranchRadius();

        for (int j = 0; j < numberOfPositionsInBranch; j++)
        {
            double spherePos[3];
            spherePos[0] = positions(0,j);
            spherePos[1] = positions(1,j);
            spherePos[2] = positions(2,j);
            airwaysVolumePtr = addSphereToImage(airwaysVolumePtr, spherePos, radius);
        }
    }
    return airwaysVolumePtr;
}

vtkImageDataPtr AirwaysFromCenterline::addSphereToImage(vtkImageDataPtr airwaysVolumePtr, double position[3], double radius)
{
    int value = 1;
    int centerIndex[3];
    int sphereBoundingBoxIndex[6];

    for (int i=0; i<3; i++)
    {
        centerIndex[i] = static_cast<int>(boost::math::round( (position[i]-mOrigin[i]) / mSpacing[i] ));
        sphereBoundingBoxIndex[2*i] = std::max(
                    static_cast<int>(boost::math::round( (position[i]-mOrigin[i] - radius) / mSpacing[i] )),
                    0);
        sphereBoundingBoxIndex[2*i+1] = std::min(
                    static_cast<int>(boost::math::round( (position[i]-mOrigin[i] + radius) / mSpacing[i] )),
                    mDim[i]-1);
    }


    for (int x = sphereBoundingBoxIndex[0]; x<=sphereBoundingBoxIndex[1]; x++)
        for (int y = sphereBoundingBoxIndex[2]; y<=sphereBoundingBoxIndex[3]; y++)
            for (int z = sphereBoundingBoxIndex[4]; z<=sphereBoundingBoxIndex[5]; z++)
            {
                double distanceFromCenter = sqrt((x-centerIndex[0])*mSpacing[0]*(x-centerIndex[0])*mSpacing[0] +
                                                 (y-centerIndex[1])*mSpacing[1]*(y-centerIndex[1])*mSpacing[1] +
                                                 (z-centerIndex[2])*mSpacing[2]*(z-centerIndex[2])*mSpacing[2]);

                if (distanceFromCenter < radius)
                {
                    unsigned char* dataPtrImage = static_cast<unsigned char*>(airwaysVolumePtr->GetScalarPointer(x,y,z));
                    dataPtrImage[0] = value;
                }
            }

    return airwaysVolumePtr;
}

vtkPolyDataPtr AirwaysFromCenterline::getVTKPoints()
{
    vtkPolyDataPtr retval = vtkPolyDataPtr::New();
    vtkPointsPtr points = vtkPointsPtr::New();
    vtkCellArrayPtr lines = vtkCellArrayPtr::New();

    if (!mBranchListPtr)
            return retval;

		double minPointDistance = 0.5; //mm
		mBranchListPtr->excludeClosePositionsInCTCenterline(minPointDistance); // to reduce number of positions in smoothed centerline

    std::vector<BranchPtr> branches  = mBranchListPtr->getBranches();
    int pointIndex = 0;

    for (int i = 0; i < branches.size(); i++)
    {
        Eigen::MatrixXd positions = branches[i]->getPositions();
        int numberOfPositions = positions.cols();

        if (branches[i]->getParentBranch()) // Add parents last position to get connected centerlines
        {
            Eigen::MatrixXd parentPositions = branches[i]->getParentBranch()->getPositions();
            points->InsertNextPoint(parentPositions(0,parentPositions.cols()-1),parentPositions(1,parentPositions.cols()-1),parentPositions(2,parentPositions.cols()-1));
            pointIndex += 1;
        }

        for (int j = 0; j < numberOfPositions; j++)
        {
            points->InsertNextPoint(positions(0,j),positions(1,j),positions(2,j));

            if (j>1 || branches[i]->getParentBranch())
            {
                vtkIdType connection[2] = {pointIndex-1, pointIndex};
                lines->InsertNextCell(2, connection);
            }
            pointIndex += 1;
        }
    }

    retval->SetPoints(points);
    retval->SetLines(lines);
    return retval;
}

} /* namespace cx */
