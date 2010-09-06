#include "sscDataManagerImpl.h"

#include <sstream>
#include <vtkImageData.h>
#include <vtkMetaImageReader.h>
#include <vtkSmartPointer.h>
#include <vtkMetaImageWriter.h>
typedef vtkSmartPointer<class vtkMetaImageReader> vtkMetaImageReaderPtr;
typedef vtkSmartPointer<class vtkMetaImageWriter> vtkMetaImageWriterPtr;

#include <vtkPolyData.h>
#include <vtkPolyDataReader.h>
#include <vtkSTLReader.h>
#include <vtkImageChangeInformation.h>
typedef vtkSmartPointer<class vtkPolyDataReader> vtkPolyDataReaderPtr;
typedef vtkSmartPointer<class vtkSTLReader> vtkSTLReaderPtr;
typedef vtkSmartPointer<class vtkImageChangeInformation> vtkImageChangeInformationPtr;

#include <QtCore>
#include <QDomDocument>
#include <QFileInfo>
#include <QFile>
#include <QTextStream>
#include <QDir>

#include "sscTransform3D.h"
#include "sscRegistrationTransform.h"
#include "sscMessageManager.h"

namespace ssc
{

//-----
DataPtr MetaImageReader::load(const std::string& uid, const std::string& filename)
{
  //messageManager()->sendDebug("load filename: "+string_cast(filename));
  //read the specific TransformMatrix-tag from the header
  Vector3D p_r(0, 0, 0);
  Vector3D e_x(1, 0, 0);
  Vector3D e_y(0, 1, 0);
  Vector3D e_z(0, 0, 1);

  QString creator;
  QFile file(filename.c_str());

  QString line;
  if (file.open(QIODevice::ReadOnly))
  {
    QTextStream t(&file);
    while (!t.atEnd())
    {
      line.clear();
      line = t.readLine();
      // do something with the line
      if (line.startsWith("Position", Qt::CaseInsensitive) || line.startsWith("Offset", Qt::CaseInsensitive))
      {
        QStringList list = line.split(" ", QString::SkipEmptyParts);
        p_r = Vector3D(list[2].toDouble(), list[3].toDouble(), list[4].toDouble());
      }
      else if (line.startsWith("TransformMatrix", Qt::CaseInsensitive) || line.startsWith("Orientation",
          Qt::CaseInsensitive))
      {
        QStringList list = line.split(" ", QString::SkipEmptyParts);

        e_x = Vector3D(list[2].toDouble(), list[3].toDouble(), list[4].toDouble());
        e_y = Vector3D(list[5].toDouble(), list[6].toDouble(), list[7].toDouble());
        e_z = cross(e_x, e_y);
      }
      else if (line.startsWith("Creator", Qt::CaseInsensitive))
      {
        QStringList list = line.split(" ", QString::SkipEmptyParts);
        creator = list[2];
      }
    }
    file.close();
  }

  // MDH is a volatile format: warn if we dont know the source
  if ((creator != "Ingerid") && (creator != "DICOMtoMetadataFilter"))
  {
    //std::cout << "WARNING: Loading file " + filename + ": unrecognized creator. Position/Orientation cannot be trusted" << std::endl;
  }

  Transform3D rMd;

  // add rotational part
  for (unsigned i = 0; i < 3; ++i)
  {
    rMd[i][0] = e_x[i];
    rMd[i][1] = e_y[i];
    rMd[i][2] = e_z[i];
  }

  // Special Ingerid Reinertsen fix: Position is stored as p_d instead of p_r: convert here
  if (creator == "Ingerid")
  {
    std::cout << "ingrid fixing" << std::endl;
    p_r = rMd.coord(p_r);
  }

  // add translational part
  rMd[0][3] = p_r[0];
  rMd[1][3] = p_r[1];
  rMd[2][3] = p_r[2];

  //load the image from file
  vtkMetaImageReaderPtr reader = vtkMetaImageReaderPtr::New();
  reader->SetFileName(filename.c_str());
  reader->ReleaseDataFlagOn();
  //reader->GetOutput()->ReleaseDataFlagOn();
  reader->Update();

  vtkImageChangeInformationPtr zeroer = vtkImageChangeInformationPtr::New();
  zeroer->SetInput(reader->GetOutput());
  zeroer->SetOutputOrigin(0, 0, 0);

  vtkImageDataPtr imageData = zeroer->GetOutput();
  imageData->Update();

  ImagePtr image(new Image(uid, imageData));

  RegistrationTransform regTrans(rMd, QFileInfo(file.fileName()).lastModified(), "From MHD file");
  image->get_rMd_History()->addRegistration(regTrans);

  //std::cout << "ImagePtr MetaImageReader::load" << std::endl << std::endl;
  return image;
}

//-----
DataPtr PolyDataMeshReader::load(const std::string& uid, const std::string& fileName)
{
  vtkPolyDataReaderPtr reader = vtkPolyDataReaderPtr::New();
  reader->SetFileName(fileName.c_str());
  reader->Update();
  vtkPolyDataPtr polyData = reader->GetOutput();

  //return MeshPtr(new Mesh(fileName, fileName, polyData));
  MeshPtr tempMesh(new Mesh(uid, "PolyData", polyData));
  return tempMesh;

}

DataPtr StlMeshReader::load(const std::string& uid, const std::string& fileName)
{
  vtkSTLReaderPtr reader = vtkSTLReaderPtr::New();
  reader->SetFileName(fileName.c_str());
  reader->Update();
  vtkPolyDataPtr polyData = reader->GetOutput();

  //return MeshPtr(new Mesh(fileName, fileName, polyData));
  MeshPtr tempMesh(new Mesh(uid, "PolyData", polyData));
  return tempMesh;

}

///--------------------------------------------------------
///--------------------------------------------------------
///--------------------------------------------------------

void DataManagerImpl::initialize()
{
  setInstance(new DataManagerImpl());
}

DataManagerImpl::DataManagerImpl()
{
  mMedicalDomain = mdLABORATORY;
  //  mMedicalDomain = mdLAPAROSCOPY;
  mDataReaders[rtMETAIMAGE].reset(new MetaImageReader());
  mDataReaders[rtPOLYDATA].reset(new PolyDataMeshReader());
  mDataReaders[rtSTL].reset(new StlMeshReader());
  //	mCenter = Vector3D(0,0,0);
  //	mActiveImage.reset();
  this->clear();
}

DataManagerImpl::~DataManagerImpl()
{
}

void DataManagerImpl::clear()
{
  mImages.clear();
  mCenter = ssc::Vector3D(0, 0, 0);
  //mMedicalDomain = mdLABORATORY; must be set explicitly
  mMeshes.clear();
  mActiveImage.reset();
  mLandmarkProperties.clear();

  emit centerChanged();
  emit activeImageChanged("");
  emit activeImageTransferFunctionsChanged();
  emit landmarkPropertiesChanged();
  //emit medicalDomainChanged();
  emit dataLoaded();
}

Vector3D DataManagerImpl::getCenter() const
{
  return mCenter;
}
void DataManagerImpl::setCenter(const Vector3D& center)
{
  mCenter = center;
  emit centerChanged();
}
ImagePtr DataManagerImpl::getActiveImage() const
{
  return mActiveImage;
}
void DataManagerImpl::setActiveImage(ImagePtr activeImage)
{
  if (mActiveImage == activeImage)
    return;

  if (mActiveImage)
  {
    disconnect(mActiveImage.get(), SIGNAL(vtkImageDataChanged()), this, SLOT(vtkImageDataChangedSlot()));
    //disconnect(mActiveImage.get(), SIGNAL(transformChanged()), this, SLOT(transformChangedSlot()));
    disconnect(mActiveImage.get(), SIGNAL(transferFunctionsChanged()), this, SLOT(transferFunctionsChangedSlot()));
  }

  mActiveImage = activeImage;

  if (mActiveImage)
  {
    connect(mActiveImage.get(), SIGNAL(vtkImageDataChanged()), this, SLOT(vtkImageDataChangedSlot()));
    //connect(mActiveImage.get(), SIGNAL(transformChanged()), this, SLOT(transformChangedSlot()));
    connect(mActiveImage.get(), SIGNAL(transferFunctionsChanged()), this, SLOT(transferFunctionsChangedSlot()));
  }
  this->vtkImageDataChangedSlot();
}

void DataManagerImpl::setLandmarkNames(std::vector<std::string> names)
{
  mLandmarkProperties.clear();
  for (unsigned i = 0; i < names.size(); ++i)
  {
    LandmarkProperty prop(string_cast(i + 1), names[i]); // generate 1-indexed uids (keep users happy)
    mLandmarkProperties[prop.getUid()] = prop;
  }
  emit landmarkPropertiesChanged();
}

std::string DataManagerImpl::addLandmark()
{
  int max = 0;
  std::map<std::string, LandmarkProperty>::iterator iter;
  for (iter = mLandmarkProperties.begin(); iter != mLandmarkProperties.end(); ++iter)
  {
    max = std::max(max, qstring_cast(iter->second.getName()).toInt());
  }
  std::string uid = string_cast(max + 1);
  mLandmarkProperties[uid] = LandmarkProperty(uid);

  emit landmarkPropertiesChanged();
  return uid;
}

void DataManagerImpl::setLandmarkName(std::string uid, std::string name)
{
  mLandmarkProperties[uid].setName(name);
  emit landmarkPropertiesChanged();
}

std::map<std::string, LandmarkProperty> DataManagerImpl::getLandmarkProperties() const
{
  return mLandmarkProperties;
}

void DataManagerImpl::setLandmarkActive(std::string uid, bool active)
{
  mLandmarkProperties[uid].setActive(active);
  emit landmarkPropertiesChanged();
}

ImagePtr DataManagerImpl::loadImage(const std::string& uid, const std::string& filename, READER_TYPE type)
{
  //TODO
  if (mImages.count(uid)) // dont load same image twice
  {
    return mImages[uid];
    std::cout << "WARNING: Image with uid: " + uid + " already exists, abort loading.";
  }

  if (type == rtAUTO)
  {
    // do special stuff
    return ImagePtr();
  }

  // identify type
  DataPtr current = mDataReaders[type]->load(uid, filename);
  this->loadData(current);
  return boost::shared_dynamic_cast<ssc::Image>(current);
}

void DataManagerImpl::loadImage(ImagePtr image)
{
  if (image)
  {
    mImages[image->getUid()] = image;
//    std::cout << "DataManagerImpl::loadImage B " << image->getName() << std::endl;
    emit dataLoaded();
//    std::cout << "DataManagerImpl::loadImage E" << std::endl;
  }
}
void DataManagerImpl::loadMesh(MeshPtr data)
{
  if (data)
  {
    mMeshes[data->getUid()] = data;
//    std::cout << "DataManagerImpl::loadImage B " << image->getName() << std::endl;
    emit dataLoaded();
//    std::cout << "DataManagerImpl::loadImage E" << std::endl;
  }
}

void DataManagerImpl::loadData(DataPtr data)
{
  if (boost::shared_dynamic_cast<Image>(data))
  {
    this->loadImage(boost::shared_dynamic_cast<Image>(data));
  }
  if (boost::shared_dynamic_cast<Mesh>(data))
  {
    this->loadMesh(boost::shared_dynamic_cast<Mesh>(data));
  }
}

void DataManagerImpl::saveImage(ImagePtr image, const std::string& basePath)
{
  vtkMetaImageWriterPtr writer = vtkMetaImageWriterPtr::New();
  writer->SetInput(image->getBaseVtkImageData());
  writer->SetFileDimensionality(3);
  std::string filename = basePath + "/" + image->getFilePath();
  writer->SetFileName(filename.c_str());

  //Rename ending from .mhd to .raw
  QStringList splitName = qstring_cast(filename).split(".");
  splitName[splitName.size() - 1] = "raw";
  filename = string_cast(splitName.join("."));

  writer->SetRAWFileName(filename.c_str());
  writer->SetCompression(false);
  writer->Update();
  writer->Write();
}

// meshes
MeshPtr DataManagerImpl::loadMesh(const std::string& uid, const std::string& fileName,
    READER_TYPE meshType)
{
  if (mMeshes.count(uid)) // dont load same mesh twice
  {
    return mMeshes[uid];
    std::cout << "WARNING: Mesh with uid: " + uid + " already exists, abort loading.";
  }
  // identify type
  DataPtr newMesh = mDataReaders[meshType]->load(uid, fileName);
  if (newMesh)
  {
    this->loadData(newMesh);
//    mMeshes[newMesh->getUid()] = newMesh;
//    emit dataLoaded();
  }
  return boost::shared_dynamic_cast<ssc::Mesh>(newMesh);
}

ImagePtr DataManagerImpl::getImage(const std::string& uid)
{
  if (!mImages.count(uid))
    return ImagePtr();
  return mImages[uid];
}

std::map<std::string, ImagePtr> DataManagerImpl::getImages()
{
  return mImages;
}

std::map<std::string, std::string> DataManagerImpl::getImageUidsAndNames() const
{
  std::map<std::string, std::string> retval;
  for (ImagesMap::const_iterator iter = mImages.begin(); iter != mImages.end(); ++iter)
    retval[iter->first] = iter->second->getName();
  return retval;
}

std::vector<std::string> DataManagerImpl::getImageNames() const
{
  std::vector<std::string> retval;
  for (ImagesMap::const_iterator iter = mImages.begin(); iter != mImages.end(); ++iter)
    retval.push_back(iter->second->getName());
  return retval;
}

std::vector<std::string> DataManagerImpl::getImageUids() const
{
  std::vector<std::string> retval;
  for (ImagesMap::const_iterator iter = mImages.begin(); iter != mImages.end(); ++iter)
    retval.push_back(iter->first);
  return retval;
}

MeshPtr DataManagerImpl::getMesh(const std::string& uid)
{
  if (!mMeshes.count(uid))
    return MeshPtr();
  return mMeshes[uid];
}

std::map<std::string, MeshPtr> DataManagerImpl::getMeshes()
{
  return mMeshes;
}

std::map<std::string, std::string> DataManagerImpl::getMeshUidsWithNames() const
{
  std::map<std::string, std::string> retval;
  for (MeshMap::const_iterator iter = mMeshes.begin(); iter != mMeshes.end(); ++iter)
    retval[iter->first] = iter->second->getName();
  return retval;
}
std::vector<std::string> DataManagerImpl::getMeshUids() const
{
  return std::vector<std::string>();
}
std::vector<std::string> DataManagerImpl::getMeshNames() const
{
  return std::vector<std::string>();
}

void DataManagerImpl::addXml(QDomNode& parentNode)
{
  QDomDocument doc = parentNode.ownerDocument();
  QDomElement dataManagerNode = doc.createElement("datamanager");
  parentNode.appendChild(dataManagerNode);

  QDomElement activeImageNode = doc.createElement("activeImageUid");
  if (mActiveImage)
    activeImageNode.appendChild(doc.createTextNode(mActiveImage->getUid().c_str()));
  dataManagerNode.appendChild(activeImageNode);

  QDomElement landmarkPropsNode = doc.createElement("landmarkprops");
  LandmarkPropertyMap::iterator it = mLandmarkProperties.begin();
  for (; it != mLandmarkProperties.end(); ++it)
  {
    QDomElement landmarkPropNode = doc.createElement("landmarkprop");
    it->second.addXml(landmarkPropNode);
    landmarkPropsNode.appendChild(landmarkPropNode);
  }
  dataManagerNode.appendChild(landmarkPropsNode);

  //TODO
  /*QDomElement activeMeshNode = doc.createElement("activeMesh");
   if(mActiveMesh)
   activeMeshNode.appendChild(doc.createTextNode(mActiveMesh->getUid().c_str()));
   dataManagerNode.appendChild(activeMeshNode);*/

  QDomElement centerNode = doc.createElement("center");
  std::stringstream centerStream;
  centerStream << mCenter;
  centerNode.appendChild(doc.createTextNode(centerStream.str().c_str()));
  dataManagerNode.appendChild(centerNode);


  for (ImagesMap::const_iterator iter = mImages.begin(); iter != mImages.end(); ++iter)
  {
    QDomElement dataNode = doc.createElement("data");
    dataManagerNode.appendChild(dataNode);
    iter->second->addXml(dataNode);
  }
  for (std::map<std::string, MeshPtr>::const_iterator iter = mMeshes.begin(); iter != mMeshes.end(); ++iter)
  {
    QDomElement dataNode = doc.createElement("data");
    dataManagerNode.appendChild(dataNode);
    iter->second->addXml(dataNode);
  }
}

void DataManagerImpl::parseXml(QDomNode& dataManagerNode, QString absolutePath)
{
  QDomNode landmarksNode = dataManagerNode.namedItem("landmarkprops");
  QDomElement landmarkNode = landmarksNode.firstChildElement("landmarkprop");
  for (; !landmarkNode.isNull(); landmarkNode = landmarkNode.nextSiblingElement("landmarkprop"))
  {
    LandmarkProperty landmarkProp;
    landmarkProp.parseXml(landmarkNode);
    mLandmarkProperties[landmarkProp.getUid()] = landmarkProp;
    //std::cout << "Loaded landmarkprop with name: " << landmarkProp.getName() << std::endl;
    emit landmarkPropertiesChanged();
  }

  // All images must be created from the DataManager, so the image nodes
  // are parsed here
  QDomNode child = dataManagerNode.firstChild();
  QDomElement nameNode;
  QString path;//Path to data file

  while (!child.isNull())
  {
    if (child.nodeName() == "data")
    {
      QString uidNodeString = child.namedItem("uid").toElement().text();

      nameNode = child.namedItem("name").toElement();
      QDomElement filePathNode = child.namedItem("filePath").toElement();
      if (!filePathNode.isNull())
      {
        path = filePathNode.text();
        QDir relativePath = QDir(QString(path));
        if (!absolutePath.isEmpty())
          if (relativePath.isRelative())
            path = absolutePath + "/" + path;
          else //Make relative
          {
            QDir patientDataDir(absolutePath);
            relativePath.setPath(patientDataDir.relativeFilePath(relativePath.path()));
          }

        if (!path.isEmpty())
        {
          ssc::DataPtr data;

          QFileInfo fileInfo(path);
          QString fileType = fileInfo.suffix();
          READER_TYPE readerType = this->getReaderType(fileType);
          data = mDataReaders[readerType]->load(uidNodeString.toStdString(), path.toStdString());

          if (!mDataReaders.count(readerType))
          {
            messageManager()->sendWarning("Unknown file: " + string_cast(fileInfo.absolutePath()));
            return;
          }

//          if (fileType.compare("mhd", Qt::CaseInsensitive) == 0 || fileType.compare("mha", Qt::CaseInsensitive) == 0)
//          {
//            data = mImageReaders[ssc::rtMETAIMAGE]->load(uidNodeString.toStdString(), path.toStdString());
//          }
//          else if (fileType.compare("stl", Qt::CaseInsensitive) == 0)
//          {
//            data = mMeshReaders[ssc::mrtSTL]->load(uidNodeString.toStdString(), path.toStdString());
//          }
//          else if (fileType.compare("vtk", Qt::CaseInsensitive) == 0)
//          {
//            data = mMeshReaders[ssc::mrtPOLYDATA]->load(uidNodeString.toStdString(), path.toStdString());
//          }
//          else
//          {
//            messageManager()->sendWarning("Unknown file: " + string_cast(fileInfo.absolutePath()));
//            return;
//          }

          if (nameNode.text().isEmpty())
            data->setName(fileInfo.fileName().toStdString());
          else
            data->setName(nameNode.text().toStdString());
          data->setFilePath(relativePath.path().toStdString());
          data->parseXml(child);

          this->loadData(data);
        }
        else
        {
          std::cout << "Warning: DataManager::parseXml() empty filePath for data";
          std::cout << std::endl;
        }
      }
      // Don't use uid as path
      //else if(!uidNode.isNull())
      //  path = uidNode.text().toStdString();
      else
      {
        std::cout << "Warning: DataManager::parseXml() found no filePath for data";
        std::cout << std::endl;
      }
    }
    child = child.nextSibling();
  }

  //we need to make sure all images are loaded before we try to set an active image
  child = dataManagerNode.firstChild();
  while (!child.isNull())
  {
    if (child.toElement().tagName() == "activeImageUid")
    {
      const QString activeImageString = child.toElement().text();
      //std::cout << "Found a activeImage with uid: " << activeImageString.toStdString().c_str() << std::endl;
      if (!activeImageString.isEmpty())
      {
        ImagePtr image = this->getImage(activeImageString.toStdString());
        //std::cout << "Got an image with uid: " << image->getUid().c_str() << std::endl;
        this->setActiveImage(image);
      }
    }
    //TODO add activeMesh
    if (child.toElement().tagName() == "center")
    {
      const QString centerString = child.toElement().text();
      if (!centerString.isEmpty())
      {
        Vector3D center = Vector3D::fromString(centerString);
        this->setCenter(center);
        //std::cout << "Center: " << center << std::endl;
      }
    }
    child = child.nextSibling();
  }
}

READER_TYPE DataManagerImpl::getReaderType(QString fileType)
{
  if (fileType.compare("mhd", Qt::CaseInsensitive) == 0 || fileType.compare("mha", Qt::CaseInsensitive) == 0)
  {
    return ssc::rtMETAIMAGE;
  }
  else if (fileType.compare("stl", Qt::CaseInsensitive) == 0)
  {
    return ssc::rtSTL;
  }
  else if (fileType.compare("vtk", Qt::CaseInsensitive) == 0)
  {
    return ssc::rtPOLYDATA;
  }
  return ssc::rtCOUNT;
}

void DataManagerImpl::vtkImageDataChangedSlot()
{
  std::string uid = "";
  if (mActiveImage)
    uid = mActiveImage->getUid();

  emit activeImageChanged(uid);
  messageManager()->sendInfo("Active image set to " + string_cast(uid));
}

void DataManagerImpl::transferFunctionsChangedSlot()
{
  emit activeImageTransferFunctionsChanged();
}

MEDICAL_DOMAIN DataManagerImpl::getMedicalDomain() const
{
  return mMedicalDomain;
}

void DataManagerImpl::setMedicalDomain(MEDICAL_DOMAIN domain)
{
  if (mMedicalDomain==domain)
    return;
  mMedicalDomain = domain;
  emit medicalDomainChanged();
}


} // namespace ssc

