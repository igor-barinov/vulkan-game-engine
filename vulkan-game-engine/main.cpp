#define GLFW_INCLUDE_VULKAN
#include <glfw/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include <iostream>

#include "VulkanClient.h"
#include "VulkanInstance.h"

int main(int argc, char* argv[])
{

    VulkanInstance& vulkan = VulkanInstance::instance();
    VulkanClient client;

    client.add_window("Game Engine", 1920, 1080);
    /**
    for (int i = 0; i < 1; i++)
    {
        client.add_window("Small", 800, 600);
    }
    //*/
    client.add_shader("vert.spv", Shader::VERTEX);
    client.add_shader("frag.spv", Shader::FRAGMENT);
    client.init({ VK_KHR_SWAPCHAIN_EXTENSION_NAME });
    client.run();

	return 0;
}