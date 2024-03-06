#include "VulkanClient.h"

#include <future>
#include <unordered_set>
#include <iterator>
#include <iostream>

#include "VulkanInstance.h"

/*
* CTORS / ASSIGNMENT DEFINITIONS
*/

VulkanClient::VulkanClient()
	: _device({}),
	_swapChains({}),
	_windows({}),
	_shaderFiles({}),
	_pipelines({}),
	_commandPools({})
{
}

VulkanClient::~VulkanClient()
{
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
	_create_swap_chains();
	_create_graphics_pipelines();

	for (size_t i = 0; i < _swapChains.size(); ++i)
	{
		_swapChains[i].init_framebuffers(_pipelines[i].render_pass());
	}

	_create_command_pools();
}

void VulkanClient::run()
{
	if (_windows.size() > 1)
	{
		std::vector<std::future<void>> windowFutures;
		for (size_t i = 0; i < _windows.size(); ++i)
		{
			auto task = [this](Window& win, SwapChain& swapChain, GraphicsPipeline& pipeline, CommandPool& cmdPool) {
				_render_frames(win, swapChain, pipeline, cmdPool);
				};

			windowFutures.push_back(std::async(
				std::launch::async,
				task,
				std::ref(_windows[i]),
				std::ref(_swapChains[i]),
				std::ref(_pipelines[i]),
				std::ref(_commandPools[i])
			));
		}


		for (auto& result : windowFutures)
		{
			result.get();
		}
	}
	else
	{
		_render_frames(_windows[0], _swapChains[0], _pipelines[0], _commandPools[0]);
	}
	
}





/*
* PRIVATE CONST METHOD DEFINITONS
*/

int VulkanClient::_num_surfaces_compatible(VkPhysicalDevice physicalDevice) const
{
	int result = 0;
	QueueFamilyInfo queueFamilyInfo;
	for (const auto& win : _windows)
	{
		queueFamilyInfo.load_queue_family_indices(physicalDevice, win.surface_handle());

		if (queueFamilyInfo.all_queue_families_are_supported())
		{
			result++;
		}
	}

	return result;
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

VkPhysicalDevice VulkanClient::_pick_physical_device(const std::vector<const char*>& deviceExtensions) const
{
	auto& vulkan = VulkanInstance::instance();
	const auto& physicalDevices = vulkan.get_physical_devices();

	VkPhysicalDevice bestDevice = VK_NULL_HANDLE;
	int maxSurfacesSupported = 0;

	for (const auto& device : physicalDevices)
	{
		if (!_device_supports_extensions(device, deviceExtensions)) { continue; }
		if (!_device_supports_swap_chain(device)) { continue; }

		int surfacesSupported = _num_surfaces_compatible(device);
		if (surfacesSupported > maxSurfacesSupported)
		{
			bestDevice = device;
			maxSurfacesSupported = surfacesSupported;
		}
	}

	return bestDevice;
}





/*
* PRIVATE METHOD DEFINITIONS
*/

void VulkanClient::_create_logical_device(const std::vector<const char*>& deviceExtensions)
{
	auto& vulkan = VulkanInstance::instance();
	const auto& validationLayers = (vulkan.validation_is_enabled()) ? vulkan.get_validation_layers() : std::vector<const char*>{};
	const auto& physicalDevice = _pick_physical_device(deviceExtensions);
	auto queueFamilyInfo = QueueFamilyInfo::info_for(physicalDevice, _windows.front().surface_handle());

	_device = VulkanDevice(physicalDevice, queueFamilyInfo, deviceExtensions, validationLayers);
}

void VulkanClient::_create_swap_chains()
{
	auto formatFilter = [](VkSurfaceFormatKHR format) 
		{
			return format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
		};

	auto presentModeFilter = [](VkPresentModeKHR mode)
		{
			return mode == VK_PRESENT_MODE_MAILBOX_KHR;
		};

	for (const auto& win : _windows)
	{
		_swapChains.push_back(SwapChain(_device, win, formatFilter, presentModeFilter));
	}
}

void VulkanClient::_create_graphics_pipelines()
{
	std::vector<Shader> loadedShaders;
	for (const auto& shaderToLoad : _shaderFiles)
	{
		loadedShaders.push_back(Shader(shaderToLoad.first, shaderToLoad.second, _device));
	}

	for (const auto& swapChain : _swapChains)
	{
		_pipelines.push_back(GraphicsPipeline(_device, swapChain, loadedShaders));
	}
}

void VulkanClient::_create_command_pools()
{
	for (size_t i = 0; i < _windows.size(); ++i)
	{
		_commandPools.push_back(CommandPool(_device, _device.queue_family_info()));
	}
}

void VulkanClient::_render_frames(Window& window, SwapChain& swapChain, GraphicsPipeline& pipeline, CommandPool& cmdPool)
{
	while (!window.should_close())
	{
		window.poll();

		cmdPool.wait_for_fences();

		bool swapChainIsOutofDate = false;
		auto imageIndex = swapChain.get_next_image(cmdPool.image_availability_semaphore(), swapChainIsOutofDate);
		if (swapChainIsOutofDate)
		{
			_recreate_swap_chain(window, swapChain, pipeline);
			return;
		}

		cmdPool.reset_fences();
		cmdPool.record_render_pass(
			pipeline.render_pass(),
			swapChain.frame_buffer_at(imageIndex),
			swapChain.surface_extent(),
			pipeline.handle()
		);

		mutex.lock(); // Queue handles cannot be used in multiple threads
		auto graphicsQueue = _device.queue_family_info().get_queue_handle(QueueFamilyType::Graphics);
		cmdPool.submit_to_queue(graphicsQueue);

		auto presentQueue = _device.queue_family_info().get_queue_handle(QueueFamilyType::Present);
		auto waitSemaphore = cmdPool.render_finished_semaphore();
		swapChainIsOutofDate = swapChain.present_image(presentQueue, &waitSemaphore, &imageIndex);
		mutex.unlock();

		if (swapChainIsOutofDate || window.was_resized())
		{
			window.reset_resize_status();
			_recreate_swap_chain(window, swapChain, pipeline);
		}

		cmdPool.increment_frame_counter();
	}

	vkDeviceWaitIdle(_device.handle());
}

void VulkanClient::_recreate_swap_chain(Window& window, SwapChain& swapChain, GraphicsPipeline& pipeline)
{
	int width = 0, height = 0;
	glfwGetFramebufferSize(window.handle(), &width, &height);
	while (width == 0 || height == 0) {
		glfwGetFramebufferSize(window.handle(), &width, &height);
		glfwWaitEvents();
	}

	vkDeviceWaitIdle(_device.handle());

	swapChain.~SwapChain();

	auto formatFilter = [](VkSurfaceFormatKHR format)
		{
			return format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
		};

	auto presentModeFilter = [](VkPresentModeKHR mode)
		{
			return mode == VK_PRESENT_MODE_MAILBOX_KHR;
		};

	swapChain = SwapChain(_device, window, formatFilter, presentModeFilter);
	swapChain.init_framebuffers(pipeline.render_pass());
}