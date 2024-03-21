#pragma once

#include <vulkan/vulkan.h>

#include <algorithm>

#include "Device.h"
#include "Buffer.h"
#include "PNGImage.h"
#include "CommandPool.h"

class Texture
{
public:

	using Handle = VkImage;

	friend void swap(Texture& texA, Texture& texB)
	{
		using std::swap;

		swap(texA._image, texB._image);
		swap(texA._imageMem, texB._imageMem);
		swap(texA._imageView, texB._imageView);
		swap(texA._imgWidth, texB._imgWidth);
		swap(texA._imgHeight, texB._imgHeight);
		swap(texA._imgChannels, texB._imgChannels);
		swap(texA._deviceHandle, texB._deviceHandle);
		swap(texA._physicalDeviceHandle, texB._physicalDeviceHandle);
	}

	Texture();
	Texture(PNGImage& texture, Device& device, CommandPool& cmdPool);
	Texture(const Texture& other);
	Texture(Texture&& other) noexcept;
	Texture& operator=(Texture other);
	~Texture();

	inline VkImageView get_image_view() const { return _imageView; }

private:

	static constexpr VkFormat _IMAGE_FORMAT = VK_FORMAT_R8G8B8A8_SRGB;

	Handle _image;
	VkDeviceMemory _imageMem;
	VkImageView _imageView;
	uint32_t _imgWidth;
	uint32_t _imgHeight;
	uint32_t _imgChannels;
	VkDevice _deviceHandle;
	VkPhysicalDevice _physicalDeviceHandle;

	void _configure_image(VkImageCreateInfo* pCreateInfo) const;
	void _configure_image_memory(VkMemoryAllocateInfo* pAllocInfo, VkMemoryRequirements requirements, VkMemoryPropertyFlags properties) const;
	void _configure_image_view(VkImageViewCreateInfo* pCreateInfo) const;

	void _record_image_layout_transition(VkImageLayout oldLayout, VkImageLayout newLayout, const CommandPool& commandPool, VkQueue graphicsQueue);
	void _record_copy_image_to_buffer(VkBuffer stagingBuffer, const CommandPool& commandPool, VkQueue graphicsQueue);

};