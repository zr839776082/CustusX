#ifndef SSCTESTSLICEPLANEREP_H_
#define SSCTESTSLICEPLANEREP_H_
#include <QtGui>
#include <map>
#include "sscView.h"

#include "sscSliceProxy.h"
#include "sscSlicerRepSW.h"


class SingleLayout
{
public:
	ssc::View* mView;
	ssc::SliceProxyPtr mSlicer;
	ssc::SliceRepSWPtr mSliceRep;
};

class TestSlicePlaneRep : public QMainWindow
{
	Q_OBJECT
public:
	TestSlicePlaneRep();
	virtual ~TestSlicePlaneRep();
private:
	QWidget* mWidget;
	void start();
	ssc::View* view(const std::string& uid);
	void generateSlice(const std::string& uid, ssc::ToolPtr tool, ssc::ImagePtr image, ssc::PLANE_TYPE plane);
	void generateView(const std::string& uid);

	typedef std::map<std::string, SingleLayout> LayoutMap;
	LayoutMap mLayouts;	
	LayoutMap::iterator iter; 

private slots:
	void updateRender();
};


#endif


