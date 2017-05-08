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

#include "cxCenterlineRegistrationWidget.h"
#include "cxCenterlineRegistration.h"
#include <vtkPolyData.h>
#include "cxTransform3D.h"
#include "cxDataSelectWidget.h"
#include "cxTrackingService.h"
#include "cxMesh.h"
#include "cxSelectDataStringProperty.h"
#include "cxRecordSessionWidget.h"
#include "cxRecordSession.h"
#include "cxView.h"
#include "cxToolRep3D.h"
#include "cxToolTracer.h"
#include "cxLogger.h"
#include "cxTypeConversions.h"
#include "cxPatientModelService.h"
#include "cxRegistrationService.h"
#include "cxViewService.h"
#include "cxStringProperty.h"
#include "cxLabeledComboBoxWidget.h"
#include "cxTrackingService.h"
#include "cxDoubleProperty.h"
#include "cxProfile.h"
#include "cxHelperWidgets.h"
#include "cxBoolProperty.h"
#include "cxCheckBoxWidget.h"
#include "cxRepContainer.h"
#include "cxWidgetObscuredListener.h"
#include "cxViewGroupData.h"
#include "cxStringPropertySelectTool.h"
#include "cxHelperWidgets.h"
#include "cxAcquisitionService.h"
#include "cxRegServices.h"
#include "cxRecordTrackingWidget.h"


namespace cx
{
CenterlineRegistrationWidget::CenterlineRegistrationWidget(RegServicesPtr services, QWidget* parent) :
    RegistrationBaseWidget(services, parent, "org_custusx_registration_method_centerline_widget",
                           "Centerline Registration"),
	mServices(services),
	mRecordTrackingWidget(NULL)
{
	mVerticalLayout = new QVBoxLayout(this);
}

void CenterlineRegistrationWidget::prePaintEvent()
{
	if (!mRecordTrackingWidget)
	{
		this->setup();
	}
}

void CenterlineRegistrationWidget::setup()
{
    mOptions = profile()->getXmlSettings().descend("centerlineregistrationwidget");

	mSelectMeshWidget = StringPropertySelectMesh::New(mServices->patient());
	mSelectMeshWidget->setValueName("Centerline: ");

    mCenterlineRegistration = CenterlineRegistrationPtr(new CenterlineRegistration());

    connect(mServices->patient().get(),&PatientModelService::patientChanged,this,&CenterlineRegistrationWidget::clearDataOnNewPatient);


	mRegisterButton = new QPushButton("Register");
	connect(mRegisterButton, SIGNAL(clicked()), this, SLOT(registerSlot()));
	mRegisterButton->setToolTip(this->defaultWhatsThis());

	mRecordTrackingWidget = new RecordTrackingWidget(mOptions.descend("recordTracker"),
													 mServices->acquisition(), mServices,
													 "bronc_path",
													 this);
    mRecordTrackingWidget->getSessionSelector()->setHelp("Select tool path for registration");
    mRecordTrackingWidget->getSessionSelector()->setDisplayName("Tool path");

	mVerticalLayout->setMargin(0);
	mVerticalLayout->addWidget(new DataSelectWidget(mServices->view(), mServices->patient(), this, mSelectMeshWidget));

    this->selectXtranslation(mOptions.getElement());
    this->selectYtranslation(mOptions.getElement());
    this->selectZtranslation(mOptions.getElement());
    this->selectXrotation(mOptions.getElement());
    this->selectYrotation(mOptions.getElement());
    this->selectZrotation(mOptions.getElement());

	mVerticalLayout->addWidget(mRecordTrackingWidget);
    mVerticalLayout->addWidget(new CheckBoxWidget(this, mUseXtranslation));
    mVerticalLayout->addWidget(new CheckBoxWidget(this, mUseYtranslation));
    mVerticalLayout->addWidget(new CheckBoxWidget(this, mUseZtranslation));
    mVerticalLayout->addWidget(new CheckBoxWidget(this, mUseXrotation));
    mVerticalLayout->addWidget(new CheckBoxWidget(this, mUseYrotation));
    mVerticalLayout->addWidget(new CheckBoxWidget(this, mUseZrotation));
	mVerticalLayout->addWidget(mRegisterButton);

	mVerticalLayout->addStretch();
}

QString CenterlineRegistrationWidget::defaultWhatsThis() const
{
	return QString();
}

void CenterlineRegistrationWidget::initializeTrackingService()
{
	if(mServices->tracking()->getState() < Tool::tsCONFIGURED)
		mServices->tracking()->setState(Tool::tsCONFIGURED);
}


void CenterlineRegistrationWidget::registerSlot()
{

    if(!mSelectMeshWidget->getMesh())
    {
        reportError("No centerline");
        return;
    }
    vtkPolyDataPtr centerline = mSelectMeshWidget->getMesh()->getVtkPolyData();//input
    Transform3D rMd = mSelectMeshWidget->getMesh()->get_rMd();

    mCenterlineRegistration->UpdateScales(
                mUseXrotation->getValue(), mUseYrotation->getValue(), mUseZrotation->getValue(),
                mUseXtranslation->getValue(), mUseYtranslation->getValue(), mUseZtranslation->getValue());

	Transform3D old_rMpr = mServices->patient()->get_rMpr();//input to registrationAlgorithm

	TimedTransformMap trackerRecordedData_prMt = mRecordTrackingWidget->getRecordedTrackerData_prMt();

	if(trackerRecordedData_prMt.empty())
	{
        reportError("No positions");
        return;
    }

	Transform3D new_rMpr;

    new_rMpr = mCenterlineRegistration->runCenterlineRegistration(centerline, rMd, trackerRecordedData_prMt, old_rMpr);

    std::cout << "Running centerline registration." << std::endl;

    new_rMpr = new_rMpr*old_rMpr;//output
    mServices->registration()->addPatientRegistration(new_rMpr, "Centerline centerline to tracking data");

    Eigen::Matrix4d display_rMpr = Eigen::Matrix4d::Identity();
            display_rMpr = new_rMpr*display_rMpr;
    std::cout << "New prMt: " << std::endl;
        for (int i = 0; i < 4; i++)
            std::cout << display_rMpr.row(i) << std::endl;


}

void CenterlineRegistrationWidget::selectXtranslation(QDomElement root)
{
    mUseXtranslation = BoolProperty::initialize("X translation:", "",
                                                                            "Select if X translation be used in registration", true,
                                                                                root);
}

void CenterlineRegistrationWidget::selectYtranslation(QDomElement root)
{
    mUseYtranslation = BoolProperty::initialize("Y translation:", "",
                                                                            "Select if Y translation be used in registration", true,
                                                                                root);
}

void CenterlineRegistrationWidget::selectZtranslation(QDomElement root)
{
    mUseZtranslation = BoolProperty::initialize("Z translation:", "",
                                                                            "Select if Z translation be used in registration", true,
                                                                                root);
}

void CenterlineRegistrationWidget::selectXrotation(QDomElement root)
{
    mUseXrotation = BoolProperty::initialize("X rotation:    ", "",
                                                                            "Select if X rotation be used in registration", true,
                                                                                root);
}

void CenterlineRegistrationWidget::selectYrotation(QDomElement root)
{
    mUseYrotation = BoolProperty::initialize("Y rotation:    ", "",
                                                                            "Select if Y rotation be used in registration", true,
                                                                                root);
}

void CenterlineRegistrationWidget::selectZrotation(QDomElement root)
{
    mUseZrotation = BoolProperty::initialize("Z rotation:    ", "",
                                                                            "Select if Z rotation be used in registration", true,
                                                                                root);
}



void CenterlineRegistrationWidget::clearDataOnNewPatient()
{
	mMesh.reset();
}
} //namespace cx
