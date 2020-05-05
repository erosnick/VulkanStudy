#pragma once

#include <Windows.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_win32.h>
#include <vector>
#include <fstream>

namespace Color
{
	const  VkClearValue CornFlower = { 1.0f / 255 * 100.0f, 1.0f / 255 * 149.0f, 1.0f / 255 * 237.0f };
	const  VkClearValue White = { 1.0f, 1.0f, 1.0f };
	const  VkClearValue Black = { 0.0f, 0.0f, 0.0f };
}

VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData);

VkResult createDebugUtilsMessengerEXT(VkInstance instance,
	const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
	const VkAllocationCallbacks* pAllocator,
	VkDebugUtilsMessengerEXT* pDebugMessager);

void destroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);

std::vector<char> readFile(const std::string& fileName);

#define  VKCHECK(result, message)		\
if (result != VK_SUCCESS)				\
{										\
	throw std::runtime_error(message);	\
}										\

void vkRuntimeError(const std::string& message);


// Wrapper functions for aligned memory allocation
// There is currently no standard for this in C++ that works across all platforms and vendors, so we abstract this
void* alignedAlloc(size_t size, size_t alignment);
void alignedFree(void* data);

int randRange(int min, int max);