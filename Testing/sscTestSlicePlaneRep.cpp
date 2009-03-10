#include "sscTestSlicePlaneRep.h"

#include <iostream>
#include <string>
#include <vector>

//#include <vtkPolyData.h>
#include <vtkImageData.h>
#include <vtkMetaImageReader.h>
#include <vtkImagePlaneWidget.h>
#include <vtkRenderer.h>
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"


#include "sscTestUtilities.h"
#include "sscDataManager.h"
#include "sscImage.h"
//#include "sscMesh.h"
//#include "sscGeometricRep.h"
#include "sscAxesRep.h"
#include "sscSliceRep.h"

#include "sscVolumetricRep.h"
#include "sscSliceComputer.h"
#include "sscVector3D.h"
#include "sscTransform3D.h"
#include "sscToolRep3D.h"
#include "sscDummyToolManager.h"
#include "sscDummyTool.h"
#include "sscSliceProxy.h"
#include "sscSlicerRepSW.h"
#include "sscSlicePlaneRep.h"

using ssc::Vector3D;
using ssc::Transform3D;

int main(int argc, char **argv)
{
	QApplication app(argc, argv);
	TestSlicePlaneRep test;
	int val = app.exec();
	return val;
}

TestSlicePlaneRep::TestSlicePlaneRep()
{
	this->resize(QSize(800, 600));
	this->show();
	mWidget = new QWidget;
	this->setCentralWidget( mWidget );
	start();
}
TestSlicePlaneRep::~TestSlicePlaneRep()
{}
void TestSlicePlaneRep::generateView(const std::string& uid)
{
	mLayouts[uid].mView = new ssc::View();
}

void TestSlicePlaneRep::generateSlice(const std::string& uid, ssc::ToolPtr tool, ssc::ImagePtr image, ssc::PLANE_TYPE plane)
{
	
	
	ssc::SliceProxyPtr proxy(new ssc::SliceProxy());
	proxy->setTool(tool);
	proxy->setFollowType(ssc::ftFIXED_CENTER);
	proxy->setOrientation(ssc::otORTHOGONAL);
	proxy->setPlane(plane);

	ssc::SliceRepSWPtr rep = ssc::SliceRepSW::New(uid);
	rep->setImage(image);
	rep->setSliceProxy(proxy);
	rep->update();
	
	mLayouts[uid].mSlicer = proxy;
	mLayouts[uid].mSliceRep = rep;
	mLayouts[uid].mView->addRep(rep);
}

ssc::View* TestSlicePlaneRep::view(const std::string& uid)
{
	return mLayouts[uid].mView;
}

void TestSlicePlaneRep::start()
{
	QVBoxLayout *mainLayout = new QVBoxLayout;
	mWidget->setLayout(mainLayout);

	// generate imageFileName
	//std::string imageFileName1 = ssc::TestUtilities::ExpandDataFileName("Fantomer/Kaisa/MetaImage/Kaisa.mhd");
	std::string imageFileName1 = ssc::TestUtilities::ExpandDataFileName("MetaImage/20070309T105136_MRT1.mhd");
	//std::string imageFileName1 = ssc::TestUtilities::ExpandDataFileName("MetaImage/20070309T102309_MRA.mhd");
	std::cout << imageFileName1 << std::endl;

	// read image
	ssc::ImagePtr image1 = ssc::DataManager::getInstance()->loadImage(imageFileName1, ssc::rtMETAIMAGE);
	std::cout<<" count components"<<image1->getBaseVtkImageData()->GetNumberOfScalarComponents()<<std::endl;
	
	
	generateView("A");
	generateView("C");
	generateView("S");
	generateView("3D");
	// Slices
	
	// volume rep
//	ssc::VolumetricRepPtr mRepPtr = ssc::VolumetricRep::New( image1->getUid() );
//	mRepPtr->setImage(image1);
//	mRepPtr->setName(image1->getName());
//	view("3D")->addRep(mRepPtr);
	
	

	//ssc::AxesRepPtr axesRep = ssc::AxesRep::New("AxesRepUID");
	//view->addRep(axesRep);

	// Initialize dummy toolmanager.
	ssc::ToolManager* mToolmanager = ssc::DummyToolManager::getInstance();
	mToolmanager->configure();
	mToolmanager->initialize();
	mToolmanager->startTracking();

	ssc::ToolPtr tool = mToolmanager->getDominantTool();
	connect( tool.get(), SIGNAL( toolTransformAndTimestamp(Transform3D ,double) ), this, SLOT( updateRender()));
	generateSlice("A", tool, image1, ssc::ptAXIAL);
	generateSlice("C", tool, image1, ssc::ptCORONAL);
	generateSlice("S", tool, image1, ssc::ptSAGITTAL);
	//Plane rep
	ssc::SlicePlaneRepPtr mPlaneRep = ssc::SlicePlaneRep::New(image1->getUid());
	std::vector<ssc::SliceRepSWPtr> tmp;
	for (LayoutMap::iterator iter=mLayouts.begin(); iter!=mLayouts.end(); ++iter)
	{	
		std::cout<<"add sliceRep"<<std::endl;	
		if(iter->first == "3D")
		{
			continue;
		}
		tmp.push_back(iter->second.mSliceRep);
		
	}
	mPlaneRep->setSliceReps(tmp);
		//mPlaneRep->setImage(image1);
	view("3D")->addRep(mPlaneRep);
	// Tool 3D rep
//	ssc::ToolRep3DPtr toolRep = ssc::ToolRep3D::New( tool->getUid(), tool->getName() );
//	toolRep->setTool(tool);
//	view("3D")->addRep(toolRep);

	

	QGridLayout* sliceLayout = new QGridLayout;
	QHBoxLayout *buttonLayout = new QHBoxLayout;

	mainLayout->addLayout(sliceLayout);//Slice layout
	sliceLayout->addWidget(view("C"), 0, 0);
	sliceLayout->addWidget(view("S"), 0, 1);
	sliceLayout->addWidget(view("A"), 1, 0);
	sliceLayout->addWidget(view("3D"), 1, 1);
	
	
	mainLayout->addLayout(buttonLayout); //Buttons
	for (LayoutMap::iterator iter=mLayouts.begin(); iter!=mLayouts.end(); ++iter)
		iter->second.mView->getRenderer()->ResetCamera();
	updateRender();
}

void TestSlicePlaneRep::updateRender()
{
//	std::cout<<"render begin"<<std::endl;
	for (LayoutMap::iterator iter=mLayouts.begin(); iter!=mLayouts.end(); ++iter)
		iter->second.mView->getRenderWindow()->Render();
//	std::cout<<"render end"<<std::endl;
}


