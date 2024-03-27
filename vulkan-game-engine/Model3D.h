#pragma once

#include "Mesh.h"
#include "Texture.h"
#include "ObjFile.h"

class Model3D
{
public:

	void from_obj(const std::string& objFilepath);
	inline void set_texture(const Texture& texture) { _texture = texture; }
	inline const Texture& get_texture() const { return _texture; }
	inline const Mesh& get_mesh() const { return _mesh; }

	inline void scale(float scalar) { _mesh.scale(scalar); }
	inline void rotate_x(float degrees) { _mesh.rotate_x(degrees); }
	inline void rotate_y(float degrees) { _mesh.rotate_y(degrees); }
	inline void rotate_z(float degrees) { _mesh.rotate_z(degrees); }

private:
	Mesh _mesh;
	Texture _texture;
};

