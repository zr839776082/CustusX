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
#include "cxTreeRepository.h"
#include "cxPatientModelService.h"
#include "cxDefinitions.h"
#include "cxData.h"
#include "cxTreeNode.h"
#include "cxDataTreeNode.h"
#include "cxSpaceTreeNode.h"
#include "cxGroupTreeNode.h"
#include "cxTopTreeNode.h"
#include "cxToolTreeNode.h"
#include "cxLogger.h"
#include "cxTrackingService.h"
#include "cxActiveData.h"
#include "cxVisServices.h"
#include "cxShowDataTreeNode.h"

namespace cx
{

void WidgetTypeRepository::add(QWidget *widget)
{
#include <QObject>
	mWidgets.push_back(widget);
}

//---------------------------------------------------------
//---------------------------------------------------------
//---------------------------------------------------------

TreeRepositoryPtr TreeRepository::create(XmlOptionFile options, VisServicesPtr services)
{
	TreeRepositoryPtr retval(new TreeRepository(options, services));
	retval->mSelf = retval;
	retval->insertTopNode();
	return retval;
}

TreeRepository::TreeRepository(XmlOptionFile options, VisServicesPtr services) :
	mInvalid(true),
	mServices(services),
	mOptions(options)
{
	mAllModes << "spaces" << "flat";
	mMode = this->getModeOption().readValue(mAllModes.front());

	mAllNodeTypes << "data" << "metric" << "image" << "model" << "tool";
	mVisibleNodeTypes = this->getVisibleNodeTypesOption().readValue(mAllNodeTypes.join(";")).split(";");

	mWidgetTypeRepository.reset(new WidgetTypeRepository());

	this->startListen();
}

TreeRepository::~TreeRepository()
{
	this->stopListen();
	this->getModeOption().writeValue(mMode);
	this->getVisibleNodeTypesOption().writeValue(mVisibleNodeTypes.join(";"));
}

void TreeRepository::update()
{
//	CX_LOG_CHANNEL_DEBUG("CA") << "TreeRepository::update(), invalid=" << mInvalid;
	if (mInvalid)
		this->rebuild();
	mInvalid = true;
}

void TreeRepository::invalidate()
{
//	CX_LOG_CHANNEL_DEBUG("CA") << "TreeRepository::invalidate()";
	mInvalid = true;
	emit invalidated();
}

void TreeRepository::startListen()
{
	connect(this->getServices()->patient().get(), SIGNAL(dataAddedOrRemoved()), this, SLOT(invalidate()));
	connect(this->getServices()->tracking().get(), &TrackingService::stateChanged, this, &TreeRepository::invalidate);
	connect(this->getServices()->patient()->getActiveData().get(), &ActiveData::activeDataChanged, this, &TreeRepository::changed);
}
void TreeRepository::stopListen()
{
	disconnect(this->getServices()->patient()->getActiveData().get(), &ActiveData::activeDataChanged, this, &TreeRepository::changed);
	disconnect(this->getServices()->tracking().get(), &TrackingService::stateChanged, this, &TreeRepository::invalidate);
	disconnect(this->getServices()->patient().get(), SIGNAL(dataAddedOrRemoved()), this, SLOT(invalidate()));
}

XmlOptionItem TreeRepository::getModeOption()
{
	return XmlOptionItem("mode", mOptions.getElement());
}

XmlOptionItem TreeRepository::getVisibleNodeTypesOption()
{
	return XmlOptionItem("visible_node_types", mOptions.getElement());
}

std::vector<TreeNodePtr> TreeRepository::getNodes()
{
	return mNodes;
}

TreeNodePtr TreeRepository::getNode(QString uid)
{
	for (unsigned i=0; i<mNodes.size(); ++i)
	{
		if (mNodes[i]->getUid()==uid)
			return mNodes[i];
	}
	return TreeNodePtr();
}

TreeNodePtr TreeRepository::getTopNode()
{
	std::vector<TreeNodePtr> all = this->getNodes();
	for (unsigned i=0; i<all.size(); ++i)
		if (all[i]->getUid() == "node::invisible_top")
			return all[i];
	CX_LOG_CHANNEL_DEBUG("CA") << "invalid tree - did not find top node";
	return TreeNodePtr();
}

VisServicesPtr TreeRepository::getServices()
{
	return mServices;
}

WidgetTypeRepositoryPtr TreeRepository::getWidgetTypeRepository()
{
	return mWidgetTypeRepository;
}

void TreeRepository::insertTopNode()
{
	TreeNodePtr topnode(new TopTreeNode(mSelf));
	mNodes.push_back(topnode);
//	CX_LOG_CHANNEL_DEBUG("CA") << "  - built topnode";
}

void TreeRepository::rebuild()
{
	mNodes.clear();
	this->insertTopNode();

	QStringList groups = QStringList() << "tool" << "data" << "space";
	for (unsigned i=0; i<groups.size(); ++i)
		this->insertGroupNode(groups[i]);

	this->insertSpaceNode(CoordinateSystem(csREF));
//	CX_LOG_CHANNEL_DEBUG("CA") << "  - built refspacenode";

	std::map<QString, DataPtr> source = this->getServices()->patient()->getData();
	for (std::map<QString, DataPtr>::const_iterator iter = source.begin(); iter != source.end(); ++iter)
	{
		this->insertDataNode(iter->second);
	}
//	CX_LOG_CHANNEL_DEBUG("CA") << "  - built datanodes";
	for (std::map<QString, DataPtr>::const_iterator iter = source.begin(); iter != source.end(); ++iter)
	{
		QString space = iter->second->getParentSpace();
		if (space.isEmpty())
			continue;
		if (source.count(space))
			continue;
		this->insertSpaceNode(CoordinateSystem(csDATA, space));
	}
//	CX_LOG_CHANNEL_DEBUG("CA") << "  - built spacenodes";

	this->insertSpaceNode(CoordinateSystem(csPATIENTREF));

	std::map<QString, ToolPtr> tools = this->getServices()->tracking()->getTools();
	for (std::map<QString, ToolPtr>::const_iterator iter = tools.begin(); iter != tools.end(); ++iter)
	{
		this->insertToolNode(iter->second);
	}

//	CX_LOG_CHANNEL_DEBUG("CA") << "  - built all nodes";
}

void TreeRepository::insertGroupNode(QString groupname)
{
	if (this->getNodeForGroup(groupname))
		return;
	this->appendNode(new GroupTreeNode(mSelf, groupname));
}

TreeNodePtr TreeRepository::getNodeForGroup(QString groupname)
{
	return this->getNode(QString("group_%1").arg(groupname));
}

void TreeRepository::insertToolNode(ToolPtr tool)
{
	if (this->getNode(tool->getUid()))
		return;
	this->appendNode(new ToolTreeNode(mSelf, tool));
}

void TreeRepository::insertDataNode(DataPtr data)
{
	if (this->getNode(data->getUid()))
		return;

	this->appendNode(new DataTreeNode(mSelf, data));

	this->appendNode(new ShowVolumeDataTreeNode(mSelf, data));
	this->appendNode(new ShowSlice2DDataTreeNode(mSelf, data));
	this->appendNode(new ShowSlice3DDataTreeNode(mSelf, data));
}

void TreeRepository::appendNode(TreeNode* rawNode)
{
	TreeNodePtr node(rawNode);
	mNodes.push_back(node);
}

void TreeRepository::insertSpaceNode(CoordinateSystem space)
{
	if (this->getNode(space.toString()) || this->getNode(space.mRefObject))
		return;
	this->appendNode(new SpaceTreeNode(mSelf, space));
}


} // namespace cx
