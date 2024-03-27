#include "Mesh.h"

#include <algorithm>

/*
* CTOR / ASSIGNMENT DEFINITIONS
*/

Mesh::Mesh()
	: _vertices({}),
	_indices({})
{
}

Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t> indices)
	: _vertices(vertices),
	_indices(indices)
{
}

void Mesh::scale(float scalar)
{
	for (auto& vert : _vertices)
	{
		vert.scale(scalar);
	}
}

void Mesh::rotate_x(float degrees)
{
	for (auto& vert : _vertices)
	{
		vert.rotate_x(degrees);
	}
}

void Mesh::rotate_y(float degrees)
{
	for (auto& vert : _vertices)
	{
		vert.rotate_y(degrees);
	}
}

void Mesh::rotate_z(float degrees)
{
	for (auto& vert : _vertices)
	{
		vert.rotate_z(degrees);
	}
}