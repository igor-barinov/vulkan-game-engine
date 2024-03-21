#pragma once

#include <array>
#include <mutex>
#include <algorithm>

#include "Device.h"
#include "Shader.h"
#include "SwapChain.h"
#include "GraphicsPipeline.h"
#include "CommandPool.h"
#include "DescriptorPool.h"
#include "Buffer.h"
#include "UBO.h"
#include "Window.h"
#include "Mesh.h"
#include "Texture.h"
#include "TextureSampler.h"

class VulkanRenderer
{

public:

	friend void swap(VulkanRenderer& rendA, VulkanRenderer& rendB)
	{
		using std::swap;

		swap(rendA._device, rendB._device);
		swap(rendA._window, rendB._window);
		swap(rendA._mesh, rendB._mesh);
		swap(rendA._swapChain, rendB._swapChain);
		swap(rendA._pipeline, rendB._pipeline);
		swap(rendA._commandPool, rendB._commandPool);
		swap(rendA._commandBuffers, rendB._commandBuffers);
		swap(rendA._descriptorPool, rendB._descriptorPool);
		swap(rendA._vertexBuffer, rendB._vertexBuffer);
		swap(rendA._indexBuffer, rendB._indexBuffer);
		swap(rendA._uniformBuffers, rendB._uniformBuffers);
		swap(rendA._uniformBufMemory, rendB._uniformBufMemory);
		swap(rendA._ubo, rendB._ubo);
		swap(rendA._textureSampler, rendB._textureSampler);
	}

	VulkanRenderer();
	VulkanRenderer(const Device& device, const Window& window, const std::vector<Shader>& shaders, const Mesh& mesh, const Texture& texture);
	VulkanRenderer(const VulkanRenderer& other);
	VulkanRenderer(VulkanRenderer&& other) noexcept;
	VulkanRenderer& operator=(VulkanRenderer other);
	~VulkanRenderer();

	void render(bool isAsync = false);

private:

	static constexpr uint32_t _NUM_FRAMES_IN_FLIGHT = 2;

	Device _device;
	Window _window;
	Mesh _mesh;
	SwapChain _swapChain;
	GraphicsPipeline _pipeline;
	CommandPool _commandPool;
	CommandBufferPool _commandBuffers;
	DescriptorPool _descriptorPool;
	Buffer _vertexBuffer;
	Buffer _indexBuffer;
	std::array<Buffer, _NUM_FRAMES_IN_FLIGHT> _uniformBuffers;
	std::array<void*, _NUM_FRAMES_IN_FLIGHT> _uniformBufMemory;
	UBO _ubo;
	TextureSampler _textureSampler;
	std::mutex _mutex;

	void _init_swap_chain();
	void _init_descriptor_pool();
	void _init_graphics_pipeline(const std::vector<Shader>& shaders);
	void _init_command_pool();
	void _init_texture_sampler();
	void _init_buffers();
	void _init_descriptor_data(const Texture& texture);
	void _init_command_buffers();

	void _configure_render_pass_cmd(VkCommandBufferBeginInfo* pCommandInfo, VkRenderPassBeginInfo* pPassInfo, VkFramebuffer frameBuffer);
	void _record_render_pass(VkFramebuffer frameBuffer);
	void _recreate_swap_chain();
	void _update_ubo(const UBO& src, size_t frameNum);
};

