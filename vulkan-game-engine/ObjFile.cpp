#define TINYOBJLOADER_IMPLEMENTATION

#include "ObjFile.h"

#include <stdexcept>
#include <unordered_map>

ObjFile::ObjFile()
{
}

ObjFile::ObjFile(const std::string& filepath)
{
	read(filepath);
}


void ObjFile::read(const std::string& filepath)
{
	std::string error;

	if (!tinyobj::LoadObj(&_attrib, &_shapes, &_materials, &error, filepath.c_str()))
	{
		throw std::runtime_error(error);
	}
}

Mesh ObjFile::get_mesh() const
{
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    std::unordered_map<Vertex, uint32_t> uniqueVertices{};

    for (const auto& shape : _shapes) 
    {
        for (const auto& index : shape.mesh.indices) 
        {
            std::array<float, 3> pos = {
                _attrib.vertices[3 * index.vertex_index + 0],
                _attrib.vertices[3 * index.vertex_index + 1],
                _attrib.vertices[3 * index.vertex_index + 2]
            };

            std::array<float, 3> color = { 1.0f, 1.0f, 1.0f };

            std::array<float, 2> texCoords = {
                _attrib.texcoords[2 * index.texcoord_index + 0],
                1.0f - _attrib.texcoords[2 * index.texcoord_index + 1]
            };

            Vertex vertex(pos, color, texCoords);

            if (uniqueVertices.count(vertex) == 0) 
            {
                uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                vertices.push_back(vertex);
            }

            indices.push_back(uniqueVertices[vertex]);
        }
    }

    return Mesh(vertices, indices);
}
