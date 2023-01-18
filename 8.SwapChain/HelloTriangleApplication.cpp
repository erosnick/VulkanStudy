#include "HelloTriangleApplication.h"
#include <set>
#include <algorithm>

// 在Vulkan中创建、实例化相关的函数参数一半遵循如下原则定义
// 1.使用有关creation info的结构体指针
// 2.使用自定义分配器回调的指针
// 3.使用保存新对象句柄的指针

// vkCreateDebugReportCallbackEXT函数是一个扩展功能，并不会被自动加载，
// 必须使用vkGetInstanceProcAddr通过函数地址来调用。
VkResult CreateDebugReportCallbackEXT(VkInstance Instance,
	const VkDebugReportCallbackCreateInfoEXT* pCreateInfo,
	const VkAllocationCallbacks* pAllocator,
	VkDebugReportCallbackEXT* pCallback)
{
	auto Function = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(Instance, "vkCreateDebugReportCallbackEXT");

	if (Function != nullptr)
	{
		return Function(Instance, pCreateInfo, pAllocator, pCallback);
	}
	else
	{
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

// 用于清理VkDebugReportCallbackEXT对象的vkDestroyDebugReportCallbackEXT函数也是一个扩展，获取方法同上。
void DestroyDebugReportCallbackEXT(VkInstance Instance, VkDebugReportCallbackEXT Callback, const VkAllocationCallbacks* pAllocator)
{
	auto Function = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(Instance, "vkDestroyDebugReportCallbackEXT");

	if (Function != nullptr)
	{
		Function(Instance, Callback, pAllocator);
	}
}

void HelloTriangleApplication::CreateInstance()
{
	if (EnableValidationLayers && !CheckValidationLayerSupport())
	{
		throw std::runtime_error("Validation layers requested, but not available!");
	}

	VkApplicationInfo ApplicationInfo = {};

	ApplicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	ApplicationInfo.pNext = nullptr;
	ApplicationInfo.pApplicationName = "Hello Triangle";
	ApplicationInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	ApplicationInfo.pEngineName = "No Engine";
	ApplicationInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	ApplicationInfo.apiVersion = VK_API_VERSION_1_3;

	VkInstanceCreateInfo CreateInfo = {};
	CreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	CreateInfo.pApplicationInfo = &ApplicationInfo;

	auto Extensions = GetRequiredExtensions();

	CreateInfo.enabledExtensionCount = static_cast<uint32_t>(Extensions.size());
	CreateInfo.ppEnabledExtensionNames = Extensions.data();
	
	if (EnableValidationLayers)
	{
		CreateInfo.enabledLayerCount = static_cast<uint32_t>(ValidationLayers.size());
		CreateInfo.ppEnabledLayerNames = ValidationLayers.data();
	}
	else
	{
		CreateInfo.enabledLayerCount = 0;
	}

	if (vkCreateInstance(&CreateInfo, nullptr, &Instance) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create instance!");
	}
}

void HelloTriangleApplication::CheckExtensions()
{
	uint32_t ExtensionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &ExtensionCount, nullptr);

	std::vector<VkExtensionProperties> Extensions(ExtensionCount);

	vkEnumerateInstanceExtensionProperties(nullptr, &ExtensionCount, Extensions.data());

	std::cout << "Available extensions:" << std::endl;

	for (const auto& Extension : Extensions)
	{
		std::cout << "\t" << Extension.extensionName << std::endl;
	}
}

bool HelloTriangleApplication::CheckDeviceExtensionSupport(VkPhysicalDevice Device)
{
	uint32_t ExtensionCount;

	vkEnumerateDeviceExtensionProperties(Device, nullptr, &ExtensionCount, nullptr);

	std::vector<VkExtensionProperties> AvailableExtensions(ExtensionCount);

	vkEnumerateDeviceExtensionProperties(Device, nullptr, &ExtensionCount, AvailableExtensions.data());

	std::set<std::string> RequiredExtensions(DeviceExtensions.begin(), DeviceExtensions.end());

	for (const auto& Extension : AvailableExtensions)
	{
		RequiredExtensions.erase(Extension.extensionName);
	}

	return RequiredExtensions.empty();
}

bool HelloTriangleApplication::CheckValidationLayerSupport()
{
	uint32_t LayerCount;

	vkEnumerateInstanceLayerProperties(&LayerCount, nullptr);

	std::vector<VkLayerProperties> AvailableLayers(LayerCount);
	vkEnumerateInstanceLayerProperties(&LayerCount, AvailableLayers.data());

	for (const char* LayerName : ValidationLayers)
	{
		bool LayerFound = false;

		for (const auto& LayerProperty : AvailableLayers)
		{
			if (strcmp(LayerName, LayerProperty.layerName) == 0)
			{
				LayerFound = true;
				break;
			}
		}

		if (!LayerFound)
		{
			return false;
		}
	}

	return true;
}

std::vector<const char*> HelloTriangleApplication::GetRequiredExtensions()
{
	std::vector<const char*> Extensions;

	uint32_t GLFWExtensionCount = 0;
	const char** GLFWExtensios;

	GLFWExtensios = glfwGetRequiredInstanceExtensions(&GLFWExtensionCount);

	for (uint32_t i = 0; i < GLFWExtensionCount; i++)
	{
		Extensions.push_back(GLFWExtensios[i]);
	}

	if (EnableValidationLayers)
	{
		Extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
	}

	return Extensions;
}

void HelloTriangleApplication::SetupDebugCallback()
{
	if (!EnableValidationLayers)
	{
		return;
	}

	// Validation layers的行为可以有更多的设置，不仅仅是VkDebugReportCallbackCreateInfoEXT结构中指定的标志位信息。
	// 浏览Vulkan SDK的Config目录。找到vk_layer_settings.txt文件，里面有说明如何配置layers
	VkDebugReportCallbackCreateInfoEXT CreateInfo = {};

	CreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
	CreateInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | -VK_DEBUG_REPORT_WARNING_BIT_EXT;
	CreateInfo.pfnCallback = DebugCallback;

	if (CreateDebugReportCallbackEXT(Instance, &CreateInfo, nullptr, &Callback) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to set up debug callback!");
	}
}


void HelloTriangleApplication::CreateSurface()
{
	if (glfwCreateWindowSurface(Instance, Window, NULL, &Surface) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create window surface!");
	}
}

void HelloTriangleApplication::InitWindow()
{
	glfwInit();

	// 最初GLFW是为OpenGL创建上下文，
	// 所以在这里我们需要告诉它不要调用OpenGL相关的初始化操作。
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	Window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
}

void HelloTriangleApplication::InitVulkan()
{
	CreateInstance();
	CheckExtensions();
	SetupDebugCallback();
	CreateSurface();
	PickPhysicalDevice();
	CreateLogicalDevice();
	CreateSwapChain();
}

void HelloTriangleApplication::MainLoop()
{
	while (!glfwWindowShouldClose(Window))
	{
		glfwPollEvents();
	}
}

void HelloTriangleApplication::Cleanup()
{
	vkDestroySwapchainKHR(LogicalDevice, SwapChain, nullptr);
	vkDestroySurfaceKHR(Instance, Surface, nullptr);
	vkDestroyDevice(LogicalDevice, nullptr);
	DestroyDebugReportCallbackEXT(Instance, Callback, nullptr);
	vkDestroyInstance(Instance, nullptr);
	glfwDestroyWindow(Window);
	glfwTerminate();
}

void HelloTriangleApplication::PickPhysicalDevice()
{
	PhysicalDevice = VK_NULL_HANDLE;

	uint32_t PhysicalDeviceCount = 0;
	vkEnumeratePhysicalDevices(Instance, &PhysicalDeviceCount, nullptr);

	if (PhysicalDeviceCount == 0)
	{
		throw std::runtime_error("Failed to find GPUs with Vulkan support!");
	}

	std::vector<VkPhysicalDevice> PhysicalDevices(PhysicalDeviceCount);
	vkEnumeratePhysicalDevices(Instance, &PhysicalDeviceCount, PhysicalDevices.data());

	for (const auto& Device : PhysicalDevices)
	{
		if (IsDeviceSuitable(Device))
		{
			PhysicalDevice = Device;
			break;
		}
	}

	if (PhysicalDevice == VK_NULL_HANDLE)
	{
		throw std::runtime_error("Failed to find a suitable GPU!");
	}

	VkPhysicalDeviceProperties DeviceProperties;
	vkGetPhysicalDeviceProperties(PhysicalDevice, &DeviceProperties);

	VkPhysicalDeviceFeatures DeviceFeatures;
	vkGetPhysicalDeviceFeatures(PhysicalDevice, &DeviceFeatures);
}

bool HelloTriangleApplication::IsDeviceSuitable(VkPhysicalDevice Device)
{
	QueueFamilyIndices Indices = FindQueueFamilies(Device);

	bool ExtensionsSupported = CheckDeviceExtensionSupport(Device);

	bool SwapChainAdequate = false;

	if (ExtensionsSupported)
	{
		SwapChainSupprotDetails Details = QuerySwapChainSupport(Device);

		SwapChainAdequate = !Details.Formats.empty() && !Details.PresentModes.empty();
	}

	return Indices.IsComplete() && ExtensionsSupported && SwapChainAdequate;
}

QueueFamilyIndices HelloTriangleApplication::FindQueueFamilies(VkPhysicalDevice Device)
{
	QueueFamilyIndices FamilyIndices;

	uint32_t QueueFamilyCount = 0;

	vkGetPhysicalDeviceQueueFamilyProperties(Device, &QueueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> QueueFamilyProperties(QueueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(Device, &QueueFamilyCount, QueueFamilyProperties.data());

	int i = 0;

	for (const auto& QueueFamily : QueueFamilyProperties)
	{
		if (QueueFamily.queueCount > 0 && QueueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			FamilyIndices.GraphicsFamily = i;
		}

		VkBool32 PresentSupport = false;

		vkGetPhysicalDeviceSurfaceSupportKHR(Device, i, Surface, &PresentSupport);

		if (QueueFamily.queueCount > 0 && PresentSupport)
		{
			FamilyIndices.PresentFamily = i;
		}

		if (FamilyIndices.IsComplete())
		{
			break;
		}

		i++;
	}

	return FamilyIndices;
}

SwapChainSupprotDetails HelloTriangleApplication::QuerySwapChainSupport(VkPhysicalDevice Device)
{
	SwapChainSupprotDetails Details;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(Device, Surface, &Details.Capabilities);

	uint32_t FormatCount;

	vkGetPhysicalDeviceSurfaceFormatsKHR(Device, Surface, &FormatCount, nullptr);

	if (FormatCount != 0)
	{
		Details.Formats.resize(FormatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(Device, Surface, &FormatCount, Details.Formats.data());
	}

	uint32_t PresentModeCount;

	vkGetPhysicalDeviceSurfacePresentModesKHR(Device, Surface, &PresentModeCount, nullptr);

	if (PresentModeCount != 0)
	{
		Details.PresentModes.resize(PresentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(Device, Surface, &PresentModeCount, Details.PresentModes.data());
	}

	return Details;
}

VkSurfaceFormatKHR HelloTriangleApplication::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& AvailableFormats)
{
	if (AvailableFormats.size() == 1 && AvailableFormats[0].format == VK_FORMAT_UNDEFINED)
	{
		return { VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
	}

	for (const auto& AvailableFormat : AvailableFormats)
	{
		if (AvailableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && AvailableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			return AvailableFormat;
		}
	}

	return AvailableFormats[0];
}

VkPresentModeKHR HelloTriangleApplication::ChooseSwapPresentaMode(const std::vector<VkPresentModeKHR> AvailablePresentModes)
{
	VkPresentModeKHR BestMode = VK_PRESENT_MODE_FIFO_KHR;

	for (const auto& AvailablePresentMode : AvailablePresentModes)
	{
		if (AvailablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			return AvailablePresentMode;
		}
		else if (AvailablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR)
		{
			BestMode = AvailablePresentMode;
		}
	}

	return BestMode;
}

VkExtent2D HelloTriangleApplication::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& Capabilities)
{
	if (Capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
	{
		return Capabilities.currentExtent;
	}
	else
	{
		VkExtent2D ActualExtent = { WIDTH, HEIGHT };

		ActualExtent.width = std::max(Capabilities.minImageExtent.width, std::min(Capabilities.maxImageExtent.width, ActualExtent.width));
		ActualExtent.height = std::max(Capabilities.minImageExtent.height, std::min(Capabilities.maxImageExtent.height, ActualExtent.height));
	
		return ActualExtent;
	}
}

void HelloTriangleApplication::CreateLogicalDevice()
{
	QueueFamilyIndices Indices = FindQueueFamilies(PhysicalDevice);

	std::vector<VkDeviceQueueCreateInfo> QueueCreateInfos = {};
	std::set<int> UniqueQueueFamilies = { Indices.GraphicsFamily, Indices.PresentFamily };

	float QueuePriority = 1.0f;

	for (int QueueFamily : UniqueQueueFamilies)
	{
		VkDeviceQueueCreateInfo QueueCreateInfo = {};

		QueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		QueueCreateInfo.queueFamilyIndex = QueueFamily;
		QueueCreateInfo.queueCount = 1;
		QueueCreateInfo.pQueuePriorities = &QueuePriority;
		QueueCreateInfos.push_back(QueueCreateInfo);
	}

	VkPhysicalDeviceFeatures DeviceFeatures = {};

	VkDeviceCreateInfo DeviceCreateInfo = {};

	DeviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	DeviceCreateInfo.pQueueCreateInfos = QueueCreateInfos.data();
	DeviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(QueueCreateInfos.size());
	DeviceCreateInfo.pEnabledFeatures = &DeviceFeatures;
	DeviceCreateInfo.enabledExtensionCount = 0;

	if (EnableValidationLayers)
	{
		DeviceCreateInfo.enabledLayerCount = static_cast<uint32_t>(ValidationLayers.size());
		DeviceCreateInfo.ppEnabledLayerNames = ValidationLayers.data();
	}
	else
	{
		DeviceCreateInfo.enabledLayerCount = 0;
	}

	if (vkCreateDevice(PhysicalDevice, &DeviceCreateInfo, nullptr, &LogicalDevice) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create logical device!");
	}

	vkGetDeviceQueue(LogicalDevice, Indices.GraphicsFamily, 0, &GraphicQueue);
	vkGetDeviceQueue(LogicalDevice, Indices.PresentFamily, 0, &PresentQueue);
}

void HelloTriangleApplication::CreateSwapChain()
{
	SwapChainSupprotDetails Details = QuerySwapChainSupport(PhysicalDevice);

	VkSurfaceFormatKHR SurfaceFormat = ChooseSwapSurfaceFormat(Details.Formats);
	VkPresentModeKHR PresentMode = ChooseSwapPresentaMode(Details.PresentModes);
	VkExtent2D Extent = ChooseSwapExtent(Details.Capabilities);

	SwapChainImageFormat = SurfaceFormat.format;
	SwapChainExtent = Extent;

	uint32_t ImageCount = Details.Capabilities.minImageCount + 1;

	if (Details.Capabilities.maxImageCount > 0 && ImageCount > Details.Capabilities.maxImageCount)
	{
		ImageCount = Details.Capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR SwapChainCreateInfo = {};

	SwapChainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	SwapChainCreateInfo.surface = Surface;
	SwapChainCreateInfo.minImageCount = ImageCount;
	SwapChainCreateInfo.imageFormat = SurfaceFormat.format;
	SwapChainCreateInfo.imageColorSpace = SurfaceFormat.colorSpace;
	SwapChainCreateInfo.imageExtent = Extent;
	SwapChainCreateInfo.imageArrayLayers = 1;

	// imageArrayLayers指定每个图像组成的层数。除非我们开发3D应用程序，
	// 否则始终为1。imageUsage位字段指定在交换链中对图像进行的具体操作。
	// 在本小节中，我们将直接对它们进行渲染，这意味着它们作为颜色附件。
	// 也可以首先将图像渲染为单独的图像，进行后处理操作。
	// 在这种情况下可以使用像VK_IMAGE_USAGE_TRANSFER_DST_BIT这样的值，并使用内存操作将渲染的图像传输到交换链图像队列。
	SwapChainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	// 接下来，我们需要指定如何处理跨多个队列簇的交换链图像。
	// 如果graphics队列簇与presentation队列簇不同，会出现如下情形。
	// 我们将从graphics队列中绘制交换链的图像，然后在另一个presentation队列中提交他们。多队列处理图像有两种方法:
	// VK_SHARING_MODE_EXCLUSIVE: 同一时间图像只能被一个队列簇占用，如果其他队列簇需要其所有权需要明确指定。这种方式提供了最好的性能。
	// VK_SHARING_MODE_CONCURRENT : 图像可以被多个队列簇访问，不需要明确所有权从属关系。
	QueueFamilyIndices FamilyIndices = FindQueueFamilies(PhysicalDevice);

	uint32_t Indices[] = {(uint32_t)FamilyIndices.GraphicsFamily, (uint32_t)FamilyIndices.PresentFamily};

	if (FamilyIndices.GraphicsFamily != FamilyIndices.PresentFamily)
	{
		SwapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		SwapChainCreateInfo.queueFamilyIndexCount = 2;
		SwapChainCreateInfo.pQueueFamilyIndices = Indices;
	}
	else
	{
		SwapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		SwapChainCreateInfo.queueFamilyIndexCount = 0;
		SwapChainCreateInfo.pQueueFamilyIndices = nullptr;
	}

	// 如果交换链支持(supportedTransforms in capabilities),我们可以为交换链图像指定某些转换逻辑，
	// 比如90度顺时针旋转或者水平反转。如果不需要任何transoform操作，可以简单的设置为currentTransoform。
	SwapChainCreateInfo.preTransform = Details.Capabilities.currentTransform;
	// 混合Alpha字段指定alpha通道是否应用与与其他的窗体系统进行混合操作。如果忽略该功能，简单的填VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR。
	SwapChainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	SwapChainCreateInfo.presentMode = PresentMode;
	// 如果clipped成员设置为VK_TRUE，意味着我们不关心被遮蔽的像素数据，比如由于其他的窗体置于前方时或者渲染的部分内容存在于可是区域之外，
	// 除非真的需要读取这些像素获数据进行处理，否则可以开启裁剪获得最佳性能。
	SwapChainCreateInfo.clipped = VK_TRUE;
	SwapChainCreateInfo.oldSwapchain = VK_NULL_HANDLE;

	if (vkCreateSwapchainKHR(LogicalDevice, &SwapChainCreateInfo, nullptr, &SwapChain) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create swap chain!");
	}

	vkGetSwapchainImagesKHR(LogicalDevice, SwapChain, &ImageCount, nullptr);
	SwapChainImages.resize(ImageCount);
	vkGetSwapchainImagesKHR(LogicalDevice, SwapChain, &ImageCount, SwapChainImages.data());
}

void HelloTriangleApplication::Run()
{
	InitWindow();
	InitVulkan();
	MainLoop();
	Cleanup();
}

HelloTriangleApplication::~HelloTriangleApplication()
{
	Cleanup();
}