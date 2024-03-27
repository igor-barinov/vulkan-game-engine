#pragma once


// #define TINYOBJLOADER_IMPLEMENTATION
#include <tinyobj/tiny_obj_loader.h>




#include <string>

#include "Mesh.h"

class ObjFile
{
public:

	ObjFile();
	ObjFile(const std::string& filepath);

	void read(const std::string& filepath);

	Mesh get_mesh() const;

private:
	tinyobj::attrib_t _attrib;
	std::vector<tinyobj::shape_t> _shapes;
	std::vector<tinyobj::material_t> _materials;
};

