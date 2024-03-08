#pragma once

#include <vector>

#include "Vertex.h"

/*
* Class describing a mesh in 3D space
*/
class Mesh
{
public:

	/*
	* CTORS / ASSIGNMENT
	*/

	Mesh();

	/*
	* @param vertices List of vertices
	* @param indices List of vertex indexes
	*/
	Mesh(const std::vector<Vertex>& vertices, const std::vector<uint16_t> indices);



	/*
	* PUBLIC CONST METHODS
	*/

	/* @brief Returns list of vertices
	*/
	inline const std::vector<Vertex>& vertices() const { return _vertices; }

	/* @brief Returns list of indices
	*/
	inline const std::vector<uint16_t>& indices() const { return _indices; }

	/* @brief Returns size in bytes of vertices
	*/
	inline size_t size_of_vertices() const { return sizeof(_vertices[0]) * _vertices.size(); }

	/* @brief Returns size in bytes of indices
	*/
	inline size_t size_of_indices() const { return sizeof(_indices[0]) * _indices.size(); }

	/* @brief Returns pointer to vertex data
	*/
	inline const Vertex* vertex_data() const { return _vertices.data(); }

	/* @brief Returns pointer to index data
	*/
	inline const uint16_t* index_data() const { return _indices.data(); }

private:

	/*
	* PRIVATE MEMBERS
	*/
	std::vector<Vertex> _vertices;
	std::vector<uint16_t> _indices;
};

