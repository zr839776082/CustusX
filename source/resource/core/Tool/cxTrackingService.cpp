/*=========================================================================
This file is part of CustusX, an Image Guided Therapy Application.
                 
Copyright (c) SINTEF Department of Medical Technology.
All rights reserved.
                 
CustusX is released under a BSD 3-Clause license.
                 
See Lisence.txt (https://github.com/SINTEFMedtek/CustusX/blob/master/License.txt) for details.
=========================================================================*/

#include "cxTrackingService.h"
#include "cxTrackingServiceNull.h"
#include "cxNullDeleter.h"

namespace cx
{
TrackingServicePtr TrackingService::getNullObject()
{
	static TrackingServicePtr mNull;
	if (!mNull)
		mNull.reset(new TrackingServiceNull, null_deleter());
	return mNull;
}
} //cx
