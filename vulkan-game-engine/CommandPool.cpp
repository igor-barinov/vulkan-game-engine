#include "CommandPool.h"

#include <stdexcept>

VkDescriptorSetLayout CommandPool::_descriptorSetLayout = VK_NULL_HANDLE;

VkDescriptorSetLayout CommandPool::get_descriptor_set_layout(VkDevice device)
{
	if (_descriptorSetLayout == VK_NULL_HANDLE)
	{
		VkDescriptorSetLayoutBinding uboBinding{};
		_configure_ubo_layout_binding(&uboBinding);

		VkDescriptorSetLayoutBinding samplerBinding{};
		_configure_texture_sampler_layout_binding(&samplerBinding);

		VkDescriptorSetLayoutCreateInfo descriptorSetInfo{};
		std::vector< VkDescriptorSetLayoutBinding> bindings = { uboBinding, samplerBinding };
		_configure_descriptor_set_layouts(&descriptorSetInfo, bindings);

		if (vkCreateDescriptorSetLayout(device, &descriptorSetInfo, nullptr, &_descriptorSetLayout) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create descriptor set layout");
		}
	}

	return _descriptorSetLayout;
}

void CommandPool::_configure_ubo_layout_binding(VkDescriptorSetLayoutBinding* pBinding)
{
	memset(pBinding, 0, sizeof(VkDescriptorSetLayoutBinding));
	pBinding->binding = 0;
	pBinding->descriptorCount = 1;
	pBinding->descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	pBinding->pImmutableSamplers = nullptr;
	pBinding->stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
}

void CommandPool::_configure_texture_sampler_layout_binding(VkDescriptorSetLayoutBinding* pBinding)
{
	memset(pBinding, 0, sizeof(VkDescriptorSetLayoutBinding));
	pBinding->binding = 1;
	pBinding->descriptorCount = 1;
	pBinding->descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	pBinding->pImmutableSamplers = nullptr;
	pBinding->stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
}

void CommandPool::_configure_descriptor_set_layouts(VkDescriptorSetLayoutCreateInfo* pCreateInfo, const std::vector<VkDescriptorSetLayoutBinding>& bindings)
{
	memset(pCreateInfo, 0, sizeof(VkDescriptorSetLayoutCreateInfo));
	pCreateInfo->sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	pCreateInfo->bindingCount = static_cast<uint32_t>(bindings.size());
	pCreateInfo->pBindings = bindings.data();
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
	_textureSampler(VK_NULL_HANDLE),
	_deviceHandle(VK_NULL_HANDLE)
{
}

CommandPool::CommandPool(const VulkanDevice& device, VkImageView textureImageView, int maxFramesInFlight)
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
	_textureSampler(VK_NULL_HANDLE),
	_deviceHandle(device.handle())
{
	// 1) Create command pool object
	const auto& queueFamilyInfo = device.queue_family_info();
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

	// 4) Create texture sampler
	VkSamplerCreateInfo samplerInfo{};
	_configure_texture_sampler(&samplerInfo, device.physical_properties());

	if (vkCreateSampler(_deviceHandle, &samplerInfo, nullptr, &_textureSampler) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create texture sampler");
	}

	// 5) Create uniform buffers
	_uniformBuffers = std::vector<Buffer>(_numFramesInFlight, Buffer(device, Buffer::Type::UNIFORM, sizeof(_UBO)));
	_uniBufMappedMem.resize(_numFramesInFlight);
	for (size_t i = 0; i < _numFramesInFlight; i++)
	{
		_uniformBuffers[i].map_memory(&_uniBufMappedMem[i]);
	}

	// 6) Create descriptor set layout if needed
	get_descriptor_set_layout(_deviceHandle);

	// 7) Create descriptor pool
	VkDescriptorPoolSize uboPoolSize{};
	uboPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uboPoolSize.descriptorCount = static_cast<uint32_t>(_numFramesInFlight);

	VkDescriptorPoolSize samplerPoolSize{};
	samplerPoolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerPoolSize.descriptorCount = static_cast<uint32_t>(_numFramesInFlight);

	std::vector<VkDescriptorPoolSize> poolSizes = { uboPoolSize, samplerPoolSize };

	VkDescriptorPoolCreateInfo descriptorPoolInfo{};
	_configure_descriptor_pool(&descriptorPoolInfo, poolSizes);

	if (vkCreateDescriptorPool(_deviceHandle, &descriptorPoolInfo, nullptr, &_descriptorPool) != VK_SUCCESS) 
	{
		throw std::runtime_error("Failed to create descriptor pool");
	}

	// 8) Create descriptor sets
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
		std::vector<VkWriteDescriptorSet> writeSet;

		VkDescriptorBufferInfo uboInfo{};
		auto uboDescriptorSet = _configure_ubo_descriptor_set(&uboInfo, _uniformBuffers[i].handle(), _descriptorSets[i]);
		writeSet.push_back(uboDescriptorSet);

		if (textureImageView != VK_NULL_HANDLE)
		{
			VkDescriptorImageInfo imageInfo{};
			auto samplerDescriptorSet = _configure_sampler_descriptor_set(&imageInfo, textureImageView, _descriptorSets[i]);
			writeSet.push_back(samplerDescriptorSet);
		}

		vkUpdateDescriptorSets(_deviceHandle, static_cast<uint32_t>(writeSet.size()), writeSet.data(), 0, nullptr);
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
	_textureSampler(other._textureSampler),
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

void CommandPool::record_image_layout_transition(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, VkQueue graphicsQueue)
{
	VkCommandBuffer commandBuffer = _begin_one_time_command();

	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = image;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;

	VkPipelineStageFlags sourceStage;
	VkPipelineStageFlags destinationStage;

	if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) 
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) 
	{
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else 
	{
		throw std::invalid_argument("Image layout transition is unsupported");
	}

	vkCmdPipelineBarrier(
		commandBuffer,
		sourceStage, destinationStage,
		0,
		0, nullptr,
		0, nullptr,
		1, &barrier
	);

	_end_one_time_command(commandBuffer, graphicsQueue);
}

void CommandPool::record_copy_image_to_buffer(VkBuffer buffer, VkImage image, uint32_t imgWidth, uint32_t imgHeight, VkQueue graphicsQueue)
{
	VkCommandBuffer commandBuffer = _begin_one_time_command();

	VkBufferImageCopy region{};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;
	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;
	region.imageOffset = { 0, 0, 0 };
	region.imageExtent = {
		imgWidth,
		imgHeight,
		1
	};

	vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

	_end_one_time_command(commandBuffer, graphicsQueue);
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

VkCommandBuffer CommandPool::_begin_one_time_command() const
{
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = _cmdPool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(_deviceHandle, &allocInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	return commandBuffer;
}

void CommandPool::_end_one_time_command(VkCommandBuffer cmdBuffer, VkQueue queue) const
{
	vkEndCommandBuffer(cmdBuffer);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &cmdBuffer;

	vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(queue);

	vkFreeCommandBuffers(_deviceHandle, _cmdPool, 1, &cmdBuffer);
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

void CommandPool::_configure_descriptor_pool(VkDescriptorPoolCreateInfo* pCreateInfo, const std::vector<VkDescriptorPoolSize>& poolSizes) const
{
	memset(pCreateInfo, 0, sizeof(VkDescriptorPoolCreateInfo));
	pCreateInfo->sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	pCreateInfo->poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	pCreateInfo->pPoolSizes = poolSizes.data();
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

VkWriteDescriptorSet CommandPool::_configure_ubo_descriptor_set(VkDescriptorBufferInfo* pBufInfo, VkBuffer uniformBuffer, VkDescriptorSet descriptorSet) const
{
	memset(pBufInfo, 0, sizeof(VkDescriptorBufferInfo));
	pBufInfo->buffer = uniformBuffer;
	pBufInfo->offset = 0;
	pBufInfo->range = sizeof(_UBO);

	VkWriteDescriptorSet uboDescriptorSet{};
	uboDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	uboDescriptorSet.dstSet = descriptorSet;
	uboDescriptorSet.dstBinding = 0;
	uboDescriptorSet.dstArrayElement = 0;
	uboDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uboDescriptorSet.descriptorCount = 1;
	uboDescriptorSet.pBufferInfo = pBufInfo;

	return uboDescriptorSet;
}

VkWriteDescriptorSet CommandPool::_configure_sampler_descriptor_set(VkDescriptorImageInfo* pImageInfo, VkImageView imageView, VkDescriptorSet descriptorSet) const
{
	memset(pImageInfo, 0, sizeof(VkDescriptorImageInfo));
	pImageInfo->imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	pImageInfo->imageView = imageView;
	pImageInfo->sampler = _textureSampler;

	VkWriteDescriptorSet samplerDescriptorSet{};
	samplerDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	samplerDescriptorSet.dstSet = descriptorSet;
	samplerDescriptorSet.dstBinding = 1;
	samplerDescriptorSet.dstArrayElement = 0;
	samplerDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerDescriptorSet.descriptorCount = 1;
	samplerDescriptorSet.pImageInfo = pImageInfo;

	return samplerDescriptorSet;
}

void CommandPool::_configure_texture_sampler(VkSamplerCreateInfo* pCreateInfo, VkPhysicalDeviceProperties deviceProps) const
{
	memset(pCreateInfo, 0, sizeof(VkSamplerCreateInfo));
	pCreateInfo->sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	pCreateInfo->magFilter = VK_FILTER_LINEAR;
	pCreateInfo->minFilter = VK_FILTER_LINEAR;
	pCreateInfo->addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	pCreateInfo->addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	pCreateInfo->addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	pCreateInfo->anisotropyEnable = VK_TRUE;
	pCreateInfo->maxAnisotropy = deviceProps.limits.maxSamplerAnisotropy;
	pCreateInfo->borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	pCreateInfo->unnormalizedCoordinates = VK_FALSE;
	pCreateInfo->compareEnable = VK_FALSE;
	pCreateInfo->compareOp = VK_COMPARE_OP_ALWAYS;
	pCreateInfo->mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
}