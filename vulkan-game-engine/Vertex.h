#pragma once

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

#include <array>

class Vertex
{
public:

	Vertex(float x, float y, float r, float g, float b);

	static VkVertexInputBindingDescription getBindingDescription();
	static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions();

private:
	glm::vec2 _pos;
	glm::vec3 _color;
};

