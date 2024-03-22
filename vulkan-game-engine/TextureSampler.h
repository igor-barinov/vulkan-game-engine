#pragma once

#include <vulkan/vulkan.h>
#include <algorithm>

#include "VulkanObject.h"
#include "Device.h"

class TextureSampler : public VulkanObject<VkSampler>
{
public:

	friend void swap(TextureSampler& samplerA, TextureSampler& samplerB)
	{
		using std::swap;

		swap(samplerA._handle, samplerB._handle);
		swap(samplerA._deviceHandle, samplerB._deviceHandle);
	}

	TextureSampler();
	TextureSampler(const Device& device);
	TextureSampler(const TextureSampler& other);
	TextureSampler(TextureSampler&& other) noexcept;
	TextureSampler& operator=(TextureSampler other);
	~TextureSampler();

private:	
	void _configure_sampler(VkSamplerCreateInfo* pCreateInfo, VkPhysicalDeviceProperties deviceProps);
};

