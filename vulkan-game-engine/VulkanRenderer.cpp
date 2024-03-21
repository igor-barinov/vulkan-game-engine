#include "VulkanRenderer.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

VulkanRenderer::VulkanRenderer()
	: _device({}),
	_window(Window()),
	_mesh({}),
	_swapChain({}),
	_pipeline({}),
	_commandPool({}),
	_commandBuffers({}),
	_descriptorPool({}),
	_vertexBuffer({}),
	_indexBuffer({}),
	_uniformBuffers({}),
	_uniformBufMemory({}),
	_ubo({})
{
}

VulkanRenderer::VulkanRenderer(const Device& device, const Window& window, const std::vector<Shader>& shaders, const Mesh& mesh, const Texture& texture)
	: _device(device),
	_window(window),
	_mesh(mesh),
	_swapChain({}),
	_pipeline({}),
	_commandPool({}),
	_commandBuffers({}),
	_descriptorPool({}),
	_vertexBuffer({}),
	_indexBuffer({}),
	_uniformBuffers({}),
	_uniformBufMemory({}),
	_ubo({})
{
	_init_swap_chain();
	_init_descriptor_pool();
	_init_graphics_pipeline(shaders);
	_init_command_pool();
	_init_texture_sampler();
	_init_buffers();
	_init_descriptor_data(texture);
	_init_command_buffers();
}

VulkanRenderer::VulkanRenderer(const VulkanRenderer& other)
	: _device(other._device),
	_window(other._window),
	_mesh(other._mesh),
	_swapChain(other._swapChain),
	_pipeline(other._pipeline),
	_commandPool(other._commandPool),
	_descriptorPool(other._descriptorPool),
	_vertexBuffer(other._vertexBuffer),
	_indexBuffer(other._indexBuffer),
	_uniformBuffers(other._uniformBuffers),
	_uniformBufMemory(other._uniformBufMemory),
	_ubo(other._ubo)
{
}

VulkanRenderer::VulkanRenderer(VulkanRenderer&& other) noexcept
	: VulkanRenderer()
{
	swap(*this, other);
}

VulkanRenderer& VulkanRenderer::operator=(VulkanRenderer other)
{
	swap(*this, other);
	return *this;
}

VulkanRenderer::~VulkanRenderer()
{
}



void VulkanRenderer::render(bool isAsync)
{
	while (!_window.should_close())
	{
		auto currentFrame = _commandPool.get_current_frame_num();

		// Poll for events
		_window.poll();

		// Wait for fences
		_commandPool.wait_for_fences();

		// Get next image from swap chain
		bool swapChainIsOutdated = false;
		auto imgIndex = _swapChain.get_next_image(_commandPool.image_availability_semaphore(), swapChainIsOutdated);

		if (swapChainIsOutdated) 
		{
			_recreate_swap_chain();
			continue;
		}

		// UBO
		static auto startTime = std::chrono::high_resolution_clock::now();

		auto currentTime = std::chrono::high_resolution_clock::now();
		float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
		auto extent = _swapChain.surface_extent();

		auto model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		auto view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		auto proj = glm::perspective(glm::radians(45.0f), extent.width / (float)extent.height, 0.1f, 10.0f);
		proj[1][1] *= -1;

		_update_ubo(UBO(model, view, proj), _commandPool.get_current_frame_num());

		// Reset fences and record render pass command
		VkBuffer vertexBuffers[] = { _vertexBuffer.handle() };
		_commandPool.reset_fences();
		_record_render_pass(_swapChain.frame_buffer_at(imgIndex));

		// Submit command
		auto submitCommand = [this, currentFrame]() {
			auto graphicsQueue = _device.queue_family_info().get_queue_handle(QueueFamilyType::Graphics);
			auto cmdBufHandle = _commandBuffers[currentFrame];
			_commandPool.submit_to_queue(&cmdBufHandle, graphicsQueue);
			};

		if (isAsync)
		{
			_mutex.lock();
			submitCommand();
			_mutex.unlock();
		}
		else
		{
			submitCommand();
		}

		

		// Present image to swap chain
		auto presentImg = [this](uint32_t* pImgIndex, bool& swapChainIsOutdated) {
			auto presentQueue = _device.queue_family_info().get_queue_handle(QueueFamilyType::Present);
			auto waitSemaphore = _commandPool.render_finished_semaphore();
			swapChainIsOutdated = _swapChain.present_image(presentQueue, &waitSemaphore, pImgIndex);
			};

		if (isAsync)
		{
			_mutex.lock();
			presentImg(&imgIndex, swapChainIsOutdated);
			_mutex.unlock();
		}
		else
		{
			presentImg(&imgIndex, swapChainIsOutdated);
		}

		if (swapChainIsOutdated || _window.was_resized())
		{
			_window.reset_resize_status();
			_recreate_swap_chain();
		}

		// Update command pool frame counter
		_commandPool.increment_frame_counter();
	}
}



void VulkanRenderer::_init_swap_chain()
{
	auto formatFilter = [](VkSurfaceFormatKHR format)
		{
			return format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
		};

	auto presentModeFilter = [](VkPresentModeKHR mode)
		{
			return mode == VK_PRESENT_MODE_MAILBOX_KHR;
		};

	_swapChain = SwapChain(_device, _window, formatFilter, presentModeFilter);
}

void VulkanRenderer::_init_descriptor_pool()
{
	_descriptorPool = DescriptorPool(
		_device,
		_NUM_FRAMES_IN_FLIGHT,
		{ DescriptorPool::BindingType::UBO, DescriptorPool::BindingType::TEXTURE_SAMPLER }
	);
}

void VulkanRenderer::_init_graphics_pipeline(const std::vector<Shader>& shaders)
{
	_pipeline = GraphicsPipeline(_device, _swapChain, shaders, _descriptorPool);
	_swapChain.init_framebuffers(_pipeline.render_pass());
}

void VulkanRenderer::_init_command_pool()
{
	_commandPool = CommandPool(_device, _NUM_FRAMES_IN_FLIGHT);
}

void VulkanRenderer::_init_texture_sampler()
{
	_textureSampler = TextureSampler(_device);
}

void VulkanRenderer::_init_buffers()
{
	auto cmdPool = _commandPool.handle();
	auto graphicsQueue = _device.queue_family_info().get_queue_handle(QueueFamilyType::Graphics);

	// Create staging buffers for vertex/index buffers
	auto vertexBufSize = _mesh.size_of_vertices();
	auto indexBufSize = _mesh.size_of_indices();
	Buffer vertexStagingBuf(_device, Buffer::Type::STAGING, vertexBufSize);
	Buffer indexStagingBuf(_device, Buffer::Type::STAGING, indexBufSize);

	// Create vertex buffer
	_vertexBuffer = Buffer(_device, Buffer::Type::VERTEX, vertexBufSize);
	vertexStagingBuf.copy_to_mapped_mem(_mesh.vertex_data());
	vertexStagingBuf.copy_to(_vertexBuffer, cmdPool, graphicsQueue);

	// Create index buffer
	_indexBuffer = Buffer(_device, Buffer::Type::INDEX, indexBufSize);
	indexStagingBuf.copy_to_mapped_mem(_mesh.index_data());
	indexStagingBuf.copy_to(_indexBuffer, cmdPool, graphicsQueue);

	// Create uniform buffers
	_uniformBuffers = std::array<Buffer, _NUM_FRAMES_IN_FLIGHT>();
	_uniformBuffers.fill(Buffer(_device, Buffer::Type::UNIFORM, sizeof(UBO)));

	_uniformBufMemory = std::array<void*, _NUM_FRAMES_IN_FLIGHT>();
	for (size_t i = 0; i < _NUM_FRAMES_IN_FLIGHT; ++i)
	{
		_uniformBuffers[i].map_memory(&_uniformBufMemory[i]);
	}
}

void VulkanRenderer::_init_descriptor_data(const Texture& texture)
{
	std::vector<std::vector<DescriptorPool::DescriptorData>> descriptorData(_NUM_FRAMES_IN_FLIGHT);
	for (uint32_t i = 0; i < _NUM_FRAMES_IN_FLIGHT; ++i)
	{
		DescriptorPool::DescriptorData uboData{};
		uboData.uboSize = sizeof(UBO);
		uboData.uniformBuffer = _uniformBuffers[i].handle();

		DescriptorPool::DescriptorData samplerData{};
		samplerData.textureImageView = texture.get_image_view();
		samplerData.textureSampler = _textureSampler.handle();

		descriptorData[i] = { uboData, samplerData };
	}
	_descriptorPool.write_descriptor_set(descriptorData);
}

void VulkanRenderer::_init_command_buffers()
{
	_commandBuffers = CommandBufferPool(_device.handle(), _NUM_FRAMES_IN_FLIGHT, _commandPool.handle());
}

void VulkanRenderer::_record_render_pass(VkFramebuffer frameBuffer)
{
	auto currentFrame = _commandPool.get_current_frame_num();
	auto cmdBufHandle = _commandBuffers[currentFrame];

	_commandBuffers.reset_one(currentFrame);

	VkCommandBufferBeginInfo cmdBeginInfo{};
	VkRenderPassBeginInfo passBeginInfo{};

	_configure_render_pass_cmd(&cmdBeginInfo, &passBeginInfo, frameBuffer);
	_commandBuffers.begin_one(cmdBeginInfo, currentFrame);

	vkCmdBeginRenderPass(cmdBufHandle, &passBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
	vkCmdBindPipeline(cmdBufHandle, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline.handle());

	auto extent = _swapChain.surface_extent();
	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)extent.width;
	viewport.height = (float)extent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(cmdBufHandle, 0, 1, &viewport);

	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = extent;
	vkCmdSetScissor(cmdBufHandle, 0, 1, &scissor);

	VkDeviceSize offsets[] = { 0 };
	VkBuffer pVertexBuffers[] = {_vertexBuffer.handle()};
	vkCmdBindVertexBuffers(cmdBufHandle, 0, 1, pVertexBuffers, offsets);
	vkCmdBindIndexBuffer(cmdBufHandle, _indexBuffer.handle(), 0, VK_INDEX_TYPE_UINT16);

	auto descriptor = _descriptorPool[currentFrame];
	vkCmdBindDescriptorSets(cmdBufHandle, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline.layout_handle(), 0, 1, &descriptor, 0, nullptr);
	vkCmdDrawIndexed(cmdBufHandle, static_cast<uint32_t>(_mesh.indices().size()), 1, 0, 0, 0);

	vkCmdEndRenderPass(cmdBufHandle);

	_commandBuffers.end_one(currentFrame);
}

void VulkanRenderer::_configure_render_pass_cmd(VkCommandBufferBeginInfo* pCommandInfo, VkRenderPassBeginInfo* pPassInfo, VkFramebuffer frameBuffer)
{
	memset(pCommandInfo, 0, sizeof(VkCommandBufferBeginInfo));
	pCommandInfo->sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	memset(pPassInfo, 0, sizeof(VkRenderPassBeginInfo));
	pPassInfo->sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	pPassInfo->renderPass = _pipeline.render_pass();
	pPassInfo->framebuffer = frameBuffer;
	pPassInfo->renderArea.offset = { 0, 0 };
	pPassInfo->renderArea.extent = _swapChain.surface_extent();

	VkClearValue clearColor = { {{0.0f, 0.0f, 0.0f, 1.0f}} };
	pPassInfo->clearValueCount = 1;
	pPassInfo->pClearValues = &clearColor;
}

void VulkanRenderer::_recreate_swap_chain()
{
	while (_window.is_minimized())
	{
		_window.idle();
	}

	vkDeviceWaitIdle(_device.handle());

	_swapChain.~SwapChain();
	_init_swap_chain();
	_swapChain.init_framebuffers(_pipeline.render_pass());
}

void VulkanRenderer::_update_ubo(const UBO& src, size_t frameNum)
{
	_ubo = src;
	memcpy(_uniformBufMemory[frameNum], &_ubo, sizeof(_ubo));
}