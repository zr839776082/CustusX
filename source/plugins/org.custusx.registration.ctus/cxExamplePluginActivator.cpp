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

#include "cxExamplePluginActivator.h"

#include <QtPlugin>
#include <iostream>

#include "cxExampleGUIExtenderService.h"

namespace cx
{

ExamplePluginActivator::ExamplePluginActivator()
: mContext(0)
{
	std::cout << "Created ExamplePluginActivator" << std::endl;
}

ExamplePluginActivator::~ExamplePluginActivator()
{

}

void ExamplePluginActivator::start(ctkPluginContext* context)
{
	std::cout << "Started ExamplePluginActivator" << std::endl;
	this->mContext = context;

	mPlugin.reset(new ExampleGUIExtenderService);
	std::cout << "created example service" << std::endl;
	try
	{
		mRegistration = context->registerService(QStringList(GUIExtenderService_iid), mPlugin.get());
	}
	catch(ctkRuntimeException& e)
	{
		std::cout << e.what() << std::endl;
		mPlugin.reset();
	}
	std::cout << "registered example service" << std::endl;
}

void ExamplePluginActivator::stop(ctkPluginContext* context)
{
	mRegistration.unregister();
	if(mPlugin)
		mPlugin.reset();
	std::cout << "Stopped ExamplePluginActivator" << std::endl;
	Q_UNUSED(context)
}

} // namespace cx

Q_EXPORT_PLUGIN2(ExamplePluginActivator_irrelevant_string, cx::ExamplePluginActivator)

