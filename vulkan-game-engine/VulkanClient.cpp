#include "VulkanClient.h"

#include <unordered_set>
#include <iterator>
#include <iostream>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "VulkanInstance.h"

/*
* CTORS / ASSIGNMENT DEFINITIONS
*/

VulkanClient::VulkanClient()
	: _device({}),
	_windows({}),
	_shaderFiles({})
{
	_meshes = {
		Mesh({
			Vertex(-0.5, -0.5, 1.0, 0.0, 0.0, 1.0, 0.0),
			Vertex(0.5, -0.5, 0.0, 1.0, 0.0, 0.0, 0.0),
			Vertex(0.5, 0.5, 0.0, 0.0, 1.0, 0.0, 1.0),
			Vertex(-0.5, 0.5, 1.0, 1.0, 1.0, 1.0, 1.0)
		},
		{0, 1, 2, 2, 3, 0})
	};
}

VulkanClient::~VulkanClient()
{
	stop();
}





/*
* PUBLIC METHOD DEFINITIONS
*/

void VulkanClient::add_window(const char* title, uint32_t width, uint32_t height)
{
	_windows.push_back(Window(title, width, height));
}

void VulkanClient::add_shader(const std::string& filepath, Shader::Type shaderType)
{
	_shaderFiles.push_back({ filepath, shaderType });
}

void VulkanClient::init(const std::vector<const char*>& deviceExtensions)
{
	_create_logical_device(deviceExtensions);

	CommandPool tmpPool(_device);
	PNGImage texture("test.png");
	_tex = Texture(texture, _device, tmpPool);

	auto shaders = _load_shaders();
	_create_renderers(shaders);
}

void VulkanClient::run()
{

	_renderers[0].render();

	/**
	for (size_t i = 0; i < _windows.size(); ++i)
	{
		auto task = [this](VulkanRenderer& renderer) {
			renderer.render();
			};

		_windowFutures.push_back(std::async(
			std::launch::async,
			task,
			std::ref(_renderers[i])
		));
	}
	//*/
}

void VulkanClient::stop()
{
	for (auto& future : _windowFutures)
	{
		future.get();
	}
}





/*
* PRIVATE CONST METHOD DEFINITONS
*/

bool VulkanClient::_device_compatible_with_surfaces(VkPhysicalDevice physicalDevice) const
{
	QueueFamilyInfo queueFamilyInfo;
	for (const auto& win : _windows)
	{
		queueFamilyInfo.load_queue_family_indices(physicalDevice, win.surface_handle());

		if (!queueFamilyInfo.all_queue_families_are_supported())
		{
			return false;
		}
	}

	return true;
}

bool VulkanClient::_device_supports_extensions(VkPhysicalDevice physicalDevice, const std::vector<const char*>& deviceExtensions) const
{
	uint32_t extensionCount = 0;
	vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr);

	if (extensionCount == 0) { return false; }

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, availableExtensions.data());

	std::unordered_set<std::string> availableExtNames;
	std::transform(availableExtensions.begin(), availableExtensions.end(), std::inserter(availableExtNames, availableExtNames.begin()), [](const VkExtensionProperties& prop) { return prop.extensionName; });

	for (const auto& requiredExt : deviceExtensions)
	{
		auto search = availableExtNames.find(std::string(requiredExt));
		if (search == availableExtNames.end())
		{
			return false;
		}
	}
	
	return true;
}

bool VulkanClient::_device_supports_swap_chain(VkPhysicalDevice physicalDevice) const
{
	for (const auto& win : _windows)
	{
		if (!SwapChain::query_support_for(physicalDevice, win.surface_handle()))
		{
			return false;
		}
	}

	return true;
}

bool VulkanClient::_device_supports_features(VkPhysicalDevice physicalDevice) const
{
	VkPhysicalDeviceFeatures supportedFeatures;
	vkGetPhysicalDeviceFeatures(physicalDevice, &supportedFeatures);
	
	return supportedFeatures.samplerAnisotropy;
}

VkPhysicalDevice VulkanClient::_pick_physical_device(const std::vector<const char*>& deviceExtensions) const
{
	auto& vulkan = VulkanInstance::instance();
	const auto& physicalDevices = vulkan.get_physical_devices();

	for (const auto& device : physicalDevices)
	{
		if (!_device_compatible_with_surfaces(device)) { continue; }
		if (!_device_supports_extensions(device, deviceExtensions)) { continue; }
		if (!_device_supports_swap_chain(device)) { continue; }
		if (!_device_supports_features(device)) { continue; }
		
		return device;
	}

	return VK_NULL_HANDLE;
}





/*
* PRIVATE METHOD DEFINITIONS
*/

void VulkanClient::_create_logical_device(const std::vector<const char*>& deviceExtensions)
{
	auto& vulkan = VulkanInstance::instance();
	const auto& validationLayers = (vulkan.validation_is_enabled()) ? vulkan.get_validation_layers() : std::vector<const char*>{};
	const auto& physicalDevice = _pick_physical_device(deviceExtensions);
	if (physicalDevice == VK_NULL_HANDLE)
	{
		throw std::runtime_error("Failed to find a supported physical device");
	}

	auto queueFamilyInfo = QueueFamilyInfo::info_for(physicalDevice, _windows.front().surface_handle());

	_device = Device(physicalDevice, queueFamilyInfo, deviceExtensions, validationLayers);
}

std::vector<Shader> VulkanClient::_load_shaders()
{
	std::vector<Shader> shaders;
	for (auto shaderInfo : _shaderFiles)
	{
		shaders.push_back(Shader(shaderInfo.first, shaderInfo.second, _device));
	}

	return shaders;
}

void VulkanClient::_create_renderers(const std::vector<Shader>& shaders)
{
	for (size_t i = 0; i < _windows.size(); ++i)
	{
		_renderers.push_back(VulkanRenderer(
			_device,
			_windows[i],
			shaders,
			_meshes[0],
			_tex
		));
	}
}