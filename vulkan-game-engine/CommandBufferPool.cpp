#include "CommandBufferPool.h"

#include <stdexcept>

CommandBufferPool::CommandBufferPool()
	: _cmdBuffers({}),
	_cmdPool(VK_NULL_HANDLE),
	_size(0),
	_deviceHandle(VK_NULL_HANDLE)
{

}

CommandBufferPool::CommandBufferPool(VkDevice device, size_t size, VkCommandPool commandPool)
	: _cmdBuffers(size),
	_cmdPool(commandPool),
	_size(size),
	_deviceHandle(device)
{
	VkCommandBufferAllocateInfo allocInfo{};
	_configure_alloc_info(&allocInfo);
	if (vkAllocateCommandBuffers(_deviceHandle, &allocInfo, _cmdBuffers.data()) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to allocate command buffers");
	}
}

CommandBufferPool::CommandBufferPool(const CommandBufferPool& other)
	: _cmdBuffers(other._cmdBuffers),
	_cmdPool(other._cmdPool),
	_size(other._size),
	_deviceHandle(other._deviceHandle)
{

}

CommandBufferPool::CommandBufferPool(CommandBufferPool&& other) noexcept
	: CommandBufferPool()
{
	swap(*this, other);
}

CommandBufferPool& CommandBufferPool::operator=(CommandBufferPool other)
{
	swap(*this, other);
	return *this;
}

CommandBufferPool::~CommandBufferPool()
{
	if (_cmdPool != VK_NULL_HANDLE)
	{
		vkFreeCommandBuffers(_deviceHandle, _cmdPool, _cmdBuffers.size(), _cmdBuffers.data());
	}
}

void CommandBufferPool::begin_all(VkCommandBufferBeginInfo beginInfo)
{
	for (size_t i = 0; i < _size; ++i)
	{
		begin_one(beginInfo, i);
	}
}
void CommandBufferPool::begin_one(VkCommandBufferBeginInfo beginInfo, size_t index)
{
	vkBeginCommandBuffer(_cmdBuffers[index], &beginInfo);
}
void CommandBufferPool::end_all()
{
	for (size_t i = 0; i < _size; ++i)
	{
		end_one(i);
	}
}
void CommandBufferPool::end_one(size_t index)
{
	vkEndCommandBuffer(_cmdBuffers[index]);
}

void CommandBufferPool::reset_one(size_t index)
{
	vkResetCommandBuffer(_cmdBuffers[index], 0);
}
void CommandBufferPool::reset_all()
{
	for (size_t i = 0; i < _size; ++i)
	{
		reset_one(i);
	}
}

void CommandBufferPool::submit_one_to_queue(VkQueue queue, size_t index)
{
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &_cmdBuffers[index];

	vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(queue);
}

void CommandBufferPool::submit_all_to_queue(VkQueue queue)
{
	for (size_t i = 0; i < _size; ++i)
	{
		submit_one_to_queue(queue, i);
	}
}

void CommandBufferPool::_configure_alloc_info(VkCommandBufferAllocateInfo* pAllocInfo) const
{
	memset(pAllocInfo, 0, sizeof(VkCommandBufferAllocateInfo));
	pAllocInfo->sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	pAllocInfo->commandPool = _cmdPool;
	pAllocInfo->level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	pAllocInfo->commandBufferCount = (uint32_t)_size;
}