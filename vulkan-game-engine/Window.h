#pragma once

#include <algorithm>

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

/* 
* Class describing window for display
*/
class Window
{
public:

	/*
	* TYPEDEFS
	*/

	using WinHandle = GLFWwindow*;
	using SurfaceHandle = VkSurfaceKHR;



	/*
	* PUBLIC FRIEND METHODS
	*/

	/* @brief Swap implementation for Window class
	*/
	friend void swap(Window& winA, Window& winB)
	{
		using std::swap;

		swap(winA._pWin, winB._pWin);
		swap(winA._width, winB._width);
		swap(winA._height, winB._height);
		swap(winA._surface, winB._surface);
		swap(winA._frameBufferResized, winB._frameBufferResized	);
	}



	/*
	* CTORS / ASSIGNMENT
	*/

	/*
	* @param title Title of window
	* @param width Widht of window in pixels
	* @param height Height of window in pixels
	*/
	Window(const char* title, uint32_t width = 800, uint32_t height = 600);
	Window(const Window& other);
	Window(Window&& other) noexcept;
	Window& operator=(Window other);
	~Window();

	/*
	* PUBLIC METHODS
	*/

	/* @brief Displays window and polls for events
	*/
	void poll();

	/* @brief Resets the status of the window being resized or not
	*/
	inline void reset_resize_status() { _frameBufferResized = false; }



	/*
	* PUBLIC CONST METHODS
	*/

	/* @brief Returns handle to window
	*/
	inline WinHandle handle() const { return _pWin; }

	/* @brief Returns handle to window surface
	*/
	inline SurfaceHandle surface_handle() const { return _surface; }

	/* @brief Returns window width
	*/
	inline uint32_t width() const { return _width; }

	/* @brief Returns window height
	*/
	inline uint32_t height() const { return _height; }

	/* @brief Checks if the window should be closed or not
	*/
	inline bool should_close() const { return glfwWindowShouldClose(_pWin); }

	/* @brief Checks if the window was resized
	*/
	inline bool was_resized() const { return _frameBufferResized; }

private:

	/*
	* PRIVATE MEMBERS
	*/

	/* Handle to window
	*/
	WinHandle _pWin;

	/* Window width in pixels
	*/
	uint32_t _width;

	/* Window height in pixels
	*/
	uint32_t _height;

	/* Handle to surface object
	*/
	SurfaceHandle _surface;

	/* Has window been resized
	*/
	bool _frameBufferResized;



	/*
	* PRIVATE METHODS
	*/

	/* @brief Sets GLFW window settings
	*/
	void _set_window_hints();

	static void _framebuffer_resize_callback(WinHandle handle, int width, int height);
};