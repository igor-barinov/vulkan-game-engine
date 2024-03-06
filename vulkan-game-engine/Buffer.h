#pragma once

#include <vulkan/vulkan.h>

#include <algorithm>

#include "VulkanDevice.h"

class Buffer
{
public:

	enum Type
	{
		STAGING,
		VERTEX,
		INDEX,
		NONE
	};

	using Handle = VkBuffer;

	friend void swap(Buffer& bufA, Buffer& bufB)
	{
		using std::swap;

		swap(bufA._buf, bufB._buf);
		swap(bufA._bufMem, bufB._bufMem);
		swap(bufA._bufSize, bufB._bufSize);
		swap(bufA._deviceHandle, bufB._deviceHandle);
		swap(bufA._physicalDeviceHandle, bufB._physicalDeviceHandle);
	}

	Buffer();
	Buffer(const VulkanDevice& device, Buffer::Type bufferType, size_t bufferSize);
	Buffer(const Buffer& other);
	Buffer(Buffer&& other) noexcept;
	Buffer& operator=(Buffer other);
	~Buffer();

	void map_host_data(const void* data);
	void copy_to(Buffer& destBuf, VkCommandPool commandPool, VkQueue graphicsQueue);

	inline Handle handle() const { return _buf; }

private:
	Handle _buf;
	VkDeviceMemory _bufMem;
	VkDeviceSize _bufSize;
	VkDevice _deviceHandle;
	VkPhysicalDevice _physicalDeviceHandle;

	void _create_buffer(VkBufferUsageFlags usage, VkMemoryPropertyFlags memPropFlags);

	uint32_t _find_memory_type(uint32_t typeMask, VkMemoryPropertyFlags memPropFlags) const;
	void _configure_buffer(VkBufferCreateInfo* pCreateInfo, VkBufferUsageFlags usage) const;
	void _configure_mem_alloc(VkMemoryAllocateInfo* pAllocInfo, VkMemoryRequirements memRequirements, VkMemoryPropertyFlags memPropFlags) const;
};

