#pragma once

#include <vulkan/vulkan.h>

#include <algorithm>

#include "Device.h"

/*
* Class implementing a Vulkan buffer
*/
class Buffer
{
public:

	/*
	* ENUMS / TYPEDEFS
	*/

	enum Type
	{
		STAGING,
		VERTEX,
		INDEX,
		UNIFORM,
		NONE
	};

	using Handle = VkBuffer;



	/*
	* PUBLIC FRIEND METHODS
	*/

	/* @brief Swap implementation for Buffer class
	*/
	friend void swap(Buffer& bufA, Buffer& bufB)
	{
		using std::swap;

		swap(bufA._buf, bufB._buf);
		swap(bufA._usageFlags, bufB._usageFlags);
		swap(bufA._memFlags, bufB._memFlags);
		swap(bufA._bufMem, bufB._bufMem);
		swap(bufA._bufSize, bufB._bufSize);
		swap(bufA._deviceHandle, bufB._deviceHandle);
		swap(bufA._physicalDeviceHandle, bufB._physicalDeviceHandle);
	}



	/*
	* CTORS / ASSIGNMENT
	*/

	Buffer();

	/*
	* @param device Device that will use buffer
	* @param bufferType The type of buffer to create
	* @param bufferSize The size required to allocate buffer
	*/
	Buffer(const Device& device, Buffer::Type bufferType, size_t bufferSize);
	Buffer(const Buffer& other);
	Buffer(Buffer&& other) noexcept;
	Buffer& operator=(Buffer other);
	~Buffer();



	/*
	* PUBLIC METHODS
	*/

	/* @brief Maps host data to GPU memory
	*/
	inline void map_memory(void** pData) { vkMapMemory(_deviceHandle, _bufMem, 0, _bufSize, 0, pData); }

	/* @brief Unmaps GPU memory
	*/
	inline void unmap_memory() { vkUnmapMemory(_deviceHandle, _bufMem); }

	/* @brief Copies the given data into mapped memory
	*/
	void copy_to_mapped_mem(const void* data);

	/* @brief Copies data from this buffer to the given buffer
	*/
	void copy_to(Buffer& destBuf, VkCommandPool commandPool, VkQueue graphicsQueue);



	/*
	* PUBLIC CONST METHODS
	*/

	/* @brief Returns a handle to the buffer object
	*/
	inline Handle handle() const { return _buf; }

private:

	/*
	* PRIVATE MEMBERS
	*/

	/* Handle to buffer object
	*/
	Handle _buf;

	/* Buffer usage flags
	*/
	VkBufferUsageFlags _usageFlags;

	/* Memory property flags
	*/
	VkMemoryPropertyFlags _memFlags;

	/* Handle to device memory
	*/
	VkDeviceMemory _bufMem;

	/* Buffer size
	*/
	VkDeviceSize _bufSize;

	/* Handle to logical device
	*/
	VkDevice _deviceHandle;

	/* Handle to physical device
	*/
	VkPhysicalDevice _physicalDeviceHandle;



	/*
	* PRIVATE METHODS
	*/

	/* @brief Creates a buffer with the given flags
	*/
	void _create_buffer(VkBufferUsageFlags usage, VkMemoryPropertyFlags memPropFlags);



	/*
	* PRIVATE CONST METHODS
	*/

	/* @brief Fills struct with necessary info for creating a buffer
	*/
	void _configure_buffer(VkBufferCreateInfo* pCreateInfo, VkBufferUsageFlags usage) const;

	/* @brief Fills struct with necessary info for allocating buffer memory
	*/
	void _configure_mem_alloc(VkMemoryAllocateInfo* pAllocInfo, VkMemoryRequirements memRequirements, VkMemoryPropertyFlags memPropFlags) const;
};

