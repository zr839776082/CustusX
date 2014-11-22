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

#include "cxWorkflowStateMachine.h"
#include <QAbstractTransition>
#include <QMenu>
#include <QToolBar>
#include "cxWorkflowState.h"
#include "cxRequestEnterStateTransition.h"
#include "cxStateServiceBackend.h"
#include "cxPatientModelService.h"

namespace cx
{

WorkflowStateMachine::WorkflowStateMachine(StateServiceBackendPtr backend) : mBackend(backend)
{
	mStarted = false;
	connect(this, SIGNAL(started()), this, SLOT(startedSlot()));
	mActionGroup = new QActionGroup(this);

	mParentState = new ParentWorkflowState(this, mBackend);

	WorkflowState* patientData = this->newState(new PatientDataWorkflowState(mParentState, mBackend));
	this->newState(new RegistrationWorkflowState(mParentState, mBackend));
	this->newState(new PreOpPlanningWorkflowState(mParentState, mBackend));
	this->newState(new NavigationWorkflowState(mParentState, mBackend));
	this->newState(new IntraOpImagingWorkflowState(mParentState, mBackend));
	this->newState(new PostOpControllWorkflowState(mParentState, mBackend));

	//set initial state on all levels
	this->setInitialState(mParentState);
	mParentState->setInitialState(patientData);

	connect(mBackend->getPatientService().get(), SIGNAL(clinicalApplicationChanged()), this, SLOT(clinicalApplicationChangedSlot()));
}

void WorkflowStateMachine::clinicalApplicationChangedSlot()
{
	this->setActiveState("PatientDataUid");
}

void WorkflowStateMachine::startedSlot()
{
	mStarted = true;
}

WorkflowState* WorkflowStateMachine::newState(WorkflowState* state)
{
	RequestEnterStateTransition* transToState = new RequestEnterStateTransition(state->getUid());
	transToState->setTargetState(state);
	mParentState->addTransition(transToState);

	connect(state, SIGNAL(entered()), this, SIGNAL(activeStateChanged()));

	mStates[state->getUid()] = state;

	connect(state, SIGNAL(aboutToExit()), this, SIGNAL(activeStateAboutToChange()));

	return state;
}

WorkflowStateMachine::~WorkflowStateMachine()
{
}

QActionGroup* WorkflowStateMachine::getActionGroup()
{
	mActionGroup->setExclusive(true);
	//TODO rebuild action list when we need dynamic lists. Must rethink memory management then.
	for (WorkflowStateMap::iterator iter = mStates.begin(); iter != mStates.end(); ++iter)
	{
		iter->second->createAction(mActionGroup);
	}

	return mActionGroup;
}

void WorkflowStateMachine::fillMenu(QMenu* menu)
{
	this->fillMenu(menu, mParentState);
}

void WorkflowStateMachine::fillMenu(QMenu* menu, WorkflowState* current)
{
	std::vector<WorkflowState*> childStates = current->getChildStates();

	if (childStates.empty())
	{
		menu->addAction(current->createAction(mActionGroup));
	}
	else // current is a node. create submenu and fill in recursively
	{
		QMenu* submenu = menu;
		if (current != mParentState) // ignore creation of submenu for parent state
			submenu = menu->addMenu(current->getName());
		for (unsigned i = 0; i < childStates.size(); ++i)
			this->fillMenu(submenu, childStates[i]);
	}
}

void WorkflowStateMachine::setActiveState(QString uid)
{

	if (mStarted)
		this->postEvent(new RequestEnterStateEvent(uid));
}

void WorkflowStateMachine::fillToolBar(QToolBar* toolbar)
{
	this->fillToolbar(toolbar, mParentState);
}

void WorkflowStateMachine::fillToolbar(QToolBar* toolbar, WorkflowState* current)
{
	std::vector<WorkflowState*> childStates = current->getChildStates();

	if (childStates.empty())
	{
		toolbar->addAction(current->createAction(mActionGroup));
	}
	else // current is a node. fill in recursively
	{
		for (unsigned i = 0; i < childStates.size(); ++i)
			this->fillToolbar(toolbar, childStates[i]);
	}
}

QString WorkflowStateMachine::getActiveUidState()
{
	QSet<QAbstractState *> states = this->configuration();
	for (QSet<QAbstractState *>::iterator iter = states.begin(); iter != states.end(); ++iter)
	{
		WorkflowState* wfs = dynamic_cast<WorkflowState*>(*iter);
		if (!wfs || !wfs->getChildStates().empty())
			continue;

		return wfs->getUid();
	}
	return QString();
}

} //namespace cx
