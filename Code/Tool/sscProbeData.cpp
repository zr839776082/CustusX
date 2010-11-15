#include "sscProbeData.h"


#include "vtkImageData.h"
#include <vtkPointData.h>
#include <vtkUnsignedCharArray.h>
#include <vtkPolyData.h>
#include <vtkCellArray.h>


namespace ssc
{

namespace
{
  vtkImageDataPtr generateVtkImageData(Vector3D dim, Vector3D spacing, const unsigned char initValue)
  {
    vtkImageDataPtr data = vtkImageDataPtr::New();
    data->SetSpacing(spacing[0], spacing[1], spacing[2]);
    data->SetExtent(0, dim[0]-1, 0, dim[1]-1, 0, dim[2]-1);
    data->SetScalarTypeToUnsignedChar();
    data->SetNumberOfScalarComponents(1);

    int scalarSize = dim[0]*dim[1]*dim[2];

    unsigned char *rawchars = (unsigned char*)malloc(scalarSize+1);
    std::fill(rawchars,rawchars+scalarSize, initValue);

    vtkUnsignedCharArrayPtr array = vtkUnsignedCharArrayPtr::New();
    array->SetNumberOfComponents(1);
    //TODO: Whithout the +1 the volume is black
    array->SetArray(rawchars, scalarSize+1, 0); // take ownership
    data->GetPointData()->SetScalars(array);

    // A trick to get a full LUT in ssc::Image (automatic LUT generation)
    // Can't seem to fix this by calling Image::resetTransferFunctions() after volume is modified
    rawchars[0] = 255;
    data->GetScalarRange();// Update internal data in vtkImageData. Seems like it is not possible to update this data after the volume has been changed.
    rawchars[0] = 0;

    return data;
  }
}

ProbeData::ProbeData() : mType(tNONE)
{

  // testdata:
  mType = tSECTOR;
  mDepthStart = 0;
//  mDepthEnd = 350;
  mDepthEnd = 200;
  mWidth = M_PI/2;
//  mSpacing = Vector3D(0.928,0.928,1);
  mSpacing = Vector3D(1,1,1); // using this spacing gives correct image - investigate!
  mSpacing = Vector3D(0.5,0.5,0);
  mSize = QSize(512,512);

  //mOrigin_u = multiply_elems(Vector3D(mSize.width()/2, mSize.height()*0.75, 0), mSpacing);
  mOrigin_u = multiply_elems(Vector3D(mSize.width()/2, mSize.height()*1.0, 0), mSpacing);

  //mCachedCenter_v = this->get_uMv().inv().coord(mOrigin_u) - mDepthStart * Vector3D(0,1,0);
}

void ProbeData::setSector(ProbeSector data)
{
  mType = (TYPE) data.mType;
  mDepthStart = data.mDepthStart;
  mDepthEnd = data.mDepthEnd;
  mWidth = data.mWidth;
}


//ProbeData::ProbeData(TYPE type, double depthStart, double depthEnd, double width) :
//	mType(type),
//	mDepthStart(depthStart),
//	mDepthEnd(depthEnd),
//	mWidth(width),
//  mOrigin_u(0,0,0),
//  mSpacing(1,1,1),
//  mSize(256,256)
//{
//}

/**return true if p_v, given in the upper-left space v,
 * is inside the us beam sector
 *
 */
bool ProbeData::isInside(Vector3D p_v)
{
  Vector3D d = p_v - mCachedCenter_v;
//  ssc::Vector3D d = p_u - mOrigin_u;
//  std::cout << "d " << d << std::endl;

  if (mType==tSECTOR)
  {
    double angle = atan2(d[1], d[0]);
    angle -= M_PI_2; // center angle on us probe axis at 90*.
    if (angle < -M_PI)
      angle += 2.0 * M_PI;
//    std::cout << "angle " << angle << std::endl;
//    std::cout << "|d| " << d.length() << std::endl;

    if (fabs(angle) > mWidth/2.0)
      return false;
    if (d.length() < mDepthStart)
      return false;
    if (d.length() > mDepthEnd)
      return false;
    return true;
  }
  else // tLINEAR
  {
    if (fabs(d[0]) > mWidth/2.0)
      return false;
    if (d[1] < mDepthStart)
      return false;
    if (d[1] > mDepthEnd)
      return false;
    return true;
  }
}

/** Return a 2D mask image identifying the US beam inside the image
 *  data stream.
 */
vtkImageDataPtr ProbeData::getMask()
{
//  std::cout << "start" << std::endl;
  mCachedCenter_v = this->get_uMv().inv().coord(mOrigin_u) - mDepthStart * Vector3D(0,1,0);
 // std::cout << "mCachedCenter_v " << mCachedCenter_v << std::endl;

//  Vector3D p_test = multiply_elems(Vector3D(256,10,0), mSpacing);
//  bool inside = this->isInside(p_test);
//  std::cout << "p_test "<< p_test << ", inside=" << inside << std::endl;

  vtkImageDataPtr retval;
  // use data from probe here
  retval = generateVtkImageData(Vector3D(mSize.width(),mSize.height(),1), mSpacing, 0);

  int* dim(retval->GetDimensions());
  unsigned char* dataPtr = static_cast<unsigned char*>(retval->GetScalarPointer());
  for(int x = 0; x < dim[0]; x++)
    for(int y = 0; y < dim[1]; y++)
    {
      bool inside = this->isInside(multiply_elems(Vector3D(x,y,0), mSpacing));
//      bool inside = (Vector3D(x,y,0)-Vector3D(255,255,0)).length() < 250;

      if(inside)
        dataPtr[x + y*dim[0]] = 1;
      else
        dataPtr[x + y*dim[0]] = 0;
    }

//  retval->SetSpacing(1,1,1);

//  std::cout << "mCachedCenter_v " << mCachedCenter_v << std::endl;
//  std::cout << "end" << std::endl;
  return retval;
}

void ProbeData::test()
{
  Transform3D tMu = this->get_tMu();
  Vector3D e_x(1,0,0);
  Vector3D e_y(0,1,0);
  Vector3D e_z(0,0,1);

  // zero = tMu * mOrigin_u
  std::cout << "zero = tMu * mOrigin_u, zero: " << tMu.coord(mOrigin_u) << ", mOrigin_u: " << mOrigin_u << std::endl;

  // e_z = tMu * -e_y
  std::cout << "e_z = tMu * -e_y " << tMu.vector(-e_y) <<std::endl;

  // e_y = tMu * -e_x
  std::cout << "e_y = tMu * -e_x " << tMu.vector(-e_x) <<std::endl;
}

Transform3D ProbeData::get_tMu() const
{
  Transform3D Rx = ssc::createTransformRotateX(-M_PI/2.0);
  Transform3D Rz = ssc::createTransformRotateY(M_PI/2.0);
  ssc::Transform3D R = (Rx*Rz);
  ssc::Transform3D T = ssc::createTransformTranslate(-mOrigin_u);

  ssc::Transform3D tMu = R*T;
  return tMu;
}

Transform3D ProbeData::get_uMv() const
{
  double H = mSize.height() * mSpacing[1];
  return createTransformRotateX(M_PI) * createTransformTranslate(Vector3D(0,-H,0));
//  return createTransformTranslate(Vector3D(0,H,0)) * createTransformRotateX(M_PI);
}

vtkPolyDataPtr ProbeData::getSector()
{
  this->updateSector();
  return mPolyData;
}

void ProbeData::updateSector()
{
  if (!mPolyData)
    mPolyData = vtkPolyDataPtr::New();

  //TODO: Merge with USProbe2D in sscVtkHelperClasses?
  Vector3D p(0,0,0); // tool position in local space
  // first define the shape of the probe in a xy-plane.
  // after that, transform into yz-plane because thats the tool plane (probe point towards positive z)
  // then transform to global space.
  Transform3D tMl = createTransformIJC(Vector3D(0,1,0), Vector3D(0,0,1), Vector3D(0,0,0));

//  Transform3D M = m_wMt*tMl;
  Transform3D M = tMl;
  Vector3D e_x = unitVector(0);
  Vector3D e_y = unitVector(M_PI_2);
  Vector3D e_z(0,0,1);

  vtkPointsPtr points = vtkPointsPtr::New();
  vtkCellArrayPtr sides = vtkCellArrayPtr::New();

//  if (!mThreeDimensions && !similar(dot(M.coord(e_z), e_z), 1, 0.1))
//  {
//    mPolyData->SetPoints(points);
//    mPolyData->SetLines(sides);
//    mPolyData->Update();
//    return;
//  }

  //Fix to make sure the sector is shown in front of the 2D volume slice in the 2D views
  //double defZero = 0.001;

  if (mType == tLINEAR)
  {
    Vector3D cr = mDepthStart * e_y + mWidth/2 * e_x;
    Vector3D cl = mDepthStart * e_y - mWidth/2 * e_x;

    Vector3D pr = mDepthEnd * e_y + mWidth/2 * e_x;
    Vector3D pl = mDepthEnd * e_y - mWidth/2 * e_x;

    cl = M.coord(cl);
    cr = M.coord(cr);
    pl = M.coord(pl);
    pr = M.coord(pr);
//    //Fix to make sure the sector is shown in front of the 2D volume slice in the 2D views
//    if(!mThreeDimensions)
//    {
//      cl[2] += defZero;
//      cr[2] += defZero;
//      pl[2] += defZero;
//      pr[2] += defZero;
//    }

    points->Allocate(4);
    points->InsertNextPoint(cl.begin());
    points->InsertNextPoint(cr.begin());
    points->InsertNextPoint(pr.begin());
    points->InsertNextPoint(pl.begin());

    vtkIdType cells[5] = { 0,1,2,3,0};
    sides->InsertNextCell(5, cells);
  }
  else if (mType == tSECTOR)
  {

    Vector3D c = - mDepthStart * e_y;  // arc center point

    int arcRes = 20;//Number of points in arc
    double angleIncrement = mWidth/arcRes;
    double startAngle = M_PI_2 - mWidth/2.0;
    double stopAngle = M_PI_2 + mWidth/2.0;

    points->Allocate(arcRes*2);//TODO: Don't use the same number of points in top as in bottom?
    for(int i = 0; i <= arcRes; i++)
    {
      double theta = startAngle + i*angleIncrement;
      Vector3D startTheta = c + mDepthStart * unitVector(theta);
      startTheta = M.coord(startTheta);
//      if(!mThreeDimensions)
//        startTheta[2] += defZero;
      points->InsertNextPoint(startTheta.begin());
    }
    for(int i = 0; i <= arcRes; i++)
    {
      double theta = stopAngle - i*angleIncrement;
      Vector3D endTheta = c + mDepthEnd * unitVector(theta);
      endTheta = M.coord(endTheta);
//      if(!mThreeDimensions)
//        endTheta[2] += defZero;
      points->InsertNextPoint(endTheta.begin());
    }

    sides->InsertNextCell(arcRes*2+2+1);
    for(int i = 0; i < arcRes*2+2; i++)
      sides->InsertCellPoint(i);
    sides->InsertCellPoint(0);
  }

  mPolyData->SetPoints(points);
  mPolyData->SetLines(sides);
  //mPolyData->SetPolys(sides);
  mPolyData->Update();
}


}
