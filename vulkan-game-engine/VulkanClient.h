#pragma once

#include <vector>
#include <string>
#include <vulkan/vulkan.h>
#include <optional>
#include <mutex>
#include <future>

#include "Window.h"
#include "VulkanRenderer.h"
#include "Mesh.h"
#include "Texture.h"

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

	static constexpr size_t _NUM_FRAMES_IN_FLIGHT = 2;

	/*
	* PRIVATE MEMBERS
	*/

	/* Device used for processing
	*/
	Device _device;

	/* List of windows to draw
	*/
	std::vector<Window> _windows;

	/* List of shader files and types
	*/
	std::vector < std::pair < std::string, Shader::Type> > _shaderFiles;

	/* List of renderers, one per window
	*/
	std::vector<VulkanRenderer> _renderers;

	/* List of futures for window renders
	*/
	std::vector<std::future<void>> _windowFutures;

	/* Mutex for locking down queue handles
	*/
	std::mutex queueMtx;

	/* List of meshes
	*/
	std::vector<Mesh> _meshes;
		Texture _tex;



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

	/* @brief Checks if the given device supports certain features
	*/
	bool _device_supports_features(VkPhysicalDevice physicalDevice) const;

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

	std::vector<Shader> _load_shaders();

	/* @brief Creates the renderers that will draw to windows
	*/
	void _create_renderers(const std::vector<Shader>& shaders);
};

