/*=========================================================================
This file is part of CustusX, an Image Guided Therapy Application.

Copyright (c) 2008-2014, SINTEF Department of Medical Technology
All rights reserved.

Redistribution and use in source and binary forms, with or without 
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, 
   this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, 
   this list of conditions and the following disclaimer in the documentation 
   and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors 
   may be used to endorse or promote products derived from this software 
   without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE 
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL 
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, 
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=========================================================================*/

#include "cxViewFollower.h"

#include <QTimer>
#include "cxSliceProxy.h"
#include "cxSettings.h"
#include "cxSliceComputer.h"
#include "cxTool.h"
#include "cxUtilHelpers.h"
#include "cxDefinitionStrings.h"

#include "cxPatientModelService.h"
#include "cxLogger.h"
#include "cxRegionOfInterestMetric.h"
#include "cxSliceAutoViewportCalculator.h"

namespace cx
{

ViewFollowerPtr ViewFollower::create(PatientModelServicePtr dataManager)
{
	return ViewFollowerPtr(new ViewFollower(dataManager));
}

ViewFollower::ViewFollower(PatientModelServicePtr dataManager) :
	mDataManager(dataManager)
{
//	mROI_s = DoubleBoundingBox3D::zero();
	mCalculator.reset(new SliceAutoViewportCalculator);
}

ViewFollower::~ViewFollower()
{

}

void ViewFollower::setSliceProxy(SliceProxyPtr sliceProxy)
{
//	if (mSliceProxy)
//	{
//		disconnect(mSliceProxy.get(), SIGNAL(toolTransformAndTimestamp(Transform3D, double)), this, SLOT(ensureCenterWithinView()));
//	}

	mSliceProxy = sliceProxy;

//	if (mSliceProxy)
//	{
//		connect(mSliceProxy.get(), SIGNAL(toolTransformAndTimestamp(Transform3D, double)), this, SLOT(updateView()));
//	}
}

void ViewFollower::setView(DoubleBoundingBox3D bb_s)
{
	mBB_s = bb_s;
//	this->updateView();
}

void ViewFollower::setAutoZoomROI(QString uid)
{
	mRoi = uid;
//	if (mRoi)
//		disconnect(mRoi.get(), &Data::transformChanged, this, &ViewFollower::updateView);

//	DataPtr data = mDataManager->getData(uid);
//	mRoi = boost::dynamic_pointer_cast<RegionOfInterestMetric>(data);

//	if (mRoi)
//		connect(mRoi.get(), &Data::transformChanged, this, &ViewFollower::updateView);

//	this->updateView();
}

SliceAutoViewportCalculator::ReturnType ViewFollower::calculate()
{
	if (!mSliceProxy)
		return SliceAutoViewportCalculator::ReturnType();
	if (!mSliceProxy->getTool())
		return SliceAutoViewportCalculator::ReturnType();

	mCalculator->mFollowTooltip = settings()->value("Navigation/followTooltip").value<bool>();
	mCalculator->mFollowTooltipBoundary = settings()->value("Navigation/followTooltipBoundary").toDouble();
	mCalculator->mBB_s = mBB_s;
	mCalculator->mTooltip_s = this->findVirtualTooltip_s();
	mCalculator->mFollowType = mSliceProxy->getComputer().getFollowType();
	mCalculator->mROI_s = this->getROI_BB_s();

	SliceAutoViewportCalculator::ReturnType result = mCalculator->calculate();
	return result;

//	if (!similar(result.zoom, 1.0))
//	{
//		CX_LOG_CHANNEL_DEBUG("CA") << this << " autozoom zoom " << result.zoom;
//		emit newZoom(1.0/result.zoom);
//	}
//	if (!similar(result.center_shift_s, Vector3D::Zero()))
//	{
//		Vector3D newcenter_r = this->findCenter_r_fromShift_s(result.center_shift_s);
//		CX_LOG_CHANNEL_DEBUG("CA") << this << "autozoom shift " << result.center_shift_s;
//		mDataManager->setCenter(newcenter_r);
//	}
}

DoubleBoundingBox3D ViewFollower::getROI_BB_s()
{
	DataPtr data = mDataManager->getData(mRoi);
	RegionOfInterestMetricPtr roi = boost::dynamic_pointer_cast<RegionOfInterestMetric>(data);

	if (!roi)
	{
		return DoubleBoundingBox3D::zero();
	}

//	CX_LOG_CHANNEL_DEBUG("CA") << "generate bb_roi_s";
	Transform3D sMr = mSliceProxy->get_sMr();
	DoubleBoundingBox3D bb_s = roi->getROI().getBox(sMr);
	return bb_s;
}

Vector3D ViewFollower::findVirtualTooltip_s()
{
	ToolPtr tool = mSliceProxy->getTool();
	Transform3D sMr = mSliceProxy->get_sMr();
	Transform3D rMpr = mDataManager->get_rMpr();
	Transform3D prMt = tool->get_prMt();
	Vector3D pt_s = sMr * rMpr * prMt.coord(Vector3D(0,0,tool->getTooltipOffset()));
	pt_s[2] = 0; // project into plane
	return pt_s;
}

Vector3D ViewFollower::findCenter_r_fromShift_s(Vector3D shift_s)
{
	Transform3D sMr = mSliceProxy->get_sMr();
	Vector3D c_s = sMr.coord(mDataManager->getCenter());

	Vector3D newcenter_s = c_s + shift_s;

	Vector3D newcenter_r = sMr.inv().coord(newcenter_s);
	return newcenter_r;
}

} // namespace cx

