#include "Buffer.h"

#include "CommandBufferPool.h"

/*
* CTORS / ASSIGNMENT DEFINITIONS
*/

Buffer::Buffer()
	: VulkanObject(),
	_usageFlags(0),
	_memFlags(0),
	_bufMem(VK_NULL_HANDLE),
	_bufSize(0),
	_physicalDeviceHandle(VK_NULL_HANDLE)
{
}

Buffer::Buffer(const Device& device, Buffer::Type bufferType, size_t bufferSize)
	: VulkanObject(device.handle()),
	_usageFlags(0),
	_memFlags(0),
	_bufMem(VK_NULL_HANDLE),
	_bufSize(bufferSize),
	_physicalDeviceHandle(device.get_physical_device())
{
	switch (bufferType)
	{
	case Buffer::STAGING:
		_usageFlags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		_memFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
		break;
	case Buffer::VERTEX:
		_usageFlags = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		_memFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
		break;
	case Buffer::INDEX:
		_usageFlags = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
		_memFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
		break;
	case Buffer::UNIFORM:
		_usageFlags = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		_memFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
		break;
	}

	_create_buffer(_usageFlags, _memFlags);
}

Buffer::Buffer(const Buffer& other)
	: VulkanObject(other),
	_usageFlags(other._usageFlags),
	_memFlags(other._memFlags),
	_bufMem(VK_NULL_HANDLE),
	_bufSize(other._bufSize),
	_physicalDeviceHandle(other._physicalDeviceHandle)
{
	_create_buffer(_usageFlags, _memFlags);
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
	if (_handle != VK_NULL_HANDLE)
	{
		vkDestroyBuffer(_deviceHandle, _handle, nullptr);
		vkFreeMemory(_deviceHandle, _bufMem, nullptr);
		_handle = VK_NULL_HANDLE;
		_bufMem = VK_NULL_HANDLE;
	}
}





/*
* PUBLIC METHOD DEFINITIONS
*/

void Buffer::copy_to_mapped_mem(const void* data)
{
	void* mappedData;
	map_memory(&mappedData);
	memcpy(mappedData, data, (size_t)_bufSize);
	unmap_memory();
}

void Buffer::copy_to(Buffer& destBuf, VkCommandPool commandPool, VkQueue graphicsQueue)
{
	CommandBufferPool commandBuffer(_deviceHandle, 1, commandPool);
	auto cmdBufHandle = commandBuffer[0];

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	commandBuffer.begin_all(beginInfo);

	VkBufferCopy copyRegion{};
	copyRegion.size = _bufSize;
	vkCmdCopyBuffer(cmdBufHandle, _handle, destBuf._handle, 1, &copyRegion);

	commandBuffer.end_all();
	commandBuffer.submit_all_to_queue(graphicsQueue);
}





/*
* PRIVATE METHOD DEFINITIONS
*/

void Buffer::_create_buffer(VkBufferUsageFlags usage, VkMemoryPropertyFlags memPropFlags)
{
	VkBufferCreateInfo bufferInfo{};
	_configure_buffer(&bufferInfo, usage);

	if (vkCreateBuffer(_deviceHandle, &bufferInfo, nullptr, &_handle) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create buffer");
	}

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(_deviceHandle, _handle, &memRequirements);

	VkMemoryAllocateInfo memAllocInfo{};
	_configure_mem_alloc(&memAllocInfo, memRequirements, memPropFlags);

	if (vkAllocateMemory(_deviceHandle, &memAllocInfo, nullptr, &_bufMem) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to allocate buffer memory");
	}

	vkBindBufferMemory(_deviceHandle, _handle, _bufMem, 0);
}





/*
* PRIVATE CONST METHOD DEFINITIONS
*/

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
	pAllocInfo->memoryTypeIndex = Device::find_memory_type(_physicalDeviceHandle, memRequirements.memoryTypeBits, memPropFlags);
}