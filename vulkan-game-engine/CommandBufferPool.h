#pragma once

#include <vulkan/vulkan.h>

#include <algorithm>
#include <vector>

class CommandBufferPool
{
public:
	friend void swap(CommandBufferPool& cmdBufA, CommandBufferPool& cmdBufB)
	{
		using std::swap;

		swap(cmdBufA._cmdBuffers, cmdBufB._cmdBuffers);
		swap(cmdBufA._cmdPool, cmdBufB._cmdPool);
		swap(cmdBufA._size, cmdBufB._size);
		swap(cmdBufA._deviceHandle, cmdBufB._deviceHandle);
	}

	CommandBufferPool();
	CommandBufferPool(VkDevice device, size_t size, VkCommandPool commandPool);
	CommandBufferPool(const CommandBufferPool& other);
	CommandBufferPool(CommandBufferPool&& other) noexcept;
	CommandBufferPool& operator=(CommandBufferPool other);
	~CommandBufferPool();

	void begin_all(VkCommandBufferBeginInfo beginInfo);
	void begin_one(VkCommandBufferBeginInfo beginInfo, size_t index);
	void end_all();
	void end_one(size_t index);
	void reset_one(size_t index);
	void reset_all();
	void submit_one_to_queue(VkQueue queue, size_t index);
	void submit_all_to_queue(VkQueue queue);

	inline VkCommandBuffer operator[](size_t index) { return _cmdBuffers[index]; }
	inline VkCommandBuffer* buffers() { return _cmdBuffers.data(); }

	inline VkCommandBuffer operator[](size_t index) const { return _cmdBuffers[index]; }

private:
	std::vector<VkCommandBuffer> _cmdBuffers;
	VkCommandPool _cmdPool;
	size_t _size;
	VkDevice _deviceHandle;

	void _configure_alloc_info(VkCommandBufferAllocateInfo* pAllocInfo) const;
};

