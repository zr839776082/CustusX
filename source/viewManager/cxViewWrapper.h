#ifndef CXVIEWWRAPPER_H_
#define CXVIEWWRAPPER_H_

#include <QVariant>
#include <QObject>
#include "sscView.h"
#include "sscImage.h"
#include "sscDefinitions.h"
#include "cxForwardDeclarations.h"

class QMenu;
class QActionGroup;
typedef vtkSmartPointer<class vtkCamera> vtkCameraPtr;

namespace cx
{

typedef boost::shared_ptr<class SyncedValue> SyncedValuePtr;

class SyncedValue : public QObject
{
  Q_OBJECT
public:
  SyncedValue(QVariant val=QVariant());
  static SyncedValuePtr create(QVariant val=QVariant());
  void set(QVariant val);
  QVariant get() const;
private:
  QVariant mValue;
signals:
  void changed();
};


typedef boost::shared_ptr<class CameraData> CameraDataPtr;

/** Class encapsulating the view transform of a camera. Use with vtkCamera
 */
class CameraData
{
public:
  CameraData();
  static CameraDataPtr create() { return CameraDataPtr(new CameraData()); }
//	void writeCamera(vtkCameraPtr camera);
//	void readCamera(vtkCameraPtr camera);

	void setCamera(vtkCameraPtr camera);
	vtkCameraPtr getCamera() const;

	void addXml(QDomNode dataNode) const; ///< store internal state info in dataNode
	void parseXml(QDomNode dataNode);///< load internal state info from dataNode

//	ssc::Vector3D mPosition;
//	ssc::Vector3D mFocalPoint;
//	ssc::Vector3D mViewUp;
//	double mNearClip, mFarClip;
//	double mParallelScale;
private:
	mutable vtkCameraPtr mCamera;
	void addTextElement(QDomNode parentNode, QString name, QString value) const;
};


typedef boost::shared_ptr<class ViewGroupData> ViewGroupDataPtr;

/** Container for data shared between all members of a view group
 */
class ViewGroupData : public QObject
{
  Q_OBJECT
public:
  ViewGroupData();
  void requestInitialize();
  std::vector<ssc::DataPtr> getData() const;
  void addData(ssc::DataPtr data);
  void removeData(ssc::DataPtr data);
  void clearData();
  std::vector<ssc::ImagePtr> getImages() const;
  std::vector<ssc::MeshPtr> getMeshes() const;

  //void setCamera3D(CameraDataPtr transform) { mCamera3D = transform; }
  CameraDataPtr getCamera3D() { return mCamera3D; }

signals:
  void dataAdded(QString uid);
  void dataRemoved(QString uid);
  void initialized();
private:
  std::vector<ssc::DataPtr> mData;
  CameraDataPtr mCamera3D;
};

/**
 * \class cxViewWrapper.h
 *
 * \brief
 *
 * \date 6. apr. 2010
 * \author: jbake
 */
class ViewWrapper : public QObject
{
  Q_OBJECT
public:
  virtual ~ViewWrapper() {}
  virtual void initializePlane(ssc::PLANE_TYPE plane) {}
  virtual void setRegistrationMode(ssc::REGISTRATION_STATUS mode) {}
  virtual ssc::View* getView() = 0;
  virtual void setSlicePlanesProxy(ssc::SlicePlanesProxyPtr proxy) = 0;
  virtual void setViewGroup(ViewGroupDataPtr group);

  virtual void setZoom2D(SyncedValuePtr value) {}
  virtual void setOrientationMode(SyncedValuePtr value) {}

signals:
  void orientationChanged(ssc::ORIENTATION_TYPE type);

protected slots:
  void contextMenuSlot(const QPoint& point);
  void dataActionSlot();

  void dataAddedSlot(QString uid);
  void dataRemovedSlot(QString uid);

protected:
  virtual void imageAdded(ssc::ImagePtr image) = 0;
  virtual void meshAdded(ssc::MeshPtr mesh) = 0;
  virtual void imageRemoved(const QString& uid) = 0;
  virtual void meshRemoved(const QString& uid) = 0;

  void connectContextMenu(ssc::View* view);
  virtual void appendToContextMenu(QMenu& contextMenu) = 0;
  void addDataAction(std::string uid, QMenu* contextMenu);

  ViewGroupDataPtr mViewGroup;
};

}//namespace cx
#endif /* CXVIEWWRAPPER_H_ */
