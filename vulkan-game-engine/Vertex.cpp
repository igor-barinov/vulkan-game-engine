#include "Vertex.h"

/*
* STATIC METHOD DEFINITIONS
*/

VkVertexInputBindingDescription Vertex::getBindingDescription()
{
    VkVertexInputBindingDescription bindingDescription{};
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(Vertex);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    return bindingDescription;
}

std::array<VkVertexInputAttributeDescription, 3> Vertex::getAttributeDescriptions()
{
    std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};

    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[0].offset = offsetof(Vertex, _pos);

    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[1].offset = offsetof(Vertex, _color);

    attributeDescriptions[2].binding = 0;
    attributeDescriptions[2].location = 2;
    attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[2].offset = offsetof(Vertex, _texCoord);

    return attributeDescriptions;
}





/*
* CTOR / ASSIGNMENT DEFINITIONS
*/

Vertex::Vertex(float x, float y, float z, float r, float g, float b, float textureX, float textureY)
    : _pos(x, y, z),
    _color(r, g, b),
    _texCoord(textureX, textureY)
{
}

Vertex::Vertex(std::array<float, 3> position, std::array<float, 3> color, std::array<float, 2> textureCoords)
    : Vertex(position[0], position[1], position[2], color[0], color[1], color[2], textureCoords[0], textureCoords[1])
{
}

void Vertex::_rotate(float degrees, int axis)
{
    glm::vec3 axisVec(0);
    switch (axis)
    {
    case 0:
        axisVec = glm::vec3(1, 0, 0);
        break;
    case 1:
        axisVec = glm::vec3(0, 1, 0);
        break;
    case 2:
        axisVec = glm::vec3(0, 0, 1);
        break;
    }

    auto rotMat = glm::rotate(glm::mat4(1), glm::radians(degrees), axisVec);
    _pos = glm::vec3(rotMat * glm::vec4(_pos, 1.0));
}