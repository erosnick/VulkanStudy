#pragma once

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <stdexcept>
#include <functional>
#include <vector>

const int WIDTH = 800;
const int HEIGHT = 600;

// Vulkan API的设计核心是尽量最小化驱动程序的额外开销，所谓额外开销更多的是指向渲染以外的运算。
// 其中一个具体的表现就是默认条件下，Vulkan API的错误检查的支持非常有限。即使遍历不正确的值或者
// 将需要的参数传递为空指针，也不会有明确的处理逻辑，并且直接导致崩溃或者未定义的异常行为。之所以
// 这样，是因为Vulkan要求每一个步骤定义都非常明确，导致很容易造成小错误，例如使用新的GPU功能，
// 但是忘记了逻辑设备创建时请求它。

// Validation Layers是可选组件，可以挂在到Vulkan函数中调用，以回调其他的操作。Validation Layers的常见操作情景有：
// 1.根据规范检查参数数值，最终确认是否存在与预期不符的情况
// 2.跟踪对象的创建和销毁，以查找是否存在资源的泄露
// 3.跟踪线程的调用链，确认线程执行过程中的安全性
// 4.将每次函数调用所使用的参数记录到标准的输出中，进行初步的Vulkan概要分析

// Vulkan SDK提供的标准诊断层。
const std::vector<const char*> ValidationLayers = { "VK_LAYER_LUNARG_standard_validation" };

#ifdef NDEBUG
	const bool EnableValidationLayers = false;
#else
	const bool EnableValidationLayers = true;
#endif

struct QueueFamilyIndices
{
	int GraphicsFamily = -1;
	int PresentFamily = -1;

	bool IsComplete()
	{
		return GraphicsFamily >= 0 && PresentFamily >= 0;
	}
};

class HelloTriangleApplication
{
public:

	// 启动渲染循环。
	void Run();

	~HelloTriangleApplication();

private:

	// 创建VkInstance。
	void CreateInstance();

	// 检查受支持的扩展情况。
	void CheckExtensions();

	// 检查Validation Layer的支持情况。
	bool CheckValidationLayerSupport();

	// 获取需要的扩展。
	std::vector<const char*> GetRequiredExtensions();

	// 设置用于Validdation Layer的回调函数。
	void SetupDebugCallback();

	void CreateSurface();

	// Validation Layer的回调函数，只有这样才能在出问题时获得汇报信息。
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

	// 初始化GLFW窗口系统。
	void InitWindow();

	// 初始化Vulkan库。
	void InitVulkan();

	// 获取可用的物理设备(显卡)。
	void PickPhysicalDevice();

	// 检查物理设备是否适用。
	bool IsDeviceSuitable(VkPhysicalDevice Device);

	// 获取队列家族。
	QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice Device);

	// 创建逻辑设备。
	void CreateLogicalDevice();

	// 渲染循环。
	void MainLoop();

	// 清理资源。
	void Cleanup();

private:

	GLFWwindow* Window;
	VkSurfaceKHR Surface;
	VkInstance Instance;
	VkDebugReportCallbackEXT Callback;
	VkPhysicalDevice PhysicalDevice;
	VkDevice LogicalDevice;
	VkQueue GraphicQueue;
	VkQueue PresentQueue;
};