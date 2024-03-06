#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <sstream>

#include "VulkanDevice.h"

class Shader
{
public:

	using Handle = VkShaderModule;

	enum Type
	{
		VERTEX,
		FRAGMENT,
		COMPUTE,
		NONE
	};

	friend void swap(Shader& shaderA, Shader& shaderB)
	{
		using std::swap;

		swap(shaderA._module, shaderB._module);
		swap(shaderA._shaderType, shaderB._shaderType);
		swap(shaderA._shaderCode, shaderB._shaderCode);
		swap(shaderA._filepath, shaderB._filepath);
	}

	Shader();
	Shader(const std::string& filepath, Type shaderType, const VulkanDevice& device);
	Shader(const Shader& other);
	Shader(Shader&& other) noexcept;
	Shader& operator=(Shader other);
	~Shader();

	void load_shader(const std::string& filepath, Type shaderType);

	inline Handle handle() const { return _module; }

	inline Type shader_type() const { return _shaderType; }

private:
	VkShaderModule _module;
	Type _shaderType;
	std::string _filepath;
	std::vector<char> _shaderCode;

	void _configure_module(VkShaderModuleCreateInfo* pCreateInfo) const;
};

