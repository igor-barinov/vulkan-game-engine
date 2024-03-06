#include "CommandPool.h"

#include <stdexcept>

CommandPool::CommandPool()
	: _cmdPool(VK_NULL_HANDLE),
	_cmdBuffers({}),
	_currentFrameNum(0),
	_numFramesInFlight(0),
	_imgAvailableSemaphores({}),
	_renderFinishedSemaphores({}),
	_inFlightFences({}),
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
	_deviceHandle(device.handle())
{
	VkCommandPoolCreateInfo poolInfo{};
	_configure_command_pool(&poolInfo, queueFamilyInfo[QueueFamilyType::Graphics]);

	if (vkCreateCommandPool(_deviceHandle, &poolInfo, nullptr, &_cmdPool) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create command pool");
	}

	VkCommandBufferAllocateInfo bufferInfo{};
	_cmdBuffers.resize(maxFramesInFlight);
	_configure_command_buffers(&bufferInfo);

	if (vkAllocateCommandBuffers(_deviceHandle, &bufferInfo, _cmdBuffers.data()) != VK_SUCCESS) 
	{
		throw std::runtime_error("Failed to allocate command buffers");
	}

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
}

CommandPool::CommandPool(const CommandPool& other)
	: _cmdPool(other._cmdPool),
	_cmdBuffers(other._cmdBuffers),
	_currentFrameNum(other._currentFrameNum),
	_numFramesInFlight(other._numFramesInFlight),
	_imgAvailableSemaphores(other._imgAvailableSemaphores),
	_renderFinishedSemaphores(other._renderFinishedSemaphores),
	_inFlightFences(other._inFlightFences),
	_deviceHandle(other._deviceHandle)
{

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
		for (size_t i = 0; i < _numFramesInFlight; i++)
		{
			vkDestroySemaphore(_deviceHandle, _renderFinishedSemaphores[i], nullptr);
			vkDestroySemaphore(_deviceHandle, _imgAvailableSemaphores[i], nullptr);
			vkDestroyFence(_deviceHandle, _inFlightFences[i], nullptr);
		}

		vkDestroyCommandPool(_deviceHandle, _cmdPool, nullptr);
	}
}

void CommandPool::record_render_pass(VkRenderPass renderPass, VkFramebuffer frameBuffer, VkExtent2D extent, VkPipeline pipeline)
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

	vkCmdDraw(_cmdBuffers[_currentFrameNum], 3, 1, 0, 0);
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

void CommandPool::_configure_command_pool(VkCommandPoolCreateInfo* pCreateInfo, uint32_t graphicsQueueFamilyIndex) const
{
	memset(pCreateInfo, 0, sizeof(VkCommandPoolCreateInfo));
	pCreateInfo->sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	pCreateInfo->flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	pCreateInfo->queueFamilyIndex = graphicsQueueFamilyIndex;
}

void CommandPool::_configure_command_buffers(VkCommandBufferAllocateInfo* pCreateInfo) const
{
	memset(pCreateInfo, 0, sizeof(VkCommandBufferAllocateInfo));
	pCreateInfo->sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	pCreateInfo->commandPool = _cmdPool;
	pCreateInfo->level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	pCreateInfo->commandBufferCount = (uint32_t) _cmdBuffers.size();
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