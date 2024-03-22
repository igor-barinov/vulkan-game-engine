#include "Image.h"

Image::Image()
	: VulkanObject(),
	_imageMemory(VK_NULL_HANDLE),
	_physicalDeviceHandle(VK_NULL_HANDLE),
	_imageView(VK_NULL_HANDLE),
	_props({})
{
}

Image::Image(const Device& device, ImageProperties properties)
	: VulkanObject(device.handle()),
	_imageMemory(VK_NULL_HANDLE),
	_physicalDeviceHandle(device.get_physical_device()),
	_imageView(VK_NULL_HANDLE),
	_props(properties)
{
	VkImageCreateInfo imageInfo{};
	_configure_image(&imageInfo);

	if (vkCreateImage(_deviceHandle, &imageInfo, nullptr, &_handle) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create image handle");
	}

	VkMemoryAllocateInfo allocInfo{};
	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(_deviceHandle, _handle, &memRequirements);

	_configure_alloc(&allocInfo, memRequirements, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	if (vkAllocateMemory(_deviceHandle, &allocInfo, nullptr, &_imageMemory) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to allocate image memory");
	}

	vkBindImageMemory(_deviceHandle, _handle, _imageMemory, 0);

	VkImageViewCreateInfo imgViewInfo{};
	_configure_image_view(&imgViewInfo);

	if (vkCreateImageView(_deviceHandle, &imgViewInfo, nullptr, &_imageView) != VK_SUCCESS) 
	{
		throw std::runtime_error("Failed to create image view");
	}
}

Image::Image(const Image& other)
	: VulkanObject(other),
	_imageMemory(other._imageMemory),
	_physicalDeviceHandle(other._physicalDeviceHandle),
	_imageView(other._imageView),
	_props(other._props)
{
}

Image::Image(Image&& other) noexcept
	: Image()
{
	swap(*this, other);
}

Image& Image::operator=(Image other)
{
	swap(*this, other);
	return *this;
}

Image::~Image()
{
	if (_handle != VK_NULL_HANDLE)
	{
		vkDestroyImageView(_deviceHandle, _imageView, nullptr);
		vkDestroyImage(_deviceHandle, _handle, nullptr);
		vkFreeMemory(_deviceHandle, _imageMemory, nullptr);

		_handle = VK_NULL_HANDLE;
		_imageView = VK_NULL_HANDLE;
	}
}

void Image::_configure_image(VkImageCreateInfo* pCreateInfo) const
{
	memset(pCreateInfo, 0, sizeof(VkImageCreateInfo));
	pCreateInfo->sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	pCreateInfo->imageType = VK_IMAGE_TYPE_2D;
	pCreateInfo->extent.width = _props.width;
	pCreateInfo->extent.height = _props.height;
	pCreateInfo->extent.depth = 1;
	pCreateInfo->mipLevels = 1;
	pCreateInfo->arrayLayers = 1;
	pCreateInfo->format = _props.format;
	pCreateInfo->tiling = _props.tiling;
	pCreateInfo->initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	pCreateInfo->usage = _props.usage;
	pCreateInfo->samples = VK_SAMPLE_COUNT_1_BIT;
	pCreateInfo->sharingMode = VK_SHARING_MODE_EXCLUSIVE;
}

void Image::_configure_alloc(VkMemoryAllocateInfo* pAllocInfo, VkMemoryRequirements requirements, VkMemoryPropertyFlags properties) const
{
	memset(pAllocInfo, 0, sizeof(VkMemoryAllocateInfo));
	pAllocInfo->sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	pAllocInfo->allocationSize = requirements.size;
	pAllocInfo->memoryTypeIndex = Device::find_memory_type(_physicalDeviceHandle, requirements.memoryTypeBits, properties);
}

void Image::_configure_image_view(VkImageViewCreateInfo* pCreateInfo) const
{
	memset(pCreateInfo, 0, sizeof(VkImageViewCreateInfo));
	pCreateInfo->sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	pCreateInfo->image = _handle;
	pCreateInfo->viewType = VK_IMAGE_VIEW_TYPE_2D;
	pCreateInfo->format = _props.format;
	pCreateInfo->subresourceRange.aspectMask = _props.aspect;
	pCreateInfo->subresourceRange.baseMipLevel = 0;
	pCreateInfo->subresourceRange.levelCount = 1;
	pCreateInfo->subresourceRange.baseArrayLayer = 0;
	pCreateInfo->subresourceRange.layerCount = 1;
}