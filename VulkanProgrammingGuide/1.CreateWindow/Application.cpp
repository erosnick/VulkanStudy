#include "Application.h"
#include "Utils.h"
#include <iostream>
#include <set>
#include <algorithm>
#include <GLFW/glfw3native.h>

Application::Application(int inWindowWidth, int inWindowHeight, const std::string title)
	: windowWidth(inWindowWidth), 
	  windowHeight(inWindowHeight),
	  windowTitle(title),
	  window(nullptr),
	  instance(VK_NULL_HANDLE),
	  device(VK_NULL_HANDLE),
	  physicalDevice(VK_NULL_HANDLE),
	  debugMessenger(VK_NULL_HANDLE),
	  graphicsQueue(VK_NULL_HANDLE),
	  presentQueue(VK_NULL_HANDLE),
	  surface(VK_NULL_HANDLE),
	  swapChain(VK_NULL_HANDLE),
	  swapChainImageFormat(VK_FORMAT_UNDEFINED),
	  swapChainExtent(VkExtent2D())
{
}

void Application::createWindow(int inWindowWidth, int inWindowHeight, const std::string title)
{
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	window = glfwCreateWindow(inWindowWidth, inWindowHeight, title.c_str(), nullptr, nullptr);
}

void Application::initializeVulkan()
{
	queryInstanceExtensions();
	createInstance();
	setupDebugMessenger();
	createSurface();
	pickPhysicalDevice();
	//queryDeviceLayers();
	//queryDeviceExtensions();
	//queryDeviceProperties();
	createLogicalDevice();
	createSwapChain();
	createGraphicsPipeline();
}

bool Application::checkValidationLayerSupport()
{
	uint32_t instanceLayerPropertyCount = 0;
	std::vector<VkLayerProperties> availableInstanceLayerProperties;

	if (!vkEnumerateInstanceLayerProperties(&instanceLayerPropertyCount, nullptr))
	{
		availableInstanceLayerProperties.resize(instanceLayerPropertyCount);
		if (vkEnumerateInstanceLayerProperties(&instanceLayerPropertyCount, availableInstanceLayerProperties.data()))
		{
			std::cout << "Enumerate instance layer properties failed." << std::endl;
		}
	}

	//std::cout << "Available instance layers:" << std::endl;

	for (const char* layerName : validationLayers)
	{
		bool layerFound = false;

		for (const auto layerProperty : availableInstanceLayerProperties)
		{
			if (strcmp(layerName, layerProperty.layerName) == 0)
			{
				layerFound = true;
			}

			//std::cout << "\t" << layerProperty.layerName << std::endl;
		}

		//std::cout << std::endl;

		if (!layerFound)
		{
			return false;
		}
	}

	return true;
}


void Application::queryInstanceExtensions()
{
	uint32_t instanceExtensionCount = 0;
	std::vector<VkExtensionProperties> deviceExtensions;

	if (!vkEnumerateInstanceExtensionProperties(nullptr, &instanceExtensionCount, nullptr))
	{
		deviceExtensions.resize(instanceExtensionCount);
		if (vkEnumerateInstanceExtensionProperties(nullptr, &instanceExtensionCount, deviceExtensions.data()))
		{
			std::cout << "Enumerate instance extensions failed." << std::endl;
		}
	}

	std::cout << "Available instance extensions:" << std::endl;

	for (const auto& extension : deviceExtensions)
	{
		std::cout << "\t" << extension.extensionName << std::endl;
	}

	std::cout << std::endl;
}

std::vector<const char*> Application::getRequiredExtensions()
{
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;

	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

	if (enableValidationLayers)
	{
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

	return extensions;
}

void Application::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
{
	createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

	createInfo.pfnUserCallback = debugCallback;
	createInfo.pUserData = nullptr;	// Optional
}

void Application::setupDebugMessenger()
{
	if (!enableValidationLayers)
	{
		return;
	}

	VkDebugUtilsMessengerCreateInfoEXT createInfo;
	populateDebugMessengerCreateInfo(createInfo);

	if (createDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
		throw std::runtime_error("failed to set up debug messenger!");
	}
}

void Application::createSurface()
{
	//VkWin32SurfaceCreateInfoKHR createInfo = {};
	//createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	//createInfo.hwnd = glfwGetWin32Window(window);
	//createInfo.hinstance = GetModuleHandle(nullptr);

	if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create window surface!");
	}
}

void Application::createInstance()
{
	if (enableValidationLayers && !checkValidationLayerSupport()) 
	{
		throw std::runtime_error("validation layers requested, but not available!");
	}

	const VkApplicationInfo applicationInfo = {
	
		VK_STRUCTURE_TYPE_APPLICATION_INFO,	// sType
		nullptr,							// pNext
		"Vulkan Application",				// pApplicationName
		VK_MAKE_VERSION(1, 0, 0),			// applicationVersion
		"No Engine",						// pEngineName
		VK_MAKE_VERSION(1, 0, 0),			// engineVersion
		VK_API_VERSION_1_1					// apiVersion
	};

	auto extensions = getRequiredExtensions();

	VkInstanceCreateInfo instanceCreateInfo = {
	
		VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,		// sType
		nullptr,									// pNext
		0,											// flags
		&applicationInfo,							// pApplicationInfo
		0,											// enabledLayerCount
		nullptr,									// ppEnabledLayerNames
		static_cast<uint32_t>(extensions.size()),	// enabledExtensionCount
		extensions.data()							// ppEnabledExtensionNames
	};

	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;

	if (enableValidationLayers)
	{
		instanceCreateInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		instanceCreateInfo.ppEnabledLayerNames = validationLayers.data();

		populateDebugMessengerCreateInfo(debugCreateInfo);
		instanceCreateInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
	}

	// 这里注意vkResult返回成功(VK_SUCCESS)值为0，非0值代表各种错误和其他情况
	// 详细列表可见vulkan_core.h
	if (vkCreateInstance(&instanceCreateInfo, nullptr, &instance))
	{
		std::cout << "Create Vulkan instance failed." << std::endl;
		//throw std::runtime_error("Failed to create instance!");
	}
}

void Application::queryDeviceLayers()
{
	uint32_t deviceLayerPropertyCount = 0;
	std::vector<VkLayerProperties> deviceLayerProperties;

	if (!vkEnumerateDeviceLayerProperties(physicalDevice, &deviceLayerPropertyCount, nullptr))
	{
		deviceLayerProperties.resize(deviceLayerPropertyCount);
		if (vkEnumerateDeviceLayerProperties(physicalDevice, &deviceLayerPropertyCount, deviceLayerProperties.data()))
		{
			std::cout << "Enumerate device layer properties failed." << std::endl;
		}
	}

	std::cout << "Available device layers:" << std::endl;

	for (auto layerProperty : deviceLayerProperties)
	{
		std::cout << "\t" << layerProperty.layerName << std::endl;
	}

	std::cout << std::endl;
}

bool Application::checkDeviceExtensionSupport(VkPhysicalDevice device)
{
	uint32_t deviceExtensionCount = 0;
	std::vector<VkExtensionProperties> availableExtensions;
	if (!vkEnumerateDeviceExtensionProperties(device, nullptr, &deviceExtensionCount, nullptr))
	{
		availableExtensions.resize(deviceExtensionCount);
		if (vkEnumerateDeviceExtensionProperties(device, nullptr, &deviceExtensionCount, availableExtensions.data()))
		{
			std::cout << "Enumerate device extensions failed." << std::endl;
		}
	}

	std::cout << "Available device extensions:" << std::endl;

	for (const auto& extension : availableExtensions)
	{
		std::cout << "\t" << extension.extensionName << std::endl;
	}

	std::cout << std::endl;

	std::set<std::string>requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

	for (const auto extension : availableExtensions)
	{
		requiredExtensions.erase(extension.extensionName);
	}

	return requiredExtensions.empty();
}

bool Application::isDeviceSuitable(VkPhysicalDevice device)
{
	VkPhysicalDeviceProperties physicalDeviceProperties;
	VkPhysicalDeviceFeatures physicalDeviceFeatures;
	vkGetPhysicalDeviceProperties(device, &physicalDeviceProperties);
	vkGetPhysicalDeviceFeatures(device, &physicalDeviceFeatures);

	QueueFamilyIndices indices = findQueueFamilies(device);

	bool extensionsSupported = checkDeviceExtensionSupport(device);

	bool swapChainAdequate = false;

	if (extensionsSupported)
	{
		SwapChainSupportDetails swapChainSupport = querySwapCahinSupport(device);
		swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModeS.empty();
	}

	// Our application only usable for dedicated graphics cards that support geometry shaders
	return physicalDeviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
		   physicalDeviceFeatures.geometryShader &&
		   indices.isComplete() &&
		   extensionsSupported &&
		   swapChainAdequate; 
}

int Application::rateDeviceSuitability(VkPhysicalDevice device)
{
	int score = 0;

	VkPhysicalDeviceProperties physicalDeviceProperties;
	VkPhysicalDeviceFeatures physicalDeviceFeatures;
	vkGetPhysicalDeviceProperties(device, &physicalDeviceProperties);
	vkGetPhysicalDeviceFeatures(device, &physicalDeviceFeatures);

	// Discrete GPUs have a significant performance advantage
	if (physicalDeviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
	{
		score += 1000;
	}

	// Maximum possible size of textures affects graphics quality
	score += physicalDeviceProperties.limits.maxImageDimension2D;

	// Application can't function without geometry shaders
	if (!physicalDeviceFeatures.geometryShader)
	{
		return 0;
	}

	return score;
}

void Application::pickPhysicalDevice()
{
	uint32_t physicalDeviceCount = 0;

	std::vector<VkPhysicalDevice> physicalDevices;

	if (!vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, nullptr))
	{
		if (physicalDeviceCount == 0) {
			throw std::runtime_error("failed to find GPUs with Vulkan support!");
		}

		physicalDevices.resize(physicalDeviceCount);
		if (vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, physicalDevices.data()))
		{
			std::cout << "Enumerate physical devices failed." << std::endl;
		}
	}

	for (const auto device : physicalDevices)
	{
		if (isDeviceSuitable(device))
		{
			physicalDevice = device;
			break;
		}
	}

	if (physicalDevice == VK_NULL_HANDLE) {
		throw std::runtime_error("failed to find a suitable GPU!");
	}
}

void Application::queryDeviceProperties()
{
	VkPhysicalDeviceProperties physicalDeviceProperties;
	vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);

	VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties;
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &physicalDeviceMemoryProperties);

	uint32_t queueFamilyPropertyCount = 0;
	std::vector<VkQueueFamilyProperties> queueFamilyProperties;

	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyPropertyCount, nullptr);

	queueFamilyProperties.resize(queueFamilyPropertyCount);

	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyPropertyCount, queueFamilyProperties.data());

	//vkGetPhysicalDeviceFormatProperties(physicalDevice, )
}

QueueFamilyIndices Application::findQueueFamilies(VkPhysicalDevice device)
{
	QueueFamilyIndices indices;

	uint32_t queueFamilyCount = 0;

	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

	int i = 0;

	for (const auto queueFamily : queueFamilies)
	{
		if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			indices.graphicsFamily = i;
		}

		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

		if (presentSupport)
		{
			indices.presentFamily = i;
		}

		if (indices.isComplete())
		{
			break;
		}

		i++;
	}

	return indices;
}

void Application::createLogicalDevice()
{
	QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };

	float queuePriority = 1.0f;
	for (uint32_t queueFamily : uniqueQueueFamilies)
	{
		VkDeviceQueueCreateInfo queueCreateInfo = {};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfos.push_back(queueCreateInfo);
	}

	VkPhysicalDeviceFeatures physicalDeviceFeatures;
	vkGetPhysicalDeviceFeatures(physicalDevice, &physicalDeviceFeatures);

	VkDeviceCreateInfo deviceCreateInfo = {

		VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,			// sType
		nullptr,										// pNext
		0,												// flags
		static_cast<uint32_t>(queueCreateInfos.size()),	// queueCreateInfoCount
		queueCreateInfos.data(),						// pQueueCreateInfos
		0,												// enabledLayerCount
		nullptr,										// ppEnabledLayerNames
		static_cast<uint32_t>(deviceExtensions.size()),	// enabledExtensionCount
		deviceExtensions.data(),						// ppEnabledExtensionNames
		&physicalDeviceFeatures							// pEnabledFeatures
	};

	if (enableValidationLayers)
	{
		deviceCreateInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		deviceCreateInfo.ppEnabledLayerNames = validationLayers.data();
	}

	if (vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &device))
	{
		std::cout << "Create device failed." << std::endl;
	}

	vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
	vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
}

SwapChainSupportDetails Application::querySwapCahinSupport(VkPhysicalDevice device)
{
	SwapChainSupportDetails details;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

	uint32_t formatCount = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

	if (formatCount != 0)
	{
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
	}

	uint32_t presentModeCount = 0;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

	if (presentModeCount != 0)
	{
		details.presentModeS.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModeS.data());
	}

	return details;
}

VkSurfaceFormatKHR Application::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
	for (const auto availableFormat : availableFormats)
	{
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
			availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			return availableFormat;
		}
	}

	return availableFormats[0];
}

VkPresentModeKHR Application::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
{
	for (const auto availablePresentMode : availablePresentModes)
	{
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			return availablePresentMode;
		}
	}

	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D Application::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
{
	if (capabilities.currentExtent.width != UINT32_MAX)
	{
		return capabilities.currentExtent;
	}
	else
	{
		VkExtent2D actualExtent = { windowWidth, windowHeight };

		actualExtent.width = max(capabilities.minImageExtent.width,
							 min(capabilities.maxImageExtent.width, actualExtent.width));

		actualExtent.height = max(capabilities.minImageExtent.height,
							  min(capabilities.maxImageExtent.height, actualExtent.height));

		return actualExtent;
	}
}

void Application::createSwapChain()
{
	SwapChainSupportDetails swapChainSupport = querySwapCahinSupport(physicalDevice);

	VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
	VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModeS);
	VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

	uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;

	if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
	{
		imageCount = swapChainSupport.capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = surface;
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

	uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

	if (indices.graphicsFamily != indices.presentFamily)
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0;
		createInfo.pQueueFamilyIndices = nullptr;
	}

	createInfo.preTransform = swapChainSupport.capabilities.currentTransform;	// No transform
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;				// Ignore window alpha
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;	// Best performance
	createInfo.oldSwapchain = VK_NULL_HANDLE;

	if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create swap chain!");
	}

	vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
	swapChainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());

	swapChainImageFormat = surfaceFormat.format;
	swapChainExtent = extent;
}

void Application::createImageViews()
{
	swapChainImageViews.resize(swapChainImages.size());

	for (size_t i = 0; i < swapChainImages.size(); i++)
	{
		VkImageViewCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = swapChainImages[i];
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = swapChainImageFormat;
		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		if (vkCreateImageView(device, &createInfo, nullptr, &swapChainImageViews[i]) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create image views!");
		}
	}
}

VkShaderModule Application::createShaderModule(const std::vector<char>& code)
{
	VkShaderModuleCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	// VERY IMPORTANT! cause pCode is uint32_t pointer and byte code is
	// specified in bytes
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

	VkShaderModule shaderModule;
	if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create shader module!");
	}

	return shaderModule;
}

void Application::createGraphicsPipeline()
{
	auto vertexShaderCode = readFile("shaders/vert.spv");
	auto fragShaderCode = readFile("shaders/frag.spv");

	std::cout << "vert.spv size: " << vertexShaderCode.size() << std::endl;
	std::cout << "frag.spv size: " << fragShaderCode.size() << std::endl;

	// Shader modules
	VkShaderModule vertexShaderModule = createShaderModule(vertexShaderCode);
	VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);
	
	VkPipelineShaderStageCreateInfo vertexShaderStageInfo = {};
	vertexShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertexShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertexShaderStageInfo.module = vertexShaderModule;
	vertexShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo fragmentShaderStageInfo = {};
	vertexShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertexShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	vertexShaderStageInfo.module = fragShaderModule;
	vertexShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo shaderStages[] = { vertexShaderStageInfo, fragmentShaderStageInfo };

	// Vertex input
	VkPipelineVertexInputStateCreateInfo vertexInput = {};
	vertexInput.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInput.vertexBindingDescriptionCount = 0;
	vertexInput.pVertexAttributeDescriptions = nullptr;
	vertexInput.vertexAttributeDescriptionCount;
	vertexInput.pVertexAttributeDescriptions = nullptr;

	// Input assembly
	VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	// Viewport and scissors
	VkViewport viewport = {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)swapChainExtent.width;
	viewport.height = (float)swapChainExtent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor = {};
	scissor.offset = { 0, 0 };
	scissor.extent = swapChainExtent;

	VkPipelineViewportStateCreateInfo viewportState;
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

	vkDestroyShaderModule(device, fragShaderModule, nullptr);
	vkDestroyShaderModule(device, vertexShaderModule, nullptr);
}

void Application::createBuffer()
{
	const VkBufferCreateInfo bufferCreateInfo = {
		VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		nullptr,
		0,
		1024 * 1024,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		VK_SHARING_MODE_EXCLUSIVE,
		0,
		nullptr
	};

	VkBuffer buffer = VK_NULL_HANDLE;
	vkCreateBuffer(device, &bufferCreateInfo, nullptr, &buffer);
}

void Application::mainLoop()
{
	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
	}
}

void Application::cleanup()
{
	if (enableValidationLayers)
	{
		destroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
	}

	vkDeviceWaitIdle(device);
	vkDestroyDevice(device, nullptr);
	vkDestroySurfaceKHR(instance, surface, nullptr);
	
	for (auto imageView : swapChainImageViews)
	{
		vkDestroyImageView(device, imageView, nullptr);
	}

	vkDestroySwapchainKHR(device, swapChain, nullptr);
	vkDestroyInstance(instance, nullptr);

	glfwDestroyWindow(window);
	glfwTerminate();
}

Application::~Application()
{
	cleanup();
}

void Application::run()
{
	createWindow(windowWidth, windowHeight, windowTitle);
	initializeVulkan();
	mainLoop();
}
