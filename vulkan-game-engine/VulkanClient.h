#pragma once

#include <vector>
#include <string>
#include <vulkan/vulkan.h>
#include <optional>
#include <mutex>
#include <future>

#include "Window.h"
#include "VulkanDevice.h"
#include "SwapChain.h"
#include "GraphicsPipeline.h"
#include "CommandPool.h"
#include "Buffer.h"
#include "Mesh.h"

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

	/* @brief Stops the client
	*/
	void stop();

private:

	/*
	* PRIVATE MEMBERS
	*/

	/* Device used for processing
	*/
	VulkanDevice _device;
	
	/* Swap chain used for drawing
	*/
	std::vector<SwapChain> _swapChains;

	/* List of windows to draw
	*/
	std::vector<Window> _windows;

	/* List of shader files and types
	*/
	std::vector < std::pair < std::string, Shader::Type> > _shaderFiles;

	/* Graphics pipelines
	*/
	std::vector<GraphicsPipeline> _pipelines;

	/* Command pools used for drawing
	*/
	std::vector<CommandPool> _commandPools;

	/* List of staging buffers
	*/
	std::vector<Buffer> _stagingBuffers;

	/* List of vertex buffers
	*/
	std::vector<Buffer> _vertexBuffers;

	/* List of index buffers
	*/
	std::vector<Buffer> _indexBuffers;

	/* List of futures for window renders
	*/
	std::vector<std::future<void>> _windowFutures;

	/* Mutex for locking down queue handles
	*/
	std::mutex queueMtx;

	/* List of meshes
	*/
	std::vector<Mesh> _meshes;



	/*
	* PRIVATE CONST METHODS
	*/

	/* @brief Checks if the given device is compatible with all surfaces
	*/
	bool _device_compatible_with_surfaces(VkPhysicalDevice physicalDevice) const;

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

	/* @brief Creates staging, vertex, and index buffers
	*/
	void _create_buffers();

	/* @brief Renders a frame for the given window
	*/
	void _render_frames(
		Window& window,
		SwapChain& swapChain,
		GraphicsPipeline& pipeline,
		CommandPool& cmdPool,
		Buffer& vertexBuffer,
		Buffer& indexBuffer
	);

	/* @brief Destroys and recreates swap chain
	*/
	void _recreate_swap_chain(Window& window, SwapChain& swapChain, GraphicsPipeline& pipeline);
};

