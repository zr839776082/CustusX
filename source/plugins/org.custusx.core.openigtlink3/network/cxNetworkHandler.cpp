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

#include "cxNetworkHandler.h"

#include <QTimer>

#include "igtlioLogic.h"
#include "igtlioImageDevice.h"
#include "igtlioTransformDevice.h"
#include "igtlioCommandDevice.h"
#include "igtlioStatusDevice.h"

#include "igtlioConnector.h"

#include "igtlioImageConverter.h"
#include "igtlioTransformConverter.h"
#include "igtlioCommandConverter.h"
#include "igtlioStatusConverter.h"

#include "cxLogger.h"

namespace cx
{

NetworkHandler::NetworkHandler(igtlio::LogicPointer logic) :
	mTimer(new QTimer(this))
{
	qRegisterMetaType<Transform3D>("Transform3D");
	qRegisterMetaType<ImagePtr>("ImagePtr");

	mLogic = logic;

	this->connectToConnectionEvents();
	this->connectToDeviceEvents();

	connect(mTimer, SIGNAL(timeout()), this, SLOT(periodicProcess()));
	mTimer->start(5);
}

NetworkHandler::~NetworkHandler()
{
	mTimer->stop();
}

igtlio::SessionPointer NetworkHandler::requestConnectToServer(std::string serverHost, int serverPort, igtlio::SYNCHRONIZATION_TYPE sync, double timeout_s)
{
	mSession = mLogic->ConnectToServer(serverHost, serverPort, sync, timeout_s);
	return mSession;
}

void NetworkHandler::hackEmitProbeDefintionForPlusTestSetup(QString deviceName)
{
	ProbeDefinitionPtr probeDefinition(new ProbeDefinition(ProbeDefinition::tLINEAR));
	probeDefinition->setUid("Image_Reference");
	Vector3D origin_p(100, 0, 0);
	probeDefinition->setOrigin_p(origin_p);
	Vector3D spacing(1, 1, 1);
	probeDefinition->setSpacing(spacing);
	DoubleBoundingBox3D clipRect_p(0, 199, 0, 149);
	probeDefinition->setClipRect_p(clipRect_p);
	probeDefinition->setSector(0, 149, 200);
	QSize size(200, 150);
	probeDefinition->setSize(size);
	probeDefinition->setUseDigitalVideo(true);

	emit probedefinition(deviceName, probeDefinition);
}

void NetworkHandler::onDeviceModified(vtkObject* caller_device, void* unknown, unsigned long event , void*)
{
	vtkSmartPointer<igtlio::Device> receivedDevice(reinterpret_cast<igtlio::Device*>(caller_device));

	igtlio::BaseConverter::HeaderData header = receivedDevice->GetHeader();
	std::string device_type = receivedDevice->GetDeviceType();

	CX_LOG_DEBUG() << "Incoming " << device_type << " on device: " << receivedDevice->GetDeviceName();

	if(device_type == igtlio::ImageConverter::GetIGTLTypeName())
	{
		igtlio::ImageDevicePointer imageDevice = igtlio::ImageDevice::SafeDownCast(receivedDevice);

		igtlio::ImageConverter::ContentData content = imageDevice->GetContent();

		QString deviceName(header.deviceName.c_str());
		ImagePtr cximage = ImagePtr(new Image(deviceName, content.image));
		// get timestamp from igtl second-format:;
		double timestampMS = header.timestamp * 1000;
		cximage->setAcquisitionTime( QDateTime::fromMSecsSinceEpoch(timestampMS));
		//this->decode_rMd(msg, retval);

		emit image(cximage);
	}
	else if(device_type == igtlio::TransformConverter::GetIGTLTypeName())
	{
		igtlio::TransformDevicePointer transformDevice = igtlio::TransformDevice::SafeDownCast(receivedDevice);
		igtlio::TransformConverter::ContentData content = transformDevice->GetContent();

		QString deviceName(content.deviceName.c_str());
		Transform3D cxtransform = Transform3D::fromVtkMatrix(content.transform);
		double timestamp = header.timestamp;
		emit transform(deviceName, cxtransform, timestamp);

		//HACK - START probe definition is not received
		this->hackEmitProbeDefintionForPlusTestSetup(deviceName);
		//HACK - END
	}
	else if(device_type == igtlio::CommandConverter::GetIGTLTypeName())
	{
		CX_LOG_DEBUG() << "Received command message.";
		igtlio::CommandDevicePointer command = igtlio::CommandDevice::SafeDownCast(receivedDevice);

		igtlio::CommandConverter::ContentData content = command->GetContent();
		CX_LOG_DEBUG() << "COMMAND: "	<< " id: " << content.id
										<< " name: " << content.name
										<< " content: " << content.content;
		//TODO
		QString deviceName(content.name.c_str());
		QString xml(content.content.c_str());
		emit commandRespons(deviceName, xml);

	}
	else if(device_type == igtlio::StatusConverter::GetIGTLTypeName())
	{
		igtlio::StatusDevicePointer status = igtlio::StatusDevice::SafeDownCast(receivedDevice);

		igtlio::StatusConverter::ContentData content = status->GetContent();
		//TODO

	}
	else
	{
		CX_LOG_WARNING() << "Found unhandled devicetype: " << device_type;
	}

}

void NetworkHandler::onConnectionEvent(vtkObject* caller, void* connector, unsigned long event , void*)
{
	if (event==igtlio::Logic::ConnectionAddedEvent)
	{
		emit connected();
	}
	if (event==igtlio::Logic::ConnectionAboutToBeRemovedEvent)
	{
		emit disconnected();
	}
}

void NetworkHandler::onDeviceAddedOrRemoved(vtkObject* caller, void* void_device, unsigned long event, void* callData)
{
	if (event==igtlio::Logic::NewDeviceEvent)
	{
		igtlio::DevicePointer device(reinterpret_cast<igtlio::Device*>(void_device));
		if(device)
		{
			CX_LOG_DEBUG() << " NetworkHandler is listening to " << device->GetDeviceName();
			qvtkReconnect(NULL, device, igtlio::Device::ModifiedEvent, this, SLOT(onDeviceModified(vtkObject*, void*, unsigned long, void*)));
		}
	}
	if (event==igtlio::Logic::RemovedDeviceEvent)
	{
		CX_LOG_WARNING() << "TODO: on remove device event, not implemented";
	}
}

void NetworkHandler::periodicProcess()
{
	mLogic->PeriodicProcess();
}

void NetworkHandler::connectToConnectionEvents()
{
	foreach(int eventId, QList<int>()
			<< igtlio::Logic::ConnectionAddedEvent
			<< igtlio::Logic::ConnectionAboutToBeRemovedEvent
			)
	{
		qvtkReconnect(NULL, mLogic, eventId,
					  this, SLOT(onConnectionEvent(vtkObject*, void*, unsigned long, void*)));
	}
}

void NetworkHandler::connectToDeviceEvents()
{
	foreach(int eventId, QList<int>()
			<< igtlio::Logic::NewDeviceEvent
			<< igtlio::Logic::RemovedDeviceEvent
			)
	{
		qvtkReconnect(NULL, mLogic, eventId,
					this, SLOT(onDeviceAddedOrRemoved(vtkObject*, void*, unsigned long, void*)));
	}
}

} // namespace cx
