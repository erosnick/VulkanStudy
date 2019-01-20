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

// Swap Chain扩展。
const std::vector<const char*> DeviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

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

// 关于SwapChain创建的详细信息。
struct SwapChainSupprotDetails
{
	VkSurfaceCapabilitiesKHR Capabilities;
	std::vector<VkSurfaceFormatKHR> Formats;
	std::vector<VkPresentModeKHR> PresentModes;
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

	// 检查Device扩展的支持情况。
	bool CheckDeviceExtensionSupport(VkPhysicalDevice Device);

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

	// 获取SwapChain支持细节。
	SwapChainSupprotDetails QuerySwapChainSupport(VkPhysicalDevice Device);

	// 选择Surface的格式。
	// 每个VkSurfaceFormatKHR结构都包含一个format和一个colorSpace成员。format成员变量指定色彩通道和类型。比如，VK_FORMAT_B8G8R8A8_UNORM代表了我们使用B, G, R和alpha次序的通道，且每一个通道为无符号8bit整数，每个像素总计32bits。colorSpace成员描述SRGB颜色空间是否通过VK_COLOR_SPACE_SRGB_NONLINEAR_KHR标志支持
	VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& AvailableForamt);

	// 选择交换链的呈现模式。
	// Presentation模式对于交换链是非常重要的，因为它代表了在屏幕呈现图像的条件。在Vulkan中有四个模式可以使用:

	// 1.VK_PRESENT_MODE_IMMEDIATE_KHR: 应用程序提交的图像被立即传输到屏幕呈现，这种模式可能会造成撕裂效果。

	// 2.VK_PRESENT_MODE_FIFO_KHR : 交换链被看作一个队列，当显示内容需要刷新的时候，显示设备从队列的前面获取图像，
	// 并且程序将渲染完成的图像插入队列的后面。如果队列是满的程序会等待。这种规模与视频游戏的垂直同步很类似。显示设备的刷新时刻被成为“垂直中断”。

	// 3.VK_PRESENT_MODE_FIFO_RELAXED_KHR : 该模式与上一个模式略有不同的地方为，如果应用程序存在延迟，即接受最后一个垂直同步信号时队列空了，
	// 将不会等待下一个垂直同步信号，而是将图像直接传送。这样做可能导致可见的撕裂效果。

	// 4.VK_PRESENT_MODE_MAILBOX_KHR : 这是第二种模式的变种。当交换链队列满的时候，选择新的替换旧的图像，从而替代阻塞应用程序的情形。
	// 这种模式通常用来实现三重缓冲区，与标准的垂直同步双缓冲相比，它可以有效避免延迟带来的撕裂效果。
	VkPresentModeKHR ChooseSwapPresentaMode(const std::vector<VkPresentModeKHR> AvailablePresentModes);

	// 获取交换链的交换范围。
	// 交换范围是指交换链图像的分辨率，它几乎总是等于我们绘制窗体的分辨率。
	// 分辨率的范围被定义在VkSurfaceCapabilitiesKHR结构体中。
	// Vulkan告诉我们通过设置currentExtent成员的width和height来匹配窗体的分辨率。
	// 然而，一些窗体管理器允许不同的设置，意味着将currentExtent的width和height设置为特殊的数值表示:uint32_t的最大值。
	// 在这种情况下，我们参考窗体minImageExtent和maxImageExtent选择最匹配的分辨率。
	VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& Capabilities);

	// 创建逻辑设备。
	void CreateLogicalDevice();

	// 创建交换链。
	void CreateSwapChain();

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
	VkSwapchainKHR SwapChain;
	std::vector<VkImage> SwapChainImages;
	VkFormat SwapChainImageFormat;
	VkExtent2D SwapChainExtent;
};