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

#ifndef CXVIEWWRAPPER_H_
#define CXVIEWWRAPPER_H_

#include "org_custusx_core_visualization_Export.h"

#include <vector>
#include <QVariant>
#include <QObject>
#include "cxDefinitions.h"
#include "vtkForwardDeclarations.h"
#include "cxForwardDeclarations.h"
#include "cxViewGroupData.h"
#include "cxSyncedValue.h"

typedef vtkSmartPointer<class vtkPolyDataAlgorithm> vtkPolyDataAlgorithmPtr;
class QMenu;
class QActionGroup;

namespace cx
{

typedef boost::shared_ptr<class CameraData> CameraDataPtr;
typedef boost::shared_ptr<class CoreServices> CoreServicesPtr;

/**
 * \file
 * \addtogroup cx_service_visualization
 * @{
 */


typedef boost::shared_ptr<class DataViewPropertiesInteractor> DataViewPropertiesInteractorPtr;
/** Provide an action list for showing data in views.
  *
  */
class org_custusx_core_visualization_EXPORT DataViewPropertiesInteractor : public QObject
{
	Q_OBJECT
public:
	DataViewPropertiesInteractor(CoreServicesPtr backend, ViewGroupDataPtr groupData);
	void addDataActions(QWidget* parent);
	void setDataViewProperties(DataViewProperties properties);

private slots:
	void dataActionSlot();
private:
	void addDataAction(QString uid, QWidget* parent);
	CoreServicesPtr mBackend;
	ViewGroupDataPtr mGroupData;
	DataViewProperties mProperties;

	QString mLastDataActionUid;
};

/**
 * \brief Superclass for ViewWrappers.
 *
 *
 * \date 6. apr. 2010
 * \\author jbake
 */
class org_custusx_core_visualization_EXPORT ViewWrapper: public QObject
{
Q_OBJECT
public:
	virtual ~ViewWrapper() {}
	virtual void initializePlane(PLANE_TYPE plane) {}
	virtual ViewPtr getView() = 0;
	virtual void setSlicePlanesProxy(SlicePlanesProxyPtr proxy) = 0;
	virtual void setViewGroup(ViewGroupDataPtr group);

//	virtual void setZoom2D(SyncedValuePtr value) {}
	virtual void setOrientationMode(SyncedValuePtr value) {}

	virtual void updateView() = 0;

signals:
	void orientationChanged(ORIENTATION_TYPE type);

protected slots:
	void contextMenuSlot(const QPoint& point);

	virtual void dataViewPropertiesChangedSlot(QString uid);
	virtual void videoSourceChangedSlot(QString uid) {}

protected:
	ViewWrapper(CoreServicesPtr backend);

	void connectContextMenu(ViewPtr view);
	virtual void appendToContextMenu(QMenu& contextMenu) = 0;
	QStringList getAllDataNames(DataViewProperties properties) const;

	ViewGroupDataPtr mGroupData;
	CoreServicesPtr mBackend;
	DataViewPropertiesInteractorPtr mDataViewPropertiesInteractor;
	DataViewPropertiesInteractorPtr mShow3DSlicesInteractor;


};

/**
 * @}
 */
} //namespace cx

#endif /* CXVIEWWRAPPER_H_ */