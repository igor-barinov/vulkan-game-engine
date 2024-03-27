#include "Model3D.h"

void Model3D::from_obj(const std::string& objFilepath)
{
	_mesh = ObjFile(objFilepath).get_mesh();
}