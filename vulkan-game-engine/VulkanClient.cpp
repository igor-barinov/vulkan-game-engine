#include "VulkanClient.h"

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
	_commandPools({}),
	_stagingBuffers({}),
	_vertexBuffers({}),
	_indexBuffers({})
{
	_meshes = {
		Mesh({
			Vertex(-0.5, -0.5, 1.0, 0.0, 0.0),
			Vertex(0.5, -0.5, 0.0, 1.0, 0.0),
			Vertex(0.5, 0.5, 0.0, 0.0, 1.0),
			Vertex(-0.5, 0.5, 1.0, 1.0, 1.0)
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
	_create_swap_chains();
	_create_graphics_pipelines();

	for (size_t i = 0; i < _swapChains.size(); ++i)
	{
		_swapChains[i].init_framebuffers(_pipelines[i].render_pass());
	}

	_create_command_pools();
	_create_buffers();
}

void VulkanClient::run()
{
	_render_frames(_windows[0], _swapChains[0], _pipelines[0], _commandPools[0], _vertexBuffers[0], _indexBuffers[0]);
	/**
	for (size_t i = 0; i < _windows.size(); ++i)
	{
		auto task = [this](Window& win, SwapChain& swapChain, GraphicsPipeline& pipeline, CommandPool& cmdPool) {
			_render_frames(win, swapChain, pipeline, cmdPool);
			};

		_windowFutures.push_back(std::async(
			std::launch::async,
			task,
			std::ref(_windows[i]),
			std::ref(_swapChains[i]),
			std::ref(_pipelines[i]),
			std::ref(_commandPools[i])
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

VkPhysicalDevice VulkanClient::_pick_physical_device(const std::vector<const char*>& deviceExtensions) const
{
	auto& vulkan = VulkanInstance::instance();
	const auto& physicalDevices = vulkan.get_physical_devices();

	for (const auto& device : physicalDevices)
	{
		if (!_device_compatible_with_surfaces(device)) { continue; }
		if (!_device_supports_extensions(device, deviceExtensions)) { continue; }
		if (!_device_supports_swap_chain(device)) { continue; }
		
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

void VulkanClient::_create_buffers()
{
	auto vertexBufSize = _meshes[0].size_of_vertices();
	auto indexBufSize = _meshes[0].size_of_indices();
	Buffer vertexStagingBuf(_device, Buffer::Type::STAGING, vertexBufSize);
	Buffer indexStagingBuf(_device, Buffer::Type::STAGING, indexBufSize);

	for (size_t i = 0; i < _windows.size(); ++i)
	{
		auto cmdPool = _commandPools[i].handle();
		auto graphicsQueue = _device.queue_family_info().get_queue_handle(QueueFamilyType::Graphics);

		_vertexBuffers.push_back(Buffer(_device, Buffer::Type::VERTEX, vertexBufSize));
		vertexStagingBuf.map_host_data(_meshes[0].vertex_data());
		vertexStagingBuf.copy_to(_vertexBuffers[i], cmdPool, graphicsQueue);

		_indexBuffers.push_back(Buffer(_device, Buffer::Type::INDEX, indexBufSize));
		indexStagingBuf.map_host_data(_meshes[0].index_data());
		indexStagingBuf.copy_to(_indexBuffers[i], cmdPool, graphicsQueue);
	}
}

void VulkanClient::_render_frames(
	Window& window, 
	SwapChain& swapChain, 
	GraphicsPipeline& pipeline, 
	CommandPool& cmdPool, 
	Buffer& vertexBuffer, 
	Buffer& indexBuffer
)
{
	while (!window.should_close())
	{
		// 1) Poll for events
		window.poll();

		// 2) Wait for fences
		cmdPool.wait_for_fences();

		// 3) Get next image from swap chain
		bool swapChainIsOutofDate = false;
		auto imageIndex = swapChain.get_next_image(cmdPool.image_availability_semaphore(), swapChainIsOutofDate);
		if (swapChainIsOutofDate)
		{
			_recreate_swap_chain(window, swapChain, pipeline);
			continue;
		}

		// 4) Reset fences and record render pass command
		VkBuffer vertexBuffers[] = {vertexBuffer.handle()};
		cmdPool.reset_fences();
		cmdPool.record_render_pass(
			pipeline.render_pass(),
			swapChain.frame_buffer_at(imageIndex),
			swapChain.surface_extent(),
			pipeline.handle(),
			vertexBuffers,
			indexBuffer.handle(),
			_meshes[0].indices()
		);
		
		// 5) Submit command
		queueMtx.lock();
		auto graphicsQueue = _device.queue_family_info().get_queue_handle(QueueFamilyType::Graphics);
		cmdPool.submit_to_queue(graphicsQueue);
		queueMtx.unlock();

		// 6) Present image to swap chain
		queueMtx.lock();
		auto presentQueue = _device.queue_family_info().get_queue_handle(QueueFamilyType::Present);
		auto waitSemaphore = cmdPool.render_finished_semaphore();
		swapChainIsOutofDate = swapChain.present_image(presentQueue, &waitSemaphore, &imageIndex);
		queueMtx.unlock();
		if (swapChainIsOutofDate || window.was_resized())
		{
			window.reset_resize_status();
			_recreate_swap_chain(window, swapChain, pipeline);
		}

		// 7) Update command pool frame counter
		cmdPool.increment_frame_counter();
	}

	vkDeviceWaitIdle(_device.handle());
}

void VulkanClient::_recreate_swap_chain(Window& window, SwapChain& swapChain, GraphicsPipeline& pipeline)
{
	while (window.is_minimized()) 
	{
		window.idle();
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