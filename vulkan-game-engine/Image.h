#pragma once

#include <algorithm>

#include "VulkanObject.h"
#include "Device.h"

class Image : public VulkanObject<VkImage>
{
public:

	struct ImageProperties
	{
		uint32_t width;
		uint32_t height;
		VkFormat format;
		VkImageTiling tiling;
		VkImageUsageFlags usage;
		VkImageAspectFlags aspect;
	};

	friend void swap(Image& imgA, Image& imgB)
	{
		using std::swap;

		swap(imgA._handle, imgB._handle);
		swap(imgA._deviceHandle, imgB._deviceHandle);
		swap(imgA._imageMemory, imgB._imageMemory);
		swap(imgA._physicalDeviceHandle, imgB._physicalDeviceHandle);
		swap(imgA._imageView, imgB._imageView);
		swap(imgA._props, imgB._props);
	}

	Image();
	Image(const Device& device, ImageProperties properties);
	Image(const Image& other);
	Image(Image&& other) noexcept;
	Image& operator=(Image other);
	virtual ~Image();

	inline VkImageView get_image_view() const { return _imageView; }
	inline ImageProperties properties() const { return _props; }
	inline uint32_t width() const { return _props.width; }
	inline uint32_t height() const { return _props.height; }

protected:
	VkDeviceMemory _imageMemory;
	VkPhysicalDevice _physicalDeviceHandle;
	VkImageView _imageView;
	ImageProperties _props;

	void _configure_image(VkImageCreateInfo* pCreateInfo) const;
	void _configure_alloc(VkMemoryAllocateInfo* pAllocInfo, VkMemoryRequirements requirements, VkMemoryPropertyFlags properties) const;
	void _configure_image_view(VkImageViewCreateInfo* pCreateInfo) const;
};

