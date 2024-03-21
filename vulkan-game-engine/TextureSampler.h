#pragma once

#include <vulkan/vulkan.h>
#include <algorithm>

#include "Device.h"

class TextureSampler
{
public:

	using Handle = VkSampler;

	friend void swap(TextureSampler& samplerA, TextureSampler& samplerB)
	{
		using std::swap;

		swap(samplerA._sampler, samplerB._sampler);
		swap(samplerA._deviceHandle, samplerB._deviceHandle);
	}

	TextureSampler();
	TextureSampler(const Device& device);
	TextureSampler(const TextureSampler& other);
	TextureSampler(TextureSampler&& other) noexcept;
	TextureSampler& operator=(TextureSampler other);
	~TextureSampler();

	inline Handle handle() const { return _sampler; }

private:
	VkSampler _sampler;
	VkDevice _deviceHandle;
	
	void _configure_sampler(VkSamplerCreateInfo* pCreateInfo, VkPhysicalDeviceProperties deviceProps);
};

