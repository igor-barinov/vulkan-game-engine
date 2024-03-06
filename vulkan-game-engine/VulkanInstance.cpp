#include "VulkanInstance.h"

#define VK_USE_PLATFORM_WIN32_KHR
#include <GLFW/glfw3.h>

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <algorithm>
#include <iterator>
#include <limits>
#include <stdexcept>
#include <iostream>
#include <unordered_set>

/*
* STATIC VAR INITIALIZATION
*/

// Enable validation layers if debug mode is active
#ifdef NDEBUG
const bool VulkanInstance::_VK_VALIDATION_IS_ENABLED = false;
#else
const bool VulkanInstance::_VK_VALIDATION_IS_ENABLED = true;
#endif

const VkApplicationInfo VulkanInstance::_VK_APP_INFO = {
    VK_STRUCTURE_TYPE_APPLICATION_INFO,     // Struct type
    nullptr,                                // pNext
    "Game Engine",                          // App name
    VK_MAKE_VERSION(1, 0, 0),               // App version
    "No Engine",                            // Engine name
    VK_MAKE_VERSION(1, 0, 0),               // Engine version
    VK_API_VERSION_1_0,                     // API version
};

const std::vector<const char*> VulkanInstance::_VK_VALIDATION_LAYERS = {
    "VK_LAYER_KHRONOS_validation"
};





/*
* STATIC METHOD DEFINITIONS
*/

VKAPI_ATTR VkBool32 VKAPI_CALL VulkanInstance::_debug_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData)
{
    if (pCallbackData->messageIdNumber > 0)
    {
        std::cerr << "Vulkan Message ID: " << pCallbackData->messageIdNumber << std::endl;
        std::cerr << "Vulkan Message Name: " << pCallbackData->pMessageIdName << std::endl;
    }
    
    std::cerr << pCallbackData->pMessage << std::endl;
    std::cerr << std::endl;

    return VK_FALSE;
}

VkResult VulkanInstance::_create_debug_utils_messengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
{
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    }
    else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void VulkanInstance::_destroy_debug_utils_messengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}

VulkanInstance& VulkanInstance::instance()
{
    static VulkanInstance singleton;
    return singleton;
}





/*
* CTOR DEFINITIONS
*/

VulkanInstance::VulkanInstance()
    : _vkInstance(VkInstance{}), 
    _debugMessenger(VkDebugUtilsMessengerEXT{})
{
    glfwInit(); // Required to use GLFW methods

    if (_VK_VALIDATION_IS_ENABLED && !validation_layers_are_supported(_VK_VALIDATION_LAYERS))
    {
        throw std::runtime_error("Requested validation layers are not supported");
    }

    // Get required extensions
    auto extensions = _get_gflw_extensions();
    if (_VK_VALIDATION_IS_ENABLED)
    {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    // Configure and create vulkan instance
    VkInstanceCreateInfo createInfo{};
    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
    _configure_instance(&createInfo, &_VK_APP_INFO, extensions, &debugCreateInfo);

    if (vkCreateInstance(&createInfo, nullptr, &_vkInstance) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create Vulkan instance");
    }

    // Create debug messenger
    if (_VK_VALIDATION_IS_ENABLED)
    {
        VkDebugUtilsMessengerCreateInfoEXT createInfo;
        _configure_debug_messenger(&createInfo);

        if (_create_debug_utils_messengerEXT(_vkInstance, &createInfo, nullptr, &_debugMessenger) != VK_SUCCESS) {
            throw std::runtime_error("failed to set up debug messenger!");
        }
    }
}

VulkanInstance::~VulkanInstance()
{
    // Destroy debug messenger
    if (_VK_VALIDATION_IS_ENABLED) {
        _destroy_debug_utils_messengerEXT(_vkInstance, _debugMessenger, nullptr);
    }

    // Destroy instance and stop GLFW
    vkDestroyInstance(_vkInstance, nullptr);
    glfwTerminate();
}





/*
* PUBLIC CONST METHOD DEFINITIONS
*/

std::vector<VkPhysicalDevice> VulkanInstance::get_physical_devices() const
{
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(_vkInstance, &deviceCount, nullptr);

    if (deviceCount == 0)
    {
        throw std::runtime_error("Zero physical devices found");
    }

    std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
    vkEnumeratePhysicalDevices(_vkInstance, &deviceCount, physicalDevices.data());

    return physicalDevices;
}

bool VulkanInstance::validation_layers_are_supported(const std::vector<const char*>& validationLayers) const
{
    uint32_t layerCount = 0;
    
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    if (layerCount == 0)
    {
        throw std::runtime_error("Zero instance layers enumerated");
    }

    std::vector<VkLayerProperties> foundLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, foundLayers.data());

    // Convert list of found layers to a set
    std::unordered_set<std::string> foundLayerNames;
    std::transform(foundLayers.begin(), foundLayers.end(), std::inserter(foundLayerNames, foundLayerNames.begin()), [](const VkLayerProperties& prop) { return std::string(prop.layerName); });

    // Iterate over given layers while searching set
    for (const auto& layerToSupport : validationLayers)
    {
        auto search = foundLayerNames.find(std::string(layerToSupport));
        if (search == foundLayerNames.end())
        {
            return false;
        }
    }
    
    return true;
}





/*
* PRIVATE CONST METHOD DEFINITIONS
*/

std::vector<const char*> VulkanInstance::_get_gflw_extensions() const
{
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
    return extensions;
}

void VulkanInstance::_configure_instance(VkInstanceCreateInfo* pDest, const VkApplicationInfo* pAppInfo, const std::vector<const char*>& extensions, VkDebugUtilsMessengerCreateInfoEXT* pDebugCreateInfo) const
{
    memset(pDest, 0, sizeof(VkInstanceCreateInfo));
    pDest->sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    pDest->pApplicationInfo = pAppInfo;
    pDest->enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    pDest->ppEnabledExtensionNames = extensions.data();

    if (_VK_VALIDATION_IS_ENABLED) {
        pDest->enabledLayerCount = static_cast<uint32_t>(_VK_VALIDATION_LAYERS.size());
        pDest->ppEnabledLayerNames = _VK_VALIDATION_LAYERS.data();

        _configure_debug_messenger(pDebugCreateInfo);
        pDest->pNext = (VkDebugUtilsMessengerCreateInfoEXT*)pDebugCreateInfo;
    }
    else {
        pDest->enabledLayerCount = 0;
        pDest->pNext = nullptr;
    }
}

void VulkanInstance::_configure_debug_messenger(VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo) const
{
    memset(pCreateInfo, 0, sizeof(VkDebugUtilsMessengerCreateInfoEXT));
    pCreateInfo->sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    pCreateInfo->messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    pCreateInfo->messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    pCreateInfo->pfnUserCallback = VulkanInstance::_debug_callback;
}