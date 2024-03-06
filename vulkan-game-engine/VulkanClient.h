#pragma once

#include <vector>
#include <string>
#include <vulkan/vulkan.h>
#include <optional>
#include <mutex>

#include "Window.h"
#include "VulkanDevice.h"
#include "SwapChain.h"
#include "GraphicsPipeline.h"
#include "CommandPool.h"

/*
* Class describing a client for rendering windows
*/
class VulkanClient
{
public:

	/*
	* CTORS
	*/

	VulkanClient();
	~VulkanClient();



	/*
	* PUBLIC METHODS
	*/

	/* @brief Adds a window to be displayed
	* @param title Window title
	* @param width Window width in pixels
	* @param height Window height in pixels
	*/
	void add_window(const char* title, uint32_t width, uint32_t height);

	/* @brief Loads a shader to be used for rendering
	* @param filepath The path pointing to the shader file
	* @param shaderType The type of shader being loaded
	*/
	void add_shader(const std::string& filepath, Shader::Type shaderType);

	/* @brief Initializes the client internals. Must be called before running
	* 
	* @param deviceExtensions List of device extensions to support
	*/
	void init(const std::vector<const char*>& deviceExtensions = {});

	/* @brief Runs the client. Client must be initialized before running
	*/
	void run();

private:

	/*
	* PRIVATE MEMBERS
	*/

	/* Device used for processing
	*/
	VulkanDevice _device;

	std::mutex mutex;
	
	/* Swap chain used for drawing
	*/
	std::vector<SwapChain> _swapChains;

	/* List of windows to draw
	*/
	std::vector<Window> _windows;

	/* List of shader files and types
	*/
	std::vector < std::pair < std::string, Shader::Type> > _shaderFiles;

	/* Graphics pipeline
	*/
	std::vector<GraphicsPipeline> _pipelines;

	/* Command pool used for drawing
	*/
	std::vector<CommandPool> _commandPools;



	/*
	* PRIVATE CONST METHODS
	*/

	/* @brief Returns the number of window surface that are compatible with the given device
	*/
	int _num_surfaces_compatible(VkPhysicalDevice physicalDevice) const;

	/* @brief Checks if the given device supports the given extensions
	*/
	bool _device_supports_extensions(VkPhysicalDevice physicalDevice, const std::vector<const char*>& deviceExtensions) const;

	/* @brief Checks if the given device supports swap chains with all surfaces
	*/
	bool _device_supports_swap_chain(VkPhysicalDevice physicalDevice) const;

	/* @brief Selects a physical device that meets all requirements
	*
	* @param deviceExtensions List of extensions the device must support
	*/
	VkPhysicalDevice _pick_physical_device(const std::vector<const char*>& deviceExtensions) const;



	/*
	* PRIVATE METHODS
	*/

	/* @brief Creates a logical device after choosing a physical device
	*/
	void _create_logical_device(const std::vector<const char*>& deviceExtensions);

	/* @brief Creates the swap chain
	*/
	void _create_swap_chains();

	/* @brief Creates the graphics pipeline
	*/
	void _create_graphics_pipelines();

	/* @brief Creates the command pool
	*/
	void _create_command_pools();

	/* @brief Renders a frame for the given window
	*/
	void _render_frames(Window& window, SwapChain& swapChain, GraphicsPipeline& pipeline, CommandPool& cmdPool);

	/* Destroys and recreates swap chain
	*/
	void _recreate_swap_chain(Window& window, SwapChain& swapChain, GraphicsPipeline& pipeline);
};

