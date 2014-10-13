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

#ifndef CXLANDMARKPATIENTREGISTRATIONWIDGET_H_
#define CXLANDMARKPATIENTREGISTRATIONWIDGET_H_

#include "cxPluginRegistrationExport.h"

#include "cxLandmarkRegistrationWidget.h"
#include "cxRegistrationDataAdapters.h"

#include "cxImage.h"
#include "cxTransform3D.h"
#include "cxDominantToolProxy.h"

class QVBoxLayout;
class QComboBox;
class QTableWidget;
class QPushButton;
class QString;
class QLabel;
class QSlider;
class QGridLayout;
class QSpinBox;

namespace cx
{
typedef Transform3D Transform3D;

/**
 * \file
 * \addtogroup cx_module_registration
 * @{
 */

/**
 * \class LandmarkPatientRegistrationWidget
 *
 * \brief Widget used as a tab in the ContexDockWidget for patient registration.
 *
 * \date Feb 3, 2009
 * \\author Janne Beate Bakeng, SINTEF
 */
class cxPluginRegistration_EXPORT LandmarkPatientRegistrationWidget: public LandmarkRegistrationWidget
{
Q_OBJECT

public:
	LandmarkPatientRegistrationWidget(regServices services, QWidget* parent, QString objectName,
		QString windowTitle); ///< sets up layout and connects signals and slots
	virtual ~LandmarkPatientRegistrationWidget(); ///< empty
	virtual QString defaultWhatsThis() const;

protected slots:

	void registerSlot();
	virtual void fixedDataChanged(); ///< listens to the datamanager for when the active image is changed
	void toolSampleButtonClickedSlot(); ///< reacts when the Sample Tool button is clicked
	virtual void cellClickedSlot(int row, int column); ///< when a landmark i selected from the table
	void removeLandmarkButtonClickedSlot();
	void updateToolSampleButton();

protected:
	virtual void showEvent(QShowEvent* event); ///<updates internal info before showing the widget
	virtual void hideEvent(QHideEvent* event);
    virtual void prePaintEvent(); ///< populates the table widget
    virtual LandmarkMap getTargetLandmarks() const;
	virtual Transform3D getTargetTransform() const;
	virtual void setTargetLandmark(QString uid, Vector3D p_target);
	virtual QString getTargetName() const;
	virtual void performRegistration();

	//gui
	QPushButton* mToolSampleButton; ///< the Sample Tool button
	QPushButton* mRemoveLandmarkButton;
	QPushButton* mRegisterButton;

	//data
	ImageLandmarksSourcePtr mImageLandmarkSource;
	RegistrationFixedImageStringDataAdapterPtr mFixedDataAdapter;
	DominantToolProxyPtr mDominantToolProxy;

private:
	LandmarkPatientRegistrationWidget(); ///< not implemented
};

/**
 * @}
 */
}//namespace cx

#endif /* CXLANDMARKPATIENTREGISTRATIONWIDGET_H_ */