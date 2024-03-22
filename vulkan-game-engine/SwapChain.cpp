#include "SwapChain.h"
#include <stdexcept>
#include <algorithm>

/*
* STATIC METHOD DEFINITIONS
*/

bool SwapChain::query_support_for(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
{
    auto supportInfo = _get_capabilites_for(physicalDevice, surface);
    return !supportInfo.availableFormats.empty() && !supportInfo.availablePresentModes.empty();
}

SwapChain::_SwapChainSupport SwapChain::_get_capabilites_for(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
{
    _SwapChainSupport supportInfo{};
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &supportInfo.capabilities);

    uint32_t formatCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr);

    if (formatCount != 0) {
        supportInfo.availableFormats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, supportInfo.availableFormats.data());
    }

    uint32_t presentModeCount = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr);

    if (presentModeCount != 0) {
        supportInfo.availablePresentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, supportInfo.availablePresentModes.data());
    }

    return supportInfo;
}





/*
* CTORS / ASSIGNMENT DEFINITIONS
*/

SwapChain::SwapChain()
    : VulkanObject(),
    _supportInfo(_SwapChainSupport{}),
    _imageFormat(VkFormat{}),
    _extent(VkExtent2D{}),
    _chainImages({}),
    _chainImageViews({}),
    _frameBuffers({})
{
}

SwapChain::SwapChain(const Device& device, const Window& window, FormatFilter formatFilterFn, PresentModeFilter presentModeFilterFn)
	: VulkanObject(device.handle()),
    _supportInfo(_SwapChainSupport{}),
	_imageFormat(VkFormat{}),
	_extent(VkExtent2D{}),
    _chainImages({}),
    _chainImageViews({}),
    _frameBuffers({})
{
    _supportInfo = _get_capabilites_for(device.get_physical_device(), window.surface_handle());

    auto surfaceFormat = _choose_surface_format(formatFilterFn);
    auto presentMode = _choose_present_mode(presentModeFilterFn);
    auto extent = _choose_extent(window);
    _imageFormat = surfaceFormat.format;
    _extent = extent;

    uint32_t imageCount = _supportInfo.capabilities.minImageCount + 1;
    if (_supportInfo.capabilities.maxImageCount > 0 && imageCount > _supportInfo.capabilities.maxImageCount) {
        imageCount = _supportInfo.capabilities.maxImageCount;
    }

    const auto& queueFamilyInfo = device.queue_family_info();
    const auto& indices = queueFamilyInfo.queue_family_indices();
    bool doConcurrentSharing = queueFamilyInfo[QueueFamilyType::Graphics] != queueFamilyInfo[QueueFamilyType::Present];
    
    VkSwapchainCreateInfoKHR createInfo{};
    _configure_swap_chain(
        &createInfo,
        imageCount,
        surfaceFormat,
        presentMode,
        extent,
        doConcurrentSharing,
        indices,
        window.surface_handle(),
        formatFilterFn,
        presentModeFilterFn
    );

    if (vkCreateSwapchainKHR(_deviceHandle, &createInfo, nullptr, &_handle) != VK_SUCCESS) 
    {
        throw std::runtime_error("Failed to create swap chain");
    }
    
    vkGetSwapchainImagesKHR(_deviceHandle, _handle, &imageCount, nullptr);
    _chainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(_deviceHandle, _handle, &imageCount, _chainImages.data());

    _chainImageViews.resize(imageCount);
    

    for (size_t i = 0; i < _chainImages.size(); ++i)
    {
        VkImageViewCreateInfo viewCreateInfo{};
        _configure_image_view(&viewCreateInfo, _chainImages[i]);

        if (vkCreateImageView(_deviceHandle, &viewCreateInfo, nullptr, &_chainImageViews[i]) != VK_SUCCESS) 
        {
            throw std::runtime_error("Failed to create image views");
        }
    }
}

SwapChain::SwapChain(const SwapChain& other)
    : VulkanObject(other),
    _supportInfo(other._supportInfo),
    _imageFormat(other._imageFormat),
    _extent(other._extent),
    _chainImages(other._chainImages),
    _chainImageViews(other._chainImageViews)
{
}

SwapChain::SwapChain(SwapChain&& other) noexcept
    : SwapChain()
{
    swap(*this, other);
}

SwapChain& SwapChain::operator=(SwapChain other)
{
    swap(*this, other);
    return *this;
}

SwapChain::~SwapChain()
{
    if (_handle != VK_NULL_HANDLE)
    {
        for (auto frameBuffer : _frameBuffers)
        {
            vkDestroyFramebuffer(_deviceHandle, frameBuffer, nullptr);
        }

        for (auto imageView : _chainImageViews) 
        {
            vkDestroyImageView(_deviceHandle, imageView, nullptr);
        }

        vkDestroySwapchainKHR(_deviceHandle, _handle, nullptr);
    }
}





/*
* PUBLIC METHOD DEFINITIONS
*/
void SwapChain::init_framebuffers(VkRenderPass renderPass, VkImageView depthImageView)
{
    _frameBuffers.resize(_chainImageViews.size());
    for (size_t i = 0; i < _frameBuffers.size(); ++i)
    {
        VkFramebufferCreateInfo bufferCreateInfo{};
        std::vector<VkImageView> attachments = {
            _chainImageViews[i],
            depthImageView
        };

        _configure_frame_buffer(&bufferCreateInfo, renderPass, attachments);
        if (vkCreateFramebuffer(_deviceHandle, &bufferCreateInfo, nullptr, &_frameBuffers[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create framebuffer");
        }
    }
}

uint32_t SwapChain::get_next_image(VkSemaphore semaphore, bool& isOutofDate)
{
    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(_deviceHandle, _handle, UINT64_MAX, semaphore, VK_NULL_HANDLE, &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) 
    {
        isOutofDate = true;
    }

    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) 
    {
        throw std::runtime_error("Failed to acquire swap chain image");
    }

    isOutofDate = false;
    return imageIndex;    
}

bool SwapChain::present_image(VkQueue presentQueue, VkSemaphore* pWaitSemaphore, uint32_t* pImageIndex)
{
    VkPresentInfoKHR presentInfo{};
    _configure_present_info(&presentInfo, pWaitSemaphore, pImageIndex);

    VkResult result = vkQueuePresentKHR(presentQueue, &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
    {
        return true;
    }

    if (result != VK_SUCCESS) 
    {
        throw std::runtime_error("Failed to present swap chain image");
    }

    return false;
}





/*
* PRIVATE CONST METHOD DEFINITIONS
*/

VkSurfaceFormatKHR SwapChain::_choose_surface_format(FormatFilter formatFilterFn) const
{
    for (const auto& format : _supportInfo.availableFormats)
    {
        if (formatFilterFn(format))
        {
            return format;
        }
    }

    return _supportInfo.availableFormats.front();
}

VkPresentModeKHR SwapChain::_choose_present_mode(PresentModeFilter presentModeFilterFn) const
{
    for (const auto& mode : _supportInfo.availablePresentModes)
    {
        if (presentModeFilterFn(mode))
        {
            return mode;
        }
    }

    return _supportInfo.availablePresentModes.front();
}

VkExtent2D SwapChain::_choose_extent(const Window& window) const
{
    if (_supportInfo.capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
    {
        return _supportInfo.capabilities.currentExtent;
    }

    
    int width = 0, height = 0;
    glfwGetFramebufferSize(window.handle(), &width, &height);

    VkExtent2D actualExtent = {
        static_cast<uint32_t>(width),
        static_cast<uint32_t>(height)
    };

    actualExtent.width = std::clamp(actualExtent.width, _supportInfo.capabilities.minImageExtent.width, _supportInfo.capabilities.maxImageExtent.width);
    actualExtent.height = std::clamp(actualExtent.height, _supportInfo.capabilities.minImageExtent.height, _supportInfo.capabilities.maxImageExtent.height);

    return actualExtent;
}

void SwapChain::_configure_swap_chain(
    VkSwapchainCreateInfoKHR* pCreateInfo,
    uint32_t imageCount,
    VkSurfaceFormatKHR surfaceFormat,
    VkPresentModeKHR presentMode,
    VkExtent2D extent,
    bool doConcurrentSharing,
    const std::vector<uint32_t>& queueFamilyIndices,
    VkSurfaceKHR surface,
    FormatFilter formatFilterFn,
    PresentModeFilter presentModeFilterFn) const
{
    memset(pCreateInfo, 0, sizeof(VkSwapchainCreateInfoKHR));
    pCreateInfo->sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    pCreateInfo->surface = surface;

    pCreateInfo->minImageCount = imageCount;
    pCreateInfo->imageFormat = surfaceFormat.format;
    pCreateInfo->imageColorSpace = surfaceFormat.colorSpace;
    pCreateInfo->imageExtent = extent;
    pCreateInfo->imageArrayLayers = 1;
    pCreateInfo->imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    if (doConcurrentSharing) {
        pCreateInfo->imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        pCreateInfo->queueFamilyIndexCount = 2;
        pCreateInfo->pQueueFamilyIndices = queueFamilyIndices.data();
    }
    else {
        pCreateInfo->imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    pCreateInfo->preTransform = _supportInfo.capabilities.currentTransform;
    pCreateInfo->compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    pCreateInfo->presentMode = presentMode;
    pCreateInfo->clipped = VK_TRUE;
}

void SwapChain::_configure_image_view(VkImageViewCreateInfo* pCreateInfo, VkImage image) const
{
    memset(pCreateInfo, 0, sizeof(VkImageViewCreateInfo));
    pCreateInfo->sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    pCreateInfo->image = image;
    pCreateInfo->viewType = VK_IMAGE_VIEW_TYPE_2D;
    pCreateInfo->format = _imageFormat;
    pCreateInfo->components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    pCreateInfo->components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    pCreateInfo->components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    pCreateInfo->components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    pCreateInfo->subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    pCreateInfo->subresourceRange.baseMipLevel = 0;
    pCreateInfo->subresourceRange.levelCount = 1;
    pCreateInfo->subresourceRange.baseArrayLayer = 0;
    pCreateInfo->subresourceRange.layerCount = 1;
}

void SwapChain::_configure_frame_buffer(VkFramebufferCreateInfo* pCreateInfo, VkRenderPass renderPass, std::vector<VkImageView>& attachments) const
{
    memset(pCreateInfo, 0, sizeof(VkFramebufferCreateInfo));
    pCreateInfo->sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    pCreateInfo->renderPass = renderPass;
    pCreateInfo->attachmentCount = static_cast<uint32_t>(attachments.size());
    pCreateInfo->pAttachments = attachments.data();
    pCreateInfo->width = _extent.width;
    pCreateInfo->height = _extent.height;
    pCreateInfo->layers = 1;
}

void SwapChain::_configure_present_info(VkPresentInfoKHR* pInfo, VkSemaphore* pWaitSemaphore, uint32_t* pImageIndices) const
{
    memset(pInfo, 0, sizeof(VkPresentInfoKHR));
    pInfo->sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    pInfo->waitSemaphoreCount = 1;
    pInfo->pWaitSemaphores = pWaitSemaphore;
    pInfo->swapchainCount = 1;
    pInfo->pSwapchains = &_handle;

    pInfo->pImageIndices = pImageIndices;
}