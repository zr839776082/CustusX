#ifndef SSCMESH_
#define SSCMESH_

#include <set>

#include <boost/shared_ptr.hpp>

#include "vtkSmartPointer.h"
typedef vtkSmartPointer<class vtkPolyData> vtkPolyDataPtr;

#include "sscData.h"
#include "sscRep.h"

namespace ssc
{

/**Defines a geometric structure in space..?
 * 
 */
class Mesh : public Data
{
public:
	Mesh();
	Mesh(const std::string& uid, const std::string& name, const vtkPolyDataPtr& polyData);
	virtual ~Mesh();
	
//	void setUID(const std::string& uid);
//	void setName(const std::string& name);
	void setVtkPolyData(const vtkPolyDataPtr& polyData);

//	virtual std::string getUID() const;
//	virtual std::string getName() const;
	virtual vtkPolyDataPtr getVtkPolyData();
	
//	void connectRep(const RepWeakPtr& rep); ///< called by Rep when connecting to an Image
//	void disconnectRep(const RepWeakPtr& rep); ///< called by Rep when disconnecting from an Image

private:
//	std::string mUID;
//	std::string mName;

	vtkPolyDataPtr mVtkPolyData;

//	std::set<RepWeakPtr> mReps; ///< links to Rep users.

};

typedef boost::shared_ptr<Mesh> MeshPtr;

} // namespace ssc

#endif /*SSCMESH_*/
