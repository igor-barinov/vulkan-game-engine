#include "QueueFamily.h"

/*
* STATIC CONST DEFINITIONS
*/

const int QueueFamilyInfo::UNKNOWN_INDEX = -1;





/*
* CTORS
*/

QueueFamilyInfo::QueueFamilyInfo()
	: _indices({}),
	_handles({})
{
	for (int familyType = QueueFamilyType::Graphics; familyType != QueueFamilyType::None; ++familyType)
	{
		_set_index_value(familyType, UNKNOWN_INDEX);
		_set_handle_value(familyType, VK_NULL_HANDLE);
	}
}

QueueFamilyInfo::~QueueFamilyInfo()
{
}





/*
* PUBLIC CONST METHOD DEFINITIONS
*/

bool QueueFamilyInfo::queue_families_are_supported(std::initializer_list<QueueFamilyType> families) const
{
	for (const auto& familyType : families)
	{
		auto search = _indices.find(familyType);
		if (search == _indices.end() || search->second == UNKNOWN_INDEX)
		{
			return false;
		}
	}

	return true;
}

bool QueueFamilyInfo::all_queue_families_are_supported() const
{
	for (const auto& pair : _indices)
	{
		if (pair.second == UNKNOWN_INDEX)
		{
			return false;
		}
	}

	return true;
}

std::vector<uint32_t> QueueFamilyInfo::queue_family_indices() const
{
	std::vector<uint32_t> indices;
	for (const auto& pair : _indices)
	{
		if (pair.second != UNKNOWN_INDEX)
		{
			indices.push_back(pair.second);
		}
	}

	return indices;
}

std::unordered_set<uint32_t> QueueFamilyInfo::unique_queue_family_indices() const
{
	std::unordered_set<uint32_t> uniqueIndices;
	for (const auto& pair : _indices)
	{
		if (pair.second != UNKNOWN_INDEX)
		{
			uniqueIndices.insert(pair.second);
		}
	}

	return uniqueIndices;
}





/*
* PUBLIC METHOD DEFINITIONS
*/

void QueueFamilyInfo::load_queue_family_indices(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
{
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);

	if (queueFamilyCount > 0)
	{
		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

		int i = 0;
		for (const auto& familyProps : queueFamilies)
		{
			if (familyProps.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				_set_index_value(QueueFamilyType::Graphics, i);
			}

			VkBool32 surfaceHasPresentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &surfaceHasPresentSupport);
			if (surfaceHasPresentSupport)
			{
				_set_index_value(QueueFamilyType::Present, i);
			}

			i++;
		}
	}
}

void QueueFamilyInfo::load_handles(VkDevice logicalDevice)
{
	for (int familyType = QueueFamilyType::Graphics; familyType != QueueFamilyType::None; ++familyType)
	{
		auto search = _indices.find(static_cast<QueueFamilyType>(familyType));
		if (search != _indices.end() && search -> second != UNKNOWN_INDEX)
		{
			VkQueue queueHandle;
			vkGetDeviceQueue(logicalDevice, search->second, 0, &queueHandle);
			_set_handle_value(familyType, queueHandle);
		}
	}
}