#pragma once

#include <vulkan/vulkan.h>

#include <algorithm>

#include "VulkanObject.h"
#include "Device.h"
#include "Buffer.h"
#include "PNGImage.h"
#include "CommandPool.h"
#include "Image.h"

class Texture : public Image
{
public:

	Texture();
	Texture(PNGImage& texture, Device& device, CommandPool& cmdPool);
	Texture(const Texture& other);
	Texture(Texture&& other) noexcept;
	Texture& operator=(Texture other);
	~Texture();

private:

	static constexpr VkFormat _IMAGE_FORMAT = VK_FORMAT_R8G8B8A8_SRGB;

	void _record_image_layout_transition(VkImageLayout oldLayout, VkImageLayout newLayout, const CommandPool& commandPool, VkQueue graphicsQueue);
	void _record_copy_image_to_buffer(VkBuffer stagingBuffer, const CommandPool& commandPool, VkQueue graphicsQueue);

};