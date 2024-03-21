#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <algorithm>

#include "Device.h"
#include "Window.h"

/*
* Class that implements a Vulkan swap chain
*/
class SwapChain
{
public:

	/*
	* TYPEDEFS
	*/

	using Handle = VkSwapchainKHR;
	using FormatFilter = bool(*)(VkSurfaceFormatKHR);
	using PresentModeFilter = bool(*)(VkPresentModeKHR);



	/*
	* PUBLIC FRIEND METHODS
	*/

	/* @brief Swap implementation for SwapChain class
	*/
	friend void swap(SwapChain& chainA, SwapChain& chainB)
	{
		using std::swap;

		swap(chainA._swapChain, chainB._swapChain);
		swap(chainA._supportInfo, chainB._supportInfo);
		swap(chainA._imageFormat, chainB._imageFormat);
		swap(chainA._extent, chainB._extent);
		swap(chainA._chainImages, chainB._chainImages);
		swap(chainA._chainImageViews, chainB._chainImageViews);
		swap(chainA._chainFrameBuffers, chainB._chainFrameBuffers);
		swap(chainA._deviceHandle, chainB._deviceHandle);
	}



	/*
	* CTORS / ASSIGNMENT
	*/

	SwapChain();

	/*
	* @param device Device being used for swap chain
	* @param window Window being drawn
	* @param formatFilterFn Filtering function used to select surface format
	* @param presentModeFilterFn Filtering functions used to select present mode
	*/
	SwapChain(const Device& device, const Window& window, FormatFilter formatFilterFn, PresentModeFilter presentModeFilterFn);
	SwapChain(const SwapChain& other);
	SwapChain(SwapChain&& other) noexcept;
	SwapChain& operator=(SwapChain other);
	~SwapChain();



	/*
	* PUBLIC METHODS
	*/

	/* @brief Initializes framebuffers for use
	* 
	* @param renderPass Handle to render pass that framebuffers will be used with
	*/
	void init_framebuffers(VkRenderPass renderPass);

	/* @brief Gets the next image from the swap chain
	* 
	* @param semaphore Semaphore used for signaling image acquisition
	* @param[out] isOutofDate Is set to `true` if the swap chain is outdated, `false` otherwise
	* @returns The index of the next image in swap chain
	*/
	uint32_t get_next_image(VkSemaphore semaphore, bool& isOutofDate);

	/* @brief Presents an image to swap chain
	* 
	* @param presentQueue Handle to present queue
	* @param pWaitSemaphore Pointer to semaphore used for signaling a finished render
	* @param[out] pImageIndex Pointer to index of image being presented
	*/
	bool present_image(VkQueue presentQueue, VkSemaphore* pWaitSemaphore, uint32_t* pImageIndex);



	/*
	* PUBLIC CONST METHODS
	*/

	/* @brief Returns the surface image format being used
	*/
	inline VkFormat surface_image_format() const { return _imageFormat; }

	/* @brief Returns the surface extent
	*/
	inline VkExtent2D surface_extent() const { return _extent; }

	/* @brief Returns the frame buffer at the given index
	*/
	inline VkFramebuffer frame_buffer_at(size_t index) const { return _chainFrameBuffers[index]; }



	/*
	* PUBLIC STATIC METHODS
	*/

	/* @brief Checks for swap chain support for a given device and surface
	*
	* @param physicalDevice The device that will be used
	* @param surface The surface that will be drawn
	*/
	static bool query_support_for(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);

private:

	/*
	* PRIVATE STRUCTS/CLASSES
	*/

	/* Struct containing swap chain capability information
	*/
	struct _SwapChainSupport
	{
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> availableFormats;
		std::vector<VkPresentModeKHR> availablePresentModes;
	};



	/*
	* PRIVATE MEMBERS
	*/

	/* Handle to swap chain
	*/
	Handle _swapChain;

	/* Swap chain capability info
	*/
	_SwapChainSupport _supportInfo;

	/* Surface image format
	*/
	VkFormat _imageFormat;

	/* Surface extent
	*/
	VkExtent2D _extent;

	/* Chain image list
	*/
	std::vector<VkImage> _chainImages;

	/* Chain image view list
	*/
	std::vector<VkImageView> _chainImageViews;

	/* Chain frame buffer list
	*/
	std::vector<VkFramebuffer> _chainFrameBuffers;

	/* Handle to device using swap chain
	*/
	VkDevice _deviceHandle;



	/*
	* PRIVATE CONST METHODS
	*/

	/* @brief Selects a surface format based on filtering criteria
	*/
	VkSurfaceFormatKHR _choose_surface_format(FormatFilter formatFilterFn) const;

	/* @brief Selects a present mode based on filtering criteria
	*/
	VkPresentModeKHR _choose_present_mode(PresentModeFilter presentModeFilterFn) const;

	/* @brief Selects surface extent for a given window
	*/
	VkExtent2D _choose_extent(const Window& window) const;

	/* @brief Fills struct with necessary info for creating swap chain
	*/
	void _configure_swap_chain(
		VkSwapchainCreateInfoKHR* pCreateInfo,
		uint32_t imageCount,
		VkSurfaceFormatKHR surfaceFormat,
		VkPresentModeKHR presentMode,
		VkExtent2D extent,
		bool doConcurrentSharing,
		const std::vector<uint32_t>& queueFamilyIndices,
		VkSurfaceKHR surface,
		FormatFilter formatFilterFn,
		PresentModeFilter presentModeFilterFn) const;

	/* @brief Fills struct with necessary info for creating image view
	*/
	void _configure_image_view(VkImageViewCreateInfo* pCreateInfo, VkImage image) const;

	/* @brief Fills struct with necessary info for creating frame buffer
	*/
	void _configure_frame_buffer(VkFramebufferCreateInfo* pCreateInfo, VkRenderPass renderPass, VkImageView* pAttachments) const;

	/* @brief Fills struct with necessary info for presentation
	*/
	void _configure_present_info(VkPresentInfoKHR* pInfo, VkSemaphore* pWaitSemaphore, uint32_t* pImageIndices) const;



	/*
	* PRIVATE STATIC METHODS
	*/

	/* @brief Loads swap chain capabilities for a given device and surface
	*/
	static _SwapChainSupport _get_capabilites_for(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
};

