#include "Shader.h"

#include <fstream>
#include <sstream>

/*
* CTOR / ASSIGNMENT DEFINITONS
*/

Shader::Shader()
	: _module(VK_NULL_HANDLE),
	_shaderType(Type::NONE),
	_shaderCode({}),
    _filepath({})
{
}

Shader::Shader(const std::string& filepath, Type shaderType, const VulkanDevice& device)
    : _module(VK_NULL_HANDLE),
    _shaderType(Type::NONE),
    _shaderCode({}),
    _filepath(filepath)
{
    load_shader(filepath, shaderType);

    VkShaderModuleCreateInfo createInfo{};
    _configure_module(&createInfo);

    if (vkCreateShaderModule(device.handle(), &createInfo, nullptr, &_module) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create shader module");
    }
}

Shader::Shader(const Shader& other)
    : _module(other._module),
    _shaderType(other._shaderType),
    _filepath(other._filepath),
    _shaderCode(other._shaderCode)
{
}

Shader::Shader(Shader&& other) noexcept
    : Shader()
{
    swap(*this, other);
}
Shader& Shader::operator=(Shader other)
{
    swap(*this, other);
    return *this;
}

Shader::~Shader()
{
}





/*
* PUBLIC METHOD DEFINITIONS
*/

void Shader::load_shader(const std::string & filepath, Type shaderType)
{
    std::ifstream file(filepath, std::ios::ate | std::ios::binary);

    if (!file.is_open())
    {
        throw std::runtime_error("Failed to open shader file");
    }
    file.unsetf(std::ios::skipws);

    std::streampos fileSize;
    file.seekg(0, std::ios::end);
    fileSize = file.tellg();
    file.seekg(0, std::ios::beg);
    _shaderCode.reserve(fileSize);

    _shaderCode.insert(_shaderCode.begin(),
        std::istream_iterator<char>(file),
        std::istream_iterator<char>());

    file.close();

    _shaderType = shaderType;
}





/*
* PRIVATE CONST METHOD DEFINITIONS
*/

void Shader::_configure_module(VkShaderModuleCreateInfo* pCreateInfo) const
{
    memset(pCreateInfo, 0, sizeof(VkShaderModuleCreateInfo));
    pCreateInfo->sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    pCreateInfo->codeSize = _shaderCode.size();
    pCreateInfo->pCode = reinterpret_cast<const uint32_t*>(_shaderCode.data());
}