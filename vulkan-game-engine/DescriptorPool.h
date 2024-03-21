#pragma once

#include <vulkan/vulkan.h>
#include <algorithm>

#include "Device.h"

class DescriptorPool
{
public:

	enum BindingType
	{
		UBO,
		TEXTURE_SAMPLER,
		NONE,
	};

	union DescriptorData
	{
		struct {
			VkBuffer uniformBuffer;
			size_t uboSize;
		};
		struct {
			VkSampler textureSampler;
			VkImageView textureImageView;
		};
		
	};

	friend void swap(DescriptorPool& setA, DescriptorPool& setB)
	{
		using std::swap;

		swap(setA._poolSize, setB._poolSize);
		swap(setA._bindingTypes, setB._bindingTypes);
		swap(setA._layout, setB._layout);
		swap(setA._descriptorPool, setB._descriptorPool);
		swap(setA._descriptorSets, setB._descriptorSets);
		swap(setA._deviceHandle, setB._deviceHandle);
	}

	DescriptorPool();
	DescriptorPool(const Device& device, size_t poolSize, const std::vector<BindingType>& bindings);
	DescriptorPool(const DescriptorPool& other);
	DescriptorPool(DescriptorPool&& other) noexcept;
	DescriptorPool& operator=(DescriptorPool other);
	~DescriptorPool();

	void write_descriptor_set(const std::vector<std::vector<DescriptorData>>& data);

	inline VkDescriptorSet operator[](size_t index) { return _descriptorSets[index]; }

	inline VkDescriptorSetLayout descriptor_set_layout() const { return _layout; }

private:

	size_t _poolSize;
	std::vector<BindingType> _bindingTypes;
	VkDescriptorSetLayout _layout;
	VkDescriptorPool _descriptorPool;
	std::vector<VkDescriptorSet> _descriptorSets;
	VkDevice _deviceHandle;

	void _configure_ubo_binding(VkDescriptorSetLayoutBinding* pBinding) const;
	void _configure_texture_sampler_binding(VkDescriptorSetLayoutBinding* pBinding) const;
	void _configure_descriptor_set_layout(VkDescriptorSetLayoutCreateInfo* pCreateInfo, const std::vector<VkDescriptorSetLayoutBinding>& bindings) const;
	void _configure_descriptor_pool(VkDescriptorPoolCreateInfo* pCreateInfo, const std::vector<VkDescriptorPoolSize>& poolSizes) const;
	void _configure_descriptor_set_alloc(VkDescriptorSetAllocateInfo* pAllocInfo, const std::vector<VkDescriptorSetLayout>& setLayouts) const;
	VkWriteDescriptorSet _create_ubo_write_set(VkDescriptorBufferInfo* pBufInfo, VkBuffer uniformBuffer, size_t uboSize, VkDescriptorSet descriptorSet) const;
	VkWriteDescriptorSet _create_texture_sampler_write_set(VkDescriptorImageInfo* pImageInfo, VkSampler textureSampler, VkImageView imageView, VkDescriptorSet descriptorSet) const;
};

