#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <sstream>

#include "Device.h"

/*
* Class that implements a Vulkan shader
*/
class Shader
{
public:

	/*
	* TYPEDEFS / ENUMS
	*/

	using Handle = VkShaderModule;

	enum Type
	{
		VERTEX,
		FRAGMENT,
		COMPUTE,
		NONE
	};



	/*
	* PUBLIC FRIEND METHODS
	*/

	/* @brief Swap implementation for Shader class
	*/
	friend void swap(Shader& shaderA, Shader& shaderB)
	{
		using std::swap;

		swap(shaderA._module, shaderB._module);
		swap(shaderA._shaderType, shaderB._shaderType);
		swap(shaderA._shaderCode, shaderB._shaderCode);
		swap(shaderA._filepath, shaderB._filepath);
	}



	/*
	* CTORS / ASSIGNMENT
	*/

	Shader();
	/*
	* @param filepath Path to shader file to load
	* @param device Device being used
	*/
	Shader(const std::string& filepath, Type shaderType, const Device& device);
	Shader(const Shader& other);
	Shader(Shader&& other) noexcept;
	Shader& operator=(Shader other);
	~Shader();



	/*
	* PUBLIC METHODS
	*/

	/* @brief Loads shader code from the given file
	* 
	* @param filepath Path to shader file
	* @param shaderType The type of shader that is being loaded
	*/
	void load_shader(const std::string& filepath, Type shaderType);



	/*
	* PUBLIC CONST METHODS
	*/

	/* @brief Returns the handle to the shader module
	*/
	inline Handle handle() const { return _module; }

	/* @brief Returns the type of shader that is loaded
	*/
	inline Type shader_type() const { return _shaderType; }

private:

	/*
	* PRIVATE MEMBERS
	*/

	/* Handle to shader module
	*/
	Handle _module;

	/* The type of shader currently loaded
	*/
	Type _shaderType;

	/* The path of the shader file currently loaded
	*/
	std::string _filepath;

	/* The binary content of the shader file
	*/
	std::vector<char> _shaderCode;



	/*
	* PRIVATE CONST METHODS
	*/

	/* @brief Fills struct with necessary info for creating shader module
	*/
	void _configure_module(VkShaderModuleCreateInfo* pCreateInfo) const;
};

