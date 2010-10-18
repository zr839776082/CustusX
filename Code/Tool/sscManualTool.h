#ifndef SSCMANUALTOOL_H_
#define SSCMANUALTOOL_H_

#include <QMutex>
#include "sscTool.h"
typedef vtkSmartPointer<class vtkSTLReader> vtkSTLReaderPtr;
typedef vtkSmartPointer<class vtkCursor3D> vtkCursor3DPtr;

namespace ssc
{

/**A manual tool that is unconnected to any hardware.
 * Use this tool by setting the functions setTransform() and setVisible().
 *
 * This class is Threadsafe.
 */
class ManualTool : public ssc::Tool
{
	Q_OBJECT
public:

	explicit ManualTool(const QString& uid, const QString& name ="");
	virtual ~ManualTool();
	virtual Type getType() const;
	virtual QString getGraphicsFileName() const;
	virtual vtkPolyDataPtr getGraphicsPolyData() const;
	virtual void saveTransformsAndTimestamps(){};
	virtual void setTransformSaveFile(const QString& filename){};
	virtual Transform3D get_prMt() const;
	virtual bool getVisible() const;
	virtual QString getUid() const;
	virtual QString getName() const;
	virtual int getIndex() const;
	virtual bool isCalibrated() const;
	virtual ssc::ProbeSector getProbeSector() const;
	virtual double getTimestamp() const;
	virtual void set_prMt(const Transform3D& prMt);

	virtual double getTooltipOffset() const;
	virtual void setTooltipOffset(double val);

	virtual Transform3D getCalibration_sMt() const;
	// extensions:
	void setVisible(bool vis);
	void setType(const Type& type);

private:
	void read3DCrossHair();
	// constant data
	//const QString mUid; //see sscTool
	//const QString mName; //see sscTool
	vtkPolyDataPtr mPolyData;
	vtkSTLReaderPtr mSTLReader;
	vtkCursor3DPtr mCrossHair;
	double mOffset;

	// mutex-protected mutable data
	Type mType;
	bool mVisible;
	Transform3D m_prMt;
	mutable QMutex mMutex;

//	void createPolyData();
};

typedef boost::shared_ptr<ManualTool> ManualToolPtr;

}// end namespace

#endif /*SSCMANULATOOL_H_*/
