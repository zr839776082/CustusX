/*
 * cxTreeModelItem.cpp
 *
 *  \date Jul 30, 2010
 *      \author christiana
 */

#include "cxTreeModelItem.h"
//#include "sscDataManager.h"
#include "cxTypeConversions.h"
//#include "sscToolManager.h"
#include "cxTrackingService.h"
#include "cxBrowserWidget.h"
#include "cxImage.h"
#include "cxLegacySingletons.h"
#include "cxPatientModelService.h"

namespace cx
{

TreeItemPtr TreeItemImpl::create(BrowserItemModel* model)
{
  TreeItemImpl* retval = new TreeItemImpl(TreeItemWeakPtr(),"","","");
  retval->mModel = model;
  return TreeItemPtr(retval);
}

TreeItemPtr TreeItemImpl::create(TreeItemWeakPtr parent, QString name, QString type, QString val)
{
  TreeItemPtr retval(new TreeItemImpl(parent,name,type,val));
  if (parent.lock())
    parent.lock()->addChild(retval);
  return retval;
}

TreeItemImpl::TreeItemImpl(TreeItemWeakPtr parent, QString name, QString type, QString val) :
  mParent(parent),
  mName(name), mType(type), mData(val), mModel(NULL)
{
  if (parent.lock())
  {
    mModel = parent.lock()->getModel();
  }
}

TreeItemPtr TreeItemImpl::addChild(TreeItemPtr child)
{
  mChildren.push_back(child);
  return child;
}
TreeItemPtr TreeItemImpl::getChild(int row)
{
  return mChildren[row];
}

QString TreeItemImpl::getName() const { return mName; }
QString TreeItemImpl::getType() const { return mType; }
QString TreeItemImpl::getData() const  { return mData; }

int TreeItemImpl::getChildCount() const { return static_cast<int>(mChildren.size()); }
int TreeItemImpl::getColumnCount() const { return 3; }
TreeItemWeakPtr TreeItemImpl::getParent()
{
  return mParent;
}
void TreeItemImpl::activate()
{
  // ignore
}


///--------------------------------------------------------
///--------------------------------------------------------
///--------------------------------------------------------


TreeItemPtr TreeItemImage::create(TreeItemWeakPtr parent, QString uid)
{
  TreeItemPtr retval(new TreeItemImage(parent,uid));
  if (parent.lock())
    parent.lock()->addChild(retval);
  return retval;
}

TreeItemImage::TreeItemImage(TreeItemWeakPtr parent, QString uid) :
    TreeItemImpl(parent,"","image",""),
    mUid(uid)
{
//  QObject::connect(ssc::dataManager(), SIGNAL(activeImageChanged(const QString&)), mModel, SLOT(treeItemChangedSlot()));
  connect(patientService().get(), SIGNAL(activeImageChanged(const QString&)), this, SIGNAL(changed()));
  connect(this, SIGNAL(changed()), mModel, SLOT(treeItemChangedSlot()));
}

QFont TreeItemImage::getFont() const
{
  QFont retval;
  QString image = patientService()->getActiveImageUid();
  if (image==mUid)
  {
    retval.setBold(true);
    retval.setItalic(true);
  }
  return retval;
}


TreeItemImage::~TreeItemImage()
{
	disconnect(patientService().get(), SIGNAL(activeImageChanged(const QString&)), this, SIGNAL(changed()));
  disconnect(this, SIGNAL(changed()), mModel, SLOT(treeItemChangedSlot()));
//  QObject::disconnect(ssc::dataManager(), SIGNAL(activeImageChanged(const QString&)), mModel, SLOT(treeItemChangedSlot()));
}
//void QAbstractItemModel::dataChanged ( const QModelIndex & topLeft, const QModelIndex & bottomRight )

QString TreeItemImage::getName() const
{
  if (!patientService()->getData<Image>(mUid))
    return "";
  return qstring_cast(patientService()->getData<Image>(mUid)->getName());
}

void TreeItemImage::activate()
{
  patientService()->setActiveData(mUid);
}

///--------------------------------------------------------
///--------------------------------------------------------
///--------------------------------------------------------

TreeItemPtr TreeItemToolManager::create(TreeItemWeakPtr parent)
{
  TreeItemPtr retval(new TreeItemToolManager(parent));
  if (parent.lock())
    parent.lock()->addChild(retval);

  TrackingService::ToolMap tools = trackingService()->getTools();
  for (TrackingService::ToolMap::iterator iter=tools.begin(); iter!=tools.end(); ++iter)
  {
    TreeItemTool::create(retval, iter->second);
  }

  return retval;
}

///--------------------------------------------------------
///--------------------------------------------------------
///--------------------------------------------------------

TreeItemPtr TreeItemTool::create(TreeItemWeakPtr parent, ToolPtr tool)
{
  TreeItemPtr retval(new TreeItemTool(parent, tool));
  if (parent.lock())
    parent.lock()->addChild(retval);
  return retval;
}

TreeItemTool::~TreeItemTool()
{
}

QString TreeItemTool::getName() const
{
  return qstring_cast(mTool->getName());
}


} // namespace cx
