#pragma once

#include <vulkan/vulkan.h>

#include <algorithm>
#include <vector>

#include "QueueFamily.h"
#include "VulkanDevice.h"

/*
* Class that implements a Vulkan command pool
*/
class CommandPool
{
public:

	/*
	* PUBLIC STATIC CONSTANTS
	*/

	static constexpr int DEFAULT_FRAMES_IN_FLIGHT = 2;



	/*
	* PUBLIC FRIEND METHODS
	*/

	/* @brief Swap implementation for CommandPool class
	*/
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



	/*
	* CTORS / ASSIGNMENT
	*/

	CommandPool();

	/*
	* @param device Device being used
	* @param queueFamilyInfo Info containing queue handles
	* @param maxFramesInFlight Maximum number of frames in flight
	*/
	CommandPool(const VulkanDevice& device, const QueueFamilyInfo& queueFamilyInfo, int maxFramesInFlight = DEFAULT_FRAMES_IN_FLIGHT);
	CommandPool(const CommandPool& other);
	CommandPool(CommandPool&& other) noexcept;
	CommandPool& operator=(CommandPool other);
	~CommandPool();



	/*
	* PUBLIC METHODS
	*/

	/* @brief Records a render pass command
	* 
	* @param renderPass Handle to render pass being used
	* @param frameBuffer Handle to framebuffer being used
	* @param extenr Extent of surface being drawn to
	* @param pipeline Handle to graphics pipeline being used
	*/
	void record_render_pass(
		VkRenderPass renderPass,
		VkFramebuffer frameBuffer,
		VkExtent2D extent,
		VkPipeline pipeline,
		VkBuffer* pVertexBuffers,
		VkBuffer indexBuffer,
		const std::vector<uint16_t>& indices
	);

	/* @brief Submits command buffer for current frame to the given queue
	*/
	void submit_to_queue(VkQueue queue);

	/* @brief Waits for the fences for the current frame
	*/
	inline void wait_for_fences() { vkWaitForFences(_deviceHandle, 1, &_inFlightFences[_currentFrameNum], VK_TRUE, UINT64_MAX); }

	/* @brief Resets the fences for the current frame
	*/
	inline void reset_fences() { vkResetFences(_deviceHandle, 1, &_inFlightFences[_currentFrameNum]); }

	/* @brief Increments the frame count for tracking frames in flight
	*/
	inline void increment_frame_counter() { _currentFrameNum = (_currentFrameNum + 1) % _numFramesInFlight; }



	/*
	* PUBLIC CONST METHODS
	*/

	/* @brief Returns handle to command pool object
	*/
	inline VkCommandPool handle() const { return _cmdPool; }

	/* @brief Returns the semaphore used for signaling image availability in a swap chain
	*/
	inline VkSemaphore image_availability_semaphore() const { return _imgAvailableSemaphores[_currentFrameNum]; }

	/* @brief Returns the semaphore used for signaling a finished render
	*/
	inline VkSemaphore render_finished_semaphore() const { return _renderFinishedSemaphores[_currentFrameNum]; }

private:

	/*
	* PRIVATE MEMBERS
	*/

	/* Handle to command pool
	*/
	VkCommandPool _cmdPool;

	/* List of command buffers, one per frame in flight
	*/
	std::vector<VkCommandBuffer> _cmdBuffers;

	/* The current frame number
	*/
	uint32_t _currentFrameNum;

	/* The maximum number of frames in flight
	*/
	int _numFramesInFlight;

	/* List of semaphores signaling swap chain image availability, one per frame in flight
	*/
	std::vector<VkSemaphore> _imgAvailableSemaphores;

	/* List of semaphores signaling a finished render, one per frame in flight
	*/
	std::vector<VkSemaphore> _renderFinishedSemaphores;

	/* List of fences for synchronizing frame drawing
	*/
	std::vector<VkFence> _inFlightFences;

	/* Handle to device being used
	*/
	VkDevice _deviceHandle;



	/*
	* PRIVATE CONST METHODS
	*/

	/* @brief Fills struct with info necessary for creating a command pool
	* 
	* @param[out] pCreateInfo The struct to fill
	* @param graphicsQueueFamilyIndex The index of the graphics queue family
	*/
	void _configure_command_pool(VkCommandPoolCreateInfo* pCreateInfo, uint32_t graphicsQueueFamilyIndex) const;

	/* @brief Fills struct with necessary info for allocating command buffers
	*/
	void _configure_command_buffers(VkCommandBufferAllocateInfo* pCmdBufferInfo) const;

	/* @brief Fills struct with necessary info for creating all semaphores and fences
	*/
	void _configure_sync_objects(VkSemaphoreCreateInfo* pSemaphoreInfo, VkFenceCreateInfo* pFenceInfo) const;

	/* @brief Fills struct with necessary info for creating a render pass command
	* 
	* @param[out] pCmdInfo The struct to fill with command info
	* @param[out] pRenderPassInfo The struct to fill with render pass info
	* @param renderPass Handle to render pass being used
	* @param frameBuffer The framebuffer being used
	* @param extent The extent of the surface being drawn
	*/
	void _configure_render_pass_cmd(
		VkCommandBufferBeginInfo* pCmdInfo, 
		VkRenderPassBeginInfo* pRenderPassInfo, 
		VkRenderPass renderPass,
		VkFramebuffer frameBuffer,
		VkExtent2D extent
	) const;

	/* @brief Fills struct with necessary info for submitting a command to a queue
	* 
	* @param[out] pCreateInfo The struct to fill
	* @param pWaitStages Pointer to stage flags
	*/
	void _configure_queue_submission(VkSubmitInfo* pCreateInfo, VkPipelineStageFlags* pWaitStages) const;
};