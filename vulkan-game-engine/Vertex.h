#pragma once

#include <vulkan/vulkan.h>
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
	Vertex(float x, float y, float r, float g, float b);



	/*
	* PUBLIC STATIC METHODS
	*/

	/* @brief Returns binding description used for vertex input pipeline stage
	*/
	static VkVertexInputBindingDescription getBindingDescription();

	/* @brief Returns attribute descriptions used for vertex input pipeline stage
	*/
	static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions();

private:

	/*
	* PRIVATE MEMBERS
	*/

	/* Position vector
	*/
	glm::vec2 _pos;

	/* Color vector
	*/
	glm::vec3 _color;
};

