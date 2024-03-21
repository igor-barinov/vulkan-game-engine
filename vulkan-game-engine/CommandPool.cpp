#include "CommandPool.h"

#include <stdexcept>


/*
* CTOR / ASSIGNMENT DEFINITIONS
*/

CommandPool::CommandPool()
	: _cmdPool(VK_NULL_HANDLE),
	_currentFrameNum(0),
	_numFramesInFlight(0),
	_imgAvailableSemaphores({}),
	_renderFinishedSemaphores({}),
	_inFlightFences({}),
	_deviceHandle(VK_NULL_HANDLE)
{
}

CommandPool::CommandPool(const Device& device, int maxFramesInFlight)
	: _cmdPool(VK_NULL_HANDLE),
	_currentFrameNum(0),
	_numFramesInFlight(maxFramesInFlight),
	_imgAvailableSemaphores({}),
	_renderFinishedSemaphores({}),
	_inFlightFences({}),
	_deviceHandle(device.handle())
{
	// Create command pool object
	const auto& queueFamilyInfo = device.queue_family_info();
	VkCommandPoolCreateInfo poolInfo{};
	_configure_command_pool(&poolInfo, queueFamilyInfo[QueueFamilyType::Graphics]);

	if (vkCreateCommandPool(_deviceHandle, &poolInfo, nullptr, &_cmdPool) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create command pool");
	}

	// Create sync objects
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





/*
* PUBLIC METHOD DEFINITIONS
*/

void CommandPool::submit_to_queue(VkCommandBuffer* pCmdBuffer, VkQueue queue)
{
	VkSubmitInfo submitInfo{};
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	_configure_queue_submission(&submitInfo, waitStages, pCmdBuffer);

	if (vkQueueSubmit(queue, 1, &submitInfo, _inFlightFences[_currentFrameNum]) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to submit command buffer");
	}
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

void CommandPool::_configure_sync_objects(VkSemaphoreCreateInfo* pSemaphoreInfo, VkFenceCreateInfo* pFenceInfo) const
{
	memset(pSemaphoreInfo, 0, sizeof(VkSemaphoreCreateInfo));
	pSemaphoreInfo->sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	memset(pFenceInfo, 0, sizeof(VkFenceCreateInfo));
	pFenceInfo->sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	pFenceInfo->flags = VK_FENCE_CREATE_SIGNALED_BIT;
}

void CommandPool::_configure_queue_submission(VkSubmitInfo* pCreateInfo, VkPipelineStageFlags* pWaitStages, VkCommandBuffer* pCmdBuffer) const
{
	memset(pCreateInfo, 0, sizeof(VkSubmitInfo));
	pCreateInfo->sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	pCreateInfo->waitSemaphoreCount = 1;
	pCreateInfo->pWaitSemaphores = &_imgAvailableSemaphores[_currentFrameNum];
	pCreateInfo->pWaitDstStageMask = pWaitStages;

	pCreateInfo->commandBufferCount = 1;
	pCreateInfo->pCommandBuffers = pCmdBuffer;

	pCreateInfo->signalSemaphoreCount = 1;
	pCreateInfo->pSignalSemaphores = &_renderFinishedSemaphores[_currentFrameNum];
}