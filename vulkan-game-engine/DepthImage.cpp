#include "DepthImage.h"

DepthImage::DepthImage()
	: Image()
{
}

DepthImage::DepthImage(const Device& device, const SwapChain& swapChain)
	: Image(device, {
        swapChain.surface_extent().width,
        swapChain.surface_extent().height,
        Device::select_supported_depth_format(device.get_physical_device(), SwapChain::available_depth_formats(), VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT),
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        VK_IMAGE_ASPECT_DEPTH_BIT
        })
{
}

DepthImage::DepthImage(const DepthImage& other)
    : Image(other)
{
}

DepthImage::DepthImage(DepthImage&& other) noexcept
    : Image(std::move(other))
{
}

DepthImage& DepthImage::operator=(DepthImage other)
{
    swap(*this, other);
    return *this;
}

DepthImage::~DepthImage()
{
}