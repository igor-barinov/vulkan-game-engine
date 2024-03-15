#pragma once

#include <vulkan/vulkan.h>
#include <algorithm>

#include "QueueFamily.h"

/*
* Class describing physical and logical devices and related queue families
*/
class VulkanDevice
{
public:

	/*
	* TYPEDEFS
	*/

	using Handle = VkDevice;



	/*
	* PUBLIC FRIEND METHODS
	*/

	/*
	*  @brief Swap implementation for VulkanDevice class
	*/
	friend void swap(VulkanDevice& deviceA, VulkanDevice& deviceB)
	{
		using std::swap;

		swap(deviceA._logicalDevice, deviceB._logicalDevice);
		swap(deviceA._physicalDevice, deviceB._physicalDevice);
		swap(deviceA._physicalProps, deviceB._physicalProps);
		swap(deviceA._queueFamilyInfo, deviceB._queueFamilyInfo);
	}

	/*
	* PUBLIC STATIC METHODS
	*/

	static uint32_t find_memory_type(VkPhysicalDevice physicalDevice, uint32_t typeMask, VkMemoryPropertyFlags memPropFlags);



	/*
	* CTORS / ASSIGNMENT
	*/

	VulkanDevice();

	/*
	* @param physcialDevice Handle to physical device to creat logical device from
	* @param queueFamilyInfo Queue family info for the given physical device
	* @param deviceExtensions List of extension names for device to support
	* @param validationLayers List of validation layer names for device to support
	*/
	VulkanDevice(VkPhysicalDevice physicalDevice, const QueueFamilyInfo& queueFamilyInfo, const std::vector<const char*>& deviceExtensions, const std::vector<const char*>& validationLayers = {});
	VulkanDevice(const VulkanDevice& other);
	VulkanDevice(VulkanDevice&& other) noexcept;
	VulkanDevice& operator=(VulkanDevice other);
	~VulkanDevice();



	/*
	* PUBLIC CONST METHODS
	*/

	/* @brief Returns handle to logical device
	*/
	inline Handle handle() const { return _logicalDevice; }

	/* @brief Returns handle to physical device
	*/
	inline VkPhysicalDevice get_physical_device() const { return _physicalDevice; }

	/* @brief Returns queue family info
	*/
	inline const QueueFamilyInfo& queue_family_info() const { return _queueFamilyInfo; }

	/* @brief Returns the device extensions being used
	*/
	inline const std::vector<const char*>& extensions() const { return _extensions; }

	/* @brief Returns the physical device properties
	*/
	inline VkPhysicalDeviceProperties physical_properties() const { return _physicalProps; }

private:

	/*
	* PRIVATE MEMBERS
	*/

	/* Handle to logical device
	*/
	Handle _logicalDevice;

	/* Handle to physical device
	*/
	VkPhysicalDevice _physicalDevice;

	/* Physical device properties
	*/
	VkPhysicalDeviceProperties _physicalProps;

	/* Queue family info for device
	*/
	QueueFamilyInfo _queueFamilyInfo;

	/* Device extensions being used
	*/
	std::vector<const char*> _extensions;



	/*
	* PRIVATE CONST METHODS
	*/

	/* @brief Fills struct with info necessary for logical device creation
	* 
	* @param[out] pDeviceCreateInfo Pointer to struct to configure
	* @param deviceExtensions List of extension names for device to support
	* @param validationLayers List of validation layer names for device to support
	*/
	void _configure_logical_device(
		VkDeviceCreateInfo* pDeviceCreateInfo, 
		VkPhysicalDeviceFeatures* pDeviceFeatures,
		std::vector<VkDeviceQueueCreateInfo>& queueCreateInfos,
		const std::vector<const char*>& validationLayers) const;
};

