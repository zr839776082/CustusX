/*
 * cxPatientData.cpp
 *
 *  Created on: May 18, 2010
 *      Author: christiana
 */
#include "cxPatientData.h"

#include <QtGui>
#include <QDomDocument>

#include "sscTime.h"
#include "sscMessageManager.h"
#include "cxDataManager.h"
#include "cxViewManager.h"
#include "cxRepManager.h"
#include "sscToolManager.h"
#include "sscToolManager.h"
#include "cxFileCopied.h"
#include "cxDataLocations.h"
#include "cxRegistrationManager.h"

namespace cx
{

PatientData::PatientData(QObject* parent) :
    QObject(parent),
    mSettings(DataLocations::getSettings())
{

}

QString PatientData::getActivePatientFolder() const
{
  return mActivePatientFolder;
}

void PatientData::setActivePatient(const QString& activePatientFolder)
{
  //ssc::messageManager()->sendDebug("PatientData::setActivePatient to: "+string_cast(activePatientFolder));
  mActivePatientFolder = activePatientFolder;
  //TODO
  //Update gui in some way to show which patient is active

  emit patientChanged();
}

void PatientData::newPatient(QString choosenDir)
{
  createPatientFolders(choosenDir);
  this->setActivePatient(choosenDir);
}

//void PatientData::loadPatientFileSlot()
void PatientData::loadPatient(QString choosenDir)
{
  //ssc::messageManager()->sendDebug("loadPatient() choosenDir: "+string_cast(choosenDir));
  if (choosenDir == QString::null)
    return; // On cancel
  
  QFile file(choosenDir+"/custusdoc.xml");
  if(file.open(QIODevice::ReadOnly))
  {
    QDomDocument doc;
    QString emsg;
    int eline, ecolumn;
    // Read the file
    if (!doc.setContent(&file, false, &emsg, &eline, &ecolumn))
    {
      ssc::messageManager()->sendError("Could not parse XML file :"
                                 +file.fileName().toStdString()+" because: "
                                 +emsg.toStdString()+"");
    }
    else
    {
      //Read the xml
      this->readLoadDoc(doc, choosenDir);
    }
    file.close();
  }
  else //User have created the directory create xml file and folders
  {
    //TODO: Ask the user if he want to convert the folder
    ssc::messageManager()->sendInfo("Found no CX3 data in folder: " +
                                    string_cast(choosenDir) +
                                    " Converting the folder to a patent folder...");
    createPatientFolders(choosenDir);
  }
  
  this->setActivePatient(choosenDir);
}

void PatientData::savePatient()
{

  if(mActivePatientFolder.isEmpty())
  {
//    ssc::messageManager()->sendWarning("No patient selected, select or create patient before saving!");
//    this->newPatientSlot();
    return;
  }

  //Gather all the information that needs to be saved
  QDomDocument doc;
  this->generateSaveDoc(doc);

  QFile file(mActivePatientFolder + "/custusdoc.xml");
  if(file.open(QIODevice::WriteOnly | QIODevice::Truncate))
  {
    QTextStream stream(&file);
    stream << doc.toString();
    file.close();
    ssc::messageManager()->sendInfo("Created "+file.fileName().toStdString());
  }
  else
  {
    ssc::messageManager()->sendError("Could not open "+file.fileName().toStdString()
                               +" Error: "+file.errorString().toStdString());
  }

  //Write the data to file, fx modified images... etc...
  //TODO Implement when we know what we want to save here...
}

void PatientData::importData(QString fileName)
{
  //ssc::messageManager()->sendDebug("PatientData::importData() called");
//  this->savePatientFileSlot();

//  QString fileName = QFileDialog::getOpenFileName( this,
//                                  QString(tr("Select data file")),
//                                  mSettings->value("globalPatientDataFolder").toString(),
//                                  tr("Image/Mesh (*.mhd *.mha *.stl *.vtk)"));
  if(fileName.isEmpty())
  {
    ssc::messageManager()->sendInfo("Import canceled");
    return;
  }
  
  QString patientsImageFolder = mActivePatientFolder+"/Images/";
  QString patientsSurfaceFolder = mActivePatientFolder+"/Surfaces/";

  QDir dir;
  if(!dir.exists(patientsImageFolder))
  {
    dir.mkpath(patientsImageFolder);
    ssc::messageManager()->sendInfo("Made new directory: "+patientsImageFolder.toStdString());
  }
  if(!dir.exists(patientsSurfaceFolder))
  {
    dir.mkpath(patientsSurfaceFolder);
    ssc::messageManager()->sendInfo("Made new directory: "+patientsSurfaceFolder.toStdString());
  }

  QFileInfo fileInfo(fileName);
  QString fileType = fileInfo.suffix();
  QString pathToNewFile = patientsImageFolder+fileInfo.fileName();
  QFile fromFile(fileName);
  QString uid = fileInfo.fileName()+"_"+fileInfo.created().toString(ssc::timestampSecondsFormat());

  //Need to wait for the copy to finish...

  // Read files before copy
  ssc::DataPtr data;

  if(fileType.compare("mhd", Qt::CaseInsensitive) == 0 ||
     fileType.compare("mha", Qt::CaseInsensitive) == 0)
  {
    data = ssc::dataManager()->loadImage(uid.toStdString(), fileName.toStdString(), ssc::rtMETAIMAGE);
  }else if(fileType.compare("stl", Qt::CaseInsensitive) == 0)
  {
    data = ssc::dataManager()->loadMesh(uid.toStdString(), fileName.toStdString(), ssc::mrtSTL);
    pathToNewFile = patientsSurfaceFolder+fileInfo.fileName();
  }else if(fileType.compare("vtk", Qt::CaseInsensitive) == 0)
  {
    data = ssc::dataManager()->loadMesh(uid.toStdString(), fileName.toStdString(), ssc::mrtPOLYDATA);
    pathToNewFile = patientsSurfaceFolder+fileInfo.fileName();
  }
  data->setName(fileInfo.fileName().toStdString());

  data->setShading(true);

  QDir patientDataDir(mActivePatientFolder);
  FileCopied *fileCopied = new FileCopied(pathToNewFile.toStdString(),
                                          patientDataDir.relativeFilePath(pathToNewFile).toStdString(),
                                          data);
  connect(fileCopied, SIGNAL(fileCopiedCorrectly()),
          this, SLOT(savePatient()));
  QTimer::singleShot(5000, fileCopied, SLOT(areFileCopiedSlot()));// Wait 5 seconds

  //Copy file
  if(fileName != pathToNewFile) //checks if we need to copy
  {
    QFile toFile(pathToNewFile);
    if(fromFile.copy(toFile.fileName()))
    {
      //messageMan()->sendInfo("File copied to new location: "+pathToNewFile.toStdString());
    }else
    {
      ssc::messageManager()->sendError("First copy failed!");
      return;
    }
    if(!toFile.flush())
      ssc::messageManager()->sendWarning("Failed to copy file"+toFile.fileName().toStdString());
    if(!toFile.exists())
      ssc::messageManager()->sendWarning("File not copied");
    //make sure we also copy the .raw file in case if mhd/mha
    if(fileType.compare("mhd", Qt::CaseInsensitive) == 0)
    {
      //presuming the other file is a raw file
      //TODO: what if it's not?
      QString originalRawFile = fileName.replace(".mhd", ".raw");
      QString newRawFile = pathToNewFile.replace(".mhd", ".raw");
      fromFile.setFileName(originalRawFile);
      toFile.setFileName(newRawFile);

      if(fromFile.copy(toFile.fileName()))
      {
        //messageMan()->sendInfo("File copied to new location: "+newRawFile.toStdString());
      }
      else
      {
        ssc::messageManager()->sendError("Second copy failed!");
        return;
      }
      if(!toFile.flush())
        ssc::messageManager()->sendWarning("Failed to copy file"+toFile.fileName().toStdString());
      if(!toFile.exists())
        ssc::messageManager()->sendWarning("File not copied");

    }else if(fileType.compare("mha", Qt::CaseInsensitive) == 0)
    {
      //presuming the other file is a raw file
      //TODO: what if it's not?
      QString originalRawFile = fileName.replace(".mha", ".raw");
      QString newRawFile = pathToNewFile.replace(".mha", ".raw");
      fromFile.setFileName(originalRawFile);
      toFile.setFileName(newRawFile);

      if(fromFile.copy(toFile.fileName()))
      {
        //messageMan()->sendInfo("File copied to new location: "+newRawFile.toStdString());
      }
      else
      {
        ssc::messageManager()->sendError("Second copy failed!");
        return;
      }
    }
  }
}

void PatientData::createPatientFolders(QString choosenDir)
{
  //ssc::messageManager()->sendDebug("PatientData::createPatientFolders() called");
  if(!choosenDir.endsWith(".cx3"))
    choosenDir.append(".cx3");

  ssc::messageManager()->sendInfo("Selected a patient to work with.");

  // Create folders
  if(!QDir().exists(choosenDir))
  {
    QDir().mkdir(choosenDir);
    ssc::messageManager()->sendInfo("Made a new patient folder: "+choosenDir.toStdString());
  }

  QString newDir = choosenDir;
  newDir.append("/Images");
  if(!QDir().exists(newDir))
  {
    QDir().mkdir(newDir);
    ssc::messageManager()->sendInfo("Made a new image folder: "+newDir.toStdString());
  }

  newDir = choosenDir;
  newDir.append("/Surfaces");
  if(!QDir().exists(newDir))
  {
    QDir().mkdir(newDir);
    ssc::messageManager()->sendInfo("Made a new surface folder: "+newDir.toStdString());
  }

  newDir = choosenDir;
  newDir.append("/Logs");
  if(!QDir().exists(newDir))
  {
    QDir().mkdir(newDir);
    ssc::messageManager()->sendInfo("Made a new logging folder: "+newDir.toStdString());
  }

  this->savePatient();
}


/**
 * Xml version 1.0: Knows about the nodes: \n
 * \<managers\> \n
 *   \<datamanager\> \n
 *     \<image\> \n
 *        \<uid\> //an images unique id \n
 *        \<name\> //an images name \n
 *        \<transferfunctions\> //an images transferefunction \n
 *            \<alpha\> //a transferefunctions alpha values \n
 *            \<color\> //a transferefunctions color values
 */
/**
 * Xml version 2.0: Knows about the nodes: \n
 * \<patient\> \n
 *  \<active_patient\> //relative path to this patients folder \n
 *  \<managers\> \n
 *     \<datamanager\> \n
 *       \<image\> \n
 *         \<uid\> //an images unique id \n
 *         \<name\> //an images name \n
 *         \<transferfunctions\> //an images transferefunction \n
 *            \<alpha\> //a transferefunctions alpha values \n
 *            \<color\> //a transferefunctions color values \n
 */
void PatientData::generateSaveDoc(QDomDocument& doc)
{
  doc.appendChild(doc.createProcessingInstruction("xml version =", "'2.0'"));

  QDomElement patientNode = doc.createElement("patient");
  QDomElement activePatientNode = doc.createElement("active_patient");
  activePatientNode.appendChild(doc.createTextNode(mActivePatientFolder.toStdString().c_str()));
  patientNode.appendChild(activePatientNode);
  doc.appendChild(patientNode);

  QDomElement managerNode = doc.createElement("managers");
  patientNode.appendChild(managerNode);

  ssc::dataManager()->addXml(managerNode);
  ssc::toolManager()->addXml(managerNode);
  viewManager()->addXml(managerNode);
  registrationManager()->addXml(managerNode);

  //TODO Implement
  /*
  ssc::messageManager()->getXml(doc); //TODO
  repManager->getXml(doc); //TODO
  */

  ssc::messageManager()->sendInfo("Xml file ready to be written to disk.");
}
void PatientData::readLoadDoc(QDomDocument& doc, QString patientFolder)
{
  //ssc::messageManager()->sendDebug("PatientData::readLoadDoc() called");
  //Get all the nodes
  QDomNode patientNode = doc.namedItem("patient");
  QDomNode managerNode = patientNode.namedItem("managers");

  //Evaluate the xml nodes and load what's needed
  QDomNode dataManagerNode = managerNode.namedItem("datamanager");
  if(!patientNode.isNull())
  {
    QDomElement activePatientNode = patientNode.namedItem("active_patient").toElement();
    if(!activePatientNode.isNull())
    {
      ssc::messageManager()->sendInfo("Active patient loaded to be "
                                +mActivePatientFolder.toStdString());
    }
  }
  if (!dataManagerNode.isNull())
  {
    ssc::dataManager()->parseXml(dataManagerNode, patientFolder);
  }

  QDomNode toolmanagerNode = managerNode.namedItem("toolManager");
  ssc::toolManager()->parseXml(toolmanagerNode);

  QDomNode viewmanagerNode = managerNode.namedItem("viewManager");
  viewManager()->parseXml(viewmanagerNode);

  QDomNode registrationNode = managerNode.namedItem("registrationManager");
  registrationManager()->parseXml(registrationNode);
}


} // namespace cx

