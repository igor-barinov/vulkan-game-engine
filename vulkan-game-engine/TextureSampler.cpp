#include "TextureSampler.h"

TextureSampler::TextureSampler()
	: _sampler(VK_NULL_HANDLE),
	_deviceHandle(VK_NULL_HANDLE)
{
}

TextureSampler::TextureSampler(const Device& device)
	: _sampler(VK_NULL_HANDLE),
	_deviceHandle(device.handle())
{
	VkSamplerCreateInfo samplerInfo{};
	_configure_sampler(&samplerInfo, device.physical_properties());

	if (vkCreateSampler(_deviceHandle, &samplerInfo, nullptr, &_sampler) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create texture sampler");
	}
}

TextureSampler::TextureSampler(const TextureSampler& other)
	: _sampler(other._sampler),
	_deviceHandle(other._deviceHandle)
{
}

TextureSampler::TextureSampler(TextureSampler&& other) noexcept
	: TextureSampler()
{
	swap(*this, other);
}

TextureSampler& TextureSampler::operator=(TextureSampler other)
{
	swap(*this, other);
	return *this;
}

TextureSampler::~TextureSampler()
{
	if (_sampler != VK_NULL_HANDLE)
	{
		vkDestroySampler(_deviceHandle, _sampler, nullptr);
	}
}

void TextureSampler::_configure_sampler(VkSamplerCreateInfo* pCreateInfo, VkPhysicalDeviceProperties deviceProps)
{
	memset(pCreateInfo, 0, sizeof(VkSamplerCreateInfo));
	pCreateInfo->sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	pCreateInfo->magFilter = VK_FILTER_LINEAR;
	pCreateInfo->minFilter = VK_FILTER_LINEAR;
	pCreateInfo->addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	pCreateInfo->addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	pCreateInfo->addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	pCreateInfo->anisotropyEnable = VK_TRUE;
	pCreateInfo->maxAnisotropy = deviceProps.limits.maxSamplerAnisotropy;
	pCreateInfo->borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	pCreateInfo->unnormalizedCoordinates = VK_FALSE;
	pCreateInfo->compareEnable = VK_FALSE;
	pCreateInfo->compareOp = VK_COMPARE_OP_ALWAYS;
	pCreateInfo->mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
}