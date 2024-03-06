#pragma once

#include <vulkan/vulkan.h>

#include <algorithm>
#include <vector>

#include "QueueFamily.h"
#include "VulkanDevice.h"

class CommandPool
{
public:

	friend void swap(CommandPool& poolA, CommandPool& poolB)
	{
		using std::swap;

		swap(poolA._cmdPool, poolB._cmdPool);
		swap(poolA._cmdBuffers, poolB._cmdBuffers);
		swap(poolA._currentFrameNum, poolB._currentFrameNum);
		swap(poolA._numFramesInFlight, poolB._numFramesInFlight);
		swap(poolA._imgAvailableSemaphores, poolB._imgAvailableSemaphores);
		swap(poolA._renderFinishedSemaphores, poolB._renderFinishedSemaphores);
		swap(poolA._inFlightFences, poolB._inFlightFences);
		swap(poolA._deviceHandle, poolB._deviceHandle);
	}

	static constexpr int DEFAULT_FRAMES_IN_FLIGHT = 2;

	CommandPool();
	CommandPool(const VulkanDevice& device, const QueueFamilyInfo& queueFamilyInfo, int maxFramesInFlight = DEFAULT_FRAMES_IN_FLIGHT);
	CommandPool(const CommandPool& other);
	CommandPool(CommandPool&& other) noexcept;
	CommandPool& operator=(CommandPool other);
	~CommandPool();

	void record_render_pass(VkRenderPass renderPass, VkFramebuffer frameBuffer, VkExtent2D extent, VkPipeline pipeline);
	void submit_to_queue(VkQueue queue);
	inline void wait_for_fences() { vkWaitForFences(_deviceHandle, 1, &_inFlightFences[_currentFrameNum], VK_TRUE, UINT64_MAX); }
	inline void reset_fences() { vkResetFences(_deviceHandle, 1, &_inFlightFences[_currentFrameNum]); }
	inline void increment_frame_counter() { _currentFrameNum = (_currentFrameNum + 1) % _numFramesInFlight; }

	inline VkSemaphore image_availability_semaphore() const { return _imgAvailableSemaphores[_currentFrameNum]; }
	inline VkSemaphore render_finished_semaphore() const { return _renderFinishedSemaphores[_currentFrameNum]; }

private:
	VkCommandPool _cmdPool;
	std::vector<VkCommandBuffer> _cmdBuffers;
	uint32_t _currentFrameNum;
	int _numFramesInFlight;
	std::vector<VkSemaphore> _imgAvailableSemaphores;
	std::vector<VkSemaphore> _renderFinishedSemaphores;
	std::vector<VkFence> _inFlightFences;
	VkDevice _deviceHandle;

	void _configure_command_pool(VkCommandPoolCreateInfo* pCreateInfo, uint32_t graphicsQueueFamilyIndex) const;
	void _configure_command_buffers(VkCommandBufferAllocateInfo* pCreateInfo) const;
	void _configure_sync_objects(VkSemaphoreCreateInfo* pSemaphoreInfo, VkFenceCreateInfo* pFenceInfo) const;
	void _configure_render_pass_cmd(
		VkCommandBufferBeginInfo* pCmdInfo, 
		VkRenderPassBeginInfo* pRenderPassInfo, 
		VkRenderPass renderPass,
		VkFramebuffer frameBuffer,
		VkExtent2D extent
	) const;
	void _configure_queue_submission(VkSubmitInfo* pCreateInfo, VkPipelineStageFlags* pWaitStages) const;
};