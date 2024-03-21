#include "DescriptorPool.h"

DescriptorPool::DescriptorPool()
	: _poolSize(0),
	_bindingTypes({}),
	_layout(VK_NULL_HANDLE),
	_descriptorPool(VK_NULL_HANDLE),
	_descriptorSets({}),
	_deviceHandle(VK_NULL_HANDLE)
{
}

DescriptorPool::DescriptorPool(const Device& device, size_t poolSize, const std::vector<BindingType>& bindings)
	: _poolSize(poolSize),
	_bindingTypes(bindings),
	_layout(VK_NULL_HANDLE),
	_descriptorPool(VK_NULL_HANDLE),
	_descriptorSets(poolSize),
	_deviceHandle(device.handle())
{
	// Configure bindings
	std::vector<VkDescriptorSetLayoutBinding> layoutBindings;
	std::vector<VkDescriptorPoolSize> poolSizes;
	for (auto bindingType : bindings)
	{
		VkDescriptorSetLayoutBinding binding{};
		VkDescriptorPoolSize poolSizeInfo{};

		switch (bindingType)
		{
		case DescriptorPool::UBO:
		{
			_configure_ubo_binding(&binding);
			poolSizeInfo.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			poolSizeInfo.descriptorCount = static_cast<uint32_t>(_poolSize);
		}
		break;
		case DescriptorPool::TEXTURE_SAMPLER:
		{
			_configure_texture_sampler_binding(&binding);
			poolSizeInfo.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			poolSizeInfo.descriptorCount = static_cast<uint32_t>(_poolSize);
		}
		break;
		}

		layoutBindings.push_back(binding);
		poolSizes.push_back(poolSizeInfo);
	}

	// Create descriptor set layout
	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	_configure_descriptor_set_layout(&layoutInfo, layoutBindings);

	if (vkCreateDescriptorSetLayout(_deviceHandle, &layoutInfo, nullptr, &_layout) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create descriptor set layout");
	}

	// Create descriptor pool
	VkDescriptorPoolCreateInfo descriptorPoolInfo{};
	_configure_descriptor_pool(&descriptorPoolInfo, poolSizes);
	if (vkCreateDescriptorPool(_deviceHandle, &descriptorPoolInfo, nullptr, &_descriptorPool) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create descriptor pool");
	}

	// Allocate descriptor sets
	std::vector<VkDescriptorSetLayout> descriptorSetLayouts(_poolSize, _layout);
	VkDescriptorSetAllocateInfo setAllocInfo{};
	_configure_descriptor_set_alloc(&setAllocInfo, descriptorSetLayouts);
	if (vkAllocateDescriptorSets(_deviceHandle, &setAllocInfo, _descriptorSets.data()) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to allocate descriptor sets");
	}
}

DescriptorPool::DescriptorPool(const DescriptorPool& other)
	: _poolSize(other._poolSize),
	_bindingTypes(other._bindingTypes),
	_layout(other._layout),
	_descriptorPool(other._descriptorPool),
	_descriptorSets(other._descriptorSets),
	_deviceHandle(other._deviceHandle)
{
}

DescriptorPool::DescriptorPool(DescriptorPool&& other) noexcept
	: DescriptorPool()
{
	swap(*this, other);
}

DescriptorPool& DescriptorPool::operator=(DescriptorPool other)
{
	swap(*this, other);
	return *this;
}

DescriptorPool::~DescriptorPool()
{
	if (_descriptorPool != VK_NULL_HANDLE)
	{
		vkDestroyDescriptorPool(_deviceHandle, _descriptorPool, nullptr);
		vkDestroyDescriptorSetLayout(_deviceHandle, _layout, nullptr);
	}
}

void DescriptorPool::write_descriptor_set(const std::vector<std::vector<DescriptorData>>& data)
{
	if (data.size() != _poolSize)
	{
		throw std::runtime_error("Number of descriptor datasets does not match the pool size");
	}
	

	for (size_t pool = 0; pool < _poolSize; ++pool)
	{
		const auto& bindingsData = data[pool];

		if (bindingsData.size() != _bindingTypes.size())
		{
			throw std::runtime_error("Length of descriptor data does not match the number of binding types");
		}

		std::vector<VkWriteDescriptorSet> writeSets;
		for (size_t i = 0; i < _bindingTypes.size(); ++i)
		{
			auto bindingType = _bindingTypes[i];
			auto descriptorData = bindingsData[i];

			switch (bindingType)
			{
			case DescriptorPool::UBO:
			{
				VkDescriptorBufferInfo uboInfo{};
				writeSets.push_back(_create_ubo_write_set(&uboInfo, descriptorData.uniformBuffer, descriptorData.uboSize, _descriptorSets[pool]));
			}
			break;
			case DescriptorPool::TEXTURE_SAMPLER:
			{
				VkDescriptorImageInfo samplerInfo{};
				writeSets.push_back(_create_texture_sampler_write_set(&samplerInfo, descriptorData.textureSampler, descriptorData.textureImageView, _descriptorSets[pool]));
			}
			break;
			}
		}

		vkUpdateDescriptorSets(_deviceHandle, static_cast<uint32_t>(writeSets.size()), writeSets.data(), 0, nullptr);
	}
}

void DescriptorPool::_configure_ubo_binding(VkDescriptorSetLayoutBinding* pBinding) const
{
	memset(pBinding, 0, sizeof(VkDescriptorSetLayoutBinding));
	pBinding->binding = 0;
	pBinding->descriptorCount = 1;
	pBinding->descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	pBinding->pImmutableSamplers = nullptr;
	pBinding->stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
}

void DescriptorPool::_configure_texture_sampler_binding(VkDescriptorSetLayoutBinding* pBinding) const
{
	memset(pBinding, 0, sizeof(VkDescriptorSetLayoutBinding));
	pBinding->binding = 1;
	pBinding->descriptorCount = 1;
	pBinding->descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	pBinding->pImmutableSamplers = nullptr;
	pBinding->stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
}

void DescriptorPool::_configure_descriptor_set_layout(VkDescriptorSetLayoutCreateInfo* pCreateInfo, const std::vector<VkDescriptorSetLayoutBinding>& bindings) const
{
	memset(pCreateInfo, 0, sizeof(VkDescriptorSetLayoutCreateInfo));
	pCreateInfo->sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	pCreateInfo->bindingCount = static_cast<uint32_t>(bindings.size());
	pCreateInfo->pBindings = bindings.data();
}

void DescriptorPool::_configure_descriptor_pool(VkDescriptorPoolCreateInfo* pCreateInfo, const std::vector<VkDescriptorPoolSize>& poolSizes) const
{
	memset(pCreateInfo, 0, sizeof(VkDescriptorPoolCreateInfo));
	pCreateInfo->sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	pCreateInfo->poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	pCreateInfo->pPoolSizes = poolSizes.data();
	pCreateInfo->maxSets = static_cast<uint32_t>(_poolSize);
}

void DescriptorPool::_configure_descriptor_set_alloc(VkDescriptorSetAllocateInfo* pAllocInfo, const std::vector<VkDescriptorSetLayout>& setLayouts) const
{
	memset(pAllocInfo, 0, sizeof(VkDescriptorSetAllocateInfo));
	pAllocInfo->sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	pAllocInfo->descriptorPool = _descriptorPool;
	pAllocInfo->descriptorSetCount = static_cast<uint32_t>(_poolSize);
	pAllocInfo->pSetLayouts = setLayouts.data();
}

VkWriteDescriptorSet DescriptorPool::_create_ubo_write_set(VkDescriptorBufferInfo* pBufInfo, VkBuffer uniformBuffer, size_t uboSize, VkDescriptorSet descriptorSet) const
{
	memset(pBufInfo, 0, sizeof(VkDescriptorBufferInfo));
	pBufInfo->buffer = uniformBuffer;
	pBufInfo->offset = 0;
	pBufInfo->range = uboSize;

	VkWriteDescriptorSet uboDescriptorSet{};
	uboDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	uboDescriptorSet.dstSet = descriptorSet;
	uboDescriptorSet.dstBinding = 0;
	uboDescriptorSet.dstArrayElement = 0;
	uboDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uboDescriptorSet.descriptorCount = 1;
	uboDescriptorSet.pBufferInfo = pBufInfo;

	return uboDescriptorSet;
}

VkWriteDescriptorSet DescriptorPool::_create_texture_sampler_write_set(VkDescriptorImageInfo* pImageInfo, VkSampler textureSampler, VkImageView imageView, VkDescriptorSet descriptorSet) const
{
	memset(pImageInfo, 0, sizeof(VkDescriptorImageInfo));
	pImageInfo->imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	pImageInfo->imageView = imageView;
	pImageInfo->sampler = textureSampler;

	VkWriteDescriptorSet samplerDescriptorSet{};
	samplerDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	samplerDescriptorSet.dstSet = descriptorSet;
	samplerDescriptorSet.dstBinding = 1;
	samplerDescriptorSet.dstArrayElement = 0;
	samplerDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerDescriptorSet.descriptorCount = 1;
	samplerDescriptorSet.pImageInfo = pImageInfo;

	return samplerDescriptorSet;
}