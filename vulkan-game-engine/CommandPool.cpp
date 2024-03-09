#include "CommandPool.h"

#include <stdexcept>

VkDescriptorSetLayout CommandPool::_descriptorSetLayout = VK_NULL_HANDLE;

VkDescriptorSetLayout CommandPool::get_descriptor_set_layout(VkDevice device)
{
	if (_descriptorSetLayout == VK_NULL_HANDLE)
	{
		VkDescriptorSetLayoutCreateInfo descriptorSetInfo{};
		VkDescriptorSetLayoutBinding uboBinding{};
		_configure_descriptor_set_layout(&descriptorSetInfo, &uboBinding);

		if (vkCreateDescriptorSetLayout(device, &descriptorSetInfo, nullptr, &_descriptorSetLayout) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create descriptor set layout");
		}
	}

	return _descriptorSetLayout;
}

void CommandPool::_configure_descriptor_set_layout(VkDescriptorSetLayoutCreateInfo* pCreateInfo, VkDescriptorSetLayoutBinding* pLayoutBinding)
{
	memset(pLayoutBinding, 0, sizeof(VkDescriptorSetLayoutBinding));
	pLayoutBinding->binding = 0;
	pLayoutBinding->descriptorCount = 1;
	pLayoutBinding->descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	pLayoutBinding->pImmutableSamplers = nullptr;
	pLayoutBinding->stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	memset(pCreateInfo, 0, sizeof(VkDescriptorSetLayoutCreateInfo));
	pCreateInfo->sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	pCreateInfo->bindingCount = 1;
	pCreateInfo->pBindings = pLayoutBinding;
}


/*
* CTOR / ASSIGNMENT DEFINITIONS
*/

CommandPool::CommandPool()
	: _cmdPool(VK_NULL_HANDLE),
	_cmdBuffers({}),
	_currentFrameNum(0),
	_numFramesInFlight(0),
	_imgAvailableSemaphores({}),
	_renderFinishedSemaphores({}),
	_inFlightFences({}),
	_uniformBuffers({}),
	_uniBufMappedMem({}),
	_descriptorPool(VK_NULL_HANDLE),
	_descriptorSets({}),
	_deviceHandle(VK_NULL_HANDLE)
{
}

CommandPool::CommandPool(const VulkanDevice& device, const QueueFamilyInfo& queueFamilyInfo, int maxFramesInFlight)
	: _cmdPool(VK_NULL_HANDLE),
	_cmdBuffers({}),
	_currentFrameNum(0),
	_numFramesInFlight(maxFramesInFlight),
	_imgAvailableSemaphores({}),
	_renderFinishedSemaphores({}),
	_inFlightFences({}),
	_uniformBuffers({}),
	_uniBufMappedMem({}),
	_descriptorPool(VK_NULL_HANDLE),
	_descriptorSets({}),
	_deviceHandle(device.handle())
{
	// 1) Create command pool object
	VkCommandPoolCreateInfo poolInfo{};
	_configure_command_pool(&poolInfo, queueFamilyInfo[QueueFamilyType::Graphics]);

	if (vkCreateCommandPool(_deviceHandle, &poolInfo, nullptr, &_cmdPool) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create command pool");
	}

	// 2) Allocate command buffers
	VkCommandBufferAllocateInfo bufferInfo{};
	_cmdBuffers.resize(maxFramesInFlight);
	_configure_command_buffers(&bufferInfo);

	if (vkAllocateCommandBuffers(_deviceHandle, &bufferInfo, _cmdBuffers.data()) != VK_SUCCESS) 
	{
		throw std::runtime_error("Failed to allocate command buffers");
	}

	// 3) Create sync objects
	_imgAvailableSemaphores.resize(maxFramesInFlight);
	_renderFinishedSemaphores.resize(maxFramesInFlight);
	_inFlightFences.resize(maxFramesInFlight);

	VkSemaphoreCreateInfo semaphoreInfo{};
	VkFenceCreateInfo fenceInfo{};
	_configure_sync_objects(&semaphoreInfo, &fenceInfo);

	for (size_t i = 0; i < maxFramesInFlight; i++)
	{
		if (vkCreateSemaphore(_deviceHandle, &semaphoreInfo, nullptr, &_imgAvailableSemaphores[i]) != VK_SUCCESS ||
			vkCreateSemaphore(_deviceHandle, &semaphoreInfo, nullptr, &_renderFinishedSemaphores[i]) != VK_SUCCESS ||
			vkCreateFence(_deviceHandle, &fenceInfo, nullptr, &_inFlightFences[i]) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create sync objects");
		}
	}

	// 4) Create uniform buffers
	_uniformBuffers = std::vector<Buffer>(_numFramesInFlight, Buffer(device, Buffer::Type::UNIFORM, sizeof(_UBO)));
	_uniBufMappedMem.resize(_numFramesInFlight);
	for (size_t i = 0; i < _numFramesInFlight; i++)
	{
		_uniformBuffers[i].map_memory(&_uniBufMappedMem[i]);
	}

	// 5) Create descriptor set layout if needed
	get_descriptor_set_layout(_deviceHandle);

	// 6) Create descriptor pool
	VkDescriptorPoolCreateInfo descriptorPoolInfo{};
	VkDescriptorPoolSize poolSize{};
	_configure_descriptor_pool(&descriptorPoolInfo, &poolSize);

	if (vkCreateDescriptorPool(_deviceHandle, &descriptorPoolInfo, nullptr, &_descriptorPool) != VK_SUCCESS) 
	{
		throw std::runtime_error("Failed to create descriptor pool");
	}

	// 7) Create descriptor sets
	std::vector<VkDescriptorSetLayout> descriptorSetLayouts(_numFramesInFlight, _descriptorSetLayout);
	VkDescriptorSetAllocateInfo setAllocInfo{};
	_configure_descriptor_set_alloc(&setAllocInfo, descriptorSetLayouts);

	_descriptorSets.resize(_numFramesInFlight);
	if (vkAllocateDescriptorSets(_deviceHandle, &setAllocInfo, _descriptorSets.data()) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to allocate descriptor sets");
	}

	for (size_t i = 0; i < _numFramesInFlight; ++i)
	{
		VkWriteDescriptorSet writeInfo{};
		VkDescriptorBufferInfo uniBufInfo{};
		_configure_descriptor_set(&writeInfo, &uniBufInfo, _uniformBuffers[i].handle(), _descriptorSets[i]);
		vkUpdateDescriptorSets(_deviceHandle, 1, &writeInfo, 0, nullptr);
	}
}

CommandPool::CommandPool(const CommandPool& other)
	: _cmdPool(other._cmdPool),
	_cmdBuffers(other._cmdBuffers),
	_currentFrameNum(other._currentFrameNum),
	_numFramesInFlight(other._numFramesInFlight),
	_imgAvailableSemaphores(other._imgAvailableSemaphores),
	_renderFinishedSemaphores(other._renderFinishedSemaphores),
	_inFlightFences(other._inFlightFences),
	_uniformBuffers(other._uniformBuffers),
	_uniBufMappedMem(other._uniBufMappedMem),
	_descriptorPool(other._descriptorPool),
	_descriptorSets(other._descriptorSets),
	_deviceHandle(other._deviceHandle)
{
	get_descriptor_set_layout(_deviceHandle);
}

CommandPool::CommandPool(CommandPool&& other) noexcept
	: CommandPool()
{
	swap(*this, other);
}

CommandPool& CommandPool::operator=(CommandPool other)
{
	swap(*this, other);
	return *this;
}

CommandPool::~CommandPool()
{
	if (_cmdPool != VK_NULL_HANDLE)
	{
		vkDestroyDescriptorPool(_deviceHandle, _descriptorPool, nullptr);
		vkDestroyDescriptorSetLayout(_deviceHandle, _descriptorSetLayout, nullptr);
		_descriptorSetLayout = VK_NULL_HANDLE;

		for (size_t i = 0; i < _numFramesInFlight; i++)
		{
			vkDestroySemaphore(_deviceHandle, _renderFinishedSemaphores[i], nullptr);
			vkDestroySemaphore(_deviceHandle, _imgAvailableSemaphores[i], nullptr);
			vkDestroyFence(_deviceHandle, _inFlightFences[i], nullptr);
		}

		vkDestroyCommandPool(_deviceHandle, _cmdPool, nullptr);
	}
}





/*
* PUBLIC METHOD DEFINITIONS
*/

void CommandPool::record_render_pass(
	VkRenderPass renderPass, 
	VkFramebuffer frameBuffer, 
	VkExtent2D extent, 
	VkPipeline pipeline, 
	VkPipelineLayout pipelineLayout,
	VkBuffer* pVertexBuffers, 
	VkBuffer indexBuffer, 
	const std::vector<uint16_t>& indices
)
{
	vkResetCommandBuffer(_cmdBuffers[_currentFrameNum], 0);

	VkCommandBufferBeginInfo cmdBeginInfo{};
	VkRenderPassBeginInfo passBeginInfo{};

	_configure_render_pass_cmd(&cmdBeginInfo, &passBeginInfo, renderPass, frameBuffer, extent);

	if (vkBeginCommandBuffer(_cmdBuffers[_currentFrameNum], &cmdBeginInfo) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to begin recording render pass command buffer");
	}

	vkCmdBeginRenderPass(_cmdBuffers[_currentFrameNum], &passBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
	vkCmdBindPipeline(_cmdBuffers[_currentFrameNum], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)extent.width;
	viewport.height = (float)extent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(_cmdBuffers[_currentFrameNum], 0, 1, &viewport);

	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = extent;
	vkCmdSetScissor(_cmdBuffers[_currentFrameNum], 0, 1, &scissor);

	VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers(_cmdBuffers[_currentFrameNum], 0, 1, pVertexBuffers, offsets);
	vkCmdBindIndexBuffer(_cmdBuffers[_currentFrameNum], indexBuffer, 0, VK_INDEX_TYPE_UINT16);

	vkCmdBindDescriptorSets(_cmdBuffers[_currentFrameNum], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &_descriptorSets[_currentFrameNum], 0, nullptr);
	vkCmdDrawIndexed(_cmdBuffers[_currentFrameNum], static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);

	vkCmdEndRenderPass(_cmdBuffers[_currentFrameNum]);

	if (vkEndCommandBuffer(_cmdBuffers[_currentFrameNum]) != VK_SUCCESS) 
	{
		throw std::runtime_error("Failed to record render pass command buffer");
	}
}

void CommandPool::submit_to_queue(VkQueue queue)
{
	VkSubmitInfo submitInfo{};
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	_configure_queue_submission(&submitInfo, waitStages);

	if (vkQueueSubmit(queue, 1, &submitInfo, _inFlightFences[_currentFrameNum]) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to submit command buffer");
	}
}

void CommandPool::update_ubo(glm::mat4 model, glm::mat4 view, glm::mat4 projection)
{
	_UBO ubo{
		model,
		view,
		projection
	};
	memcpy(_uniBufMappedMem[_currentFrameNum], &ubo, sizeof(ubo));
}





/*
* PRIVATE CONST METHOD DEFINITIONS
*/

void CommandPool::_configure_command_pool(VkCommandPoolCreateInfo* pCreateInfo, uint32_t graphicsQueueFamilyIndex) const
{
	memset(pCreateInfo, 0, sizeof(VkCommandPoolCreateInfo));
	pCreateInfo->sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	pCreateInfo->flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	pCreateInfo->queueFamilyIndex = graphicsQueueFamilyIndex;
}

void CommandPool::_configure_command_buffers(VkCommandBufferAllocateInfo* pCmdBufferInfo) const
{
	memset(pCmdBufferInfo, 0, sizeof(VkCommandBufferAllocateInfo));
	pCmdBufferInfo->sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	pCmdBufferInfo->commandPool = _cmdPool;
	pCmdBufferInfo->level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	pCmdBufferInfo->commandBufferCount = (uint32_t) _cmdBuffers.size();
}

void CommandPool::_configure_sync_objects(VkSemaphoreCreateInfo* pSemaphoreInfo, VkFenceCreateInfo* pFenceInfo) const
{
	memset(pSemaphoreInfo, 0, sizeof(VkSemaphoreCreateInfo));
	pSemaphoreInfo->sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	memset(pFenceInfo, 0, sizeof(VkFenceCreateInfo));
	pFenceInfo->sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	pFenceInfo->flags = VK_FENCE_CREATE_SIGNALED_BIT;
}

void CommandPool::_configure_render_pass_cmd(VkCommandBufferBeginInfo* pCmdInfo,
	VkRenderPassBeginInfo* pRenderPassInfo,
	VkRenderPass renderPass,
	VkFramebuffer frameBuffer,
	VkExtent2D extent
) const
{
	memset(pCmdInfo, 0, sizeof(VkCommandBufferBeginInfo));
	pCmdInfo->sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	memset(pRenderPassInfo, 0, sizeof(VkRenderPassBeginInfo));
	pRenderPassInfo->sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	pRenderPassInfo->renderPass = renderPass;
	pRenderPassInfo->framebuffer = frameBuffer;
	pRenderPassInfo->renderArea.offset = { 0, 0 };
	pRenderPassInfo->renderArea.extent = extent;

	VkClearValue clearColor = { {{0.0f, 0.0f, 0.0f, 1.0f}} };
	pRenderPassInfo->clearValueCount = 1;
	pRenderPassInfo->pClearValues = &clearColor;
}

void CommandPool::_configure_queue_submission(VkSubmitInfo* pCreateInfo, VkPipelineStageFlags* pWaitStages) const
{
	memset(pCreateInfo, 0, sizeof(VkSubmitInfo));
	pCreateInfo->sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	pCreateInfo->waitSemaphoreCount = 1;
	pCreateInfo->pWaitSemaphores = &_imgAvailableSemaphores[_currentFrameNum];
	pCreateInfo->pWaitDstStageMask = pWaitStages;

	pCreateInfo->commandBufferCount = 1;
	pCreateInfo->pCommandBuffers = &_cmdBuffers[_currentFrameNum];

	pCreateInfo->signalSemaphoreCount = 1;
	pCreateInfo->pSignalSemaphores = &_renderFinishedSemaphores[_currentFrameNum];
}

void CommandPool::_configure_descriptor_pool(VkDescriptorPoolCreateInfo* pCreateInfo, VkDescriptorPoolSize* pSizeInfo) const
{
	memset(pSizeInfo, 0, sizeof(VkDescriptorPoolSize));
	pSizeInfo->type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	pSizeInfo->descriptorCount = static_cast<uint32_t>(_numFramesInFlight);

	memset(pCreateInfo, 0, sizeof(VkDescriptorPoolCreateInfo));
	pCreateInfo->sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	pCreateInfo->poolSizeCount = 1;
	pCreateInfo->pPoolSizes = pSizeInfo;
	pCreateInfo->maxSets = static_cast<uint32_t>(_numFramesInFlight);
}

void CommandPool::_configure_descriptor_set_alloc(VkDescriptorSetAllocateInfo* pAllocInfo, const std::vector<VkDescriptorSetLayout>& setLayouts) const
{
	memset(pAllocInfo, 0, sizeof(VkDescriptorSetAllocateInfo));
	pAllocInfo->sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	pAllocInfo->descriptorPool = _descriptorPool;
	pAllocInfo->descriptorSetCount = static_cast<uint32_t>(_numFramesInFlight);
	pAllocInfo->pSetLayouts = setLayouts.data();
}

void CommandPool::_configure_descriptor_set(VkWriteDescriptorSet* pWriteInfo, VkDescriptorBufferInfo* pBufInfo, VkBuffer buffer, VkDescriptorSet descriptorSet) const
{
	memset(pBufInfo, 0, sizeof(VkDescriptorBufferInfo));
	pBufInfo->buffer = buffer;
	pBufInfo->offset = 0;
	pBufInfo->range = sizeof(_UBO);

	memset(pWriteInfo, 0, sizeof(VkWriteDescriptorSet));
	pWriteInfo->sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	pWriteInfo->dstSet = descriptorSet;
	pWriteInfo->dstBinding = 0;
	pWriteInfo->dstArrayElement = 0;
	pWriteInfo->descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	pWriteInfo->descriptorCount = 1;
	pWriteInfo->pBufferInfo = pBufInfo;
}