#pragma once

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include <iostream>
#include <stdexcept>
#include <functional>
#include <vector>

const int WIDTH = 800;
const int HEIGHT = 600;

const std::vector<const char*> ValidationLayers = { "VK_LAYER_LUNARG_standard_validation" };

#ifdef NDEBUG
	const bool EnableValidationLayers = false;
#else
	const bool EnableValidationLayers = true;
#endif

struct QueueFamilyIndices
{
	int GraphicsFamily = -1;
	bool IsComplete()
	{
		return GraphicsFamily >= 0;
	}
};

class HelloTriangleApplication
{
public:

	void Run();

	~HelloTriangleApplication();

private:

	void CreateInstance();

	void CheckExtensions();

	bool CheckValidationLayerSupport();

	std::vector<const char*> GetRequiredExtensions();
	
	void SetupDebugCallback();

	static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugReportFlagsEXT Flags,
		VkDebugReportObjectTypeEXT ObjectType,
		uint64_t Object,
		size_t Location,
		int32_t Code,
		const char* LayerPrefix,
		const char* Message,
		void* UserData)
	{
		std::cerr << "Validation layer: " << Message << std::endl;

		return VK_FALSE;
	}

	void InitWindow();

	void InitVulkan();

	void PickPhysicalDevice();

	bool IsDeviceSuitable(VkPhysicalDevice Device);

	QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice Device);

	void MainLoop();

	void Cleanup();

private:

	GLFWwindow* Window;
	VkInstance Instance;
	VkDebugReportCallbackEXT Callback;
};