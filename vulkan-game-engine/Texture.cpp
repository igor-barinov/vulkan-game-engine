#include "Texture.h"

Texture::Texture()
	: _image(VK_NULL_HANDLE),
	_imageMem(VK_NULL_HANDLE),
    _imageView(VK_NULL_HANDLE),
    _imgWidth(0),
    _imgHeight(0),
    _imgChannels(0),
    _stagingBuf({}),
    _deviceHandle(VK_NULL_HANDLE),
    _physicalDeviceHandle(VK_NULL_HANDLE)
{
}

Texture::Texture(PNGImage& texture, VulkanDevice& device, CommandPool& cmdPool)
    : _image(VK_NULL_HANDLE),
    _imageMem(VK_NULL_HANDLE),
    _imageView(VK_NULL_HANDLE),
    _imgWidth(texture.width()),
    _imgHeight(texture.height()),
    _imgChannels(texture.channels()),
    _stagingBuf({}),
    _deviceHandle(device.handle()),
    _physicalDeviceHandle(device.get_physical_device())
{
    size_t imageSize = _imgWidth * _imgHeight * sizeof(PNGImage::pixel_bits_t);
    _stagingBuf = Buffer(device, Buffer::Type::STAGING, imageSize);
    _stagingBuf.copy_to_mapped_mem(texture.data());

    VkImageCreateInfo createInfo{};
    _configure_image(&createInfo);

    if (vkCreateImage(_deviceHandle, &createInfo, nullptr, &_image) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create texture handle");
    }

    VkMemoryAllocateInfo allocInfo{};
    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(_deviceHandle, _image, &memRequirements);

    _configure_image_memory(&allocInfo, memRequirements, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    if (vkAllocateMemory(_deviceHandle, &allocInfo, nullptr, &_imageMem) != VK_SUCCESS) 
    {
        throw std::runtime_error("failed to allocate image memory!");
    }
    vkBindImageMemory(_deviceHandle, _image, _imageMem, 0);

    const auto& queueFamilyInfo = device.queue_family_info();
    auto graphicsQueue = queueFamilyInfo.get_queue_handle(QueueFamilyType::Graphics);
    cmdPool.record_image_layout_transition(_image, _IMAGE_FORMAT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, graphicsQueue);
    cmdPool.record_copy_image_to_buffer(_stagingBuf.handle(), _image, _imgWidth, _imgHeight, graphicsQueue);
    cmdPool.record_image_layout_transition(_image, _IMAGE_FORMAT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, graphicsQueue);

    VkImageViewCreateInfo viewInfo{};
    _configure_image_view(&viewInfo);

    if (vkCreateImageView(_deviceHandle, &viewInfo, nullptr, &_imageView) != VK_SUCCESS) 
    {
        throw std::runtime_error("Failed to create texture image view");
    }
}

Texture::Texture(const Texture& other)
	: _image(other._image),
	_imageMem(other._imageMem),
    _imageView(other._imageView),
    _imgWidth(other._imgWidth),
    _imgHeight(other._imgHeight),
    _imgChannels(other._imgChannels),
    _stagingBuf(other._stagingBuf),
    _deviceHandle(other._deviceHandle),
    _physicalDeviceHandle(other._physicalDeviceHandle)
{
}

Texture::Texture(Texture&& other) noexcept
	: Texture()
{
	swap(*this, other);
}

Texture& Texture::operator=(Texture other)
{
	swap(*this, other);
	return *this;
}

Texture::~Texture()
{
    if (_image != VK_NULL_HANDLE)
    {
        vkDestroyImage(_deviceHandle, _image, nullptr);
        vkFreeMemory(_deviceHandle, _imageMem, nullptr);
    }
}

void Texture::_configure_image(VkImageCreateInfo* pCreateInfo) const
{
    memset(pCreateInfo, 0, sizeof(VkImageCreateInfo));
    pCreateInfo->sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    pCreateInfo->imageType = VK_IMAGE_TYPE_2D;
    pCreateInfo->extent.width = _imgWidth;
    pCreateInfo->extent.height = _imgHeight;
    pCreateInfo->extent.depth = 1;
    pCreateInfo->mipLevels = 1;
    pCreateInfo->arrayLayers = 1;
    pCreateInfo->format = _IMAGE_FORMAT;
    pCreateInfo->tiling = VK_IMAGE_TILING_OPTIMAL;
    pCreateInfo->initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    pCreateInfo->usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    pCreateInfo->samples = VK_SAMPLE_COUNT_1_BIT;
    pCreateInfo->sharingMode = VK_SHARING_MODE_EXCLUSIVE;
}

void Texture::_configure_image_memory(VkMemoryAllocateInfo* pAllocInfo, VkMemoryRequirements requirements, VkMemoryPropertyFlags properties) const
{
    memset(pAllocInfo, 0, sizeof(VkMemoryAllocateInfo));
    pAllocInfo->sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    pAllocInfo->allocationSize = requirements.size;
    pAllocInfo->memoryTypeIndex =  VulkanDevice::find_memory_type(_physicalDeviceHandle, requirements.memoryTypeBits, properties);
}

void Texture::_configure_image_view(VkImageViewCreateInfo* pCreateInfo) const
{
    memset(pCreateInfo, 0, sizeof(VkImageViewCreateInfo));
    pCreateInfo->sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    pCreateInfo->image = _image;
    pCreateInfo->viewType = VK_IMAGE_VIEW_TYPE_2D;
    pCreateInfo->format = _IMAGE_FORMAT;
    pCreateInfo->subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    pCreateInfo->subresourceRange.baseMipLevel = 0;
    pCreateInfo->subresourceRange.levelCount = 1;
    pCreateInfo->subresourceRange.baseArrayLayer = 0;
    pCreateInfo->subresourceRange.layerCount = 1;
}