#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <string>

/*
* VulkanInstance class
* 
* Singleton containing vulkan instance data
*/
class VulkanInstance
{
public:
	/* 
	* TYPEDEFS
	*/

	using Handle = VkInstance;



	/*
	* DELETED METHODS
	*/

	VulkanInstance(const VulkanInstance&) = delete;



	/*
	* PUBLIC CONST METHODS
	*/

	inline Handle handle() const { return _vkInstance; }

	/* @brief Returns a list of physical device handles. Throws if 0 devices found
	*/
	std::vector<VkPhysicalDevice> get_physical_devices() const;

	/* @brief Checks if the given validation layers are supported
	*
	* @param validationLayers List of validation layer names to check
	*
	* @throws std::runtime_error if no available layers could be found
	*/
	bool validation_layers_are_supported(const std::vector<const char*>& validationLayers) const;



	/*
	* PUBLIC STATIC METHODS
	*/

	/* @brief Returns singleton instance
	*/
	static VulkanInstance& instance();

	/* @brief Checks if validation layers are enabled
	*/
	static inline constexpr bool validation_is_enabled() { return _VK_VALIDATION_IS_ENABLED; }

	/* @brief Returns list of validation layer names being used
	*/
	static inline std::vector<const char*> get_validation_layers() { return _VK_VALIDATION_LAYERS; }

private:

	/*
	* PRIVATE STATIC CONSTANTS
	*/

	/* Default application metadata
	*/
	static const VkApplicationInfo _VK_APP_INFO;

	/* Describes if validation layers are enabled
	*/
	static const bool _VK_VALIDATION_IS_ENABLED;

	/* Default validation layer names
	*/
	static const std::vector<const char*> _VK_VALIDATION_LAYERS;



	/*
	* PRIVATE MEMBER VARS
	*/

	/* Handle to vulkan instance
	*/
	Handle _vkInstance;

	/* Handle to debug messenger
	*/
	VkDebugUtilsMessengerEXT _debugMessenger;



	/*
	* PRIVATE CTORS
	*/

	VulkanInstance();
	~VulkanInstance();



	/*
	* PRIVATE CONST METHODS
	*/

	/* @brief Returns extension required when using GLFW
	*/
	std::vector<const char*> _get_gflw_extensions() const;

	/* @brief Fills struct with info necessary for creating vulkan instance
	*
	* @param[out] pDest The struct to configure
	* @param pAppInfo Pointer to struct containing application metadata
	* @param extensions List of extension names to enable
	* @param pDebugCreateInfo Pointer to struct to configure with debug messenger info
	*/
	void _configure_instance(
		VkInstanceCreateInfo* pDest, 
		const VkApplicationInfo* pAppInfo,
		const std::vector<const char*>& extensions, 
		VkDebugUtilsMessengerCreateInfoEXT* pDebugCreateInfo) const;

	/* @brief Fills struct with info necessary for creating debug messenger
	*
	* @param[out] pCreateInfo The struct to configure
	*/
	void _configure_debug_messenger(VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo) const;



	/*
	* PRIVATE STATIC METHODS
	*/

	/* @brief Callback for printing debug messages
	*
	* @param messageSeverity Severity of the message, one of verbose, info, warning, or error
	* @param messageType Message category, one of general, validation, or performance
	* @param pCallbackData Pointer to message data
	* @param pUserData Pointer to user defined data for giving additional context
	*/
	static VKAPI_ATTR VkBool32 VKAPI_CALL _debug_callback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData);

	/* @brief Loads and calls extension function for creating debug messenger
	*/
	static VkResult _create_debug_utils_messengerEXT(
		VkInstance instance, 
		const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, 
		const VkAllocationCallbacks* pAllocator, 
		VkDebugUtilsMessengerEXT* pDebugMessenger);

	/* @brief Loads and calls extension function for destroying debug messenger
	*/
	static void _destroy_debug_utils_messengerEXT(
		VkInstance instance, 
		VkDebugUtilsMessengerEXT debugMessenger,
		const VkAllocationCallbacks* pAllocator);
};

