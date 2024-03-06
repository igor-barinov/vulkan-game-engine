#include "Window.h"
#include <stdexcept>

#include "VulkanInstance.h"

void Window::_framebuffer_resize_callback(WinHandle handle, int width, int height)
{
	auto pWindow = reinterpret_cast<Window*>(glfwGetWindowUserPointer(handle));
	pWindow->_frameBufferResized = true;
}

/*
* CTORS / ASSIGNMENT DEFINITIONS
*/

Window::Window(const char* title, uint32_t width, uint32_t height)
	: _pWin(nullptr), _width(width), _height(height), _surface(VK_NULL_HANDLE), _frameBufferResized(false)
{
	_set_window_hints();
	_pWin = glfwCreateWindow(_width, _height, title, nullptr, nullptr);
	
	if (_pWin == NULL)
	{
		throw std::runtime_error("Failed to create Window handle");
	}

	if (glfwCreateWindowSurface(VulkanInstance::instance().handle(), _pWin, nullptr, &_surface) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create Vulkan surface");
	}

	glfwSetWindowUserPointer(_pWin, this);
	glfwSetFramebufferSizeCallback(_pWin, Window::_framebuffer_resize_callback);
}

Window::Window(const Window& other)
	: _pWin(other._pWin), _width(other._width), _height(other._height), _surface(other._surface), _frameBufferResized(other._frameBufferResized)
{
}

Window::Window(Window&& other) noexcept
	:_pWin(nullptr), _width(0), _height(0), _surface(VK_NULL_HANDLE), _frameBufferResized(false)
{
	swap(*this, other);
}

Window& Window::operator=(Window other)
{
	swap(*this, other);
	return *this;
}

Window::~Window()
{
	auto& vulkan = VulkanInstance::instance();
	vkDestroySurfaceKHR(vulkan.handle(), _surface, nullptr);
	glfwDestroyWindow(_pWin);
}





/*
* PUBLIC METHOD DEFINITIONS
*/

void Window::poll()
{
	glfwPollEvents();
}





/*
* PRIVATE METHOD DEFINITIONS
*/

void Window::_set_window_hints()
{
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
}