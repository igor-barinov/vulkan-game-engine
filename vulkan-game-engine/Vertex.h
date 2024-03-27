#pragma once

#include <vulkan/vulkan.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>
#include <glm/glm.hpp>

#include <array>

/*
* Class describing a mesh vertex
*/
class Vertex
{
public:

	/*
	* CTORS / ASSIGNMENT
	*/

	/*
	* @param x x-coordinate
	* @param y y-coordinate
	* @param r Red color value
	* @param g Green color value
	* @param b Blue color value
	*/
	Vertex(float x, float y, float z, float r, float g, float b, float textureX, float textureY);

	Vertex(std::array<float, 3> position, std::array<float, 3> color, std::array<float, 2> textureCoords);


	bool operator==(const Vertex& other) const {
		return _pos == other._pos && _color == other._color && _texCoord == other._texCoord;
	}

	inline void scale(float scalar)	{ _pos *= scalar; }

	inline void rotate_x(float degrees) { _rotate(degrees, 0); }
	inline void rotate_y(float degrees) { _rotate(degrees, 1); }
	inline void rotate_z(float degrees) { _rotate(degrees, 2); }


	/*
	* PUBLIC STATIC METHODS
	*/

	/* @brief Returns binding description used for vertex input pipeline stage
	*/
	static VkVertexInputBindingDescription getBindingDescription();

	/* @brief Returns attribute descriptions used for vertex input pipeline stage
	*/
	static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions();

private:

	friend struct std::hash<Vertex>;

	/*
	* PRIVATE MEMBERS
	*/

	/* Position vector
	*/
	glm::vec3 _pos;

	/* Color vector
	*/
	glm::vec3 _color;

	/* Texture coordinates vector
	*/
	glm::vec2 _texCoord;

	void _rotate(float degrees, int axis);
};

namespace std 
{
	template<> struct hash<Vertex> 
	{
		size_t operator()(Vertex const& vertex) const 
		{
			return ((hash<glm::vec3>()(vertex._pos) ^
				(hash<glm::vec3>()(vertex._color) << 1)) >> 1) ^
				(hash<glm::vec2>()(vertex._texCoord) << 1);
		}
	};
}