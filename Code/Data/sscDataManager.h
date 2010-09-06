#ifndef SSCDATAMANAGER_H_
#define SSCDATAMANAGER_H_

#include <map>
#include <string>
#include <vector>
#include <boost/shared_ptr.hpp>
#include <QObject>

#include "sscVector3D.h"
#include "sscLandmark.h"
#include "sscDefinitions.h"

namespace ssc
{
// forward declarations
typedef boost::shared_ptr<class Image> ImagePtr;
typedef boost::shared_ptr<class Mesh> MeshPtr;

//-----
enum READER_TYPE
{
	rtDICOM,
	rtSONOWAND_M3D,
	rtMETAIMAGE,
  rtPOLYDATA,
  rtSTL,
	rtAUTO,
	rtCOUNT
};
//-----
//enum MESH_READER_TYPE
//{
//	mrtPOLYDATA,
//	mrtSTL,
//};

/**Manager for images, meshes, and associated data.
 *
 * Simply calling instance() will instantiate the default manager DataManagerImpl.
 * It is also possible to subclass and use setInstance() to set another type.
 */
class DataManager : public QObject
{
	Q_OBJECT
public:
	typedef std::map<std::string, ImagePtr> ImagesMap;
	typedef std::map<std::string, MeshPtr> MeshMap;

	static DataManager* getInstance();
  static void shutdown();

	// images
	virtual ImagePtr loadImage(const std::string& uid, const std::string& filename, READER_TYPE type) = 0;
  virtual void loadImage(ImagePtr image) = 0; ///< load an image generated outside the manager.
  virtual void saveImage(ImagePtr image, const std::string& basePath) = 0; ///< Save image to file
	virtual ImagePtr getImage(const std::string& uid) = 0;
	virtual std::map<std::string, ImagePtr> getImages() = 0;

	virtual std::map<std::string, std::string> getImageUidsAndNames() const = 0;
	virtual std::vector<std::string> getImageNames() const = 0;
	virtual std::vector<std::string> getImageUids() const = 0;

	// meshes
	virtual MeshPtr loadMesh(const std::string& uid, const std::string& fileName, READER_TYPE meshType) = 0;
	virtual MeshPtr getMesh(const std::string& uid) = 0;
	virtual std::map<std::string, MeshPtr> getMeshes() = 0;

	virtual std::map<std::string, std::string> getMeshUidsWithNames() const = 0;
	virtual std::vector<std::string> getMeshUids() const = 0;
	virtual std::vector<std::string> getMeshNames() const = 0;

	// global data (move to separate class if list grows)
	virtual Vector3D getCenter() const = 0; ///< current common center point for user viewing.
	virtual void setCenter(const Vector3D& center) = 0;

	// state information
	virtual ImagePtr getActiveImage() const; ///< used for system state
	virtual void setActiveImage(ImagePtr activeImage); ///< used for system state

  virtual std::string addLandmark() { return ""; }
  virtual void setLandmarkNames(std::vector<std::string> names) {}
  virtual void setLandmarkName(std::string uid, std::string name) {}
  virtual void setLandmarkActive(std::string uid, bool active) {}
  virtual LandmarkPropertyMap getLandmarkProperties() const { return LandmarkPropertyMap(); }
  virtual MEDICAL_DOMAIN getMedicalDomain() const { return mdLABORATORY; }
  virtual void setMedicalDomain(MEDICAL_DOMAIN domain) {}
  virtual void clear() = 0; ///< remove all stuff from manager

	//virtual MeshPtr getActiveMesh() const = 0; ///< used for system state
	//virtual void setActiveMesh(MeshPtr activeMesh) = 0; ///< used for system state
  virtual void addXml(QDomNode& parentNode) {} ///< adds xml information about the datamanger and its variabels
  virtual void parseXml(QDomNode& datamangerNode, QString absolutePath = QString()) {} ///< Use a XML node to load data. \param datamangerNode A XML data representation of the DataManager. \param absolutePath Absolute path to the data elements. Used together with the relative paths stored in the filePath elements.

signals:
	void centerChanged(); ///< emitted when center is changed.
	void dataLoaded(); ///emitted when data is loaded successfully
	void activeImageChanged(const std::string& uId); ///< emitted when the active image or the image data inside the image is changed 
	void activeImageTransferFunctionsChanged(); ///< emitted when the transfer functions in active image is changed
	//void activeMeshChanged(const std::string& uId); ///< emitted when the active mesh is changed
	void landmarkPropertiesChanged(); ///< emitted when global info about a landmark changed
	void medicalDomainChanged();

protected:
	static void setInstance(DataManager* instance);
	DataManager();
	virtual ~DataManager();

private:
	static DataManager* mInstance;
};

DataManager* dataManager();

} // namespace ssc

#endif /*SSCDATAMANAGER_H_*/
