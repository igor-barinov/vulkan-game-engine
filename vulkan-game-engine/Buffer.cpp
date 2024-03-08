#include "Buffer.h"

/*
* CTORS / ASSIGNMENT DEFINITIONS
*/

Buffer::Buffer()
	: _buf(VK_NULL_HANDLE),
	_bufMem(VK_NULL_HANDLE),
	_bufSize(0),
	_deviceHandle(VK_NULL_HANDLE),
	_physicalDeviceHandle(VK_NULL_HANDLE)
{
}

Buffer::Buffer(const VulkanDevice& device, Buffer::Type bufferType, size_t bufferSize)
	: _buf(VK_NULL_HANDLE),
	_bufMem(VK_NULL_HANDLE),
	_bufSize(bufferSize),
	_deviceHandle(device.handle()),
	_physicalDeviceHandle(device.get_physical_device())
{
	VkBufferUsageFlags usageFlags = 0;
	VkMemoryPropertyFlags memPropFlags = 0;
	switch (bufferType)
	{
	case Buffer::STAGING:
		usageFlags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		memPropFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
		break;
	case Buffer::VERTEX:
		usageFlags = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		memPropFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
		break;
	case Buffer::INDEX:
		usageFlags = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
		memPropFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
		break;
	}

	_create_buffer(usageFlags, memPropFlags);
}

Buffer::Buffer(const Buffer& other)
	: _buf(other._buf),
	_bufMem(other._bufMem),
	_bufSize(other._bufSize),
	_deviceHandle(other._deviceHandle),
	_physicalDeviceHandle(other._physicalDeviceHandle)
{
}

Buffer::Buffer(Buffer&& other) noexcept
	: Buffer()
{
	swap(*this, other);
}

Buffer& Buffer::operator=(Buffer other)
{
	swap(*this, other);
	return *this;
}

Buffer::~Buffer()
{
	if (_buf != VK_NULL_HANDLE)
	{
		vkDestroyBuffer(_deviceHandle, _buf, nullptr);
		vkFreeMemory(_deviceHandle, _bufMem, nullptr);
	}
}





/*
* PUBLIC METHOD DEFINITIONS
*/

void Buffer::map_host_data(const void* data)
{
	void* mappedData;
	vkMapMemory(_deviceHandle, _bufMem, 0, _bufSize, 0, &mappedData);
	memcpy(mappedData, data, (size_t)_bufSize);
	vkUnmapMemory(_deviceHandle, _bufMem);
}

void Buffer::copy_to(Buffer& destBuf, VkCommandPool commandPool, VkQueue graphicsQueue)
{
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = commandPool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(_deviceHandle, &allocInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	VkBufferCopy copyRegion{};
	copyRegion.size = _bufSize;
	vkCmdCopyBuffer(commandBuffer, _buf, destBuf._buf, 1, &copyRegion);

	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(graphicsQueue);

	vkFreeCommandBuffers(_deviceHandle, commandPool, 1, &commandBuffer);
}





/*
* PRIVATE METHOD DEFINITIONS
*/

void Buffer::_create_buffer(VkBufferUsageFlags usage, VkMemoryPropertyFlags memPropFlags)
{
	VkBufferCreateInfo bufferInfo{};
	_configure_buffer(&bufferInfo, usage);

	if (vkCreateBuffer(_deviceHandle, &bufferInfo, nullptr, &_buf) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create buffer");
	}

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(_deviceHandle, _buf, &memRequirements);

	VkMemoryAllocateInfo memAllocInfo{};
	_configure_mem_alloc(&memAllocInfo, memRequirements, memPropFlags);

	if (vkAllocateMemory(_deviceHandle, &memAllocInfo, nullptr, &_bufMem) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to allocate buffer memory");
	}

	vkBindBufferMemory(_deviceHandle, _buf, _bufMem, 0);
}





/*
* PRIVATE CONST METHOD DEFINITIONS
*/

uint32_t Buffer::_find_memory_type(uint32_t typeMask, VkMemoryPropertyFlags memPropFlags) const
{
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(_physicalDeviceHandle, &memProperties);

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) 
	{
		if ((typeMask & (1 << i)) && 
			(memProperties.memoryTypes[i].propertyFlags & memPropFlags) == memPropFlags) 
		{
			return i;
		}
	}

	throw std::runtime_error("Failed to find memory type for buffer");
}

void Buffer::_configure_buffer(VkBufferCreateInfo* pCreateInfo, VkBufferUsageFlags usage) const
{
	memset(pCreateInfo, 0, sizeof(VkBufferCreateInfo));
	pCreateInfo->sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	pCreateInfo->size = _bufSize;
	pCreateInfo->usage = usage;
	pCreateInfo->sharingMode = VK_SHARING_MODE_EXCLUSIVE;
}

void Buffer::_configure_mem_alloc(VkMemoryAllocateInfo* pAllocInfo, VkMemoryRequirements memRequirements, VkMemoryPropertyFlags memPropFlags) const
{
	memset(pAllocInfo, 0, sizeof(VkMemoryAllocateInfo));
	pAllocInfo->sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	pAllocInfo->allocationSize = memRequirements.size;
	pAllocInfo->memoryTypeIndex = _find_memory_type(memRequirements.memoryTypeBits, memPropFlags);
}