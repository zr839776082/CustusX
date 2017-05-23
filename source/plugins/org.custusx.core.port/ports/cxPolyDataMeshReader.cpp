#include "cxPolyDataMeshReader.h"

#include <QFileInfo>
#include <vtkPolyDataReader.h>
#include <vtkPolyData.h>
#include <ctkPluginContext.h>
#include "cxMesh.h"
#include "cxErrorObserver.h"
#include "cxTypeConversions.h"

namespace cx
{

bool PolyDataMeshReader::readInto(DataPtr data, QString filename)
{
	return this->readInto(boost::dynamic_pointer_cast<Mesh>(data), filename);
}

bool PolyDataMeshReader::readInto(MeshPtr mesh, QString filename)
{
	if (!mesh)
		return false;
	vtkPolyDataPtr raw = this->loadVtkPolyData(filename);
	if(!raw)
		return false;
	mesh->setVtkPolyData(raw);
	return true;
}

vtkPolyDataPtr PolyDataMeshReader::loadVtkPolyData(QString fileName)
{
	vtkPolyDataReaderPtr reader = vtkPolyDataReaderPtr::New();
	reader->SetFileName(cstring_cast(fileName));

	if (!ErrorObserver::checkedRead(reader, fileName))
		return vtkPolyDataPtr();

	vtkPolyDataPtr polyData = reader->GetOutput();
	return polyData;
}

DataPtr PolyDataMeshReader::load(const QString& uid, const QString& filename)
{
	MeshPtr mesh(new Mesh(uid));
	this->readInto(mesh, filename);
	return mesh;
}

PolyDataMeshReader::PolyDataMeshReader(ctkPluginContext *context)
{
	this->setObjectName("PolyDataMeshReader");
}

bool PolyDataMeshReader::canLoad(const QString &type, const QString &filename)
{
	QString fileType = QFileInfo(filename).suffix();
	return ( fileType.compare("vtk", Qt::CaseInsensitive) == 0);
}


}
