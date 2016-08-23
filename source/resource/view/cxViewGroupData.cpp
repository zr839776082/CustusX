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

#include "cxViewGroupData.h"

#include <QMenu>
#include "vtkCamera.h"

#include "cxPatientModelService.h"
#include "cxMesh.h"
#include "cxTypeConversions.h"
#include "cxCameraControl.h"
#include "cxImageAlgorithms.h"
#include "cxDataMetric.h"
#include "cxView.h"
#include "cxImage.h"
#include "cxTrackedStream.h"
#include "cxInteractiveClipper.h"
#include "boost/bind.hpp"
#include "cxXMLNodeWrapper.h"
#include "cxSyncedValue.h"
#include "cxCoreServices.h"
#include "cxLogger.h"
#include "cxDefinitionStrings.h"
#include "cxStringListProperty.h"

namespace cx
{

CameraStyleData::CameraStyleData()
{
	this->clear();
}

CameraStyleData::CameraStyleData(CAMERA_STYLE_TYPE style)
{
	this->setCameraStyle(style);
}

void CameraStyleData::setCameraStyle(CAMERA_STYLE_TYPE style)
{
	this->clear();

	if (style==cstDEFAULT_STYLE)
	{
	}
	if (style==cstTOOL_STYLE)
	{
		mCameraFollowTool = true;
		mFocusFollowTool = true;
	}
	if (style==cstANGLED_TOOL_STYLE)
	{
		mCameraFollowTool = true;
		mFocusFollowTool = true;
		mElevation = 20.0/180.0*M_PI;
	}
	if (style==cstUNICAM_STYLE)
	{
		mUniCam = true;
	}
}

CAMERA_STYLE_TYPE CameraStyleData::getStyle()
{
	for (unsigned int i=0; i<cstCOUNT; ++i)
	{
		CAMERA_STYLE_TYPE current = static_cast<CAMERA_STYLE_TYPE>(i);
		if (CameraStyleData(current)==*this)
			return current;
	}
	return cstCOUNT;
}


void CameraStyleData::clear()
{
	mCameraFollowTool = false;
	mFocusFollowTool = false;
	mCameraOnTooltip = false;
	mTableLock = false;
	mElevation = 0;
	mUniCam = false;
	mAutoZoomROI = "";
	mFocusROI = "";
}

void CameraStyleData::addXml(QDomNode &dataNode)
{
	QDomElement elem = dataNode.toElement();
	elem.setAttribute("cameraFollowTool", mCameraFollowTool);
	elem.setAttribute("focusFollowTool", mFocusFollowTool);
	elem.setAttribute("cameraOnTooltip", mCameraOnTooltip);
	elem.setAttribute("tableLock", mTableLock);
	elem.setAttribute("elevation", mElevation);
	elem.setAttribute("uniCam", mUniCam);
	elem.setAttribute("autoZoomROI", mAutoZoomROI);
	elem.setAttribute("focusROI", mFocusROI);
}

void CameraStyleData::parseXml(QDomNode dataNode)
{
	QDomElement elem = dataNode.toElement();
	mCameraFollowTool = elem.attribute("cameraFollowTool", QString::number(mCameraFollowTool)).toInt();
	mFocusFollowTool = elem.attribute("focusFollowTool", QString::number(mFocusFollowTool)).toInt();
	mCameraOnTooltip = elem.attribute("cameraOnTooltip", QString::number(mCameraOnTooltip)).toInt();
	mTableLock = elem.attribute("tableLock", QString::number(mTableLock)).toInt();
	mElevation = elem.attribute("elevation", QString::number(mElevation)).toDouble();
	mUniCam = elem.attribute("uniCam", QString::number(mUniCam)).toInt();
	mAutoZoomROI = elem.attribute("autoZoomROI", mAutoZoomROI);
	mFocusROI = elem.attribute("focusROI", mFocusROI);
}

bool operator==(const CameraStyleData& lhs, const CameraStyleData& rhs)
{
	return ((lhs.mCameraFollowTool==rhs.mCameraFollowTool) &&
			(lhs.mFocusFollowTool==rhs.mFocusFollowTool) &&
			(lhs.mCameraOnTooltip==rhs.mCameraOnTooltip) &&
			(lhs.mTableLock==rhs.mTableLock) &&
			similar(lhs.mElevation, rhs.mElevation) &&
			(lhs.mUniCam==rhs.mUniCam) &&
			(lhs.mAutoZoomROI==rhs.mAutoZoomROI) &&
			(lhs.mFocusROI==rhs.mFocusROI)
			);
}




DataViewProperties DataViewProperties::createDefault()
{
	DataViewProperties retval;
	retval.mVolume3D = true;
	retval.mSlice3D = false;
	retval.mSlice2D = true;
	return retval;
}
DataViewProperties DataViewProperties::createFull()
{
	DataViewProperties retval;
	retval.mVolume3D = true;
	retval.mSlice3D = true;
	retval.mSlice2D = true;
	return retval;
}

DataViewProperties DataViewProperties::createVolume3D()
{
	DataViewProperties retval;
	retval.mVolume3D = true;
	retval.mSlice3D = false;
	retval.mSlice2D = false;
	return retval;
}

DataViewProperties DataViewProperties::createSlice3D()
{
	DataViewProperties retval;
	retval.mVolume3D = false;
	retval.mSlice3D = true;
	retval.mSlice2D = false;
	return retval;
}

DataViewProperties DataViewProperties::createSlice2D()
{
	DataViewProperties retval;
	retval.mVolume3D = false;
	retval.mSlice3D = false;
	retval.mSlice2D = true;
	return retval;
}

DataViewProperties DataViewProperties::create3D()
{
	DataViewProperties retval;
	retval.mVolume3D = true;
	retval.mSlice3D = true;
	retval.mSlice2D = false;
	return retval;
}


void DataViewProperties::addXml(QDomNode& dataNode)
{
	QDomElement elem = dataNode.toElement();
	elem.setAttribute("volume3D", mVolume3D);
	elem.setAttribute("slice3D", mSlice3D);
	elem.setAttribute("slice2D", mSlice2D);
}

void DataViewProperties::parseXml(QDomNode dataNode)
{
	QDomElement elem = dataNode.toElement();
	mVolume3D = elem.attribute("volume3D", QString::number(mVolume3D)).toInt();
	mSlice3D = elem.attribute("slice3D", QString::number(mSlice3D)).toInt();
	mSlice2D = elem.attribute("slice2D", QString::number(mSlice2D)).toInt();
}

bool DataViewProperties::empty() const
{
	return !(mVolume3D || mSlice3D || mSlice2D);
}

DataViewProperties DataViewProperties::addFlagsIn(DataViewProperties rhs) const
{
	DataViewProperties retval;
	retval.mSlice2D = mSlice2D || rhs.mSlice2D;
	retval.mSlice3D = mSlice3D || rhs.mSlice3D;
	retval.mVolume3D = mVolume3D || rhs.mVolume3D;
	return retval;
}

DataViewProperties DataViewProperties::removeFlagsIn(DataViewProperties rhs) const
{
	DataViewProperties retval;
	retval.mSlice2D = mSlice2D && !rhs.mSlice2D;
	retval.mSlice3D = mSlice3D && !rhs.mSlice3D;
	retval.mVolume3D = mVolume3D && !rhs.mVolume3D;
	return retval;
}

bool DataViewProperties::containsAnyFlagsIn(DataViewProperties required) const
{
	if (required.mSlice2D && mSlice2D) return true;
	if (required.mSlice3D && mSlice3D) return true;
	if (required.mVolume3D && mVolume3D) return true;
	return false;
}



///--------------------------------------------------------
///--------------------------------------------------------
///--------------------------------------------------------

bool dataTypeSort(const DataPtr data1, const DataPtr data2)
{
	return getPriority(data1) < getPriority(data2);
}

int getPriority(DataPtr data)
{
	if (data->getType()=="mesh")
		return 6;
	DataMetricPtr metric = boost::dynamic_pointer_cast<DataMetric>(data);
	if (metric)
		return 7;

	ImagePtr image = boost::dynamic_pointer_cast<Image>(data);
	if (image)
	{
		if (image->getModality().toUpper().contains("US"))
		{
			if (image->getImageType().toUpper().contains("B-MODE"))
				return 4;
			else // angio types
				return 5;
		}
		else if (image->getModality().toUpper().contains("MR"))
		{
			// MR, CT, SC, others
			return 2;
		}
		else if (image->getModality().toUpper().contains("CT"))
		{
			// MR, CT, SC, others
			return 1;
		}
		else
		{
			return 0;
		}
	}

	return 3;
}

///----------------------------------------------------------------------------
///----------------------------------------------------------------------------
///----------------------------------------------------------------------------

ViewGroupData::Options::Options() :
	mShowLandmarks(false), mShowPointPickerProbe(false),
	mPickerGlyph(new Mesh("PickerGlyph"))
{
}

ViewGroupData::ViewGroupData(CoreServicesPtr services, QString uid) :
	mServices(services),
	mCamera3D(CameraData::create()),
	mUid(uid)
{
	if(mServices)
		connect(mServices->patient().get(), &PatientModelService::dataAddedOrRemoved, this, &ViewGroupData::purgeDataNotExistingInPatientModelService);
	mVideoSource = "active";
	mGroup2DZoom = SyncedValue::create(1);
	mGlobal2DZoom = mGroup2DZoom;

	this->createSliceDefinitionProperty();
}

//Remove all data, and wait to emit signals until all data is removed
void ViewGroupData::purgeDataNotExistingInPatientModelService()
{
	QStringList purged;
	for (unsigned i = 0; i < mData.size(); )
	{
		QString uid = mData[i].first;
		if (!mServices->patient()->getData(uid))
		{
			if (this->contains(uid))
			{
				purged << uid;
				mData.erase(std::find_if(mData.begin(), mData.end(), data_equals(uid)));
			}
		}
		else
			++i;
	}
	//Emit delayed signals
	for(unsigned i = 0; i < purged.size(); ++i)
		emit dataViewPropertiesChanged(purged[i]);
}


void ViewGroupData::requestInitialize()
{
	emit initialized();
}

void ViewGroupData::addData(QString uid)
{
	DataViewProperties properties = this->getProperties(uid);
	properties = properties.addFlagsIn(DataViewProperties::createDefault());
	this->setProperties(uid, properties);
}

void ViewGroupData::addDataSorted(QString uid)
{
	if (this->contains(uid))
		return;

	DataViewProperties properties = DataViewProperties::createDefault();
	DataAndViewProperties item(uid, properties);

	for (int i=int(mData.size())-1; i>=0; --i)
	{
		if (!dataTypeSort(this->getData(uid), this->getData(mData[i].first)))
		{
			mData.insert(mData.begin()+i+1, item);
			break;
		}
	}
	if (!this->contains(uid))
		mData.insert(mData.begin(), item);
	emit dataViewPropertiesChanged(uid);
}

DataViewProperties ViewGroupData::getProperties(QString uid)
{
	if (this->contains(uid))
		return std::find_if(mData.begin(), mData.end(), data_equals(uid))->second;
	return DataViewProperties();
}

void ViewGroupData::setProperties(QString uid, DataViewProperties properties)
{
	if (uid.isEmpty())
		return;

	if (properties.empty())
	{
		this->removeData(uid);
		return;
	}

	if (!this->contains(uid))
	{
		DataAndViewProperties item(uid, properties);
		mData.push_back(item);
	}
	else
	{
		std::find_if(mData.begin(), mData.end(), data_equals(uid))->second = properties;
	}

	emit dataViewPropertiesChanged(uid);
}

bool ViewGroupData::contains(QString uid) const
{
	return std::count_if(mData.begin(), mData.end(), data_equals(uid));
}

bool ViewGroupData::removeData(QString uid)
{
	if (!this->contains(uid))
		return false;
	mData.erase(std::find_if(mData.begin(), mData.end(), data_equals(uid)));
	emit dataViewPropertiesChanged(uid);
	return true;
}

void ViewGroupData::clearData()
{
	while (!mData.empty())
		this->removeData(mData.front().first);
	this->setVideoSource("active");

	mGroup2DZoom->set(1.0);
	mGlobal2DZoom->set(1.0);
}

DataPtr ViewGroupData::getData(QString uid) const
{
	CX_LOG_DEBUG() << "ViewGroupData::getData";
	DataPtr data = mServices->patient()->getData(uid);
	if (!data)
	{
		reportError("Couldn't find the data: [" + uid + "] in the datamanager.");
		return DataPtr();
	}
	return data;
}

void ViewGroupData::setVideoSource(QString uid)
{
	if (mVideoSource==uid)
		return;
	mVideoSource = uid;
	emit videoSourceChanged(mVideoSource);
}

QString ViewGroupData::getVideoSource() const
{
	return mVideoSource;
}

std::vector<DataPtr> ViewGroupData::getData(DataViewProperties properties) const
{
	return this->getDataOfType<Data>(properties);
}

template<class DATA_TYPE>
std::vector<boost::shared_ptr<DATA_TYPE> > ViewGroupData::getDataOfType(DataViewProperties requiredProperties) const
{
	typedef boost::shared_ptr<DATA_TYPE> DATA_PTR;
	std::vector<DATA_PTR> retval;
	for (unsigned i = 0; i < mData.size(); ++i)
	{
		DATA_PTR data = boost::dynamic_pointer_cast<DATA_TYPE>(this->getData(mData[i].first));
		if (!data)
			continue;
		DataViewProperties properties = mData[i].second;
		if (!properties.containsAnyFlagsIn(requiredProperties))
			continue;
		retval.push_back(data);
	}
	return retval;
}

std::vector<ImagePtr> ViewGroupData::getImages(DataViewProperties properties) const
{
	return this->getDataOfType<Image>(properties);
}

std::vector<MeshPtr> ViewGroupData::getMeshes(DataViewProperties properties) const
{
	return this->getDataOfType<Mesh>(properties);
}

std::vector<TrackedStreamPtr> ViewGroupData::getTrackedStreams(DataViewProperties properties) const
{
	return this->getDataOfType<TrackedStream>(properties);
}

std::vector<TrackedStreamPtr> ViewGroupData::getTracked2DStreams(DataViewProperties properties) const
{
	std::vector<TrackedStreamPtr> streams = this->getTrackedStreams(properties);
	std::vector<TrackedStreamPtr> retval;

	for(int i = 0; i < streams.size(); ++i)
	{
		if(streams[i]->is2D() )
			retval.push_back(streams[i]);
	}
	return retval;
}

std::vector<ImagePtr> ViewGroupData::getImagesAndChangingImagesFromTrackedStreams(DataViewProperties properties, bool include2D) const
{
	std::vector<ImagePtr> images = this->getImages(properties);
	std::vector<TrackedStreamPtr> streams = this->getTrackedStreams(properties);

	for(int i = 0; i < streams.size(); ++i)
	{
		ImagePtr changingImage = streams[i]->getChangingImage();
		if(streams[i]->is3D())
			images.push_back(changingImage);
		if(include2D && streams[i]->is2D())
			images.push_back(changingImage);
	}
	return images;
}

ViewGroupData::Options ViewGroupData::getOptions() const
{
	return mOptions;
}

void ViewGroupData::setOptions(ViewGroupData::Options options)
{
	mOptions = options;
	emit optionsChanged();
}

void ViewGroupData::initializeGlobal2DZoom(SyncedValuePtr val)
{
	mGlobal2DZoom = val;
}

SyncedValuePtr ViewGroupData::getGroup2DZoom()
{
	return mGroup2DZoom;
}
SyncedValuePtr ViewGroupData::getGlobal2DZoom()
{
	return mGlobal2DZoom;
}

void ViewGroupData::createSliceDefinitionProperty()
{
	QStringList slicedefs;
	for (int i=0; i<ptCOUNT; ++i)
		slicedefs << enum2string(PLANE_TYPE(i));
	QStringList slicedefaults;
	slicedefaults << enum2string(ptAXIAL) << enum2string(ptCORONAL) << enum2string(ptSAGITTAL);
	mSliceDefinitionProperty = StringListProperty::initialize("slice_definition_3D",
														"3D Slices",
														"Select slice planes to view in 3D",
														slicedefaults,
														slicedefs);
	connect(mSliceDefinitionProperty.get(), &Property::changed, this, &ViewGroupData::optionsChanged);
}

PlaneTypeCollection ViewGroupData::getSliceDefinitions()
{
	QStringList val = mSliceDefinitionProperty->getValue();
	return PlaneTypeCollection::fromString(val.join("/"));
}

void ViewGroupData::setSliceDefinitions(PlaneTypeCollection val)
{
	QStringList val_list = val.toString().split("/");
	mSliceDefinitionProperty->setValue(val_list);
}

StringListPropertyPtr ViewGroupData::getSliceDefinitionProperty()
{
	return mSliceDefinitionProperty;
}

void ViewGroupData::addXml(QDomNode& dataNode)
{
	XMLNodeAdder base(dataNode);

	for (unsigned i = 0; i < mData.size(); ++i)
	{
		QDomElement elem;
		elem = base.addTextToElement("data", mData[i].first);
		mData[i].second.addXml(elem);
	}

	base.addObjectToElement("camera3D", this->getCamera3D());
	base.addTextToElement("slicesPlanes3D", this->getSliceDefinitions().toString());

	Options options = this->getOptions();
	base.addObjectToElement("cameraStyle", &options.mCameraStyle);
}

void ViewGroupData::parseXml(QDomNode dataNode)
{
	XMLNodeParser base(dataNode);

	QString sliceText = base.parseTextFromElement("slicesPlanes3D");
	this->setSliceDefinitions(PlaneTypeCollection::fromString(sliceText, this->getSliceDefinitions()));

	std::vector<QDomElement> dataElems = base.getDuplicateElements("data");
	for (unsigned i=0; i<dataElems.size(); ++i)
	{
		QDomElement elem = dataElems[i];
		QString uid = elem.text();
		DataViewProperties properties = DataViewProperties::createDefault();
		properties.parseXml(elem);

		this->addData(uid);
		this->setProperties(uid, properties);
	}

	base.parseObjectFromElement("camera3D", this->getCamera3D());

	Options options = this->getOptions();
	base.parseObjectFromElement("cameraStyle", &options.mCameraStyle);
	this->setOptions(options);
}

void ViewGroupData::setRegistrationMode(REGISTRATION_STATUS mode)
{
	ViewGroupData::Options options = this->getOptions();

	options.mShowLandmarks = false;
	options.mShowPointPickerProbe = false;

	if (mode == rsIMAGE_REGISTRATED)
	{
		options.mShowLandmarks = true;
		options.mShowPointPickerProbe = true;
	}
	if (mode == rsPATIENT_REGISTRATED)
	{
		options.mShowLandmarks = true;
		options.mShowPointPickerProbe = false;
	}

	this->setOptions(options);
}


} // namespace cx
