#include "Texture.h"

Texture::Texture()
    : Image()
{
}

Texture::Texture(PNGImage& texture, Device& device, CommandPool& cmdPool)
    : Image(device, {
        texture.width(),
        texture.height(),
        _IMAGE_FORMAT,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_IMAGE_ASPECT_COLOR_BIT
        })
{
    
    size_t imageSize = _props.width * _props.height * sizeof(PNGImage::pixel_bits_t);
    Buffer stagingBuffer(device, Buffer::Type::STAGING, imageSize);
    stagingBuffer.copy_to_mapped_mem(texture.data());

    const auto& queueFamilyInfo = device.queue_family_info();
    auto graphicsQueue = queueFamilyInfo.get_queue_handle(QueueFamilyType::Graphics);
    _record_image_layout_transition(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, cmdPool, graphicsQueue);
    _record_copy_image_to_buffer(stagingBuffer.handle(), cmdPool, graphicsQueue);
    _record_image_layout_transition(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, cmdPool, graphicsQueue);
}

Texture::Texture(const Texture& other)
    : Image(other)
{
}

Texture::Texture(Texture&& other) noexcept
    : Image(std::move(other))
{
}

Texture& Texture::operator=(Texture other)
{
    swap(*this, other);
    return *this;
}

Texture::~Texture()
{
}

void Texture::_record_image_layout_transition(VkImageLayout oldLayout, VkImageLayout newLayout, const CommandPool& commandPool, VkQueue graphicsQueue)
{
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    CommandBufferPool commandBuffer(_deviceHandle, 1, commandPool.handle());
    commandBuffer.begin_all(beginInfo);
    VkCommandBuffer cmdBufHandle = commandBuffer[0];

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = _handle;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else
    {
        throw std::invalid_argument("Image layout transition is unsupported");
    }

    vkCmdPipelineBarrier(
        cmdBufHandle,
        sourceStage, destinationStage,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier
    );

    commandBuffer.end_all();
    commandBuffer.submit_all_to_queue(graphicsQueue);
}

void Texture::_record_copy_image_to_buffer(VkBuffer stagingBuffer, const CommandPool& commandPool, VkQueue graphicsQueue)
{
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    CommandBufferPool commandBuffer(_deviceHandle, 1, commandPool.handle());
    commandBuffer.begin_all(beginInfo);
    VkCommandBuffer cmdBufHandle = commandBuffer[0];


    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = { 0, 0, 0 };
    region.imageExtent = {
        _props.width,
        _props.height,
        1
    };

    vkCmdCopyBufferToImage(cmdBufHandle, stagingBuffer, _handle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    commandBuffer.end_all();
    commandBuffer.submit_all_to_queue(graphicsQueue);
}