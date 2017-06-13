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


#include "cxDataMetric.h"
#include <QFileInfo>
#include "cxRegistrationTransform.h"
#include "cxTypeConversions.h"
#include "cxNullDeleter.h"
#include "cxFileManagerService.h"
#include "cxLogger.h"

namespace cx
{

DataMetric::DataMetric(const QString& uid, const QString& name, PatientModelServicePtr dataManager, SpaceProviderPtr spaceProvider) :
	Data(uid, name)
{
	mDataManager = dataManager;
	mSpaceProvider = spaceProvider;

	mColor = QColor(240, 170, 255, 255);
	m_rMd_History = RegistrationHistory::getNullObject();
}

DataMetric::~DataMetric()
{
}

QString DataMetric::getSpace()
{
    return "";
}

void DataMetric::setColor(const QColor& color)
{
	mColor = color;
	emit propertiesChanged();
}

QColor DataMetric::getColor()
{
	return mColor;
}


void DataMetric::addXml(QDomNode& dataNode)
{
	Data::addXml(dataNode);
	QDomDocument doc = dataNode.ownerDocument();

	QDomElement colorNode = doc.createElement("color");
	colorNode.appendChild(doc.createTextNode(color2string(mColor)));
	dataNode.appendChild(colorNode);
}

void DataMetric::parseXml(QDomNode& dataNode)
{
	Data::parseXml(dataNode);

	if (dataNode.isNull())
		return;

	QDomNode colorNode = dataNode.namedItem("color");
	if (!colorNode.isNull())
	{
		mColor = string2color(colorNode.toElement().text(), mColor);
	}

	emit propertiesChanged();
}

bool DataMetric::load(QString path, FileManagerServicePtr filemanager)
{
	//TODO is this good enough????

	QFileInfo to_be_loaded(path);
	CX_LOG_DEBUG() << "TRYING TO LOAD DATAMETRIC WITH PATH: " << path ;
	CX_LOG_DEBUG() << "Sym: "<< to_be_loaded.isSymLink();
	CX_LOG_DEBUG() << "Dir: "<< to_be_loaded.isDir();
	CX_LOG_DEBUG() << "File: "<< to_be_loaded.isFile();

	if(to_be_loaded.isDir() || !to_be_loaded.exists())
	{
		CX_LOG_DEBUG() << "does not exist";
		//when trying to load metrics from xml file, this function will be called without a valid path
		return true; //simulating old behaviour
	}
	else
	{
		CX_LOG_DEBUG() << "exist";
		//when trying to use the import functionality to read MNI Tag Point files for example the path will be valid
		DataMetricPtr self = DataMetricPtr(this, null_deleter());
		return filemanager->readInto(self, path);
	}
	return false;
}


}
