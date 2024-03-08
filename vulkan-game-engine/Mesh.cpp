#include "Mesh.h"

/*
* CTOR / ASSIGNMENT DEFINITIONS
*/

Mesh::Mesh()
	: _vertices({}),
	_indices({})
{
}

Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<uint16_t> indices)
	: _vertices(vertices),
	_indices(indices)
{
}
