#pragma once

#include <vulkan/vulkan.h>

template <typename T> class VulkanObject
{
public:

	/*
	* TYPEDEFS
	*/

	using Handle = T;



	/*
	* CTORS
	*/

	VulkanObject()
		: _handle(VK_NULL_HANDLE),
		_deviceHandle(VK_NULL_HANDLE)
	{
	}

	VulkanObject(VkDevice deviceHandle)
		: _handle(VK_NULL_HANDLE),
		_deviceHandle(deviceHandle)
	{
	}

	VulkanObject(T handle, VkDevice deviceHandle)
		: _handle(handle),
		_deviceHandle(deviceHandle)
	{
	}

	VulkanObject(const VulkanObject& other)
		: _handle(other._handle),
		_deviceHandle(other._deviceHandle)
	{
	}



	/*
	* PUBLIC CONST METHODS
	*/

	/* @brief Returns a handle to the vulkan object
	*/
	inline virtual Handle handle() const { return _handle; }

protected:

	/* The handle to the vulkan object
	*/
	Handle _handle;

	/* The logical device being used with the object
	*/
	VkDevice _deviceHandle;

};

