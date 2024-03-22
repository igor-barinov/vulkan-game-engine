#include "Device.h"
#include <stdexcept>

/*
* STATIC METHOD DEFINITIONS
*/

uint32_t Device::find_memory_type(VkPhysicalDevice physicalDevice, uint32_t typeMask, VkMemoryPropertyFlags memPropFlags)
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

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

VkFormat Device::select_supported_depth_format(VkPhysicalDevice physicalDevice, const std::vector<VkFormat>& availableFormats, VkImageTiling tilingType, VkFormatFeatureFlags featureFlags)
{
    for (VkFormat format : availableFormats)
    {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);

        if (tilingType == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & featureFlags) == featureFlags)
        {
            return format;
        }
        else if (tilingType == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & featureFlags) == featureFlags)
        {
            return format;
        }
    }

    throw std::runtime_error("Failed to find a depth format");
}





/*
* CTORS / ASSIGNMENT DEFINITIONS
*/

Device::Device()
    : _logicalDevice(VK_NULL_HANDLE),
    _physicalDevice(VK_NULL_HANDLE),
    _physicalProps({}),
    _queueFamilyInfo({}),
    _extensions({})
{
}

Device::Device(VkPhysicalDevice physicalDevice, const QueueFamilyInfo& queueFamilyInfo, const std::vector<const char*>& deviceExtensions, const std::vector<const char*>& validationLayers)
	: _logicalDevice(VK_NULL_HANDLE),
    _physicalDevice(physicalDevice),
	_queueFamilyInfo(queueFamilyInfo),
    _extensions(deviceExtensions)
{
    VkDeviceCreateInfo createInfo{};
    VkPhysicalDeviceFeatures deviceFeatures{};
    deviceFeatures.samplerAnisotropy = true;

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    _configure_logical_device(&createInfo, &deviceFeatures, queueCreateInfos, validationLayers);

    if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &_logicalDevice) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create logical device");
    }

    vkGetPhysicalDeviceProperties(physicalDevice, &_physicalProps);

    _queueFamilyInfo.load_handles(_logicalDevice);
}

Device::Device(const Device& other)
    : _logicalDevice(other._logicalDevice), 
    _physicalDevice(other._physicalDevice), 
    _physicalProps(other._physicalProps),
    _queueFamilyInfo(other._queueFamilyInfo)
{
}

Device::Device(Device&& other) noexcept
    : Device()
{
    swap(*this, other);
}

Device& Device::operator=(Device other)
{
    swap(*this, other);
    return *this;
}

Device::~Device()
{
    vkDestroyDevice(_logicalDevice, nullptr);
}





/*
* PRIVATE CONST METHOD DEFINITIONS
*/

void Device::_configure_logical_device(
    VkDeviceCreateInfo* pDeviceCreateInfo, 
    VkPhysicalDeviceFeatures* pDeviceFeatures,
    std::vector<VkDeviceQueueCreateInfo>& queueCreateInfos, 
    const std::vector<const char*>& validationLayers) const
{
    const auto& uniqueQueueFamilies = _queueFamilyInfo.unique_queue_family_indices();

    float queuePriority = 1.0f;
    for (uint32_t queueFamily : uniqueQueueFamilies) {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }


    memset(pDeviceCreateInfo, 0, sizeof(VkDeviceCreateInfo));
    pDeviceCreateInfo->sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

    pDeviceCreateInfo->queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    pDeviceCreateInfo->pQueueCreateInfos = queueCreateInfos.data();

    pDeviceCreateInfo->pEnabledFeatures = pDeviceFeatures;

    pDeviceCreateInfo->enabledExtensionCount = static_cast<uint32_t>(_extensions.size());
    pDeviceCreateInfo->ppEnabledExtensionNames = _extensions.data();

    if (validationLayers.size() > 0) {
        pDeviceCreateInfo->enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        pDeviceCreateInfo->ppEnabledLayerNames = validationLayers.data();
    }
    else {
        pDeviceCreateInfo->enabledLayerCount = 0;
    }
}