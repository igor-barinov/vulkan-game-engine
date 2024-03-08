#pragma once

#include <vulkan/vulkan.h>

#include <algorithm>

#include "VulkanDevice.h"

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
	Buffer(const VulkanDevice& device, Buffer::Type bufferType, size_t bufferSize);
	Buffer(const Buffer& other);
	Buffer(Buffer&& other) noexcept;
	Buffer& operator=(Buffer other);
	~Buffer();



	/*
	* PUBLIC METHODS
	*/

	/* @brief Maps and copies the given data from CPU to GPU
	*/
	void map_host_data(const void* data);

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

	/* @brief Retrieves the memory type for the given mask and flags
	*/
	uint32_t _find_memory_type(uint32_t typeMask, VkMemoryPropertyFlags memPropFlags) const;

	/* @brief Fills struct with necessary info for creating a buffer
	*/
	void _configure_buffer(VkBufferCreateInfo* pCreateInfo, VkBufferUsageFlags usage) const;

	/* @brief Fills struct with necessary info for allocating buffer memory
	*/
	void _configure_mem_alloc(VkMemoryAllocateInfo* pAllocInfo, VkMemoryRequirements memRequirements, VkMemoryPropertyFlags memPropFlags) const;
};

