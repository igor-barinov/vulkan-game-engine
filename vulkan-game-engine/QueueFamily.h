#pragma once

#include <unordered_map>
#include <unordered_set>

#include <vulkan/vulkan.h>

#include <mutex>

/* Enum describing types of queue families
*/
enum QueueFamilyType
{
	Graphics,
	Present,
	None
};

/*
* Class containing queue family data
*/
class QueueFamilyInfo
{
public:

	/*
	* PUBLIC STATIC CONSTS
	*/

	/* Represents an unknown/invalid queue family index value
	*/
	static const int UNKNOWN_INDEX;



	/*
	* CTORS
	*/
	QueueFamilyInfo();
	~QueueFamilyInfo();

	/*
	* PUBLIC CONST METHODS
	*/

	/* @brief Returns the queue family index for the given type
	*/
	inline int operator[](QueueFamilyType type) const { return _indices.at(type); }

	/* @brief Returns the queue handle for the given type
	*/
	inline VkQueue get_queue_handle(QueueFamilyType type) const { return _handles.at(type); }

	/* @brief Checks if valid indices exist for the given queue family types
	*/
	bool queue_families_are_supported(std::initializer_list<QueueFamilyType> families) const;

	/* @brief Checks if valid indices exist for all queue family types
	*/
	bool all_queue_families_are_supported() const;

	/* @brief Returns a list of queue family index values
	*/
	std::vector<uint32_t> queue_family_indices() const;

	/* @brief Returns a set of unique queue family index values
	*/
	std::unordered_set<uint32_t> unique_queue_family_indices() const;



	/*
	* PUBLIC METHODS
	*/

	/* @brief Loads queue family indices based on the given device and surface
	*/
	void load_queue_family_indices(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);

	/* @brief Loads queue handles for a given logical device
	*/
	void load_handles(VkDevice logicalDevice);



	/*
	* PUBLIC STATIC METHODS
	*/

	/* @brief Creates an instance with info for the given device and surface
	*/
	static inline QueueFamilyInfo info_for(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
	{
		QueueFamilyInfo info;
		info.load_queue_family_indices(physicalDevice, surface);
		return info;
	}

private:

	/*
	* PRIVATE MEMBERS
	*/

	/* Mapping of queue family types to index values
	*/
	std::unordered_map<QueueFamilyType, int> _indices;

	/* Mapping of queue family types to queue handles
	*/
	std::unordered_map<QueueFamilyType, VkQueue> _handles;



	/*
	* PRIVATE METHODS
	*/

	/* @brief Sets the index value for the given family type
	*/
	inline void _set_index_value(int familyType, int indexValue) { _indices[static_cast<QueueFamilyType>(familyType)] = indexValue; }

	/* @brief Sets the queue handle for the given family type
	*/
	inline void _set_handle_value(int familyType, VkQueue handle) { _handles[static_cast<QueueFamilyType>(familyType)] = handle; }
};

