#include "Application.h"
#include "Utils.h"
#include <iostream>
#include <set>
#include <algorithm>
#include <chrono>
#include <GLFW/glfw3native.h>
#include <glm/gtc/matrix_transform.hpp>
#include <unordered_map>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

const int MAX_FRAMES_IN_FLIGHT = 2;

const std::string MODEL_PATH = "models/chalet.obj";
const std::string TEXTURE_PATH = "textures/chalet.jpg";

Application::Application(int inWindowWidth, int inWindowHeight, const std::string title)
	: windowWidth(inWindowWidth), 
	  windowHeight(inWindowHeight),
	  windowTitle(title)
{
}

void Application::createWindow(int inWindowWidth, int inWindowHeight, const std::string title)
{
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

	window = glfwCreateWindow(inWindowWidth, inWindowHeight, title.c_str(), nullptr, nullptr);

	const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());

	glfwSetWindowPos(window, mode->width / 2 - inWindowWidth / 2, mode->height / 2 - inWindowHeight / 2);

	glfwSetWindowUserPointer(window, this);
	glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
	glfwSetKeyCallback(window, keyCallback);
}

void Application::framebufferResizeCallback(GLFWwindow* window, int width, int height)
{
	auto app = reinterpret_cast<Application*>(glfwGetWindowUserPointer(window));
	app->framebufferResized = true;
}

void Application::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	Application* app = reinterpret_cast<Application*>(glfwGetWindowUserPointer(window));

	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, GLFW_TRUE);
	}
	else if (key == GLFW_KEY_LEFT && action != GLFW_RELEASE)
	{
		app->rotateAngle += 1.0f;
	}
	else if (key == GLFW_KEY_RIGHT && action != GLFW_RELEASE)
	{
		app->rotateAngle -= 1.0f;
	}

	glm::vec3 forward = glm::normalize(app->center - app->eyePosition);
	glm::vec3 right = glm::normalize(glm::cross(forward, app->up));
	
	if (key == GLFW_KEY_A && action != GLFW_RELEASE)
	{
		app->center -= right * 0.1f;
		app->eyePosition -= right * 0.1f;
	}
	else if (key == GLFW_KEY_D && action != GLFW_RELEASE)
	{
		app->center += right * 0.1f;
		app->eyePosition += right * 0.1f;
	}
	else if (key == GLFW_KEY_W && action != GLFW_RELEASE)
	{
		app->center += forward * 0.1f;
		app->eyePosition += forward * 0.1f;
	}
	else if (key == GLFW_KEY_S && action != GLFW_RELEASE)
	{
		app->center -= forward * 0.1f;
		app->eyePosition -= forward * 0.1f;
	}
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
	createImageViews();
	createRenderPass();
	createDescriptorSetLayout();
	createGraphicsPipeline();
	createCommandPool();
	createDepthResources();
	createColorResources();
	createFramebuffers();
	prepareTextureImages();
	createTextureImageView();
	prepareTextureSamples();
	loadModel();
	prepareVertexBufferAndIndexBuffer();
	prepareGeometryBuffers();
	prepareModelBuffers();
	createUniformBuffers();
	createDescriptorPool();
	createDescriptorSets();
	createCommandBuffers();
	createSyncObjects();
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
			vkRuntimeError("Enumerate instance layer properties failed!");
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
			vkRuntimeError("Enumerate instance extensions failed!");
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
		vkRuntimeError("failed to set up debug messenger!");
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
		vkRuntimeError("Failed to create window surface!");
	}
}

void Application::createInstance()
{
	if (enableValidationLayers && !checkValidationLayerSupport()) 
	{
		vkRuntimeError("validation layers requested, but not available!");
	}

	const VkApplicationInfo applicationInfo = {
	
		VK_STRUCTURE_TYPE_APPLICATION_INFO,	// sType
		nullptr,							// pNext
		"Vulkan Application",				// pApplicationName
		VK_MAKE_VERSION(1, 0, 0),			// applicationVersion
		"No Engine",						// pEngineName
		VK_MAKE_VERSION(1, 0, 0),			// engineVersion
		VK_API_VERSION_1_2					// apiVersion
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

	// ����ע��vkResult���سɹ�(VK_SUCCESS)ֵΪ0����0ֵ�������ִ�����������
	// ��ϸ�б��ɼ�vulkan_core.h
	VKCHECK(vkCreateInstance(&instanceCreateInfo, nullptr, &instance), "Create Vulkan instance failed.");
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
			vkRuntimeError("Enumerate device layer properties failed!");
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
			vkRuntimeError("Enumerate device extensions failed!");
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
		   swapChainAdequate &&
		   physicalDeviceFeatures.samplerAnisotropy;
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
			vkRuntimeError("failed to find GPUs with Vulkan support!");
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
			msaaSamples = getMaxUsableSampleCount();
			break;
		}
	}

	if (physicalDevice == VK_NULL_HANDLE) {
		vkRuntimeError("failed to find a suitable GPU!");
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

		if (queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT && i != 0)
		{
			indices.transferFamily = i;
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
	std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value(), indices.transferFamily.value() };

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

	VkPhysicalDeviceFeatures physicalDeviceFeatures = {};
	vkGetPhysicalDeviceFeatures(physicalDevice, &physicalDeviceFeatures);

	VkDeviceCreateInfo deviceCreateInfo = {};

	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
	deviceCreateInfo.pEnabledFeatures = &physicalDeviceFeatures;
	deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
	deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();

	if (enableValidationLayers)
	{
		deviceCreateInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		deviceCreateInfo.ppEnabledLayerNames = validationLayers.data();
	}
	else
	{
		deviceCreateInfo.enabledLayerCount = 0;
	}

	VKCHECK(vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &device), "Failed to create device!");

	vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
	vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
	vkGetDeviceQueue(device, indices.transferFamily.value(), 0, &transferQueue);
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
		glfwGetFramebufferSize(window, &(int)windowWidth, &(int)windowHeight);

		VkExtent2D actualExtent = { static_cast<uint32_t>(windowWidth),
									static_cast<uint32_t>(windowHeight) };

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

	uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.transferFamily.value() };

	// 
	if (indices.graphicsFamily != indices.transferFamily)
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

	VKCHECK(vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain), "Failed to create swap chain!");

	vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
	swapChainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());

	swapChainImageFormat = surfaceFormat.format;
	swapChainExtent = extent;
}

void Application::cleanupSwapChain()
{
	vkDestroyImageView(device, colorImageView, nullptr);
	vkDestroyImage(device, colorImage, nullptr);
	vkFreeMemory(device, colorImageMemory, nullptr);

	vkDestroyImageView(device, depthImageView, nullptr);
	vkDestroyImage(device, depthImage, nullptr);
	vkFreeMemory(device, depthImageMemory, nullptr);

	for (auto framebuffer : swapChainFramebuffers)
	{
		vkDestroyFramebuffer(device, framebuffer, nullptr);
	}

	vkFreeCommandBuffers(device, graphicsCommandPool, static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());

	vkDestroyPipeline(device, graphicsPipeline, nullptr);
	vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
	vkDestroyRenderPass(device, renderPass, nullptr);

	for (auto imageView : swapChainImageViews)
	{
		vkDestroyImageView(device, imageView, nullptr);
	}

	vkDestroySwapchainKHR(device, swapChain, nullptr);

	for (size_t i = 0; i < swapChainImages.size(); i++)
	{
		vkDestroyBuffer(device, uniformBuffers[i], nullptr);
		vkFreeMemory(device, uniformBufferMemorys[i], nullptr);
	}

	vkDestroyDescriptorPool(device, descriptorPool, nullptr);
}

void Application::recreateSwapChain()
{
	glfwGetFramebufferSize(window, &(int)windowWidth, &(int)windowHeight);

	while (windowWidth == 0 || windowHeight == 0)
	{
		glfwGetFramebufferSize(window, &(int)windowWidth, &(int)windowHeight);
		glfwWaitEvents();
	}

	vkDeviceWaitIdle(device);

	cleanupSwapChain();

	createSwapChain();
	createImageViews();
	createRenderPass();
	createGraphicsPipeline();
	createColorResources();
	createDepthResources();
	createFramebuffers();
	createUniformBuffers();
	createDescriptorPool();
	createDescriptorSets();
	createCommandBuffers();
}

void Application::createImageViews()
{
	swapChainImageViews.resize(swapChainImages.size());

	for (size_t i = 0; i < swapChainImages.size(); i++)
	{
		swapChainImageViews[i] = createImageView(swapChainImages[i], swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
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

	VKCHECK(vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule), "Failed to create shader module!");

	return shaderModule;
}

void Application::createRenderPass()
{
	VkAttachmentDescription colorAttachment = {};
	colorAttachment.format = swapChainImageFormat;
	colorAttachment.samples = msaaSamples;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference colorAttachmentReference = {};
	colorAttachmentReference.attachment = 0;
	colorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentDescription depthAttachment = {};
	depthAttachment.format = findDepthFormat();
	depthAttachment.samples = msaaSamples;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthAttachmentReference = {};
	depthAttachmentReference.attachment = 1;
	depthAttachmentReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentDescription colorAttachmentResolve = {};
	colorAttachmentResolve.format = swapChainImageFormat;
	colorAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colorAttachmentResolveRefence = {};
	colorAttachmentResolveRefence.attachment = 2;
	colorAttachmentResolveRefence.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;	
	subpass.pColorAttachments = &colorAttachmentReference; // layout��location = 0��out vec4 outColor
	subpass.pDepthStencilAttachment = &depthAttachmentReference;
	subpass.pResolveAttachments = &colorAttachmentResolveRefence;

	VkSubpassDependency dependency = {};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	std::array<VkAttachmentDescription, 3> attachements = { colorAttachment, depthAttachment, colorAttachmentResolve };

	VkRenderPassCreateInfo renderPassCreateInfo = {};
	renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassCreateInfo.attachmentCount = static_cast<uint32_t>(attachements.size());
	renderPassCreateInfo.pAttachments = attachements.data();
	renderPassCreateInfo.subpassCount = 1;
	renderPassCreateInfo.pSubpasses = &subpass;
	renderPassCreateInfo.dependencyCount = 1;
	renderPassCreateInfo.pDependencies = &dependency;

	VKCHECK(vkCreateRenderPass(device, &renderPassCreateInfo, nullptr, &renderPass), "Failed to create redner pass!");
}

void Application::createDescriptorSetLayout()
{
	VkDescriptorSetLayoutBinding uboLayoutBinding = {};
	uboLayoutBinding.binding = 0;
	uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uboLayoutBinding.descriptorCount = 1;
	uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	uboLayoutBinding.pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutBinding geometrySamplerLayoutBinding = {};
	geometrySamplerLayoutBinding.binding = 1;
	geometrySamplerLayoutBinding.descriptorCount = 1;
	geometrySamplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	geometrySamplerLayoutBinding.pImmutableSamplers = nullptr;
	geometrySamplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	VkDescriptorSetLayoutBinding modelSamplerLayoutBinding = {};
	modelSamplerLayoutBinding.binding = 2;
	modelSamplerLayoutBinding.descriptorCount = 1;
	modelSamplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	modelSamplerLayoutBinding.pImmutableSamplers = nullptr;
	modelSamplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	std::array<VkDescriptorSetLayoutBinding, 3> bindings = { uboLayoutBinding, geometrySamplerLayoutBinding, modelSamplerLayoutBinding };

	VkDescriptorSetLayoutCreateInfo layoutInfo = {};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
	layoutInfo.pBindings = bindings.data();

	VKCHECK(vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout),
											  "Failed to create descriptor set layout!");
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

	VkPipelineShaderStageCreateInfo vertexShaderStage = {};
	vertexShaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertexShaderStage.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertexShaderStage.module = vertexShaderModule;
	vertexShaderStage.pName = "main";

	VkPipelineShaderStageCreateInfo fragmentShaderStage = {};
	fragmentShaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragmentShaderStage.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragmentShaderStage.module = fragShaderModule;
	fragmentShaderStage.pName = "main";

	VkPipelineShaderStageCreateInfo shaderStages[] = { vertexShaderStage, fragmentShaderStage };

	auto bindingDescription = Vertex::getBindingDescription();
	auto attributeDescriptions = Vertex::getAttributeDescription();

	// Vertex input
	VkPipelineVertexInputStateCreateInfo vertexInputState = {};
	vertexInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputState.vertexBindingDescriptionCount = 1;
	vertexInputState.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
	vertexInputState.pVertexBindingDescriptions = &bindingDescription;
	vertexInputState.pVertexAttributeDescriptions = attributeDescriptions.data();

	// Input assembly
	VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = {};
	inputAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssemblyState.primitiveRestartEnable = VK_FALSE;

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

	// Viewport state
	VkPipelineViewportStateCreateInfo viewportState = {};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

	// Rasterization
	VkPipelineRasterizationStateCreateInfo rasterizationState = {};
	rasterizationState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizationState.depthClampEnable = VK_FALSE;
	rasterizationState.rasterizerDiscardEnable = VK_FALSE;
	rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizationState.lineWidth = 1.0f;
	rasterizationState.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizationState.depthBiasEnable = VK_FALSE;

	// Multisampling
	VkPipelineMultisampleStateCreateInfo multisamplingState = {};
	multisamplingState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisamplingState.sampleShadingEnable = VK_TRUE;
	multisamplingState.rasterizationSamples = msaaSamples;
	multisamplingState.minSampleShading = 1.0f;
	//multisamplingState.pSampleMask = nullptr;
	//multisamplingState.alphaToCoverageEnable = VK_FALSE;
	//multisamplingState.alphaToOneEnable = VK_FALSE;

	// Depth and stencil testing
	VkPipelineColorBlendAttachmentState colorBlendAttachmentState = {};
	colorBlendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
											   VK_COLOR_COMPONENT_G_BIT |
											   VK_COLOR_COMPONENT_B_BIT |
											   VK_COLOR_COMPONENT_A_BIT;

	colorBlendAttachmentState.blendEnable = VK_TRUE;
	colorBlendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	colorBlendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	colorBlendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;

	VkPipelineColorBlendStateCreateInfo colorBlendingState = {};
	colorBlendingState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlendingState.logicOpEnable = VK_FALSE;
	colorBlendingState.logicOp = VK_LOGIC_OP_COPY;
	colorBlendingState.attachmentCount = 1;
	colorBlendingState.pAttachments = &colorBlendAttachmentState;
	colorBlendingState.blendConstants[0] = 0.0f;
	colorBlendingState.blendConstants[1] = 0.0f;
	colorBlendingState.blendConstants[2] = 0.0f;
	colorBlendingState.blendConstants[3] = 0.0f;

	// Dynamic state
	VkDynamicState dynamicStates[] = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};

	VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo = {};
	dynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicStateCreateInfo.dynamicStateCount = 2;
	dynamicStateCreateInfo.pDynamicStates = dynamicStates;

	// Pipeline layout
	VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
	pipelineLayoutInfo.pushConstantRangeCount = 0;

	VKCHECK(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout),
		"Failed to create pipeline layout!");

	VkPipelineDepthStencilStateCreateInfo depthStencilState = {};
	depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencilState.depthTestEnable = VK_TRUE;
	depthStencilState.depthWriteEnable = VK_TRUE;
	depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS;
	depthStencilState.depthBoundsTestEnable = VK_FALSE;
	depthStencilState.minDepthBounds = 0.0f;
	depthStencilState.maxDepthBounds = 1.0f;
	depthStencilState.stencilTestEnable = VK_FALSE;
	depthStencilState.front = {};
	depthStencilState.back = {};

	VkGraphicsPipelineCreateInfo pipelineInfo = {};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStages;
	pipelineInfo.pVertexInputState = &vertexInputState;
	pipelineInfo.pInputAssemblyState = &inputAssemblyState;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizationState;
	pipelineInfo.pDepthStencilState = &depthStencilState;
	pipelineInfo.pMultisampleState = &multisamplingState;
	pipelineInfo.pColorBlendState = &colorBlendingState;
	pipelineInfo.pDynamicState = &dynamicStateCreateInfo;
	pipelineInfo.layout = pipelineLayout;
	pipelineInfo.renderPass = renderPass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

	VKCHECK(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline),
		"Failed to create graphics pipeline!");

	vkDestroyShaderModule(device, fragShaderModule, nullptr);
	vkDestroyShaderModule(device, vertexShaderModule, nullptr);
}

void Application::createFramebuffers()
{
	swapChainFramebuffers.resize(swapChainImageViews.size());

	for (size_t i = 0; i < swapChainImageViews.size(); i++)
	{
		std::array<VkImageView, 3> attachments = {
			colorImageView,
			depthImageView,
			swapChainImageViews[i]
		};

		VkFramebufferCreateInfo framebufferInfo = {};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = renderPass;
		framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		framebufferInfo.pAttachments = attachments.data();
		framebufferInfo.width = swapChainExtent.width;
		framebufferInfo.height = swapChainExtent.height;
		framebufferInfo.layers = 1;

		VKCHECK(vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapChainFramebuffers[i]),
			"Failed to create framebuffer!");
	}
}

void Application::createCommandPool()
{
	QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);

	VkCommandPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

	VKCHECK(vkCreateCommandPool(device, &poolInfo, nullptr, &graphicsCommandPool), "Failed to create command pool!");

	VkCommandPoolCreateInfo transformPoolInfo = {};
	transformPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	transformPoolInfo.queueFamilyIndex = queueFamilyIndices.transferFamily.value();

	VKCHECK(vkCreateCommandPool(device, &transformPoolInfo, nullptr, &transferCommandPool),
		"Failed to create transfer command pool!");
}

VkFormat Application::findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
{
	VkFormatProperties properties;

	for (VkFormat format : candidates)
	{
		vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &properties);

		if (tiling == VK_IMAGE_TILING_LINEAR && (properties.linearTilingFeatures & features) == features)
		{
			return format;
		}
		else if (tiling == VK_IMAGE_TILING_OPTIMAL && (properties.optimalTilingFeatures & features) == features)
		{
			return format;
		}
	}

	throw std::runtime_error("Failed to find supported format!");
}

VkFormat Application::findDepthFormat()
{
	return findSupportedFormat({ VK_FORMAT_D32_SFLOAT, 
								 VK_FORMAT_D32_SFLOAT_S8_UINT, 
								 VK_FORMAT_D24_UNORM_S8_UINT },
								 VK_IMAGE_TILING_OPTIMAL,
								 VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

bool Application::hasStencilComponent(VkFormat format)
{
	return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

void Application::createDepthResources()
{
	VkFormat depthFormat = findDepthFormat();

	createImage(swapChainExtent.width, swapChainExtent.height, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depthImage, depthImageMemory, 1, msaaSamples);
	
	depthImageView = createImageView(depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, 1);

	transitionImageLayout(depthImage, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 1);
}

void Application::createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling,
VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory, uint32_t mipLevels, VkSampleCountFlagBits numSamples)
{

	VkImageCreateInfo imageInfo = {};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = static_cast<uint32_t>(width);
	imageInfo.extent.height = static_cast<uint32_t>(height);
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = mipLevels;
	imageInfo.arrayLayers = 1;
	imageInfo.format = format;
	imageInfo.tiling = tiling;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage = usage;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageInfo.samples = numSamples;
	imageInfo.flags = 0;

	VKCHECK(vkCreateImage(device, &imageInfo, nullptr, &image), "Failed to create image!");

	VkMemoryRequirements memoryRequirements;
	vkGetImageMemoryRequirements(device, image, &memoryRequirements);

	VkMemoryAllocateInfo allocatedInfo = {};
	allocatedInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocatedInfo.allocationSize = memoryRequirements.size;
	allocatedInfo.memoryTypeIndex = findMemoryType(memoryRequirements.memoryTypeBits, properties);

	VKCHECK(vkAllocateMemory(device, &allocatedInfo, nullptr, &imageMemory), "Failed to allocate image memory!");

	vkBindImageMemory(device, image, imageMemory, 0);
}

void Application::transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels)
{
	VkCommandBuffer commandBuffer = beginSingleTimeCommands(graphicsCommandPool);

	VkImageMemoryBarrier barrier = {};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = image;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = mipLevels;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;
	barrier.srcAccessMask = 0;
	barrier.dstAccessMask = 0;

	VkPipelineStageFlags sourceStage;
	VkPipelineStageFlags destinationStage;

	if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	}
	else
	{
		throw std::invalid_argument("Unsupported layout transition!");
	}

	if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
	{
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

		if (hasStencilComponent(format))
		{
			barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
		}
	}
	else
	{
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	}

	vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0,
		0, nullptr,
		0, nullptr,
		1, &barrier);

	endSingleTimeCommands(commandBuffer, graphicsQueue, graphicsCommandPool);
}

void Application::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
{
	VkCommandBuffer commandBuffer = beginSingleTimeCommands(transferCommandPool);

	VkBufferImageCopy region = {};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;

	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;

	region.imageOffset = { 0, 0, 0 };
	region.imageExtent = { width, height, 1 };

	vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

	endSingleTimeCommands(commandBuffer, transferQueue, transferCommandPool);
}

void Application::prepareTextureImages()
{
	createTextureImage("textures/texture.jpg", geometryTextureImage, geometryTextureDeviceMemory, geometryMipLevels);
	createTextureImage(TEXTURE_PATH, modelTextureImage, modelTextureImageMemory, modelMipLevels);
}

void Application::createTextureImage(std::string filePath, VkImage& image, VkDeviceMemory& imageMemory, uint32_t& mipLevels)
{
	int textureWidth;
	int textureHeight;
	int textureChannels;

	stbi_uc* pixels = stbi_load(filePath.c_str(), &textureWidth, &textureHeight, &textureChannels, STBI_rgb_alpha);

	mipLevels = static_cast<uint32_t>(std::floor(std::log2(max(textureWidth, textureHeight)))) + 1;

	VkDeviceSize imageSize = textureWidth * textureHeight * 4;

	if (!pixels)
	{
		vkRuntimeError("Failed to load texture image!");
	}

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;

	createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
							VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
							VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
							stagingBuffer, stagingBufferMemory);

	void* data;
	vkMapMemory(device, stagingBufferMemory, 0, imageSize, 0, &data);
	memcpy_s(data, static_cast<size_t>(imageSize), pixels, static_cast<size_t>(imageSize));
	vkUnmapMemory(device, stagingBufferMemory);

	stbi_image_free(pixels);

	createImage(textureWidth, textureHeight, VK_FORMAT_R8G8B8A8_SRGB, 
											 VK_IMAGE_TILING_OPTIMAL,
											 VK_IMAGE_USAGE_TRANSFER_DST_BIT | 
											 VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
											 VK_IMAGE_USAGE_SAMPLED_BIT, 
											 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
											 image, imageMemory, mipLevels);

	transitionImageLayout(image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mipLevels);
	copyBufferToImage(stagingBuffer, image, static_cast<uint32_t>(textureWidth), static_cast<uint32_t>(textureHeight));
	//transitionImageLayout(image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, mipLevels);

	generateMipmaps(image, VK_FORMAT_R8G8B8A8_SRGB, textureWidth, textureHeight, mipLevels);

	vkDestroyBuffer(device, stagingBuffer, nullptr);
	vkFreeMemory(device, stagingBufferMemory, nullptr);
}

VkImageView Application::createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels)
{
	VkImageViewCreateInfo imageViewInfo = {};
	imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	imageViewInfo.image = image;
	imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	imageViewInfo.format = format;
	imageViewInfo.subresourceRange.aspectMask = aspectFlags;
	imageViewInfo.subresourceRange.baseMipLevel = 0;
	imageViewInfo.subresourceRange.levelCount = mipLevels;
	imageViewInfo.subresourceRange.baseArrayLayer = 0;
	imageViewInfo.subresourceRange.layerCount = 1;

	VkImageView imageView;

	VKCHECK(vkCreateImageView(device, &imageViewInfo, nullptr, &imageView), "Failed to create texture image view!");

	return imageView;
}

void Application::createTextureImageView()
{
	geometryTextureImageView = createImageView(geometryTextureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, geometryMipLevels);
	modelTextureImageView = createImageView(modelTextureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, modelMipLevels);
}

void Application::createTextureSampler(VkSampler& sampler, uint32_t mipLevels)
{
	VkSamplerCreateInfo samplerInfo = {};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.anisotropyEnable = VK_TRUE;
	samplerInfo.maxAnisotropy = 16;
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.mipLodBias = 0.0f;
	samplerInfo.minLod = 0;// static_cast<float>(mipLevels / 2);
	samplerInfo.maxLod = static_cast<float>(mipLevels);

	VKCHECK(vkCreateSampler(device, &samplerInfo, nullptr, &sampler), "Failed to create texture sampler!");
}

void Application::prepareTextureSamples()
{
	createTextureSampler(geometryTextureSampler, geometryMipLevels);
	createTextureSampler(modelTextureSampler, modelMipLevels);
}

uint32_t Application::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
	VkPhysicalDeviceMemoryProperties memoryProperties;
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);

	// typedef enum VkMemoryHeapFlagBits {
	//		VK_MEMORY_HEAP_DEVICE_LOCAL_BIT = 0x00000001,
	//		VK_MEMORY_HEAP_MULTI_INSTANCE_BIT = 0x00000002,
	//		VK_MEMORY_HEAP_MULTI_INSTANCE_BIT_KHR = VK_MEMORY_HEAP_MULTI_INSTANCE_BIT,
	//		VK_MEMORY_HEAP_FLAG_BITS_MAX_ENUM = 0x7FFFFFFF
	// } VkMemoryHeapFlagBits;
	// We may have more than one desirable property, so we should 
	// check if the result of the bitwise AND is not just non-zero, 
	// but equal to the desired properties bit field. If there is 
	// a memory type suitable for the buffer that also has all of 
	// the properties we need, then we return its index, otherwise 
	// we throw an exception.
	for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++)
	{
		if ((typeFilter & (1 << i)) && (memoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
		{
			return i;
		}
	}

	vkRuntimeError("Failed to find suitable memory type!");

	return -1;
}

void Application::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
{
	VkBufferCreateInfo bufferInfo = {};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VKCHECK(vkCreateBuffer(device, &bufferInfo, nullptr, &buffer), "Failed to create vertex buffer!");

	VkMemoryRequirements memoryRequirements;
	vkGetBufferMemoryRequirements(device, buffer, &memoryRequirements);

	VkMemoryAllocateInfo allocateInfo = {};
	allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocateInfo.allocationSize = memoryRequirements.size;
	allocateInfo.memoryTypeIndex = findMemoryType(memoryRequirements.memoryTypeBits, 
												  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
												  VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);


	VKCHECK(vkAllocateMemory(device, &allocateInfo, nullptr, &bufferMemory), "Failed to allocate vertex buffer memory!");

	vkBindBufferMemory(device, buffer, bufferMemory, 0);
}

void Application::createBuffer(VkDeviceSize bufferSize, VkBufferUsageFlags usage, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
{
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;

	createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |	// RAM
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		stagingBuffer, stagingBufferMemory);

	void* data = nullptr;
	vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy_s(data, (size_t)bufferSize, geometryVertices.data(), (size_t)bufferSize);
	vkUnmapMemory(device, stagingBufferMemory);

	createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT |
							 usage,
							 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,	// VRAM
		buffer, bufferMemory);

	copyBuffer(stagingBuffer, buffer, bufferSize);

	vkDestroyBuffer(device, stagingBuffer, nullptr);
	vkFreeMemory(device, stagingBufferMemory, nullptr);
}

VkCommandBuffer Application::beginSingleTimeCommands(VkCommandPool commandPool)
{
	VkCommandBufferAllocateInfo allocateInfo = {};
	allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocateInfo.commandPool = commandPool;
	allocateInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(device, &allocateInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	return commandBuffer;
}

void Application::endSingleTimeCommands(VkCommandBuffer commandBuffer, VkQueue queue, VkCommandPool commandPool)
{
	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(queue);

	vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
}

void Application::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
	VkCommandBuffer commandBuffer = beginSingleTimeCommands(transferCommandPool);

	VkBufferCopy copyRegion = {};
	copyRegion.srcOffset = 0;
	copyRegion.dstOffset = 0;
	copyRegion.size = size;

	vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

	endSingleTimeCommands(commandBuffer, transferQueue, transferCommandPool);
}

void Application::loadModel()
{
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string warn;
	std::string error;

	if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &error, MODEL_PATH.c_str()))
	{
		throw std::runtime_error(warn + error);
	}

	std::unordered_map<Vertex, uint32_t> uniqueVertices = {};

	for (const auto& shape : shapes)
	{
		for (const auto& index : shape.mesh.indices)
		{
			Vertex vertex = {};

			vertex.position = {
				attrib.vertices[3 * index.vertex_index + 0],
				attrib.vertices[3 * index.vertex_index + 1],
				attrib.vertices[3 * index.vertex_index + 2]
			};

			if (attrib.texcoords.size() > 0)
			{
				vertex.texCoord = {
					attrib.texcoords[2 * index.texcoord_index + 0],
					1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
				};
			}

			vertex.color = { 1.0f, 1.0f, 1.0f };

			if (uniqueVertices.count(vertex) == 0)
			{
				uniqueVertices[vertex] = static_cast<uint32_t>(modelVertices.size());
				modelVertices.push_back(vertex);
			}

			modelIndices.push_back(uniqueVertices[vertex]);
		}
	}
}

void Application::prepareVertexBufferAndIndexBuffer()
{
	createBuffer(geometryVertices, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, geometryVertexBuffer, geometryVertexBufferMemory);

	createBuffer(geometryIndices, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, geometryIndexBuffer, geometryIndexBufferMemory);

	createBuffer(modelVertices, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, modelVertexBuffer, modelVertexBufferMemory);

	createBuffer(modelIndices, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, modelIndexBuffer, modelIndexBufferMemory);
}

void Application::createVertexBuffer(const std::vector<Vertex>& vertices, VkBuffer& vertexBuffer, VkDeviceMemory& vertexBufferMemory)
{
	VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;

	createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
						     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |	// RAM
						     VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
						     stagingBuffer, stagingBufferMemory);

	void* data = nullptr;
	vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy_s(data, (size_t)bufferSize, geometryVertices.data(), (size_t)bufferSize);
	vkUnmapMemory(device, stagingBufferMemory);

	createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT |
							 VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
							 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,	// VRAM
							 vertexBuffer, vertexBufferMemory);

	copyBuffer(stagingBuffer, vertexBuffer, bufferSize);

	vkDestroyBuffer(device, stagingBuffer, nullptr);
	vkFreeMemory(device, stagingBufferMemory, nullptr);
}

void Application::createIndexBuffer(const std::vector<uint32_t>& indices, VkBuffer& indexBuffer, VkDeviceMemory& indexBufferMemory)
{
	VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
							 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |	// RAM
							 VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,	
							 stagingBuffer, stagingBufferMemory);

	void* data = nullptr;
	vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy_s(data, (size_t)bufferSize, geometryIndices.data(), (size_t)bufferSize);
	vkUnmapMemory(device, stagingBufferMemory);

	createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT |
							 VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
							 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,	// VRAM
							 geometryIndexBuffer, geometryIndexBufferMemory);

	copyBuffer(stagingBuffer, geometryIndexBuffer, bufferSize);

	vkDestroyBuffer(device, stagingBuffer, nullptr);
	vkFreeMemory(device, stagingBufferMemory, nullptr);
}

void Application::prepareModelResources()
{

}

void Application::createAllInOneBuffer(const std::vector<Vertex>& vertices, 
									   const std::vector<uint32_t>& indeices, 
									   VkBuffer& buffer, VkDeviceMemory& bufferMemory)
{
	VkDeviceSize vertexBufferSize = sizeof(vertices[0]) * vertices.size();
	VkDeviceSize indexBufferSize = sizeof(indeices[0]) * indeices.size();

	VkDeviceSize allInOneBufferSize = vertexBufferSize + indexBufferSize;

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	createBuffer(allInOneBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
									 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
									 VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
									 stagingBuffer, stagingBufferMemory);

	void* data = nullptr;
	vkMapMemory(device, stagingBufferMemory, 0, allInOneBufferSize, 0, &data);
	memcpy_s(data, (size_t)vertexBufferSize, vertices.data(), (size_t)vertexBufferSize);
	memcpy_s((Vertex*)data + vertices.size(), (size_t)indexBufferSize, indeices.data(), (size_t)indexBufferSize);
	vkUnmapMemory(device, stagingBufferMemory);

	createBuffer(allInOneBufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT |
									 VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
									 VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
									 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
									 buffer, bufferMemory);

	copyBuffer(stagingBuffer, buffer, allInOneBufferSize);
	vkDestroyBuffer(device, stagingBuffer, nullptr);
	vkFreeMemory(device, stagingBufferMemory, nullptr);
}

void Application::prepareGeometryBuffers()
{
	createAllInOneBuffer(geometryVertices, geometryIndices, allInOneBuffer, allInOneBufferMemory);
}

void Application::prepareModelBuffers()
{
	createAllInOneBuffer(modelVertices, modelIndices, modelAllInOneBuffer, modelAllInOneBufferMemory);
}

void Application::createUniformBuffers()
{
	VkDeviceSize bufferSize = sizeof(UniformBufferObject);

	uniformBuffers.resize(swapChainImages.size());
	uniformBufferMemorys.resize(swapChainImages.size());

	for (size_t i = 0; i < swapChainImages.size(); i++)
	{
		createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
								 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
								 VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
								 uniformBuffers[i], uniformBufferMemorys[i]);
	}
}

void Application::createDescriptorPool()
{
	std::array<VkDescriptorPoolSize, 2> poolSizes;
	poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[0].descriptorCount = static_cast<uint32_t>(swapChainImages.size());
	poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[1].descriptorCount = static_cast<uint32_t>(swapChainImages.size());

	VkDescriptorPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	poolInfo.pPoolSizes = poolSizes.data();
	poolInfo.maxSets = static_cast<uint32_t>(swapChainImages.size());

	VKCHECK(vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool), "Failed to create descriptor pool!");
}

void Application::createDescriptorSets()
{
	// In our case we will create one descriptor set for each swap chain image, all with the same layout. 
	// Unfortunately we do need all the copies of the layout because the next function expects an array 
	// matching the number of sets.
	std::vector<VkDescriptorSetLayout> layouts(swapChainImages.size(), descriptorSetLayout);
	VkDescriptorSetAllocateInfo allocateInfo = {};
	allocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocateInfo.descriptorPool = descriptorPool;
	allocateInfo.descriptorSetCount = static_cast<uint32_t>(swapChainImages.size());
	allocateInfo.pSetLayouts = layouts.data();

	descriptorSets.resize(swapChainImages.size());

	VKCHECK(vkAllocateDescriptorSets(device, &allocateInfo, descriptorSets.data()), "Failed to allocate descriptor sets!");

	for (size_t i = 0; i < swapChainImages.size(); i++)
	{
		VkDescriptorBufferInfo bufferInfo = {};
		bufferInfo.buffer = uniformBuffers[i];
		bufferInfo.offset = 0;
		bufferInfo.range = sizeof(UniformBufferObject);

		VkDescriptorImageInfo geometryImageInfo = {};
		geometryImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		geometryImageInfo.imageView = geometryTextureImageView;
		geometryImageInfo.sampler = geometryTextureSampler;

		VkDescriptorImageInfo modelImageInfo = {};
		modelImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		modelImageInfo.imageView = modelTextureImageView;
		modelImageInfo.sampler = modelTextureSampler;

		std::array<VkWriteDescriptorSet, 3> descriptorWrites = {};
		descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[0].dstSet = descriptorSets[i];
		descriptorWrites[0].dstBinding = 0;
		descriptorWrites[0].dstArrayElement = 0;
		descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrites[0].descriptorCount = 1;
		descriptorWrites[0].pBufferInfo = &bufferInfo;
		descriptorWrites[0].pImageInfo = nullptr;

		descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[1].dstSet = descriptorSets[i];
		descriptorWrites[1].dstBinding = 1;
		descriptorWrites[1].dstArrayElement = 0;
		descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrites[1].descriptorCount = 1;
		descriptorWrites[1].pImageInfo = &geometryImageInfo;

		descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[2].dstSet = descriptorSets[i];
		descriptorWrites[2].dstBinding = 2;
		descriptorWrites[2].dstArrayElement = 0;
		descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrites[2].descriptorCount = 1;
		descriptorWrites[2].pImageInfo = &modelImageInfo;

		vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
	}
}

void Application::updateUniformBuffer(uint32_t currentImage)
{
	static auto startTime = std::chrono::high_resolution_clock::now();

	auto currentTime = std::chrono::high_resolution_clock::now();

	float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

	time = 1.0f;

	UniformBufferObject ubo = {};
	ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(rotateAngle), up);

	ubo.view = glm::lookAt(eyePosition, center, glm::vec3(0.0f, 0.0f, 1.0f));

	ubo.projection = glm::perspective(glm::radians(45.0f), swapChainExtent.width / (float)swapChainExtent.height, 0.1f, 10.0f);

	ubo.projection[1][1] *= -1;

	void* data;
	vkMapMemory(device, uniformBufferMemorys[currentImage], 0, sizeof(ubo), 0, &data);
	memcpy_s(data, sizeof(ubo), &ubo, sizeof(ubo));
	vkUnmapMemory(device, uniformBufferMemorys[currentImage]);
}

void Application::createCommandBuffers()
{
	commandBuffers.resize(swapChainFramebuffers.size());

	VkCommandBufferAllocateInfo allocateInfo = {};
	allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocateInfo.commandPool = graphicsCommandPool;
	allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocateInfo.commandBufferCount = (uint32_t)commandBuffers.size();

	VKCHECK(vkAllocateCommandBuffers(device, &allocateInfo, commandBuffers.data()), "Failed to allocate command buffers!");

	for (size_t i = 0; i < commandBuffers.size(); i++)
	{
		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		VKCHECK(vkBeginCommandBuffer(commandBuffers[i], &beginInfo), "Failed to begin recording command buffer!");

		VkRenderPassBeginInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = renderPass;
		renderPassInfo.framebuffer = swapChainFramebuffers[i];
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = swapChainExtent;

		std::array<VkClearValue, 2> clearValues = {};
		clearValues[0] = Color::CornFlower;
		clearValues[1].depthStencil = { 1.0f, 0 };

		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

		VkBuffer vertexBuffers[] = { modelAllInOneBuffer };
		VkDeviceSize offsets[] = { 0 };

		vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, vertexBuffers, offsets);

		VkDeviceSize offset = sizeof(modelVertices[0]) * modelVertices.size();

		vkCmdBindIndexBuffer(commandBuffers[i], modelAllInOneBuffer, offset, VK_INDEX_TYPE_UINT32);

		VkViewport viewport = {};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(windowWidth);
		viewport.height = static_cast<float>(windowHeight);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor = {};
		scissor.offset = {0, 0};
		scissor.extent = { windowWidth, windowHeight };

		vkCmdSetViewport(commandBuffers[i], 0, 1, &viewport);
		vkCmdSetScissor(commandBuffers[i], 0, 1, &scissor);

		vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, 
								pipelineLayout, 0, 1, &descriptorSets[i], 0, nullptr);

		vkCmdDrawIndexed(commandBuffers[i], static_cast<uint32_t>(modelIndices.size()), 1, 0, 0, 0);

		vkCmdEndRenderPass(commandBuffers[i]);

		VKCHECK(vkEndCommandBuffer(commandBuffers[i]), "Failed to record command buffer!");
	}
}

void Application::createTransferCommandBuffers()
{

}

void Application::createSyncObjects()
{
	imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
	imagesInFlight.resize(swapChainImages.size(), VK_NULL_HANDLE);

	VkSemaphoreCreateInfo semaphoreInfo = {};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceInfo = {};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		VKCHECK(vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]), 
										"Failed to create synchronization objects for a frame!");
		VKCHECK(vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]), 
										"Failed to create synchronization objects for a frame!");
		VKCHECK(vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]), 
										"Failed to create synchronization objects for a frame!");
	}
}

void Application::generateMipmaps(VkImage image, VkFormat imageFormat, int32_t textureWidth, int32_t textureHeight, uint32_t mipLevels)
{
	// Check if image format supports linear blitting
	VkFormatProperties formatProperties;
	vkGetPhysicalDeviceFormatProperties(physicalDevice, imageFormat, &formatProperties);

	if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT))
	{
		throw std::runtime_error("Texture image format does not support linear blitting!");
	}

	VkCommandBuffer commandBuffer = beginSingleTimeCommands(graphicsCommandPool);

	VkImageMemoryBarrier barrier = {};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.image = image;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.layerCount = 1;
	barrier.subresourceRange.levelCount = 1;

	int32_t mipWidth = textureWidth;
	int32_t mipHeight = textureHeight;

	for (uint32_t i = 1; i < mipLevels; i++)
	{
		barrier.subresourceRange.baseMipLevel = i - 1;
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
											VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
											0, nullptr,
											0, nullptr,
											1, &barrier);

		VkImageBlit blitRegion = {};
		blitRegion.srcOffsets[0] = { 0, 0, 0 };
		blitRegion.srcOffsets[1] = { mipWidth, mipHeight, 1 };
		blitRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blitRegion.srcSubresource.mipLevel = i - 1;
		blitRegion.srcSubresource.baseArrayLayer = 0;
		blitRegion.srcSubresource.layerCount = 1;
		blitRegion.dstOffsets[0] = { 0, 0, 0 };
		blitRegion.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
		blitRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blitRegion.dstSubresource.mipLevel = i;
		blitRegion.dstSubresource.baseArrayLayer = 0;
		blitRegion.dstSubresource.layerCount = 1;

		vkCmdBlitImage(commandBuffer, 
					   image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
					   image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
					   1, &blitRegion,
					   VK_FILTER_LINEAR);

		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		vkCmdPipelineBarrier(commandBuffer,
							 VK_PIPELINE_STAGE_TRANSFER_BIT,
							 VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
							 0, nullptr,
							 0, nullptr,
							 1, &barrier);

		if (mipWidth > 1) mipWidth /= 2;
		if (mipHeight > 1) mipHeight /= 2;
	}

	barrier.subresourceRange.baseMipLevel = mipLevels - 1;
	barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

	vkCmdPipelineBarrier(commandBuffer,
						 VK_PIPELINE_STAGE_TRANSFER_BIT,
		                 VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
						 0, nullptr,
		                 0, nullptr,
						 1, &barrier);

	endSingleTimeCommands(commandBuffer, graphicsQueue, graphicsCommandPool);
}

VkSampleCountFlagBits Application::getMaxUsableSampleCount()
{
	VkPhysicalDeviceProperties physicalDeviceProperties;
	vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);

	VkSampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts &																  physicalDeviceProperties.limits.framebufferDepthSampleCounts;

	if (counts & VK_SAMPLE_COUNT_64_BIT) { return VK_SAMPLE_COUNT_64_BIT; }
	if (counts & VK_SAMPLE_COUNT_32_BIT) { return VK_SAMPLE_COUNT_32_BIT; }
	if (counts & VK_SAMPLE_COUNT_16_BIT) { return VK_SAMPLE_COUNT_16_BIT; }
	if (counts & VK_SAMPLE_COUNT_8_BIT) { return VK_SAMPLE_COUNT_8_BIT; }
	if (counts & VK_SAMPLE_COUNT_4_BIT) { return VK_SAMPLE_COUNT_4_BIT; }
	if (counts & VK_SAMPLE_COUNT_2_BIT) { return VK_SAMPLE_COUNT_2_BIT; }

	return VK_SAMPLE_COUNT_1_BIT;
}

void Application::createColorResources()
{
	VkFormat colorFormat = swapChainImageFormat;

	createImage(swapChainExtent.width, swapChainExtent.height, colorFormat, VK_IMAGE_TILING_OPTIMAL,
				VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, colorImage, colorImageMemory, 1, msaaSamples);

	colorImageView = createImageView(colorImage, colorFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
}

void Application::drawFrame()
{
	vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

	uint32_t imageIndex = 0;
	// When the presentation engine is finished using the image, imageAvailableSemaphore will be signaled.
	// imageIndex is the index of swap chain image that has become available.
	VkResult result = vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

	if (result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		recreateSwapChain();
		return;
	}
	else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
	{
		vkRuntimeError("Failed to acquire swap chain image!");
	}

	// Check if a previous frame is using this image(i.e. there is its fence to wait on)
	if (imagesInFlight[imageIndex] != VK_NULL_HANDLE)
	{
		vkWaitForFences(device, 1, &imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
	}

	// Mark the image as now being in use by this frame
	imagesInFlight[imageIndex] = inFlightFences[currentFrame];

	updateUniformBuffer(imageIndex);

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrame] };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffers[imageIndex];

	VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame] };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	vkResetFences(device, 1, &inFlightFences[currentFrame]);

	VKCHECK(vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]), "Failed to submit draw command buffer!");

	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;

	VkSwapchainKHR swapChains[] = { swapChain };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &imageIndex;
	presentInfo.pResults = nullptr;

	result = vkQueuePresentKHR(presentQueue, &presentInfo);

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized)
	{
		framebufferResized = false;
		recreateSwapChain();
	}
	else if (result != VK_SUCCESS)
	{
		vkRuntimeError("Failed to present swap chain image!");
	}

	currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

	vkQueueWaitIdle(presentQueue);
}

void Application::mainLoop()
{
	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
		drawFrame();
	}

	vkDeviceWaitIdle(device);
}

void Application::cleanup()
{
	cleanupSwapChain();

	vkDestroySampler(device, modelTextureSampler, nullptr);
	vkDestroyImageView(device, modelTextureImageView, nullptr);
	vkDestroyImage(device, modelTextureImage, nullptr);
	vkFreeMemory(device, modelTextureImageMemory, nullptr);

	vkDestroySampler(device, geometryTextureSampler, nullptr);
	vkDestroyImageView(device, geometryTextureImageView, nullptr);

	vkDestroyImage(device, geometryTextureImage, nullptr);
	vkFreeMemory(device, geometryTextureDeviceMemory, nullptr);

	vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);

	vkDestroyBuffer(device, modelAllInOneBuffer, nullptr);
	vkFreeMemory(device, modelAllInOneBufferMemory, nullptr);

	vkDestroyBuffer(device, allInOneBuffer, nullptr);
	vkFreeMemory(device, allInOneBufferMemory, nullptr);
	
	vkDestroyBuffer(device, modelIndexBuffer, nullptr);
	vkFreeMemory(device, modelIndexBufferMemory, nullptr);

	vkDestroyBuffer(device, modelVertexBuffer, nullptr);
	vkFreeMemory(device, modelVertexBufferMemory, nullptr);

	vkDestroyBuffer(device, geometryIndexBuffer, nullptr);
	vkFreeMemory(device, geometryIndexBufferMemory, nullptr);

	vkDestroyBuffer(device, geometryVertexBuffer, nullptr);
	vkFreeMemory(device, geometryVertexBufferMemory, nullptr);

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
		vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
		vkDestroyFence(device, inFlightFences[i], nullptr);
	}

	vkDestroyCommandPool(device, transferCommandPool, nullptr);
	vkDestroyCommandPool(device, graphicsCommandPool, nullptr);

	vkDestroyDevice(device, nullptr);

	if (enableValidationLayers)
	{
		destroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
	}

	vkDestroySurfaceKHR(instance, surface, nullptr);
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