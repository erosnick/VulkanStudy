#pragma once

#include <vk_mem_alloc.h>

struct Buffer
{
	Buffer() {}
	~Buffer()
	{
	}

	void release() 
	{
		if (allocator != VK_NULL_HANDLE)
		{
			vmaDestroyBuffer(allocator, handle, allocation);
		}
	}

	VkBuffer handle = VK_NULL_HANDLE;
	VkDeviceMemory memory = VK_NULL_HANDLE;
	VkDeviceSize size = 0;
	VmaAllocation allocation{};
	VkBufferUsageFlags usage{};
	VkMemoryPropertyFlags propertyFlags{};

	static VmaAllocator allocator;
};