#pragma once

#include <vk_mem_alloc.h>

struct Buffer
{
	Buffer() {}
	~Buffer()
	{
	}

	void release(VkDevice device) 
	{
		if (useVma)
		{
			vmaDestroyBuffer(allocator, handle, allocation);
		}
		else
		{
			vkDestroyBuffer(device, handle, nullptr);
			if (mapped)
			{
				vkUnmapMemory(device, memory);
			}
			vkFreeMemory(device, memory, nullptr);
		}
	}

	VkBuffer handle = VK_NULL_HANDLE;
	VkDeviceMemory memory = VK_NULL_HANDLE;
	VkDeviceSize size = 0;
	VmaAllocation allocation{};
	VkBufferUsageFlags usage{};
	VkMemoryPropertyFlags memoryPropertyFlags{};
	void* mappedData = nullptr;

	bool mapped = false;
	bool useVma = false;

	static VmaAllocator allocator;
};

struct Image
{
	VkImage handle = VK_NULL_HANDLE;
	VkImageView view = VK_NULL_HANDLE;
	VkDeviceMemory memory = VK_NULL_HANDLE;
	VmaAllocation allocation{};

	uint32_t width = 1;
	uint32_t height = 1;
	uint32_t mipLevels = 1;
	VkSampleCountFlagBits numSamples = VK_SAMPLE_COUNT_1_BIT;
	VkFormat format = VK_FORMAT_R8G8B8A8_SRGB;
	VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL;
	VkImageUsageFlags usage = 0;
	VkMemoryPropertyFlags memoryPropertyFlags = 0;

	static VmaAllocator allocator;
};