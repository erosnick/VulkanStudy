#include "VulkanApplication.h"

#include <map>
#include <set>
#include <cstdint>
#include <limits>
#include <algorithm>
#include <fstream>
#include <chrono>
#include <random>
#include <thread>
#include <filesystem>
#include <regex>

#include "glm.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <optick.h>

#include "LoadModelObj.h"
#include "GeometryGenerator.h"
#include "labutils/error.hpp"
#include "labutils/to_string.hpp"

#include <EtcLib/Etc/Etc.h>
#include <EtcLib/Etc/EtcImage.h>
#include <EtcLib/Etc/EtcFilter.h>
#include <EtcTool/EtcFile.h>

#include "VulkanInitializers.h"

#include "Timer.h"

#include <tgen.h>

GeometryGenerator geometryGenerator;

VkResult createDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) 
{
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");

	if (func != nullptr)
	{
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	}
	else 
	{
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

void destroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) 
{
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr) 
	{
		func(instance, debugMessenger, pAllocator);
	}
}

std::vector<char> readFile(const std::string& filaName)
{
	std::ifstream file(filaName, std::ios::ate | std::ios::binary);

	if (!file.is_open())
	{
		THROW_ERROR("Failed to open file " + filaName + "!");
	}
	
	size_t fileSize = static_cast<size_t>(file.tellg());
	std::vector<char> buffer(fileSize);

	file.seekg(0);
	file.read(buffer.data(), fileSize);

	if (fileSize != buffer.size())
	{
		THROW_ERROR("Shader byte size not match!");
	}

	file.close();

	return buffer;
}

std::string VkResultToString(VkResult result)
{
	std::string message = "";

	switch (result)
	{
	case VK_SUCCESS:
		message = "VK_SUCCESS";
		break;
	case VK_NOT_READY:
		message = "VK_NOT_READY";
		break;
	case VK_TIMEOUT:
		message = "VK_TIMEOUT";
		break;
	case VK_EVENT_SET:
		message = "VK_EVENT_SET";
		break;
	case VK_EVENT_RESET:
		message = "VK_EVENT_RESET";
		break;
	case VK_INCOMPLETE:
		message = "VK_INCOMPLETE";
		break;
	case VK_ERROR_OUT_OF_HOST_MEMORY:
		message = "VK_ERROR_OUT_OF_HOST_MEMORY";
		break;
	case VK_ERROR_OUT_OF_DEVICE_MEMORY:
		message = "VK_ERROR_OUT_OF_DEVICE_MEMORY";
		break;
	case VK_ERROR_INITIALIZATION_FAILED:
		message = "VK_ERROR_INITIALIZATION_FAILED";
		break;
	case VK_ERROR_DEVICE_LOST:
		message = "VK_ERROR_DEVICE_LOST";
		break;
	case VK_ERROR_MEMORY_MAP_FAILED:
		message = "VK_ERROR_MEMORY_MAP_FAILED";
		break;
	case VK_ERROR_LAYER_NOT_PRESENT:
		message = "VK_ERROR_LAYER_NOT_PRESENT";
		break;
	case VK_ERROR_EXTENSION_NOT_PRESENT:
		message = "VK_ERROR_EXTENSION_NOT_PRESENT";
		break;
	case VK_ERROR_FEATURE_NOT_PRESENT:
		message = "VK_ERROR_FEATURE_NOT_PRESENT";
		break;
	case VK_ERROR_INCOMPATIBLE_DRIVER:
		message = "VK_ERROR_INCOMPATIBLE_DRIVER";
		break;
	case VK_ERROR_TOO_MANY_OBJECTS:
		message = "VK_ERROR_TOO_MANY_OBJECTS";
		break;
	case VK_ERROR_FORMAT_NOT_SUPPORTED:
		message = "VK_ERROR_FORMAT_NOT_SUPPORTED";
		break;
	case VK_ERROR_FRAGMENTED_POOL:
		message = "VK_ERROR_FRAGMENTED_POOL";
		break;
	case VK_ERROR_UNKNOWN:
		message = "VK_ERROR_UNKNOWN";
		break;
	case VK_ERROR_OUT_OF_POOL_MEMORY | VK_ERROR_OUT_OF_POOL_MEMORY_KHR:
		message = "VK_ERROR_OUT_OF_POOL_MEMORY";
		break;
	case VK_ERROR_INVALID_EXTERNAL_HANDLE | VK_ERROR_INVALID_EXTERNAL_HANDLE_KHR:
		message = "VK_ERROR_INVALID_EXTERNAL_HANDLE";
		break;
	case VK_ERROR_FRAGMENTATION | VK_ERROR_FRAGMENTATION_EXT:
		message = "VK_ERROR_FRAGMENTATION";
		break;
	case VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS | VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS_KHR:
		message = "VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS";
		break;
	case VK_PIPELINE_COMPILE_REQUIRED | VK_PIPELINE_COMPILE_REQUIRED_EXT | VK_ERROR_PIPELINE_COMPILE_REQUIRED_EXT:
		message = "VK_PIPELINE_COMPILE_REQUIRED";
		break;
	case VK_ERROR_SURFACE_LOST_KHR:
		message = "VK_ERROR_SURFACE_LOST_KHR";
		break;
	case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
		message = "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR";
		break;
	case VK_SUBOPTIMAL_KHR:
		message = "VK_SUBOPTIMAL_KHR";
		break;
	case VK_ERROR_OUT_OF_DATE_KHR:
		message = "VK_ERROR_OUT_OF_DATE_KHR";
		break;
	case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:
		message = "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR";
		break;
	case VK_ERROR_VALIDATION_FAILED_EXT:
		message = "VK_ERROR_VALIDATION_FAILED_EXT";
		break;
	case VK_ERROR_INVALID_SHADER_NV:
		message = "VK_ERROR_INVALID_SHADER_NV";
		break;
	case VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT:
		message = "VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT";
		break;
	case VK_ERROR_NOT_PERMITTED_KHR | VK_ERROR_NOT_PERMITTED_EXT:
		message = "VK_ERROR_NOT_PERMITTED_KHR";
		break;
	case VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT:
		message = "VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT";
		break;
	case VK_THREAD_IDLE_KHR:
		message = "VK_THREAD_IDLE_KHR";
		break;
	case VK_THREAD_DONE_KHR:
		message = "VK_THREAD_DONE_KHR";
		break;
	case VK_OPERATION_DEFERRED_KHR:
		message = "VK_OPERATION_DEFERRED_KHR";
		break;
	case VK_OPERATION_NOT_DEFERRED_KHR:
		message = "VK_OPERATION_NOT_DEFERRED_KHR";
		break;
	case VK_ERROR_COMPRESSION_EXHAUSTED_EXT:
		message = "VK_ERROR_COMPRESSION_EXHAUSTED_EXT";
		break;
	default:
		break;
	}

	return message;
}

VulkanApplication::VulkanApplication()
: window(nullptr),
  allocator(VK_NULL_HANDLE),
  instance(VK_NULL_HANDLE), 
  physicalDevice(VK_NULL_HANDLE),
  device(VK_NULL_HANDLE),
  graphicsQueue(VK_NULL_HANDLE),
  computeQueue(VK_NULL_HANDLE),
  presentQueue(VK_NULL_HANDLE),
  transferQueue(VK_NULL_HANDLE),
  surface(VK_NULL_HANDLE),
  swapChain(VK_NULL_HANDLE),
  renderPass(VK_NULL_HANDLE),
  graphicsDescriptorSetLayout(VK_NULL_HANDLE),
  computeDescriptorSetLayout(VK_NULL_HANDLE),
  graphicsPipelineLayout(VK_NULL_HANDLE),
  particlePipelineLayout(VK_NULL_HANDLE),
  computePipelineLayout(VK_NULL_HANDLE),
  graphicsPipeline(VK_NULL_HANDLE),
  particlePipeline(VK_NULL_HANDLE),
  computePipeline(VK_NULL_HANDLE),
  graphicsCommandPool(VK_NULL_HANDLE),
  transferCommandPool(VK_NULL_HANDLE),
  descriptorPool(VK_NULL_HANDLE),
  textureSampler(VK_NULL_HANDLE),
  swapChainImageFormat(VK_FORMAT_B8G8R8A8_SRGB),
  swapChainExtent{ 0, 0 },
  debugMessenger(VK_NULL_HANDLE),
  minImageCount(0),
  imageCount(0),
  currentFrame(0),
  framebufferResized(false),
  frameCount(0)
{
}

VulkanApplication::~VulkanApplication()
{
}

void VulkanApplication::run()
{
	initOptick();
	initWindow();
	initVulkan();
	initImGui();
	mainLoop();
	cleanup();
}

void VulkanApplication::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, true);
	}

	if (key == GLFW_KEY_UP && action == GLFW_PRESS)
	{
		turnOnLightCount = (turnOnLightCount + 1) % (LightCount + 1);
	}

	if (key == GLFW_KEY_DOWN && action == GLFW_PRESS)
	{
		turnOnLightCount = (turnOnLightCount - 1) % (LightCount + 1);
	}

	if (key == GLFW_KEY_MINUS && action == GLFW_PRESS)
	{
		particleSpeed -= 1000.0f;
	}

	if (key == GLFW_KEY_EQUAL && action == GLFW_PRESS)
	{
		particleSpeed += 1000.0f;
	}

	if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
	{
		bloom = !bloom;
	}

	if (key == GLFW_KEY_1 && action == GLFW_PRESS)
	{
		tonemapping = true;
	}

	if (key == GLFW_KEY_2 && action == GLFW_PRESS)
	{
		tonemapping = false;
	}
}

void VulkanApplication::createInstance()
{
	if (EnableValidationLayers && !checkValidationLayerSupport())
	{
		THROW_ERROR("Validation layers requested, but not available!");
	}

	VkApplicationInfo appInfo{};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pNext = nullptr;
	appInfo.pApplicationName = "Vulkan Application";
	appInfo.pEngineName = "Vulkan Engine";
	appInfo.engineVersion = VK_API_VERSION_1_3;
	appInfo.apiVersion = VK_API_VERSION_1_3;

	VkInstanceCreateInfo instanceCreateInfo{};
	instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceCreateInfo.pApplicationInfo = &appInfo;

	auto extensions = getRequiredExtensions();

	instanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	instanceCreateInfo.ppEnabledExtensionNames = extensions.data();

	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};

	if (EnableValidationLayers)
	{
		instanceCreateInfo.enabledLayerCount = static_cast<uint32_t>(ValidationLayers.size());
		instanceCreateInfo.ppEnabledLayerNames = ValidationLayers.data();

		populateDebugMessengerCreateInfo(debugCreateInfo);
		instanceCreateInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
	}
	else
	{
		instanceCreateInfo.enabledLayerCount = 0;
		instanceCreateInfo.pNext = nullptr;
	}
	
	VkCheck(vkCreateInstance(&instanceCreateInfo, allocator, &instance), "Failed to create instance!");

	// Load rest of the Vulkan API
	volkLoadInstance(instance);

	checkExtensionSupport();
}

void VulkanApplication::setupDebugMessenger()
{
	if (!EnableValidationLayers)
	{
		return;
	}

	VkDebugUtilsMessengerCreateInfoEXT debugMessengerCreateInfo{};
	populateDebugMessengerCreateInfo(debugMessengerCreateInfo);

	if (createDebugUtilsMessengerEXT(instance, &debugMessengerCreateInfo, allocator, &debugMessenger) != VK_SUCCESS)
	{
		THROW_ERROR("failed to set up debug messenger!");
	}
}

void VulkanApplication::createSurface()
{
	if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS)
	{
		THROW_ERROR("Failed to create window surface!");
	}
}

void VulkanApplication::pickPhysicalDevice()
{
	uint32_t deviceCount = 0;

	vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

	if (deviceCount == 0)
	{
		THROW_ERROR("Failed to find GPUs with Vulkan support!");
	}

	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

	for (const auto& device : devices)
	{
		if (isDeviceSuitable(device))
		{
			physicalDevice = device;
			msaaSamples = VK_SAMPLE_COUNT_1_BIT;// getMaxUsableSampleCount();
			break;
		}
	}

	if (physicalDevice == VK_NULL_HANDLE)
	{
		THROW_ERROR("Failed to find a suitable GPU!");
	}
}

void VulkanApplication::createLogicalDevice()
{
	QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

	std::set<uint32_t> uniqueQueueFamilies = { queueFamilyIndices.graphicsAndComputeFamily.value(), queueFamilyIndices.presentFamily.value(), queueFamilyIndices.transferFamily.value() };

	float queuePriority = 1.0f;

	for (uint32_t queueFamily : uniqueQueueFamilies)
	{
		VkDeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfos.emplace_back(queueCreateInfo);
	}

	VkPhysicalDeviceFeatures physicalDeviceFeatures{};

	if (anisotropyEnable)
	{
		physicalDeviceFeatures.samplerAnisotropy = VK_TRUE;
	}
	else
	{
		physicalDeviceFeatures.samplerAnisotropy = VK_FALSE;
	}

	physicalDeviceFeatures.shaderSampledImageArrayDynamicIndexing = VK_TRUE;
	physicalDeviceFeatures.sampleRateShading = VK_TRUE;

	VkDeviceCreateInfo deviceCreateInfo{};
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();

	deviceCreateInfo.pEnabledFeatures = &physicalDeviceFeatures;

	deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(DeviceExtensions.size());
	deviceCreateInfo.ppEnabledExtensionNames = DeviceExtensions.data();

	if (EnableValidationLayers)
	{
		deviceCreateInfo.enabledLayerCount = static_cast<uint32_t>(ValidationLayers.size());
		deviceCreateInfo.ppEnabledLayerNames = ValidationLayers.data();
	}
	else
	{
		deviceCreateInfo.enabledLayerCount = 0;
	}

	VkPhysicalDeviceDescriptorIndexingFeaturesEXT indexingFeatures{};
	indexingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES_EXT;
	indexingFeatures.pNext = nullptr;
	indexingFeatures.descriptorBindingPartiallyBound = VK_TRUE;
	indexingFeatures.runtimeDescriptorArray = VK_TRUE;
	indexingFeatures.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;
	indexingFeatures.descriptorBindingVariableDescriptorCount = VK_TRUE;

	deviceCreateInfo.pNext = &indexingFeatures;

	VkCheck(vkCreateDevice(physicalDevice, &deviceCreateInfo, allocator, &device), "Failed to create logical device!");

	vkGetDeviceQueue(device, queueFamilyIndices.graphicsAndComputeFamily.value(), 0, &graphicsQueue);
	vkGetDeviceQueue(device, queueFamilyIndices.graphicsAndComputeFamily.value(), 0, &computeQueue);
	vkGetDeviceQueue(device, queueFamilyIndices.presentFamily.value(), 0, &presentQueue);
	vkGetDeviceQueue(device, queueFamilyIndices.transferFamily.value(), 0, &transferQueue);

	debugUtil.setup(device);
}

void VulkanApplication::createSwapChain()
{
	SwapChainSupportDetails swapChainDetails = querySwapChainSupport(physicalDevice);

	VkSurfaceFormatKHR surfaceFormat = chooseSwapChainSurfaceFormat(swapChainDetails.formats);
	VkPresentModeKHR presentMode = chooseSwapChainPresentMode(swapChainDetails.presentModes);
	VkExtent2D extent = chooseSwapChainExtent(swapChainDetails.capabilities);

	minImageCount = swapChainDetails.capabilities.minImageCount;
	imageCount = swapChainDetails.capabilities.minImageCount + 1;

	if (swapChainDetails.capabilities.maxImageCount > 0 && imageCount > swapChainDetails.capabilities.maxImageCount)
	{
		imageCount = swapChainDetails.capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR swapChainCreateInfo{};
	swapChainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapChainCreateInfo.surface = surface;

	swapChainCreateInfo.minImageCount = imageCount;
	swapChainCreateInfo.imageFormat = surfaceFormat.format;
	swapChainCreateInfo.imageColorSpace = surfaceFormat.colorSpace;
	swapChainCreateInfo.imageExtent = extent;
	swapChainCreateInfo.imageArrayLayers = 1;
	swapChainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);

	uint32_t pQueueFamilyIndices[] = { queueFamilyIndices.graphicsAndComputeFamily.value(), queueFamilyIndices.presentFamily.value() };

	if (queueFamilyIndices.graphicsAndComputeFamily != queueFamilyIndices.presentFamily)
	{
		swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		swapChainCreateInfo.queueFamilyIndexCount = 2;
		swapChainCreateInfo.pQueueFamilyIndices = pQueueFamilyIndices;
	}
	else
	{
		swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		swapChainCreateInfo.queueFamilyIndexCount = 0;
		swapChainCreateInfo.pQueueFamilyIndices = nullptr;
	}
	
	swapChainCreateInfo.preTransform = swapChainDetails.capabilities.currentTransform;
	swapChainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapChainCreateInfo.presentMode = presentMode;
	swapChainCreateInfo.clipped = VK_TRUE;
	swapChainCreateInfo.oldSwapchain = VK_NULL_HANDLE;

	VkCheck(vkCreateSwapchainKHR(device, &swapChainCreateInfo, nullptr, &swapChain), "Failed to create swap chain!");

	swapChainImageFormat = surfaceFormat.format;
	swapChainExtent = extent;

	vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
	swapChainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());
}

void VulkanApplication::recreateSwapChain()
{
	int32_t width = 0;
	int32_t height = 0;

	glfwGetFramebufferSize(window, &width, &height);

	while (width == 0 || height == 0)
	{
		glfwGetFramebufferSize(window, &width, &height);
		glfwWaitEvents();
	}

	vkDeviceWaitIdle(device);

	cleanupSwapChain();

	createSwapChain();
	createImageViews();
	createColorResources(colorImage, msaaSamples);
	createDepthResources(depthImage, msaaSamples);
	createFramebuffers();
}

void VulkanApplication::cleanupSwapChain()
{
	destroyImage(colorImage);
	vkDestroyImageView(device, colorImage.view, allocator);

	destroyImage(depthImage);
	vkDestroyImageView(device, depthImage.view, allocator);

	for (auto& framebuffer : swapChainFramebuffers)
	{
		vkDestroyFramebuffer(device, framebuffer, allocator);
	}

	for (auto& imageView : swapChainImageViews)
	{
		vkDestroyImageView(device, imageView, allocator);
	}

	vkDestroySwapchainKHR(device, swapChain, allocator);
}

void VulkanApplication::createImageViews()
{
	swapChainImageViews.resize(swapChainImages.size());

	for (size_t i = 0; i < swapChainImages.size(); i++)
	{
		swapChainImageViews[i] = createImageView(swapChainImages[i], swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
	}
}

void VulkanApplication::createRenderPass()
{
	VkAttachmentDescription colorAttachment{};
	colorAttachment.format = swapChainImageFormat;
	colorAttachment.samples = msaaSamples;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	//colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;	// MSAA

	// A single render pass can consist of multiple subpasses. Subpasses are subsequent rendering operations 
	// that depend on the contents of framebuffers in previous passes, for example a sequence of post-processing 
	// effects that are applied one after another. If you group these rendering operations into one render pass, 
	// then Vulkan is able to reorder the operations and conserve memory bandwidth for possibly better performance.

	// Every subpass references one or more of the attachments that we've described using the structure in the previous sections. 
	// These references are themselves VkAttachmentReference structs that look like this:
	VkAttachmentReference colorAttachmentReference{};
	colorAttachmentReference.attachment = 0;
	colorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	// MSAA
	//VkAttachmentDescription colorAttachmentResolve{};
	//colorAttachmentResolve.format = swapChainImageFormat;
	//colorAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;
	//colorAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	//colorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	//colorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	//colorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	//colorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	//colorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	//VkAttachmentReference colorAttachmentResolveRef{};
	//colorAttachmentResolveRef.attachment = 2;
	//colorAttachmentResolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentDescription depthAttachment{};
	depthAttachment.format = findDepthFormat();
	depthAttachment.samples = msaaSamples;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthAttachementReference{};
	depthAttachementReference.attachment = 1;
	depthAttachementReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	// The attachment parameter specifies which attachment to reference by its index in the attachment descriptions array.
	// Our array consists of a single VkAttachmentDescription, so its index is 0. The layout specifies which layout 
	// we would like the attachment to have during a subpass that uses this reference. Vulkan will automatically transition 
	// the attachment to this layout when the subpass is started. We intend to use the attachment to function as a color buffer and the 
	// VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL layout will give us the best performance, as its name implies.
	VkSubpassDescription subpassDescription{};
	subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	// The index of the attachment in this array is directly referenced from the fragment shader with the layout(location = 0) out vec4 outColor directive!
	subpassDescription.colorAttachmentCount = 1;	
	subpassDescription.pColorAttachments = &colorAttachmentReference;
	subpassDescription.pDepthStencilAttachment = &depthAttachementReference;
	//subpassDescription.pResolveAttachments = &colorAttachmentResolveRef;	// MSAA

	//std::array<VkAttachmentDescription, 3> attachments = { colorAttachment, depthAttachment, colorAttachmentResolve }; // MSAA
	std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };

	// The following other types of attachments can be referenced by a subpass:
	// pInputAttachments: Attachments that are read from a shader
	// pResolveAttachments : Attachments used for multisampling color attachments
	// pDepthStencilAttachment : Attachment for depthand stencil data
	// pPreserveAttachments : Attachments that are not used by this subpass, but for which the data must be preserved

	VkRenderPassCreateInfo renderPassCreateInfo{};
	renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassCreateInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	renderPassCreateInfo.pAttachments = attachments.data();
	renderPassCreateInfo.subpassCount = 1;
	renderPassCreateInfo.pSubpasses = &subpassDescription;

	// The first two fields specify the indices of the dependency and the dependent subpass. The special value VK_SUBPASS_EXTERNAL
	// refers to the implicit subpass before or after the render pass depending on whether it is specified in srcSubpass or dstSubpass.
	// The index 0 refers to our subpass, which is the first and only one. The dstSubpass must always be higher than srcSubpass to prevent 
	// cycles in the dependency graph (unless one of the subpasses is VK_SUBPASS_EXTERNAL).
	VkSubpassDependency dependency{};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;

	// The next two fields specify the operations to wait on and the stages in which these operations occur. 
	// We need to wait for the swap chain to finish reading from the image before we can access it. This can 
	// be accomplished by waiting on the color attachment output stage itself.
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dependency.srcAccessMask = 0;

	// The operations that should wait on this are in the color attachment stage and involve the writing of the color attachment. 
	// These settings will prevent the transition from happening until it's actually necessary (and allowed): when we want to start writing colors to it.
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

	renderPassCreateInfo.dependencyCount = 1;
	renderPassCreateInfo.pDependencies = &dependency;

	VkCheck(vkCreateRenderPass(device, &renderPassCreateInfo, nullptr, &renderPass), "Failed to create render pass!");

	debugUtil.setObjectName(renderPass, "renderPass");
}

void VulkanApplication::createGraphicsDescriptorSetLayout()
{
	VkDescriptorSetLayoutBinding globalUniformBufferLayoutBinding{};
	globalUniformBufferLayoutBinding.binding = 0;
	globalUniformBufferLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	globalUniformBufferLayoutBinding.descriptorCount = 1;
	globalUniformBufferLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	globalUniformBufferLayoutBinding.pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutBinding objectUniformBufferLayoutBinding{};
	objectUniformBufferLayoutBinding.binding = 1;
	objectUniformBufferLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
	objectUniformBufferLayoutBinding.descriptorCount = 1;
	objectUniformBufferLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	objectUniformBufferLayoutBinding.pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutBinding materialUniformBufferLayoutBinding{};
	materialUniformBufferLayoutBinding.binding = 2;
	materialUniformBufferLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
	materialUniformBufferLayoutBinding.descriptorCount = 1;
	materialUniformBufferLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	materialUniformBufferLayoutBinding.pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutBinding lightUniformBufferLayoutBinding{};
	lightUniformBufferLayoutBinding.binding = 3;
	lightUniformBufferLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	lightUniformBufferLayoutBinding.descriptorCount = 1;
	lightUniformBufferLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	lightUniformBufferLayoutBinding.pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutBinding renderTextureSamplerLayoutBingding{};
	renderTextureSamplerLayoutBingding.binding = 4;
	renderTextureSamplerLayoutBingding.descriptorCount = 1;
	renderTextureSamplerLayoutBingding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	renderTextureSamplerLayoutBingding.pImmutableSamplers = nullptr;
	renderTextureSamplerLayoutBingding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	VkDescriptorSetLayoutBinding samplerLayoutBingding{};
	samplerLayoutBingding.binding = 5;
	samplerLayoutBingding.descriptorCount = static_cast<uint32_t>(textureImagePaths.size());
	samplerLayoutBingding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerLayoutBingding.pImmutableSamplers = nullptr;
	samplerLayoutBingding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;


	std::array<VkDescriptorSetLayoutBinding, 6> descriptorSetLayoutBindings{ globalUniformBufferLayoutBinding,
																		     objectUniformBufferLayoutBinding, 
																			 materialUniformBufferLayoutBinding, 
																			 lightUniformBufferLayoutBinding,
																			 renderTextureSamplerLayoutBingding,
																			 samplerLayoutBingding
																			  };

	VkDescriptorSetLayoutCreateInfo descriptorSetlayoutCreateInfo{};
	descriptorSetlayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptorSetlayoutCreateInfo.bindingCount = static_cast<uint32_t>(descriptorSetLayoutBindings.size());
	descriptorSetlayoutCreateInfo.pBindings = descriptorSetLayoutBindings.data();

	VkDescriptorBindingFlagsEXT bindFlags[6]{ VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT_EXT };

	//bindFlags[5] = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT_EXT;
	// 如果这里指定了VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT_EXT
	// 那么在实际调用vkAllocateDescriptorSets的时候需要通过vkDescriptorSetVariableDescriptorCountAllocateInfo指定可变的描述符个数(variable descriptor count)
	bindFlags[5] = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT_EXT | VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT_EXT;

	VkDescriptorSetLayoutBindingFlagsCreateInfoEXT extendedInfo{};
	extendedInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO_EXT;
	extendedInfo.pNext = nullptr;
	extendedInfo.bindingCount = _countof(bindFlags);
	extendedInfo.pBindingFlags = bindFlags;

	descriptorSetlayoutCreateInfo.pNext = &extendedInfo;

	VkCheck(vkCreateDescriptorSetLayout(device, &descriptorSetlayoutCreateInfo, 
										allocator, &graphicsDescriptorSetLayout), "Failed to create descriptor set layout");
}

void VulkanApplication::createOffscreenDescriptorSetLayout()
{
	VkDescriptorSetLayoutBinding color0{};
	color0.binding = 0;
	color0.descriptorCount = 1;
	color0.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	color0.pImmutableSamplers = nullptr;
	color0.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	VkDescriptorSetLayoutBinding color1{};
	color1.binding = 1;
	color1.descriptorCount = 1;
	color1.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	color1.pImmutableSamplers = nullptr;
	color1.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	std::array<VkDescriptorSetLayoutBinding, 2> descriptorSetLayoutBindings{ color0, color1 };

	VkDescriptorSetLayoutCreateInfo descriptorSetlayoutCreateInfo{};
	descriptorSetlayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptorSetlayoutCreateInfo.bindingCount = static_cast<uint32_t>(descriptorSetLayoutBindings.size());
	descriptorSetlayoutCreateInfo.pBindings = descriptorSetLayoutBindings.data();

	VkDescriptorBindingFlagsEXT bindFlags[2]{ VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT_EXT };

	//bindFlags[5] = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT_EXT;
	// 如果这里指定了VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT_EXT
	// 那么在实际调用vkAllocateDescriptorSets的时候需要通过vkDescriptorSetVariableDescriptorCountAllocateInfo指定可变的描述符个数(variable descriptor count)
	bindFlags[1] = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT_EXT | VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT_EXT;

	VkDescriptorSetLayoutBindingFlagsCreateInfoEXT extendedInfo{};
	extendedInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO_EXT;
	extendedInfo.pNext = nullptr;
	extendedInfo.bindingCount = _countof(bindFlags);
	extendedInfo.pBindingFlags = bindFlags;

	descriptorSetlayoutCreateInfo.pNext = &extendedInfo;

	VkCheck(vkCreateDescriptorSetLayout(device, &descriptorSetlayoutCreateInfo, allocator, &descriptorSetLayouts.offscreen), "Failed to create descriptor set layout");

	debugUtil.setObjectName(descriptorSetLayouts.offscreen, "descriptorSetLayouts.offscreen");

	VkCheck(vkCreateDescriptorSetLayout(device, &descriptorSetlayoutCreateInfo, allocator, &descriptorSetLayouts.bloom), "Failed to create descriptor set layout");

	debugUtil.setObjectName(descriptorSetLayouts.bloom, "descriptorSetLayouts.bloom");
}

void VulkanApplication::createComputeDescriptorSetLayout()
{
	std::array<VkDescriptorSetLayoutBinding, 3> layoutBindings{};

	layoutBindings[0].binding = 0;
	layoutBindings[0].descriptorCount = 1;
	layoutBindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	layoutBindings[0].pImmutableSamplers = nullptr;
	layoutBindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_COMPUTE_BIT;

	layoutBindings[1].binding = 1;
	layoutBindings[1].descriptorCount = 1;
	layoutBindings[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	layoutBindings[1].pImmutableSamplers = nullptr;
	layoutBindings[1].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

	layoutBindings[2].binding = 2;
	layoutBindings[2].descriptorCount = 1;
	layoutBindings[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	layoutBindings[2].pImmutableSamplers = nullptr;
	layoutBindings[2].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = static_cast<uint32_t>(layoutBindings.size());
	layoutInfo.pBindings = layoutBindings.data();

	VkCheck(vkCreateDescriptorSetLayout(device, &layoutInfo,
		allocator, &computeDescriptorSetLayout), "Failed to create descriptor set layout");
}

void VulkanApplication::createGraphicsPipeline()
{
	VkShaderModule vertexShaderModule = createShaderModule(ResourceBase + "Shaders/shader.vert.spv");
	VkShaderModule fragmentShaderModule = createShaderModule(ResourceBase + "Shaders/shader.frag.spv");

	VkPipelineShaderStageCreateInfo vertexShaderStageCreateInfo{};
	vertexShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertexShaderStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertexShaderStageCreateInfo.module = vertexShaderModule;
	vertexShaderStageCreateInfo.pName = "main";

	// There is one more(optional) member, pSpecializationInfo, which we won't be using here, 
	// but is worth discussing. It allows you to specify values for shader constants.
	// You can use a single shader module where its behavior can be configured at pipeline 
	// creation by specifying different values for the constants used in it. This is more efficient 
	// than configuring the shader using variables at render time, because the compiler can do optimizations 
	// like eliminating if statements that depend on these values. 
	vertexShaderStageCreateInfo.pSpecializationInfo = nullptr;

	VkPipelineShaderStageCreateInfo fragmentShaderStageCreateInfo{};
	fragmentShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragmentShaderStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragmentShaderStageCreateInfo.module = fragmentShaderModule;
	fragmentShaderStageCreateInfo.pName = "main";

	VkPipelineShaderStageCreateInfo shaderStages[] = { vertexShaderStageCreateInfo, fragmentShaderStageCreateInfo };

	std::vector<VkDynamicState> dynamicStateEnables =
	{
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};

	VkPipelineDynamicStateCreateInfo dynamicStatesCreateInfo = vks::initializers::pipelineDynamicStateCreateInfo(dynamicStateEnables);

	auto bindingDescription = Vertex::getBindingDescription();
	auto attributeDescriptions = Vertex::getAttributeDescriptions();

	VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo{};
	vertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputStateCreateInfo.vertexBindingDescriptionCount = 1;
	vertexInputStateCreateInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
	vertexInputStateCreateInfo.pVertexBindingDescriptions = &bindingDescription;
	vertexInputStateCreateInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

	VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo = vks::initializers::pipelineInputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, 0, VK_FALSE);

	VkPipelineViewportStateCreateInfo viewportStateCreateInfo = vks::initializers::pipelineViewportStateCreateInfo(1, 1, 0);

	VkPipelineRasterizationStateCreateInfo rasterizationStateCreateInfo = vks::initializers::pipelineRasterizationStateCreateInfo(VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE, 0);

	VkPipelineMultisampleStateCreateInfo multisampleStateCreateInfo = vks::initializers::pipelineMultisampleStateCreateInfo(msaaSamples, 0);

	bool blendEnable = false;

	VkPipelineColorBlendAttachmentState colorBlendAttachmentState = vks::initializers::pipelineColorBlendAttachmentState(
		VK_COLOR_COMPONENT_R_BIT |
					 VK_COLOR_COMPONENT_G_BIT |
					 VK_COLOR_COMPONENT_B_BIT |
					 VK_COLOR_COMPONENT_A_BIT, blendEnable);

	VkPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo = vks::initializers::pipelineColorBlendStateCreateInfo(1, &colorBlendAttachmentState);

	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
	pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutCreateInfo.setLayoutCount = 1;
	pipelineLayoutCreateInfo.pSetLayouts = &graphicsDescriptorSetLayout;
	pipelineLayoutCreateInfo.pushConstantRangeCount = 0;
	pipelineLayoutCreateInfo.pPushConstantRanges = nullptr;

	VkCheck(vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, nullptr, &graphicsPipelineLayout), "Failed to create pipeline layout!");

	debugUtil.setObjectName(graphicsPipelineLayout, "graphicsPipelineLayout");

	VkPipelineLayoutCreateInfo offscreenPipelineLayoutCreateInfo = pipelineLayoutCreateInfo;
	offscreenPipelineLayoutCreateInfo.pSetLayouts = &descriptorSetLayouts.offscreen;

	VkCheck(vkCreatePipelineLayout(device, &offscreenPipelineLayoutCreateInfo, nullptr, &pipelineLayouts.offscreen), "Failed to create pipeline layout!");

	debugUtil.setObjectName(pipelineLayouts.offscreen, "pipelineLayouts.offscreen");

	VkCheck(vkCreatePipelineLayout(device, &offscreenPipelineLayoutCreateInfo, nullptr, &pipelineLayouts.bloom), "Failed to create pipeline layout!");

	debugUtil.setObjectName(pipelineLayouts.bloom, "pipelineLayouts.bloom");

	VkCheck(vkCreatePipelineLayout(device, &offscreenPipelineLayoutCreateInfo, nullptr, &pipelineLayouts.screenQuad), "Failed to create pipeline layout!");

	debugUtil.setObjectName(pipelineLayouts.screenQuad, "pipelineLayouts.screenQuad");

	VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo{};
	graphicsPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	graphicsPipelineCreateInfo.stageCount = 2;
	graphicsPipelineCreateInfo.pStages = shaderStages;

	graphicsPipelineCreateInfo.pVertexInputState = &vertexInputStateCreateInfo;
	graphicsPipelineCreateInfo.pInputAssemblyState = &inputAssemblyStateCreateInfo;
	graphicsPipelineCreateInfo.pViewportState = &viewportStateCreateInfo;
	graphicsPipelineCreateInfo.pRasterizationState = &rasterizationStateCreateInfo;
	graphicsPipelineCreateInfo.pMultisampleState = &multisampleStateCreateInfo;

	VkPipelineDepthStencilStateCreateInfo depthStencilStateCreateInfo = vks::initializers::pipelineDepthStencilStateCreateInfo(VK_TRUE, VK_TRUE, VK_COMPARE_OP_LESS_OR_EQUAL);

	graphicsPipelineCreateInfo.pDepthStencilState = &depthStencilStateCreateInfo;
	graphicsPipelineCreateInfo.pColorBlendState = &colorBlendStateCreateInfo;
	graphicsPipelineCreateInfo.pDynamicState = &dynamicStatesCreateInfo;
	graphicsPipelineCreateInfo.layout = graphicsPipelineLayout;
	graphicsPipelineCreateInfo.renderPass = renderPass;
	graphicsPipelineCreateInfo.subpass = 0;

	// There are actually two more parameters: basePipelineHandle and basePipelineIndex. 
	// Vulkan allows you to create a new graphics pipeline by deriving from an existing pipeline. 
	// The idea of pipeline derivatives is that it is less expensive to set up pipelines when they 
	// have much functionality in common with an existing pipeline and switching between pipelines 
	// from the same parent can also be done quicker. You can either specify the handle of an existing 
	// pipeline with basePipelineHandle or reference another pipeline that is about to be created by index
	// with basePipelineIndex. Right now there is only a single pipeline, so we'll simply specify a null handle 
	// and an invalid index. These values are only used if the VK_PIPELINE_CREATE_DERIVATIVE_BIT flag is also 
	// specified in the flags field of VkGraphicsPipelineCreateInfo.
	graphicsPipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
	graphicsPipelineCreateInfo.basePipelineIndex = -1;

	// The second parameter, for which we've passed the VK_NULL_HANDLE argument, references an optional VkPipelineCache object. 
	// A pipeline cache can be used to store and reuse data relevant to pipeline creation across multiple calls to vkCreateGraphicsPipelines
	// and even across program executions if the cache is stored to a file. This makes it possible to significantly speed up pipeline creation 
	// at a later time. We'll get into this in the pipeline cache chapter.
	VkCheck(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &graphicsPipelineCreateInfo, nullptr, &graphicsPipeline), "Failed to create graphics pipeline!");

	debugUtil.setObjectName(graphicsPipeline, "graphicsPipeline");

	// Screen quad
	VkShaderModule screenQuadVertexShaderModule = createShaderModule(ResourceBase + "Shaders/screenquad.vert.spv");
	VkShaderModule screenQuadFragmentShaderModule = createShaderModule(ResourceBase + "Shaders/screenquad.frag.spv");

	VkPipelineShaderStageCreateInfo screenQuadVertexShaderStageCreateInfo{};
	screenQuadVertexShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	screenQuadVertexShaderStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	screenQuadVertexShaderStageCreateInfo.module = screenQuadVertexShaderModule;
	screenQuadVertexShaderStageCreateInfo.pName = "main";

	// There is one more(optional) member, pSpecializationInfo, which we won't be using here, 
	// but is worth discussing. It allows you to specify values for shader constants.
	// You can use a single shader module where its behavior can be configured at pipeline 
	// creation by specifying different values for the constants used in it. This is more efficient 
	// than configuring the shader using variables at render time, because the compiler can do optimizations 
	// like eliminating if statements that depend on these values. 
	vertexShaderStageCreateInfo.pSpecializationInfo = nullptr;

	VkPipelineShaderStageCreateInfo screenQuadFragmentShaderStageCreateInfo{};
	screenQuadFragmentShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	screenQuadFragmentShaderStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	screenQuadFragmentShaderStageCreateInfo.module = screenQuadFragmentShaderModule;
	screenQuadFragmentShaderStageCreateInfo.pName = "main";

	VkPipelineShaderStageCreateInfo screenQuadShaderStages[] = { screenQuadVertexShaderStageCreateInfo, screenQuadFragmentShaderStageCreateInfo };

	VkPipelineVertexInputStateCreateInfo emptyInputState{};
	emptyInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

	VkGraphicsPipelineCreateInfo screenQuadGraphicsPipelineCreateInfo = graphicsPipelineCreateInfo;
	screenQuadGraphicsPipelineCreateInfo.pStages = screenQuadShaderStages;
	screenQuadGraphicsPipelineCreateInfo.pVertexInputState = &emptyInputState;
	screenQuadGraphicsPipelineCreateInfo.layout = pipelineLayouts.offscreen;

	VkPipelineRasterizationStateCreateInfo screenQuadRasterizationStateCreateInfo = rasterizationStateCreateInfo;

	screenQuadRasterizationStateCreateInfo.cullMode = VK_CULL_MODE_FRONT_BIT;
	screenQuadRasterizationStateCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

	screenQuadGraphicsPipelineCreateInfo.pRasterizationState = &screenQuadRasterizationStateCreateInfo;

	VkSpecializationInfo tonemappingSpecializationInfo;
	std::array<VkSpecializationMapEntry, 1> tonemappingSpecializationMapEntries;

	// Set constant parameters via specialization constants
	tonemappingSpecializationMapEntries[0] = vks::initializers::specializationMapEntry(0, 0, sizeof(uint32_t));
	uint32_t tonemapping = 1;
	tonemappingSpecializationInfo = vks::initializers::specializationInfo(1, tonemappingSpecializationMapEntries.data(), sizeof(tonemapping), &tonemapping);
	screenQuadShaderStages[1].pSpecializationInfo = &tonemappingSpecializationInfo;

	VkCheck(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &screenQuadGraphicsPipelineCreateInfo, nullptr, &pipelines.screenQuad), "vkCreateGraphicsPipelines failed.");

	debugUtil.setObjectName(pipelines.screenQuad, "pipelines.screenQuad");

	tonemapping = 0;

	VkCheck(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &screenQuadGraphicsPipelineCreateInfo, nullptr, &pipelines.screenQuadNoToneMapping), "vkCreateGraphicsPipelines failed.");

	debugUtil.setObjectName(pipelines.screenQuadNoToneMapping, "pipelines.screenQuadNoToneMapping");

	vkDestroyShaderModule(device, screenQuadVertexShaderModule, allocator);
	vkDestroyShaderModule(device, screenQuadFragmentShaderModule, allocator);

	// Bloom filter
	VkShaderModule bloomVertexShaderModule = createShaderModule(ResourceBase + "Shaders/bloom.vert.spv");
	VkShaderModule bloomFragmentShaderModule = createShaderModule(ResourceBase + "Shaders/bloom.frag.spv");

	VkPipelineShaderStageCreateInfo bloomVertexShaderStageCreateInfo{};
	bloomVertexShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	bloomVertexShaderStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	bloomVertexShaderStageCreateInfo.module = bloomVertexShaderModule;
	bloomVertexShaderStageCreateInfo.pName = "main";

	// There is one more(optional) member, pSpecializationInfo, which we won't be using here, 
	// but is worth discussing. It allows you to specify values for shader constants.
	// You can use a single shader module where its behavior can be configured at pipeline 
	// creation by specifying different values for the constants used in it. This is more efficient 
	// than configuring the shader using variables at render time, because the compiler can do optimizations 
	// like eliminating if statements that depend on these values. 
	vertexShaderStageCreateInfo.pSpecializationInfo = nullptr;

	VkPipelineShaderStageCreateInfo bloomFragmentShaderStageCreateInfo{};
	bloomFragmentShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	bloomFragmentShaderStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	bloomFragmentShaderStageCreateInfo.module = bloomFragmentShaderModule;
	bloomFragmentShaderStageCreateInfo.pName = "main";

	VkPipelineShaderStageCreateInfo bloomShaderStages[] = { bloomVertexShaderStageCreateInfo, bloomFragmentShaderStageCreateInfo };

	VkPipelineColorBlendStateCreateInfo bloomColorBlendStateCreateInfo = colorBlendStateCreateInfo;

	VkPipelineColorBlendAttachmentState bloomColorBlendAttachmentState = colorBlendAttachmentState;

	bloomColorBlendStateCreateInfo.pAttachments = &bloomColorBlendAttachmentState;
	bloomColorBlendAttachmentState.blendEnable = VK_TRUE;
	bloomColorBlendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
	bloomColorBlendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
	bloomColorBlendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
	bloomColorBlendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;
	bloomColorBlendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	bloomColorBlendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_DST_ALPHA;

	VkGraphicsPipelineCreateInfo bloomGraphicsPipelineCreateInfo = graphicsPipelineCreateInfo;
	bloomGraphicsPipelineCreateInfo.pStages = bloomShaderStages;
	bloomGraphicsPipelineCreateInfo.pVertexInputState = &emptyInputState;
	bloomGraphicsPipelineCreateInfo.layout = pipelineLayouts.screenQuad;
	bloomGraphicsPipelineCreateInfo.pColorBlendState = &bloomColorBlendStateCreateInfo;

	VkPipelineRasterizationStateCreateInfo bloomRasterizationStateCreateInfo = rasterizationStateCreateInfo;

	bloomRasterizationStateCreateInfo.cullMode = VK_CULL_MODE_FRONT_BIT;
	bloomRasterizationStateCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

	bloomGraphicsPipelineCreateInfo.pRasterizationState = &bloomRasterizationStateCreateInfo;

	VkSpecializationInfo specializationInfo;
	std::array<VkSpecializationMapEntry, 1> specializationMapEntries;

	// Set constant parameters via specialization constants
	specializationMapEntries[0] = vks::initializers::specializationMapEntry(0, 0, sizeof(uint32_t));
	uint32_t dir = 1;
	specializationInfo = vks::initializers::specializationInfo(1, specializationMapEntries.data(), sizeof(dir), &dir);
	bloomShaderStages[1].pSpecializationInfo = &specializationInfo;

	VkCheck(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &bloomGraphicsPipelineCreateInfo, nullptr, &pipelines.bloom[0]), "vkCreateGraphicsPipelines failed.");

	debugUtil.setObjectName(pipelines.bloom[0], "pipelines.bloom[0]");

	// Second blur pass (into separate framebuffer)
	bloomGraphicsPipelineCreateInfo.renderPass = bloomFilterPass.renderPass;
	dir = 0;
	VkCheck(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &bloomGraphicsPipelineCreateInfo, nullptr, &pipelines.bloom[1]), "vkCreateGraphicsPipelines failed.");

	debugUtil.setObjectName(pipelines.bloom[1], "pipelines.bloom[1]");

	vkDestroyShaderModule(device, bloomVertexShaderModule, allocator);
	vkDestroyShaderModule(device, bloomFragmentShaderModule, allocator);

	// Offscreen
	// Flip cull mode
	VkShaderModule offscreenVertexShaderModule = createShaderModule(ResourceBase + "Shaders/offscreen.vert.spv");
	VkShaderModule offscreenFragmentShaderModule = createShaderModule(ResourceBase + "Shaders/offscreen.frag.spv");

	VkPipelineShaderStageCreateInfo offscreenVertexShaderStageCreateInfo{};
	offscreenVertexShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	offscreenVertexShaderStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	offscreenVertexShaderStageCreateInfo.module = offscreenVertexShaderModule;
	offscreenVertexShaderStageCreateInfo.pName = "main";

	// There is one more(optional) member, pSpecializationInfo, which we won't be using here, 
	// but is worth discussing. It allows you to specify values for shader constants.
	// You can use a single shader module where its behavior can be configured at pipeline 
	// creation by specifying different values for the constants used in it. This is more efficient 
	// than configuring the shader using variables at render time, because the compiler can do optimizations 
	// like eliminating if statements that depend on these values. 
	vertexShaderStageCreateInfo.pSpecializationInfo = nullptr;

	VkPipelineShaderStageCreateInfo offscreenFragmentShaderStageCreateInfo{};
	offscreenFragmentShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	offscreenFragmentShaderStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	offscreenFragmentShaderStageCreateInfo.module = offscreenFragmentShaderModule;
	offscreenFragmentShaderStageCreateInfo.pName = "main";

	VkPipelineShaderStageCreateInfo offscreenShaderStages[] = { offscreenVertexShaderStageCreateInfo, offscreenFragmentShaderStageCreateInfo };

	VkPipelineRasterizationStateCreateInfo offscreenRasterizationStateCreateInfo = rasterizationStateCreateInfo;

	VkGraphicsPipelineCreateInfo offscreenGraphicsPipelineCreateInfo = graphicsPipelineCreateInfo;
	offscreenRasterizationStateCreateInfo.cullMode = VK_CULL_MODE_FRONT_BIT;
	offscreenGraphicsPipelineCreateInfo.renderPass = offscreenPass.renderPass;
	offscreenGraphicsPipelineCreateInfo.pStages = offscreenShaderStages;
	//offscreenGraphicsPipelineCreateInfo.pRasterizationState = &offscreenRasterizationStateCreateInfo;

	// 0xf == VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
	std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachmentStates = 
	{ 
		vks::initializers::pipelineColorBlendAttachmentState(0xf, blendEnable),
		vks::initializers::pipelineColorBlendAttachmentState(0xf, blendEnable)
	};

	VkPipelineColorBlendStateCreateInfo offscreenColorBlendStateCreateInfo = vks::initializers::pipelineColorBlendStateCreateInfo(2, colorBlendAttachmentStates.data());

	offscreenGraphicsPipelineCreateInfo.pColorBlendState = &offscreenColorBlendStateCreateInfo;

	VkPipelineMultisampleStateCreateInfo offscreenMultisampleStateCreateInfo = multisampleStateCreateInfo;
	offscreenMultisampleStateCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	offscreenGraphicsPipelineCreateInfo.pMultisampleState = &offscreenMultisampleStateCreateInfo;

	VkCheck(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &offscreenGraphicsPipelineCreateInfo, nullptr, &pipelines.offscreen), "vkCreateGraphicsPipelines failed.");

	debugUtil.setObjectName(pipelines.offscreen, "offscreenPass.pipeline");

	vkDestroyShaderModule(device, fragmentShaderModule, allocator);
	vkDestroyShaderModule(device, vertexShaderModule, allocator);

	vkDestroyShaderModule(device, offscreenVertexShaderModule, allocator);
	vkDestroyShaderModule(device, offscreenFragmentShaderModule, allocator);

	VkShaderModule particleVertexShaderModule = createShaderModule(ResourceBase + "Shaders/particle.vert.spv");
	VkShaderModule particleFragmentShaderModule = createShaderModule(ResourceBase + "Shaders/particle.frag.spv");

	VkPipelineShaderStageCreateInfo particleVertexShaderStageCreateInfo{};
	particleVertexShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	particleVertexShaderStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	particleVertexShaderStageCreateInfo.module = particleVertexShaderModule;
	particleVertexShaderStageCreateInfo.pName = "main";

	// There is one more(optional) member, pSpecializationInfo, which we won't be using here, 
	// but is worth discussing. It allows you to specify values for shader constants.
	// You can use a single shader module where its behavior can be configured at pipeline 
	// creation by specifying different values for the constants used in it. This is more efficient 
	// than configuring the shader using variables at render time, because the compiler can do optimizations 
	// like eliminating if statements that depend on these values. 
	particleVertexShaderStageCreateInfo.pSpecializationInfo = nullptr;

	VkPipelineShaderStageCreateInfo particleFragmentShaderStageCreateInfo{};
	particleFragmentShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	particleFragmentShaderStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	particleFragmentShaderStageCreateInfo.module = particleFragmentShaderModule;
	particleFragmentShaderStageCreateInfo.pName = "main";

	VkPipelineShaderStageCreateInfo particleShaderStages[] = { particleVertexShaderStageCreateInfo, particleFragmentShaderStageCreateInfo };

	VkPipelineLayoutCreateInfo particlePipelineLayoutCreateInfo{};
	particlePipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	particlePipelineLayoutCreateInfo.setLayoutCount = 1;
	particlePipelineLayoutCreateInfo.pSetLayouts = &computeDescriptorSetLayout;
	particlePipelineLayoutCreateInfo.pushConstantRangeCount = 0;
	particlePipelineLayoutCreateInfo.pPushConstantRanges = nullptr;

	VkCheck(vkCreatePipelineLayout(device, &particlePipelineLayoutCreateInfo, nullptr, &particlePipelineLayout), "Failed to create pipeline layout!");

	debugUtil.setObjectName(particlePipelineLayout, "particlePipelineLayout");

	auto particlePipelineCreateInfo = graphicsPipelineCreateInfo;

	particlePipelineCreateInfo.pStages = particleShaderStages;
	particlePipelineCreateInfo.layout = particlePipelineLayout;

	// 用于粒子的particlePipelineLayout
	auto particleBindingDescription = Particle::getBindingDescription();
	auto particleAttributeDescriptions = Particle::getAttributeDescriptions();

	VkPipelineVertexInputStateCreateInfo particleVertexInputStateCreateInfo{};
	particleVertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	particleVertexInputStateCreateInfo.vertexBindingDescriptionCount = 1;
	particleVertexInputStateCreateInfo.pVertexBindingDescriptions = &particleBindingDescription;
	particleVertexInputStateCreateInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(particleAttributeDescriptions.size());
	particleVertexInputStateCreateInfo.pVertexAttributeDescriptions = particleAttributeDescriptions.data();

	VkPipelineInputAssemblyStateCreateInfo particleInputAssemblyStateCreateInfo{};
	particleInputAssemblyStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	particleInputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
	particleInputAssemblyStateCreateInfo.primitiveRestartEnable = VK_FALSE;

	particlePipelineCreateInfo.pVertexInputState = &particleVertexInputStateCreateInfo;
	particlePipelineCreateInfo.pInputAssemblyState = &particleInputAssemblyStateCreateInfo;

	VkCheck(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &particlePipelineCreateInfo, nullptr, &particlePipeline), "Failed to create graphics pipeline!");

	vkDestroyShaderModule(device, particleFragmentShaderModule, nullptr);
	vkDestroyShaderModule(device, particleVertexShaderModule, nullptr);
}

void VulkanApplication::createComputePipeline()
{
	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = &computeDescriptorSetLayout;

	VkCheck(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &computePipelineLayout), "Failed to create compute pipeline layout!");

	debugUtil.setObjectName(computePipelineLayout, "computePipelineLayout");

	auto computeShaderCode = readFile(ResourceBase + "Shaders/particle.comp.spv");

	VkShaderModule computeShaderModule = createShaderModule(computeShaderCode);

	VkPipelineShaderStageCreateInfo computeShaderStageInfo{};
	computeShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	computeShaderStageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
	computeShaderStageInfo.module = computeShaderModule;
	computeShaderStageInfo.pName = "main";

	VkComputePipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
	pipelineInfo.layout = computePipelineLayout;
	pipelineInfo.stage = computeShaderStageInfo;

	VkCheck(vkCreateComputePipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &computePipeline), "Failed to create compute pipeline layout!");

	vkDestroyShaderModule(device, computeShaderModule, nullptr);
}

void VulkanApplication::createGraphicsCommandPool()
{
	QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);

	VkCommandPoolCreateInfo commandPoolCreateInfo{};
	commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;

	// There are two possible flags for command pools :
	// VK_COMMAND_POOL_CREATE_TRANSIENT_BIT: Hint that command buffers are rerecorded with new commands very often(may change memory allocation behavior)
	// VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT : Allow command buffers to be rerecorded individually, without this flag they all have to be reset together
	// We will be recording a command buffer every frame, so we want to be able to reset and rerecord over it.Thus, we need to set the
	// VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT flag bit for our command pool.
	commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	commandPoolCreateInfo.queueFamilyIndex = queueFamilyIndices.graphicsAndComputeFamily.value();

	VkCheck(vkCreateCommandPool(device, &commandPoolCreateInfo, nullptr, &graphicsCommandPool), "Failed to create command pool!");
}

void VulkanApplication::createTransferCommandPool()
{
	QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);

	VkCommandPoolCreateInfo commandPoolCreateInfo{};
	commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;

	// There are two possible flags for command pools :
	// VK_COMMAND_POOL_CREATE_TRANSIENT_BIT: Hint that command buffers are rerecorded with new commands very often(may change memory allocation behavior)
	// VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT : Allow command buffers to be rerecorded individually, without this flag they all have to be reset together
	// We will be recording a command buffer every frame, so we want to be able to reset and rerecord over it.Thus, we need to set the
	// VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT flag bit for our command pool.
	commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
	commandPoolCreateInfo.queueFamilyIndex = queueFamilyIndices.graphicsAndComputeFamily.value();

	VkCheck(vkCreateCommandPool(device, &commandPoolCreateInfo, nullptr, &transferCommandPool), "Failed to create command pool!");
}

void VulkanApplication::createColorResources(Image& colorImage, VkSampleCountFlagBits numSamples)
{
	colorImage.width = swapChainExtent.width;
	colorImage.height = swapChainExtent.height;
	colorImage.mipLevels = 1;
	colorImage.numSamples = numSamples;
	colorImage.format = swapChainImageFormat;
	colorImage.tiling = VK_IMAGE_TILING_OPTIMAL;
	colorImage.usage = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	colorImage.memoryPropertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

	if (useVma)
	{
		createImageVma(colorImage, "colorImage");
	}
	else
	{
		createImage(colorImage);
	}

	colorImage.view = createImageView(colorImage.handle, colorImage.format, VK_IMAGE_ASPECT_COLOR_BIT, 1);
}

void VulkanApplication::createDepthResources(Image& depthImage, VkSampleCountFlagBits numSamples)
{
	depthImage.width = swapChainExtent.width;
	depthImage.height = swapChainExtent.height;
	depthImage.mipLevels = 1;
	depthImage.numSamples = numSamples;
	depthImage.format = findDepthFormat();
	depthImage.tiling = VK_IMAGE_TILING_OPTIMAL;
	depthImage.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	depthImage.memoryPropertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

	if (useVma)
	{
		createImageVma(depthImage, "depthImage");
	}
	else
	{
		createImage(depthImage);
	}

	depthImage.view = createImageView(depthImage.handle, depthImage.format, VK_IMAGE_ASPECT_DEPTH_BIT, depthImage.mipLevels);

	transitionImageLayout(depthImage.handle, depthImage.format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, depthImage.mipLevels);
}

void VulkanApplication::createRenderTexture()
{
	for (int i = 0; i < 2; i++)
	{
		offscreenPass.color[i].width = swapChainExtent.width;
		offscreenPass.color[i].height = swapChainExtent.height;
		offscreenPass.color[i].mipLevels = 1;
		offscreenPass.color[i].numSamples = VK_SAMPLE_COUNT_1_BIT;
		offscreenPass.color[i].format = hdrFormat;
		offscreenPass.color[i].tiling = VK_IMAGE_TILING_OPTIMAL;
		offscreenPass.color[i].usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		offscreenPass.color[i].memoryPropertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

		if (useVma)
		{
			createImageVma(offscreenPass.color[i], "offscreenPass.color");
		}
		else
		{
			createImage(offscreenPass.color[i]);
		}

		offscreenPass.color[i].view = createImageView(offscreenPass.color[i].handle, offscreenPass.color[i].format, VK_IMAGE_ASPECT_COLOR_BIT, 1);
	}

	bloomFilterPass.color[0].width = swapChainExtent.width;
	bloomFilterPass.color[0].height = swapChainExtent.height;
	bloomFilterPass.color[0].mipLevels = 1;
	bloomFilterPass.color[0].numSamples = VK_SAMPLE_COUNT_1_BIT;
	bloomFilterPass.color[0].format = hdrFormat;
	bloomFilterPass.color[0].tiling = VK_IMAGE_TILING_OPTIMAL;
	bloomFilterPass.color[0].usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	bloomFilterPass.color[0].memoryPropertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

	if (useVma)
	{
		createImageVma(bloomFilterPass.color[0], "bloomFilterPass.color");
	}
	else
	{
		createImage(bloomFilterPass.color[0]);
	}

	bloomFilterPass.color[0].view = createImageView(bloomFilterPass.color[0].handle, bloomFilterPass.color[0].format, VK_IMAGE_ASPECT_COLOR_BIT, 1);

	createDepthResources(offscreenPass.depth, VK_SAMPLE_COUNT_1_BIT);
	createDepthResources(bloomFilterPass.depth, VK_SAMPLE_COUNT_1_BIT);
}

void VulkanApplication::prepareOffscreen()
{
	createRenderTexture();

	// Offscreen render pass
	{
		offscreenPass.width = swapChainExtent.width;
		offscreenPass.height = swapChainExtent.height;

		// Create sampler to sample from the attachment in the fragment shader
		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerInfo.addressModeV = samplerInfo.addressModeU;
		samplerInfo.addressModeW = samplerInfo.addressModeU;
		samplerInfo.mipLodBias = 0.0f;
		samplerInfo.maxAnisotropy = 1.0f;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = 1.0f;
		samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;

		VkCheck(vkCreateSampler(device, &samplerInfo, nullptr, &offscreenPass.sampler), "vkCreateSampler failed.");
		VkCheck(vkCreateSampler(device, &samplerInfo, nullptr, &bloomFilterPass.sampler), "vkCreateSampler failed.");

		debugUtil.setObjectName(offscreenPass.sampler, "offscreenPass.sampler");
		debugUtil.setObjectName(bloomFilterPass.sampler, "bloomFilterPass.sampler");

		// Create a separate render pass for the offscreen rendering as it may differ from the one used for scene rendering
		std::array<VkAttachmentDescription, 3> offscreenAttchmentDescriptions = {};
		// Color attachment
		offscreenAttchmentDescriptions[0].format = offscreenPass.color[0].format;
		offscreenAttchmentDescriptions[0].samples = VK_SAMPLE_COUNT_1_BIT;
		offscreenAttchmentDescriptions[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		offscreenAttchmentDescriptions[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		offscreenAttchmentDescriptions[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		offscreenAttchmentDescriptions[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		offscreenAttchmentDescriptions[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		offscreenAttchmentDescriptions[0].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		offscreenAttchmentDescriptions[1].format = offscreenPass.color[1].format;
		offscreenAttchmentDescriptions[1].samples = VK_SAMPLE_COUNT_1_BIT;
		offscreenAttchmentDescriptions[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		offscreenAttchmentDescriptions[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		offscreenAttchmentDescriptions[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		offscreenAttchmentDescriptions[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		offscreenAttchmentDescriptions[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		offscreenAttchmentDescriptions[1].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		// Depth attachment
		offscreenAttchmentDescriptions[2].format = findDepthFormat();
		offscreenAttchmentDescriptions[2].samples = VK_SAMPLE_COUNT_1_BIT;
		offscreenAttchmentDescriptions[2].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		offscreenAttchmentDescriptions[2].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		offscreenAttchmentDescriptions[2].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		offscreenAttchmentDescriptions[2].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		offscreenAttchmentDescriptions[2].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		offscreenAttchmentDescriptions[2].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		std::vector<VkAttachmentReference> offscreenColorReferences;
		offscreenColorReferences.push_back({ 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });
		offscreenColorReferences.push_back({ 1, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });

		VkAttachmentReference offscreenDepthReference = { 2, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };

		VkSubpassDescription offscreenSubpassDescription = {};
		offscreenSubpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		offscreenSubpassDescription.colorAttachmentCount = static_cast<uint32_t>(offscreenColorReferences.size());
		offscreenSubpassDescription.pColorAttachments = offscreenColorReferences.data();
		offscreenSubpassDescription.pDepthStencilAttachment = &offscreenDepthReference;

		// Use subpass dependencies for layout transitions
		std::array<VkSubpassDependency, 2> offscreenDependencies;

		offscreenDependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
		offscreenDependencies[0].dstSubpass = 0;
		offscreenDependencies[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		offscreenDependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		offscreenDependencies[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
		offscreenDependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		offscreenDependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		offscreenDependencies[1].srcSubpass = 0;
		offscreenDependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
		offscreenDependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		offscreenDependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		offscreenDependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		offscreenDependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		offscreenDependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		// Create the actual renderpass
		VkRenderPassCreateInfo offscreenRenderPassInfo = {};
		offscreenRenderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		offscreenRenderPassInfo.attachmentCount = static_cast<uint32_t>(offscreenAttchmentDescriptions.size());
		offscreenRenderPassInfo.pAttachments = offscreenAttchmentDescriptions.data();
		offscreenRenderPassInfo.subpassCount = 1;
		offscreenRenderPassInfo.pSubpasses = &offscreenSubpassDescription;
		offscreenRenderPassInfo.dependencyCount = static_cast<uint32_t>(offscreenDependencies.size());
		offscreenRenderPassInfo.pDependencies = offscreenDependencies.data();

		VkCheck(vkCreateRenderPass(device, &offscreenRenderPassInfo, nullptr, &offscreenPass.renderPass), "vkCreateRenderPass failed.");

		debugUtil.setObjectName(offscreenPass.renderPass, "offscreenPass.renderPass");

		std::array<VkImageView, 3> offscreenAttachments;
		offscreenAttachments[0] = offscreenPass.color[0].view;
		offscreenAttachments[1] = offscreenPass.color[1].view;
		offscreenAttachments[2] = offscreenPass.depth.view;

		VkFramebufferCreateInfo offscreenFrameBufferCreateInfo{};
		offscreenFrameBufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		offscreenFrameBufferCreateInfo.renderPass = offscreenPass.renderPass;
		offscreenFrameBufferCreateInfo.attachmentCount = static_cast<uint32_t>(offscreenAttachments.size());
		offscreenFrameBufferCreateInfo.pAttachments = offscreenAttachments.data();
		offscreenFrameBufferCreateInfo.width = offscreenPass.width;
		offscreenFrameBufferCreateInfo.height = offscreenPass.height;
		offscreenFrameBufferCreateInfo.layers = 1;

		VkCheck(vkCreateFramebuffer(device, &offscreenFrameBufferCreateInfo, nullptr, &offscreenPass.frameBuffer), "vkCreateFramebuffer failed.");

		// Fill a descriptor for later use in a descriptor set
		for (int i = 0; i < 2; i++)
		{
			offscreenPass.descriptor[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			offscreenPass.descriptor[i].imageView = offscreenPass.color[i].view;
			offscreenPass.descriptor[i].sampler = offscreenPass.sampler;
		}
	}

	// Bloom filter render pass
	{
		bloomFilterPass.width = swapChainExtent.width;
		bloomFilterPass.height = swapChainExtent.height;

		// Create a separate render pass for the offscreen rendering as it may differ from the one used for scene rendering
		std::array<VkAttachmentDescription, 2> bloomFilterAttchmentDescriptions = {};

		// Color attachment
		bloomFilterAttchmentDescriptions[0].format = bloomFilterPass.color[0].format;
		bloomFilterAttchmentDescriptions[0].samples = VK_SAMPLE_COUNT_1_BIT;
		bloomFilterAttchmentDescriptions[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		bloomFilterAttchmentDescriptions[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		bloomFilterAttchmentDescriptions[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		bloomFilterAttchmentDescriptions[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		bloomFilterAttchmentDescriptions[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		bloomFilterAttchmentDescriptions[0].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	
		// Depth attachment
		bloomFilterAttchmentDescriptions[1].format = findDepthFormat();
		bloomFilterAttchmentDescriptions[1].samples = VK_SAMPLE_COUNT_1_BIT;
		bloomFilterAttchmentDescriptions[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		bloomFilterAttchmentDescriptions[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		bloomFilterAttchmentDescriptions[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		bloomFilterAttchmentDescriptions[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		bloomFilterAttchmentDescriptions[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		bloomFilterAttchmentDescriptions[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		std::vector<VkAttachmentReference> bloomFilterColorReferences;
		bloomFilterColorReferences.push_back({ 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });

		VkAttachmentReference bloomFilterDepthReference = { 1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };

		VkSubpassDescription bloomFilterSubpassDescription = {};
		bloomFilterSubpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		bloomFilterSubpassDescription.colorAttachmentCount = static_cast<uint32_t>(bloomFilterColorReferences.size());
		bloomFilterSubpassDescription.pColorAttachments = bloomFilterColorReferences.data();
		bloomFilterSubpassDescription.pDepthStencilAttachment = &bloomFilterDepthReference;

		// Use subpass dependencies for layout transitions
		std::array<VkSubpassDependency, 2> bloomFilterDependencies;

		bloomFilterDependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
		bloomFilterDependencies[0].dstSubpass = 0;
		bloomFilterDependencies[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		bloomFilterDependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		bloomFilterDependencies[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
		bloomFilterDependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		bloomFilterDependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
		
		bloomFilterDependencies[1].srcSubpass = 0;
		bloomFilterDependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
		bloomFilterDependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		bloomFilterDependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		bloomFilterDependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		bloomFilterDependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		bloomFilterDependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		// Create the actual renderpass
		VkRenderPassCreateInfo bloomFilterRenderPassInfo = {};
		bloomFilterRenderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		bloomFilterRenderPassInfo.attachmentCount = static_cast<uint32_t>(bloomFilterAttchmentDescriptions.size());
		bloomFilterRenderPassInfo.pAttachments = bloomFilterAttchmentDescriptions.data();
		bloomFilterRenderPassInfo.subpassCount = 1;
		bloomFilterRenderPassInfo.pSubpasses = &bloomFilterSubpassDescription;
		bloomFilterRenderPassInfo.dependencyCount = static_cast<uint32_t>(bloomFilterDependencies.size());
		bloomFilterRenderPassInfo.pDependencies = bloomFilterDependencies.data();

		VkCheck(vkCreateRenderPass(device, &bloomFilterRenderPassInfo, nullptr, &bloomFilterPass.renderPass), "vkCreateRenderPass failed.");

		debugUtil.setObjectName(bloomFilterPass.renderPass, "offscreenPass.renderPass");

		std::array<VkImageView, 2> bloomFilterAttachments;
		bloomFilterAttachments[0] = bloomFilterPass.color[0].view;
		bloomFilterAttachments[1] = bloomFilterPass.depth.view;

		VkFramebufferCreateInfo bloomFilterFrameBufferCreateInfo{};
		bloomFilterFrameBufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		bloomFilterFrameBufferCreateInfo.renderPass = bloomFilterPass.renderPass;
		bloomFilterFrameBufferCreateInfo.attachmentCount = static_cast<uint32_t>(bloomFilterAttachments.size());
		bloomFilterFrameBufferCreateInfo.pAttachments = bloomFilterAttachments.data();
		bloomFilterFrameBufferCreateInfo.width = bloomFilterPass.width;
		bloomFilterFrameBufferCreateInfo.height = bloomFilterPass.height;
		bloomFilterFrameBufferCreateInfo.layers = 1;

		VkCheck(vkCreateFramebuffer(device, &bloomFilterFrameBufferCreateInfo, nullptr, &bloomFilterPass.frameBuffer), "vkCreateFramebuffer failed.");

		// Fill a descriptor for later use in a descriptor set
		for (int i = 0; i < 2; i++)
		{
			bloomFilterPass.descriptor[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			bloomFilterPass.descriptor[i].imageView = bloomFilterPass.color[0].view;
			bloomFilterPass.descriptor[i].sampler = bloomFilterPass.sampler;
		}
	}
}

void VulkanApplication::createFramebuffers()
{
	swapChainFramebuffers.resize(swapChainImageViews.size());

	for (size_t i = 0; i < swapChainImageViews.size(); i++)
	{
		std::array<VkImageView, 2> attachments =
		{
			swapChainImageViews[i],
			depthImage.view
		};

		// MSAA
		//std::array<VkImageView, 3> attachments =
		//{
		//	colorImage.view,
		//	depthImage.view,
		//	swapChainImageViews[i]
		//};

		VkFramebufferCreateInfo framebufferCreateInfo{};
		framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferCreateInfo.renderPass = renderPass;
		framebufferCreateInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		framebufferCreateInfo.pAttachments = attachments.data();
		framebufferCreateInfo.width = swapChainExtent.width;
		framebufferCreateInfo.height = swapChainExtent.height;
		framebufferCreateInfo.layers = 1;

		VkCheck(vkCreateFramebuffer(device, &framebufferCreateInfo, nullptr, &swapChainFramebuffers[i]), "Failed to create framebuffer!");
	}
}

Image VulkanApplication::createTextureImage(const std::string& path, Channel requireChannels)
{
	int32_t width = 0;
	int32_t height = 0;
	int32_t channels = 0;

	stbi_uc* pixels = stbi_load(path.c_str(), &width, &height, &channels, static_cast<int32_t>(requireChannels));

	stbi_set_flip_vertically_on_load(true);

	VkDeviceSize imageSize = static_cast<uint64_t>(width) * static_cast<uint64_t>(height) * static_cast<uint64_t>(requireChannels);

	if (!pixels)
	{
		THROW_ERROR("Failed to load texture image!");
	}

	mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(width, height)))) + 1;

	Image textureImage;
	Buffer stagingBuffer;

	createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
							VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer.handle, stagingBuffer.memory);

	void* data = nullptr;

	vkMapMemory(device, stagingBuffer.memory, 0, imageSize, 0, &data);
	Utils::memcpy(data, static_cast<size_t>(imageSize), pixels, static_cast<size_t>(imageSize));
	vkUnmapMemory(device, stagingBuffer.memory);
	
	STBI_FREE(pixels);

	textureImage.width = static_cast<uint32_t>(width);
	textureImage.height = static_cast<uint32_t>(height);
	textureImage.mipLevels = mipLevels;
	textureImage.numSamples = VK_SAMPLE_COUNT_1_BIT;
	textureImage.format = VK_FORMAT_R8G8B8A8_SRGB;
	textureImage.tiling = VK_IMAGE_TILING_OPTIMAL;
	textureImage.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	textureImage.memoryPropertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

	createImage(textureImage);

	transitionImageLayout(textureImage.handle, textureImage.format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mipLevels);
	copyBufferToImage(stagingBuffer.handle, textureImage.handle, textureImage.width, textureImage.height);
	
	// transitioned to VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL while generating mipmaps
	//transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, mipLevels);

	generateMipmaps(textureImage.handle, textureImage.format, width, height, mipLevels);

	vkDestroyBuffer(device, stagingBuffer.handle, allocator);
	vkFreeMemory(device, stagingBuffer.memory, allocator);

	return textureImage;
}

Image VulkanApplication::createTextureImageVma(const std::string& path, Channel requireChannels /*= Channel::RGBAlpha*/)
{
	int32_t width = 0;
	int32_t height = 0;
	int32_t channels = 0;

	stbi_uc* pixels = stbi_load(path.c_str(), &width, &height, &channels, static_cast<int32_t>(requireChannels));

	stbi_set_flip_vertically_on_load(true);

	VkDeviceSize imageSize = static_cast<uint64_t>(width) * static_cast<uint64_t>(height) * static_cast<uint64_t>(requireChannels);

	if (!pixels)
	{
		THROW_ERROR("Failed to load texture image!");
	}

	mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(width, height)))) + 1;

	Image textureImage;
	Buffer stagingBuffer;

	stagingBuffer.size = imageSize;
	stagingBuffer.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	stagingBuffer.memoryPropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

	createBufferVma(stagingBuffer, "stagingBuffer");

	void* mappedData = nullptr;

	vmaMapMemory(vmaAllocator, stagingBuffer.allocation, &mappedData);
	Utils::memcpy(mappedData, static_cast<size_t>(imageSize), pixels, static_cast<size_t>(imageSize));
	vmaUnmapMemory(vmaAllocator, stagingBuffer.allocation);

	STBI_FREE(pixels);

	textureImage.width = static_cast<uint32_t>(width);
	textureImage.height = static_cast<uint32_t>(height);
	textureImage.mipLevels = mipLevels;
	textureImage.numSamples = VK_SAMPLE_COUNT_1_BIT;
	textureImage.format = VK_FORMAT_R8G8B8A8_SRGB;
	textureImage.tiling = VK_IMAGE_TILING_OPTIMAL;
	textureImage.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	textureImage.memoryPropertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

	createImageVma(textureImage, "textureImage");

	transitionImageLayout(textureImage.handle, textureImage.format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mipLevels);
	copyBufferToImage(stagingBuffer.handle, textureImage.handle, textureImage.width, textureImage.height);

	// transitioned to VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL while generating mipmaps
	//transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, mipLevels);

	generateMipmaps(textureImage.handle, textureImage.format, width, height, mipLevels);

	destroyBuffer(stagingBuffer);

	return textureImage;
}

VkImageView VulkanApplication::createTextureImageView(VkImage image)
{
	return createImageView(image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, mipLevels);
}

void VulkanApplication::createTextureSampler()
{
	VkSamplerCreateInfo samplerCreateInfo{};
	samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerCreateInfo.magFilter = VK_FILTER_LINEAR;
	samplerCreateInfo.minFilter = VK_FILTER_LINEAR;
	samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

	VkPhysicalDeviceProperties physicalDeviceProperties{};
	vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);

	samplerCreateInfo.maxAnisotropy = physicalDeviceProperties.limits.maxSamplerAnisotropy;

	if (anisotropyEnable)
	{
		samplerCreateInfo.anisotropyEnable = VK_TRUE;
	}
	else
	{
		samplerCreateInfo.anisotropyEnable = VK_FALSE;
		samplerCreateInfo.maxAnisotropy = 1;
	}

	samplerCreateInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerCreateInfo.unnormalizedCoordinates = VK_FALSE;
	samplerCreateInfo.compareEnable = VK_FALSE;
	samplerCreateInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerCreateInfo.mipLodBias = 0.0f;
	samplerCreateInfo.minLod = 0.0f;
	samplerCreateInfo.maxLod = static_cast<float>(mipLevels);

	VkCheck(vkCreateSampler(device, &samplerCreateInfo, allocator, &textureSampler), "Failed to create texture sampler!");
}

Buffer VulkanApplication::createVertexBuffer(const std::vector<Vertex>& vertices)
{
	VkDeviceSize bufferSize = VectorSize(Vertex, vertices);

	Buffer vertexBuffer;
	Buffer stagingBuffer;

	createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		stagingBuffer.handle, stagingBuffer.memory);

	debugUtil.setObjectName(stagingBuffer.handle, "stagingBuffer.handle");
	debugUtil.setObjectName(stagingBuffer.memory, "stagingBuffer.memory");

	void* data = nullptr;
	vkMapMemory(device, stagingBuffer.memory, 0, bufferSize, 0, &data);
	Utils::memcpy(data, bufferSize, vertices.data(), bufferSize);
	vkUnmapMemory(device, stagingBuffer.memory);

	createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		vertexBuffer.handle, vertexBuffer.memory);

	debugUtil.setObjectName(vertexBuffer.handle, "vertexBuffer.handle");
	debugUtil.setObjectName(vertexBuffer.memory, "vertexBuffer.memory");

	copyBuffer(stagingBuffer.handle, vertexBuffer.handle, bufferSize);

	vkDestroyBuffer(device, stagingBuffer.handle, allocator);
	vkFreeMemory(device, stagingBuffer.memory, allocator);

	return vertexBuffer;
}

Buffer VulkanApplication::createVertexBufferVma(const std::vector<Vertex>& vertices)
{
	VkDeviceSize bufferSize = VectorSize(Vertex, vertices);

	Buffer stagingBuffer;
	stagingBuffer.size = bufferSize;
	stagingBuffer.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	stagingBuffer.memoryPropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

	createBufferVma(stagingBuffer, "stagingBuffer");

	void* mappedData = nullptr;

	vmaMapMemory(vmaAllocator, stagingBuffer.allocation, &mappedData);
	Utils::memcpy(mappedData, bufferSize, vertices.data(), bufferSize);
	vmaUnmapMemory(vmaAllocator, stagingBuffer.allocation);

	Buffer vertexBuffer;
	vertexBuffer.size = bufferSize;
	vertexBuffer.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	vertexBuffer.memoryPropertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

	createBufferVma(vertexBuffer, "vertexBuffer");

	copyBuffer(stagingBuffer.handle, vertexBuffer.handle, bufferSize);

	destroyBuffer(stagingBuffer);

	return vertexBuffer;
}

Buffer VulkanApplication::createIndexBuffer(const std::vector<uint32_t>& indices)
{
	VkDeviceSize bufferSize = VectorSize(uint32_t, indices);

	Buffer indexBuffer;
	Buffer stagingBuffer;

	createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		stagingBuffer.handle, stagingBuffer.memory);

	debugUtil.setObjectName(stagingBuffer.handle, "stagingBuffer.handle");
	debugUtil.setObjectName(stagingBuffer.memory, "stagingBuffer.memory");

	void* data = nullptr;
	vkMapMemory(device, stagingBuffer.memory, 0, bufferSize, 0, &data);
	Utils::memcpy(data, bufferSize, indices.data(), bufferSize);
	vkUnmapMemory(device, stagingBuffer.memory);

	createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer.handle, indexBuffer.memory);

	debugUtil.setObjectName(indexBuffer.handle, "indexBuffer.handle");
	debugUtil.setObjectName(indexBuffer.memory, "indexBuffer.memory");

	copyBuffer(stagingBuffer.handle, indexBuffer.handle, bufferSize);

	vkDestroyBuffer(device, stagingBuffer.handle, allocator);
	vkFreeMemory(device, stagingBuffer.memory, allocator);

	return indexBuffer;
}

Buffer VulkanApplication::createIndexBufferVma(const std::vector<uint32_t>& indices)
{
	VkDeviceSize bufferSize = VectorSize(uint32_t, indices);

	Buffer stagingBuffer;
	stagingBuffer.size = bufferSize;
	stagingBuffer.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	stagingBuffer.memoryPropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

	createBufferVma(stagingBuffer, "stagingBuffer");

	void* mappedData = nullptr;

	vmaMapMemory(vmaAllocator, stagingBuffer.allocation, &mappedData);
	Utils::memcpy(mappedData, bufferSize, indices.data(), bufferSize);
	vmaUnmapMemory(vmaAllocator, stagingBuffer.allocation);

	Buffer indexBuffer;
	indexBuffer.size = bufferSize;
	indexBuffer.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
	indexBuffer.memoryPropertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

	createBufferVma(indexBuffer, "indexBuffer");

	copyBuffer(stagingBuffer.handle, indexBuffer.handle, bufferSize);
	
	destroyBuffer(stagingBuffer);

	return indexBuffer;
}

std::unique_ptr<MeshGeometry> VulkanApplication::createMeshGeometry(const Mesh& mesh)
{
	auto meshGeometry = std::make_unique<MeshGeometry>();
	
	if (useVma)
	{
		meshGeometry->vertexBuffer = createVertexBufferVma(mesh.getVertices());
		meshGeometry->indexBuffer = createIndexBufferVma(mesh.getIndices());
	}
	else
	{
		meshGeometry->vertexBuffer = createVertexBuffer(mesh.getVertices());
		meshGeometry->indexBuffer = createIndexBuffer(mesh.getIndices());
	}
	meshGeometry->vertexCount = static_cast<uint32_t>(mesh.getVertices().size());
	meshGeometry->indexCount = static_cast<uint32_t>(mesh.getIndices().size());
	meshGeometry->material = mesh.getMaterial();

	if (mesh.getMaterial()->hasTexture())
	{
		meshGeometry->hasTexture = true;

		if (mesh.getMaterial()->hasAlphaTexture())
		{
			meshGeometry->hasAlphaTexture = true;
		}
	}

	return meshGeometry;
}

std::unique_ptr<MeshGeometry> VulkanApplication::createMeshGeometry(const SimpleMeshInfo& mesh, const SimpleMaterialInfo& material)
{
	auto meshGeometry = std::make_unique<MeshGeometry>();
	meshGeometry->material = std::make_shared<Material>();

	auto textured = mesh.textured;

	if (useVma)
	{
		meshGeometry->vertexBuffer = createVertexBufferVma(mesh.vertices);
		meshGeometry->indexBuffer = createIndexBufferVma(mesh.indices);
	}
	else
	{
		meshGeometry->vertexBuffer = createVertexBuffer(mesh.vertices);
		meshGeometry->indexBuffer = createIndexBuffer(mesh.indices);
	}
	meshGeometry->vertexCount = static_cast<uint32_t>(mesh.vertices.size());
	meshGeometry->indexStartIndex = static_cast<uint32_t>(mesh.indexStartIndex);
	meshGeometry->indexCount = static_cast<uint32_t>(mesh.indices.size());
	meshGeometry->material->diffuseTexturePath = material.diffuseTexturePath;
	meshGeometry->material->normalTexturePath = material.normalTexturePath;
	meshGeometry->material->alphaTexturePath = material.alphaTexturePath;
	meshGeometry->material->diffuseTextureIndex = material.diffuseTextureIndex;
	meshGeometry->material->normalTextureIndex = material.normalTextureIndex;
	meshGeometry->material->roughnessTextureIndex = material.roughnessTextureIndex;
	meshGeometry->material->metallicTextureIndex = material.metallicTextureIndex;
	meshGeometry->material->alphaTextureIndex = material.alphaTextureIndex;
	meshGeometry->material->Kd = material.diffuseColor;
	meshGeometry->material->metallic = material.metallic;
	meshGeometry->material->roughness = material.roughness;
	meshGeometry->transform = mesh.transform;

	if (textured)
	{
		meshGeometry->hasTexture = true;

		if (mesh.alphaTextured)
		{
			meshGeometry->hasAlphaTexture = true;
		}

		if (mesh.normalTextured)
		{
			meshGeometry->hasNormalTexture = true;
		}
	}

	return meshGeometry;
}

void VulkanApplication::createMeshGeometries(const Model& model)
{
	for (size_t i = 0; i < model.meshes.size(); i++)
	{
		auto meshGeometry = createMeshGeometry(model.meshes[i]);

		meshGeometries.emplace_back(std::move(meshGeometry));
	}
}

void VulkanApplication::createMeshGeometries(const SimpleModel& model)
{
	for (size_t i = 0; i < model.meshes.size(); i++)
	{
		auto meshGeometry = createMeshGeometry(model.meshes[i], model.materials[model.meshes[i].materialIndex]);

		meshGeometries.emplace_back(std::move(meshGeometry));
	}
}

void VulkanApplication::createGlobalUniformBuffers()
{
	VkDeviceSize bufferSize = sizeof(GlobalUniformBufferObject);

	globalUniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
					 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			globalUniformBuffers[i].handle, globalUniformBuffers[i].memory);

		debugUtil.setObjectName(globalUniformBuffers[i].handle, "globalUniformBuffers[i].handle");
		debugUtil.setObjectName(globalUniformBuffers[i].memory, "globalUniformBuffers[i].memory");

		vkMapMemory(device, globalUniformBuffers[i].memory, 0, bufferSize, 0, &globalUniformBuffers[i].mappedData);
	}
}

void VulkanApplication::createGlobalUniformBuffersVma()
{
	VkDeviceSize bufferSize = sizeof(GlobalUniformBufferObject);

	globalUniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		globalUniformBuffers[i].size = bufferSize;
		globalUniformBuffers[i].usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		globalUniformBuffers[i].memoryPropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

		createBufferVma(globalUniformBuffers[i], "globalUniformBuffers[i]");

		vmaMapMemory(globalUniformBuffers[i].allocator, globalUniformBuffers[i].allocation, &globalUniformBuffers[i].mappedData);
	}
}

void VulkanApplication::createObjectUniformBuffers()
{
	VkDeviceSize bufferSize = objectUniformBufferAlignment * meshGeometries.size();

	objectUniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			objectUniformBuffers[i].handle, objectUniformBuffers[i].memory);

		vkMapMemory(device, objectUniformBuffers[i].memory, 0, bufferSize, 0, &objectUniformBuffers[i].mappedData);
	}
}

void VulkanApplication::createObjectUniformBuffersVma()
{
	VkDeviceSize bufferSize = objectUniformBufferAlignment * meshGeometries.size();

	objectUniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		objectUniformBuffers[i].size = bufferSize;
		objectUniformBuffers[i].usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		objectUniformBuffers[i].memoryPropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

		createBufferVma(objectUniformBuffers[i], "objectUniformBuffers[i]");

		vmaMapMemory(objectUniformBuffers[i].allocator, objectUniformBuffers[i].allocation, &objectUniformBuffers[i].mappedData);
	}
}

void VulkanApplication::createMaterialUniformBuffers()
{
	VkDeviceSize bufferSize = materialUniformBufferAlignment * meshGeometries.size();

	materialUniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			materialUniformBuffers[i].handle, materialUniformBuffers[i].memory);

		vkMapMemory(device, materialUniformBuffers[i].memory, 0, bufferSize, 0, &materialUniformBuffers[i].mappedData);
	}
}

void VulkanApplication::createMaterialUniformBuffersVma()
{
	VkDeviceSize bufferSize = materialUniformBufferAlignment * meshGeometries.size();

	materialUniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		materialUniformBuffers[i].size = bufferSize;
		materialUniformBuffers[i].usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		materialUniformBuffers[i].memoryPropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

		createBufferVma(materialUniformBuffers[i], "materialUniformBuffers[i]");

		vmaMapMemory(materialUniformBuffers[i].allocator, materialUniformBuffers[i].allocation, &materialUniformBuffers[i].mappedData);
	}
}

void VulkanApplication::createLightUniformBuffers()
{
	VkDeviceSize bufferSize = sizeof(LightUniformBufferObject);

	lightUniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			lightUniformBuffers[i].handle, lightUniformBuffers[i].memory);

		vkMapMemory(device, lightUniformBuffers[i].memory, 0, bufferSize, 0, &lightUniformBuffers[i].mappedData);
	}
}

void VulkanApplication::createLightUniformBuffersVma()
{
	VkDeviceSize bufferSize = sizeof(LightUniformBufferObject);

	lightUniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		lightUniformBuffers[i].size = bufferSize;
		lightUniformBuffers[i].usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		lightUniformBuffers[i].memoryPropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

		createBufferVma(lightUniformBuffers[i], "lightUniformBuffers[i]");

		vmaMapMemory(lightUniformBuffers[i].allocator, lightUniformBuffers[i].allocation, &lightUniformBuffers[i].mappedData);
	}
}

void VulkanApplication::createShaderStorageBuffers()
{
	shaderStorageBuffers.resize(MAX_FRAMES_IN_FLIGHT);

	// Initialize particles
	std::default_random_engine randomEngine((unsigned)time(nullptr));
	std::uniform_real_distribution<float> randomDist(0.0f, 1.0f);

	// Initial particle positions on a circle
	std::vector<Particle> particles(ParticleCount);

	for (auto& particle : particles) 
	{
		float r = 0.25f * sqrt(randomDist(randomEngine));
		float theta = randomDist(randomEngine) * 2.0f * 3.14159265358979323846f;
		float x = r * std::cos(theta) * WindowWidth / WindowHeight;
		float y = r * std::sin(theta);

		particle.position = glm::vec2(x, y);

		particle.velocity = glm::normalize(glm::vec2(x, y)) * 0.00025f;

		x *= randomDist(randomEngine) * 10.0f * 0.00025f;
		y *= randomDist(randomEngine) * 10.0f * 0.00025f;

		particle.velocity = { x, y };

		particle.color = glm::vec4(randomDist(randomEngine), randomDist(randomEngine), randomDist(randomEngine), 1.0f);
		//particle.color = { 1.0f, 1.0f, 0.0f, 1.0f };
	}

	VkDeviceSize bufferSize = sizeof(Particle) * ParticleCount;

	Buffer stagingBuffer;
	createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer.handle, stagingBuffer.memory);

	void* data;
	vkMapMemory(device, stagingBuffer.memory, 0, bufferSize, 0, &data);
	memcpy(data, particles.data(), (size_t)bufferSize);
	vkUnmapMemory(device, stagingBuffer.memory);

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) 
	{
		createBuffer(bufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, 
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, shaderStorageBuffers[i].handle, shaderStorageBuffers[i].memory);

		// Copy data from the staging buffer (host) to the shader storage buffer (GPU)
		copyBuffer(stagingBuffer.handle, shaderStorageBuffers[i].handle, bufferSize);
	}

	vkDestroyBuffer(device, stagingBuffer.handle, allocator);
	vkFreeMemory(device, stagingBuffer.memory, allocator);
}

void VulkanApplication::createShaderStorageBuffersVma()
{
	shaderStorageBuffers.resize(MAX_FRAMES_IN_FLIGHT);

	// Initialize particles
	std::default_random_engine randomEngine((unsigned)time(nullptr));
	std::uniform_real_distribution<float> randomDist(0.0f, 1.0f);

	// Initial particle positions on a circle
	std::vector<Particle> particles(ParticleCount);

	for (auto& particle : particles)
	{
		float r = 0.25f * sqrt(randomDist(randomEngine));
		float theta = randomDist(randomEngine) * 2.0f * 3.14159265358979323846f;
		float x = r * std::cos(theta) * WindowWidth / WindowHeight;
		float y = r * std::sin(theta);

		particle.position = glm::vec2(x, y);

		particle.velocity = glm::normalize(glm::vec2(x, y)) * 0.00025f;

		x *= randomDist(randomEngine) * 10.0f * 0.00025f;
		y *= randomDist(randomEngine) * 10.0f * 0.00025f;

		particle.velocity = { x, y };

		particle.color = glm::vec4(randomDist(randomEngine), randomDist(randomEngine), randomDist(randomEngine), 1.0f);
		//particle.color = { 1.0f, 1.0f, 0.0f, 1.0f };
	}

	VkDeviceSize bufferSize = sizeof(Particle) * ParticleCount;

	Buffer stagingBuffer;
	stagingBuffer.size = bufferSize;
	stagingBuffer.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	stagingBuffer.memoryPropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

	createBufferVma(stagingBuffer, "stagingBuffer");

	void* mappedData = nullptr;

	vmaMapMemory(vmaAllocator, stagingBuffer.allocation, &mappedData);
	Utils::memcpy(mappedData, static_cast<size_t>(bufferSize), particles.data(), static_cast<size_t>(bufferSize));
	vmaUnmapMemory(vmaAllocator, stagingBuffer.allocation);

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		shaderStorageBuffers[i].size = bufferSize;
		shaderStorageBuffers[i].usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		shaderStorageBuffers[i].memoryPropertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

		createBufferVma(shaderStorageBuffers[i], "shaderStorageBuffers[i]");

		// Copy data from the staging buffer (host) to the shader storage buffer (GPU)
		copyBuffer(stagingBuffer.handle, shaderStorageBuffers[i].handle, bufferSize);
	}

	destroyBuffer(stagingBuffer);
}

void VulkanApplication::createParticleUniformBuffers()
{
	VkDeviceSize bufferSize = sizeof(ParticleUniformBufferObject);

	particleUniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			particleUniformBuffers[i].handle, particleUniformBuffers[i].memory);

		vkMapMemory(device, particleUniformBuffers[i].memory, 0, bufferSize, 0, &particleUniformBuffers[i].mappedData);
	}
}

void VulkanApplication::createParticleUniformBuffersVma()
{
	VkDeviceSize bufferSize = sizeof(ParticleUniformBufferObject);

	particleUniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		particleUniformBuffers[i].size = bufferSize;
		particleUniformBuffers[i].usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		particleUniformBuffers[i].memoryPropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

		createBufferVma(particleUniformBuffers[i], "particleUniformBuffers[i]");

		vmaMapMemory(particleUniformBuffers[i].allocator, particleUniformBuffers[i].allocation, &particleUniformBuffers[i].mappedData);
	}
}

void VulkanApplication::createGraphicsCommandBuffers()
{
	graphicsCommandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

	VkCommandBufferAllocateInfo commandBufferAllocateInfo{};
	commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	commandBufferAllocateInfo.commandPool = graphicsCommandPool;

	// The level parameter specifies if the allocated command buffers are primary or secondary command buffers.
	// VK_COMMAND_BUFFER_LEVEL_PRIMARY: Can be submitted to a queue for execution, but cannot be called from other command buffers.
	// VK_COMMAND_BUFFER_LEVEL_SECONDARY : Cannot be submitted directly, but can be called from primary command buffers.
	commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	commandBufferAllocateInfo.commandBufferCount = static_cast<uint32_t>(graphicsCommandBuffers.size());

	VkCheck(vkAllocateCommandBuffers(device, &commandBufferAllocateInfo, graphicsCommandBuffers.data()), "lFaied to allocate command buffers!");
}

void VulkanApplication::createComputeCommandBuffers()
{
	computeCommandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

	VkCommandBufferAllocateInfo commandBufferAllocateInfo{};
	commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	commandBufferAllocateInfo.commandPool = graphicsCommandPool;

	// The level parameter specifies if the allocated command buffers are primary or secondary command buffers.
	// VK_COMMAND_BUFFER_LEVEL_PRIMARY: Can be submitted to a queue for execution, but cannot be called from other command buffers.
	// VK_COMMAND_BUFFER_LEVEL_SECONDARY : Cannot be submitted directly, but can be called from primary command buffers.
	commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	commandBufferAllocateInfo.commandBufferCount = static_cast<uint32_t>(computeCommandBuffers.size());

	VkCheck(vkAllocateCommandBuffers(device, &commandBufferAllocateInfo, computeCommandBuffers.data()), "lFaied to allocate command buffers!");
}

void VulkanApplication::createDescriptorPool()
{
	// Create Descriptor Pool
	{
		VkDescriptorPoolSize poolSizes[] =
		{
			{ VK_DESCRIPTOR_TYPE_SAMPLER, MAX_FRAMES_IN_FLIGHT },
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_FRAMES_IN_FLIGHT },
			{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, MAX_FRAMES_IN_FLIGHT },
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, MAX_FRAMES_IN_FLIGHT },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, MAX_FRAMES_IN_FLIGHT },
			{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, MAX_FRAMES_IN_FLIGHT },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, MAX_FRAMES_IN_FLIGHT },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, MAX_FRAMES_IN_FLIGHT * 2 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, MAX_FRAMES_IN_FLIGHT },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, MAX_FRAMES_IN_FLIGHT },
			{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, MAX_FRAMES_IN_FLIGHT }
		};
		VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {};
		descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		descriptorPoolCreateInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		descriptorPoolCreateInfo.maxSets = MAX_FRAMES_IN_FLIGHT * IM_ARRAYSIZE(poolSizes);
		descriptorPoolCreateInfo.poolSizeCount = (uint32_t)IM_ARRAYSIZE(poolSizes);
		descriptorPoolCreateInfo.pPoolSizes = poolSizes;
		VkCheck(vkCreateDescriptorPool(device, &descriptorPoolCreateInfo, allocator, &descriptorPool), "vkCreateDescriptorPool failed!");
	}
}

void VulkanApplication::createGraphicsDescriptorSets()
{
	// https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/2501
	// Did you specify the variable descriptor count through a vkDescriptorSetVariableDescriptorCountAllocateInfo to vkAllocateDescriptorSets ? 
	// I got the tip from https ://gist.github.com/NotAPenguin0/284461ecc81267fa41a7fbc472cd3afe#creating-the-descriptor-sets.

	// I received a similar validation error even though variable sized sampler2D arrays worked fine on my setup.However, variable sized image2D 
	// arrays didn't work until I added the above info, which also fixed the validation error.
	std::vector<VkDescriptorSetLayout> descriptorSetLayouts(MAX_FRAMES_IN_FLIGHT, graphicsDescriptorSetLayout);

	VkDescriptorSetVariableDescriptorCountAllocateInfo setCounts = {};
	setCounts.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO;
	setCounts.descriptorSetCount = MAX_FRAMES_IN_FLIGHT;

	// 这里counts的数量要等于上面的descriptorSetCount
	uint32_t counts[MAX_FRAMES_IN_FLIGHT]{ static_cast<uint32_t>(textureImagePaths.size()), static_cast<uint32_t>(textureImagePaths.size()) };

	setCounts.pDescriptorCounts = counts;

	VkDescriptorSetAllocateInfo descriptorSetAllocateInfo{};
	descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descriptorSetAllocateInfo.descriptorPool = descriptorPool;
	descriptorSetAllocateInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
	descriptorSetAllocateInfo.pSetLayouts = descriptorSetLayouts.data();
	descriptorSetAllocateInfo.pNext = &setCounts;

	graphicsDescriptorSets.resize(MAX_FRAMES_IN_FLIGHT);

	VkCheck(vkAllocateDescriptorSets(device, &descriptorSetAllocateInfo, graphicsDescriptorSets.data()), "Failed to allocate descriptor sets!");

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		VkDescriptorBufferInfo globalBufferInfo{};
		globalBufferInfo.buffer = globalUniformBuffers[i].handle;
		globalBufferInfo.offset = 0;
		globalBufferInfo.range = sizeof(GlobalUniformBufferObject);

		VkDescriptorBufferInfo objectBufferInfo{};
		objectBufferInfo.buffer = objectUniformBuffers[i].handle;
		objectBufferInfo.offset = 0;
		objectBufferInfo.range = sizeof(ObjectUniformBufferObject);

		VkDescriptorBufferInfo materialBufferInfo{};
		materialBufferInfo.buffer = materialUniformBuffers[i].handle;
		materialBufferInfo.offset = 0;
		materialBufferInfo.range = sizeof(MaterialUniformBufferObject);

		VkDescriptorBufferInfo lightBufferInfo{};
		lightBufferInfo.buffer = lightUniformBuffers[i].handle;
		lightBufferInfo.offset = 0;
		lightBufferInfo.range = sizeof(LightUniformBufferObject);

		std::vector<VkDescriptorImageInfo> imageInfos;
		
		imageInfos.resize(textureImages.size());

		for (size_t i = 0; i < imageInfos.size(); i++)
		{
			imageInfos[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageInfos[i].imageView = textureImages[i].view;
			imageInfos[i].sampler = textureSampler;
		}

		std::array<VkWriteDescriptorSet, 6> writeDescriptorSets{};

		writeDescriptorSets[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeDescriptorSets[0].dstSet = graphicsDescriptorSets[i];
		writeDescriptorSets[0].dstBinding = 0;
		writeDescriptorSets[0].dstArrayElement = 0;
		writeDescriptorSets[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		writeDescriptorSets[0].descriptorCount = 1;
		writeDescriptorSets[0].pBufferInfo = &globalBufferInfo;
		writeDescriptorSets[0].pImageInfo = nullptr;
		writeDescriptorSets[0].pTexelBufferView = nullptr;

		writeDescriptorSets[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeDescriptorSets[1].dstSet = graphicsDescriptorSets[i];
		writeDescriptorSets[1].dstBinding = 1;
		writeDescriptorSets[1].dstArrayElement = 0;
		writeDescriptorSets[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
		writeDescriptorSets[1].descriptorCount = 1;
		writeDescriptorSets[1].pBufferInfo = &objectBufferInfo;
		writeDescriptorSets[1].pImageInfo = nullptr;
		writeDescriptorSets[1].pTexelBufferView = nullptr;

		writeDescriptorSets[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeDescriptorSets[2].dstSet = graphicsDescriptorSets[i];
		writeDescriptorSets[2].dstBinding = 2;
		writeDescriptorSets[2].dstArrayElement = 0;
		writeDescriptorSets[2].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
		writeDescriptorSets[2].descriptorCount = 1;
		writeDescriptorSets[2].pBufferInfo = &materialBufferInfo;
		writeDescriptorSets[2].pImageInfo = nullptr;
		writeDescriptorSets[2].pTexelBufferView = nullptr;

		writeDescriptorSets[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeDescriptorSets[3].dstSet = graphicsDescriptorSets[i];
		writeDescriptorSets[3].dstBinding = 3;
		writeDescriptorSets[3].dstArrayElement = 0;
		writeDescriptorSets[3].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		writeDescriptorSets[3].descriptorCount = 1;
		writeDescriptorSets[3].pBufferInfo = &lightBufferInfo;
		writeDescriptorSets[3].pImageInfo = nullptr;
		writeDescriptorSets[3].pTexelBufferView = nullptr;

		writeDescriptorSets[4].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeDescriptorSets[4].dstSet = graphicsDescriptorSets[i];
		writeDescriptorSets[4].dstBinding = 4;
		writeDescriptorSets[4].dstArrayElement = 0;
		writeDescriptorSets[4].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		writeDescriptorSets[4].descriptorCount = 1;
		writeDescriptorSets[4].pBufferInfo = nullptr;
		writeDescriptorSets[4].pImageInfo = &offscreenPass.descriptor[0];
		writeDescriptorSets[4].pTexelBufferView = nullptr;

		writeDescriptorSets[5].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeDescriptorSets[5].dstSet = graphicsDescriptorSets[i];
		writeDescriptorSets[5].dstBinding = 5;
		writeDescriptorSets[5].dstArrayElement = 0;
		writeDescriptorSets[5].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		writeDescriptorSets[5].descriptorCount = static_cast<uint32_t>(imageInfos.size());
		writeDescriptorSets[5].pBufferInfo = nullptr;
		writeDescriptorSets[5].pImageInfo = imageInfos.data();
		writeDescriptorSets[5].pTexelBufferView = nullptr;

		vkUpdateDescriptorSets(device, static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, nullptr);
	}
}

void VulkanApplication::createOffscreenDescriptorSets()
{
	// https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/2501
	// Did you specify the variable descriptor count through a vkDescriptorSetVariableDescriptorCountAllocateInfo to vkAllocateDescriptorSets ? 
	// I got the tip from https ://gist.github.com/NotAPenguin0/284461ecc81267fa41a7fbc472cd3afe#creating-the-descriptor-sets.

	// I received a similar validation error even though variable sized sampler2D arrays worked fine on my setup.However, variable sized image2D 
	// arrays didn't work until I added the above info, which also fixed the validation error.
	std::vector<VkDescriptorSetLayout> offscreenDescriptorSetLayouts(MAX_FRAMES_IN_FLIGHT, descriptorSetLayouts.offscreen);
	std::vector<VkDescriptorSetLayout> bloomDescriptorSetLayouts(MAX_FRAMES_IN_FLIGHT, descriptorSetLayouts.bloom);

	VkDescriptorSetVariableDescriptorCountAllocateInfo setCounts = {};
	setCounts.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO;
	setCounts.descriptorSetCount = MAX_FRAMES_IN_FLIGHT;

	// 这里counts的数量要等于上面的descriptorSetCount
	uint32_t counts[MAX_FRAMES_IN_FLIGHT]{ 1, 1 };

	setCounts.pDescriptorCounts = counts;

	VkDescriptorSetAllocateInfo offscreenDescriptorSetAllocateInfo{};
	offscreenDescriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	offscreenDescriptorSetAllocateInfo.descriptorPool = descriptorPool;
	offscreenDescriptorSetAllocateInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
	offscreenDescriptorSetAllocateInfo.pSetLayouts = offscreenDescriptorSetLayouts.data();
	offscreenDescriptorSetAllocateInfo.pNext = &setCounts;

	VkDescriptorSetAllocateInfo bloomDescriptorSetAllocateInfo = offscreenDescriptorSetAllocateInfo;
	bloomDescriptorSetAllocateInfo.pSetLayouts = bloomDescriptorSetLayouts.data();

	descriptorSets.offscreen.resize(MAX_FRAMES_IN_FLIGHT);
	descriptorSets.bloom.resize(MAX_FRAMES_IN_FLIGHT);
	descriptorSets.screenQuad.resize(MAX_FRAMES_IN_FLIGHT);

	VkCheck(vkAllocateDescriptorSets(device, &offscreenDescriptorSetAllocateInfo, descriptorSets.offscreen.data()), "Failed to allocate descriptor sets!");
	VkCheck(vkAllocateDescriptorSets(device, &bloomDescriptorSetAllocateInfo, descriptorSets.bloom.data()), "Failed to allocate descriptor sets!");
	VkCheck(vkAllocateDescriptorSets(device, &bloomDescriptorSetAllocateInfo, descriptorSets.screenQuad.data()), "Failed to allocate descriptor sets!");

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		std::array<VkWriteDescriptorSet, 2> writeDescriptorSets{};

		writeDescriptorSets[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeDescriptorSets[0].dstSet = descriptorSets.offscreen[i];
		writeDescriptorSets[0].dstBinding = 0;
		writeDescriptorSets[0].dstArrayElement = 0;
		writeDescriptorSets[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		writeDescriptorSets[0].descriptorCount = 1;
		writeDescriptorSets[0].pBufferInfo = nullptr;
		writeDescriptorSets[0].pImageInfo = &offscreenPass.descriptor[0];
		writeDescriptorSets[0].pTexelBufferView = nullptr;

		writeDescriptorSets[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeDescriptorSets[1].dstSet = descriptorSets.offscreen[i];
		writeDescriptorSets[1].dstBinding = 1;
		writeDescriptorSets[1].dstArrayElement = 0;
		writeDescriptorSets[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		writeDescriptorSets[1].descriptorCount = 1;
		writeDescriptorSets[1].pBufferInfo = nullptr;
		writeDescriptorSets[1].pImageInfo = &offscreenPass.descriptor[1];
		writeDescriptorSets[1].pTexelBufferView = nullptr;

		vkUpdateDescriptorSets(device, static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, nullptr);

		writeDescriptorSets[0].dstSet = descriptorSets.bloom[i];
		writeDescriptorSets[0].pImageInfo = &offscreenPass.descriptor[0];

		writeDescriptorSets[1].dstSet = descriptorSets.bloom[i];
		writeDescriptorSets[1].pImageInfo = &offscreenPass.descriptor[1];

		vkUpdateDescriptorSets(device, static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, nullptr);

		writeDescriptorSets[0].dstSet = descriptorSets.screenQuad[i];
		writeDescriptorSets[0].pImageInfo = &offscreenPass.descriptor[0];

		writeDescriptorSets[1].dstSet = descriptorSets.screenQuad[i];
		writeDescriptorSets[1].pImageInfo = &bloomFilterPass.descriptor[0];

		vkUpdateDescriptorSets(device, static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, nullptr);
	}
}

void VulkanApplication::createComputeDescriptorSets()
{
	std::vector<VkDescriptorSetLayout> descriptorSetLayouts(MAX_FRAMES_IN_FLIGHT, computeDescriptorSetLayout);

	VkDescriptorSetAllocateInfo descriptorSetAllocateInfo{};
	descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descriptorSetAllocateInfo.descriptorPool = descriptorPool;
	descriptorSetAllocateInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
	descriptorSetAllocateInfo.pSetLayouts = descriptorSetLayouts.data();

	computeDescriptorSets.resize(MAX_FRAMES_IN_FLIGHT);

	VkCheck(vkAllocateDescriptorSets(device, &descriptorSetAllocateInfo, computeDescriptorSets.data()), "Failed to allocate descriptor sets!");

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) 
	{
		std::array<VkWriteDescriptorSet, 3> descriptorWrites{};

		VkDescriptorBufferInfo uniformBufferInfo{};
		uniformBufferInfo.buffer = particleUniformBuffers[i].handle;
		uniformBufferInfo.offset = 0;
		uniformBufferInfo.range = sizeof(ParticleUniformBufferObject);

		descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[0].dstSet = computeDescriptorSets[i];
		descriptorWrites[0].dstBinding = 0;
		descriptorWrites[0].dstArrayElement = 0;
		descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrites[0].descriptorCount = 1;
		descriptorWrites[0].pBufferInfo = &uniformBufferInfo;

		VkDescriptorBufferInfo storageBufferInfoLastFrame{};
		storageBufferInfoLastFrame.buffer = shaderStorageBuffers[(i - 1) % MAX_FRAMES_IN_FLIGHT].handle;
		storageBufferInfoLastFrame.offset = 0;
		storageBufferInfoLastFrame.range = sizeof(Particle) * ParticleCount;

		descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[1].dstSet = computeDescriptorSets[i];
		descriptorWrites[1].dstBinding = 1;
		descriptorWrites[1].dstArrayElement = 0;
		descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		descriptorWrites[1].descriptorCount = 1;
		descriptorWrites[1].pBufferInfo = &storageBufferInfoLastFrame;

		VkDescriptorBufferInfo storageBufferInfoCurrentFrame{};
		storageBufferInfoCurrentFrame.buffer = shaderStorageBuffers[i].handle;
		storageBufferInfoCurrentFrame.offset = 0;
		storageBufferInfoCurrentFrame.range = sizeof(Particle) * ParticleCount;

		descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[2].dstSet = computeDescriptorSets[i];
		descriptorWrites[2].dstBinding = 2;
		descriptorWrites[2].dstArrayElement = 0;
		descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		descriptorWrites[2].descriptorCount = 1;
		descriptorWrites[2].pBufferInfo = &storageBufferInfoCurrentFrame;

		vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
	}
}

void VulkanApplication::createSyncObjects()
{
	imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	graphicsInFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

	VkSemaphoreCreateInfo semaphoreCreateInfo{};
	semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceCreateInfo{};
	fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

	// Before we can proceed, there is a slight hiccup in our design. On the first frame we call drawFrame(),
	// which immediately waits on inFlightFence to be signaled. inFlightFence is only signaled after a frame 
	// has finished rendering, yet since this is the first frame, there are no previous frames in which to signal 
	// the fence! Thus vkWaitForFences() blocks indefinitely, waiting on something which will never happen.

	// Of the many solutions to this dilemma, there is a clever workaround built into the API.Create the fence in the
	// signaled state, so that the first call to vkWaitForFences() returns immediately since the fence is already signaled.
	// To do this, we add the VK_FENCE_CREATE_SIGNALED_BIT flag to the VkFenceCreateInfo :
	fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	computeInFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
	computeFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);

	VkSemaphoreCreateInfo computeSemaphoreInfo{};
	computeSemaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo computeFenceInfo{};
	computeFenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	computeFenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		VkCheck(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &imageAvailableSemaphores[i]), "Failed to create semaphores!");
		VkCheck(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &renderFinishedSemaphores[i]), "Failed to create semaphores!");
		VkCheck(vkCreateFence(device, &fenceCreateInfo, nullptr, &graphicsInFlightFences[i]), "Failed to create fences!");

		VkCheck(vkCreateSemaphore(device, &computeSemaphoreInfo, nullptr, &computeFinishedSemaphores[i]), "Failed to create semaphores!");
		VkCheck(vkCreateFence(device, &computeFenceInfo, nullptr, &computeInFlightFences[i]), "Failed to create fences!");
	}
}

SimpleModel VulkanApplication::mergeModels(const std::vector<SimpleModel>& models)
{
	size_t materialIndexOffset = 0;
	size_t indexOffset = 0;

	SimpleModel result;

	for (auto model : models)
	{
		result.materials.insert(result.materials.end(), model.materials.begin(), model.materials.end());

		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;

		for (auto& mesh : model.meshes)
		{
			mesh.materialIndex += materialIndexOffset;
			mesh.indexStartIndex += result.indexCount;
			vertices.insert(vertices.end(), mesh.vertices.begin(), mesh.vertices.end());
			indices.insert(indices.end(),  mesh.indices.begin(), mesh.indices.end());
		}

		std::transform(indices.begin(), indices.end(), indices.begin(), [=](uint32_t index) { return index + static_cast<uint32_t>(indexOffset); });

		materialIndexOffset += model.materials.size();
		indexOffset += model.vertices.size();
		result.indexCount += model.indexCount;

		result.meshes.insert(result.meshes.end(), model.meshes.begin(), model.meshes.end());
		result.vertices.insert(result.vertices.end(), vertices.begin(), vertices.end());
		result.indices.insert(result.indices.end(), indices.begin(), indices.end());
	}

	uint32_t textureImageViewCount = 0;
	for (auto& material : result.materials)
	{
		if (!material.diffuseTexturePath.empty())
		{
			auto findResult = std::find(textureImagePaths.begin(), textureImagePaths.end(), material.diffuseTexturePath);

			if (findResult == textureImagePaths.end())
			{
				textureImagePaths.emplace_back(material.diffuseTexturePath);
				material.diffuseTextureIndex = textureImageViewCount;
				texturePathIndexMap[material.diffuseTexturePath] = textureImageViewCount;
				textureImageViewCount++;
			}
			else
			{
				material.diffuseTextureIndex = texturePathIndexMap[material.diffuseTexturePath];
			}

			if (!material.normalTexturePath.empty())
			{
				findResult = std::find(textureImagePaths.begin(), textureImagePaths.end(), material.normalTexturePath);
				textureImagePaths.emplace_back(material.normalTexturePath);
				material.normalTextureIndex = textureImageViewCount;
				texturePathIndexMap[material.normalTexturePath] = textureImageViewCount;
				textureImageViewCount++;
			}

			if (!material.roughnessTexturePath.empty())
			{
				findResult = std::find(textureImagePaths.begin(), textureImagePaths.end(), material.roughnessTexturePath);

				if (findResult == textureImagePaths.end())
				{
					textureImagePaths.emplace_back(material.roughnessTexturePath);
					material.roughnessTextureIndex = textureImageViewCount;
					texturePathIndexMap[material.roughnessTexturePath] = textureImageViewCount;
					textureImageViewCount++;
				}
			}

			if (!material.metallicTexturePath.empty())
			{
				findResult = std::find(textureImagePaths.begin(), textureImagePaths.end(), material.metallicTexturePath);

				if (findResult == textureImagePaths.end())
				{
					textureImagePaths.emplace_back(material.metallicTexturePath);
					material.metallicTextureIndex = textureImageViewCount;
					texturePathIndexMap[material.metallicTexturePath] = textureImageViewCount;
					textureImageViewCount++;
				}
			}

			if (!material.alphaTexturePath.empty())
			{
				findResult = std::find(textureImagePaths.begin(), textureImagePaths.end(), material.alphaTexturePath);

				if (findResult == textureImagePaths.end())
				{
					textureImagePaths.emplace_back(material.alphaTexturePath);
					material.alphaTextureIndex = textureImageViewCount;
					texturePathIndexMap[material.alphaTexturePath] = textureImageViewCount;
					textureImageViewCount++;
				}
				else
				{
					material.alphaTextureIndex = texturePathIndexMap[material.alphaTexturePath];
				}
			}
		}
	}

	return result;
}

void VulkanApplication::recordGraphicsCommandBuffer(VkCommandBuffer graphicsCommandBuffer, uint32_t imageIndex)
{
	OPTICK_PUSH("recordGraphicsCommandBuffer");

	VkCommandBufferBeginInfo commandBufferBeginInfo{};
	commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	commandBufferBeginInfo.flags = 0;
	commandBufferBeginInfo.pInheritanceInfo = nullptr;

	VkCheck(vkBeginCommandBuffer(graphicsCommandBuffer, &commandBufferBeginInfo), "Failed to begin recording command buffer!");

	offscreenRenderPass(imageIndex, graphicsCommandBuffer);

	// Note: Explicit synchronization is not required between the render pass, as this is done implicit via sub pass dependencies
	//sceneRenderPass(imageIndex, graphicsCommandBuffer);

	bloomRenderPass(imageIndex, graphicsCommandBuffer);

	screenQuadRenderPass(imageIndex, graphicsCommandBuffer);

	VkCheck(vkEndCommandBuffer(graphicsCommandBuffer), "Failed to record command buffer!");

	OPTICK_POP();
}

void VulkanApplication::offscreenRenderPass(uint32_t imageIndex, VkCommandBuffer graphicsCommandBuffer)
{
	// First render pass: Offscreen rendering
	VkRenderPassBeginInfo renderPassBeginInfo{};
	renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBeginInfo.renderPass = offscreenPass.renderPass;
	renderPassBeginInfo.framebuffer = offscreenPass.frameBuffer;
	renderPassBeginInfo.renderArea.offset = { 0, 0 };
	renderPassBeginInfo.renderArea.extent = swapChainExtent;

	std::array<VkClearValue, 3> clearValues{};

	clearValues[0].color = { clearColor.x, clearColor.y, clearColor.z, clearColor.w };
	clearValues[1].color = { clearColor.x, clearColor.y, clearColor.z, clearColor.w };
	clearValues[2].depthStencil = { 1.0f, 0 };

	renderPassBeginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
	renderPassBeginInfo.pClearValues = clearValues.data();

	// The final parameter controls how the drawing commands within the render pass will be provided. It can have one of two values:
	// VK_SUBPASS_CONTENTS_INLINE: The render pass commands will be embedded in the primary command buffer itself and no secondary command buffers will be executed.
	// VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS : The render pass commands will be executed from secondary command buffers.
	vkCmdBeginRenderPass(graphicsCommandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

	vkCmdBindPipeline(graphicsCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines.offscreen);

	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(offscreenPass.width);
	viewport.height = static_cast<float>(offscreenPass.height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	vkCmdSetViewport(graphicsCommandBuffer, 0, 1, &viewport);

	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = swapChainExtent;

	vkCmdSetScissor(graphicsCommandBuffer, 0, 1, &scissor);

	// 这里vertexBUffer是全局的顶点缓冲(整个.obj模型的所有submesh数据都存在它里面)
	VkBuffer vertexBuffers[]{ vertexBuffer.handle };
	VkDeviceSize offsets[]{ 0 };
	vkCmdBindVertexBuffers(graphicsCommandBuffer, 0, 1, vertexBuffers, offsets);

	// 这里的indexBUffer同样是全局的索引缓冲
	// 绘制时通过vkCmdDrawIndexd第三个参数indexCount配合第四个参数firstIndex来进行索引
	vkCmdBindIndexBuffer(graphicsCommandBuffer, indexBuffer.handle, 0, VK_INDEX_TYPE_UINT32);

	for (size_t i = 0; i < meshGeometries.size(); i++)
	{
		const auto& meshGeometry = meshGeometries[i];

		//VkBuffer vertexBuffers[]{ meshGeometry->vertexBuffer };

		if (meshGeometry->indexBuffer.handle != nullptr)
		{
			//vkCmdBindIndexBuffer(inCommandBuffer, meshGeometry->indexBuffer, 0, VK_INDEX_TYPE_UINT32);
		}

		MaterialUniformBufferObject materialUniformBufferObject;

		if (meshGeometry->hasTexture)
		{
			materialUniformBufferObject.diffuseColor = glm::vec4(meshGeometry->material->Kd, 1.0f);

			if (meshGeometry->hasAlphaTexture)
			{
				materialUniformBufferObject.diffuseColor = glm::vec4(meshGeometry->material->Kd, 2.0f);
			}

			if (meshGeometry->hasNormalTexture)
			{
				materialUniformBufferObject.diffuseColor = glm::vec4(meshGeometry->material->Kd, 3.0f);
			}
		}
		else
		{
			materialUniformBufferObject.diffuseColor = glm::vec4(meshGeometry->material->Kd, 0.0f);
		}

		materialUniformBufferObject.metallic = meshGeometry->material->metallic;
		materialUniformBufferObject.roughness = meshGeometry->material->roughness;
		materialUniformBufferObject.diffuseTextureIndex = meshGeometry->material->diffuseTextureIndex;
		materialUniformBufferObject.normalTextureIndex = meshGeometry->material->normalTextureIndex;
		materialUniformBufferObject.roughnessTextureIndex = meshGeometry->material->roughnessTextureIndex;
		materialUniformBufferObject.metallicTextureIndex = meshGeometry->material->metallicTextureIndex;
		materialUniformBufferObject.alphaTextureIndex = meshGeometry->material->alphaTextureIndex;

		updateMaterialUniformBuffer(currentFrame, static_cast<uint32_t>(i), materialUniformBufferObject);

		ObjectUniformBufferObject objectUniformBufferObject;
		objectUniformBufferObject.model = meshGeometry->transform;

		updateObjectUniformBuffer(currentFrame, static_cast<uint32_t>(i), objectUniformBufferObject);

		// From the Vulkan specification :
		// The descriptor set contents bound by a call to vkCmdBindDescriptorSets may be consumed during host execution of the command, 
		// or during shader execution of the resulting draws, or any time in between.Thus, the contents must not be altered(overwritten
		// by an update command, or freed) between when the command is recorded and when the command completes executing on the queue.
		//updateImageView(meshGeometry->textureImageView, meshGeometry->alphaTextureImageView, currentFrame);

		uint32_t dynamicOffsets[2] = { static_cast<uint32_t>(i) * static_cast<uint32_t>(objectUniformBufferAlignment), static_cast<uint32_t>(i) * static_cast<uint32_t>(materialUniformBufferAlignment) };
		vkCmdBindDescriptorSets(graphicsCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipelineLayout, 0, 1, &graphicsDescriptorSets[currentFrame], 2, dynamicOffsets);

		// The actual vkCmdDraw function is a bit anticlimactic, but it's so simple because of all the information we specified in advance. 
		// It has the following parameters, aside from the command buffer:
		// vertexCount: Even though we don't have a vertex buffer, we technically still have 3 vertices to draw.
		// instanceCount : Used for instanced rendering, use 1 if you're not doing that.
		// firstVertex : Used as an offset into the vertex buffer, defines the lowest value of gl_VertexIndex.
		// firstInstance : Used as an offset for instanced rendering, defines the lowest value of gl_InstanceIndex.

		// 第四个参数代表当前submesh的索引在全局索引缓冲中的起始位置
		vkCmdDrawIndexed(graphicsCommandBuffer, meshGeometry->indexCount, 1, meshGeometry->indexStartIndex, 0, 0);
	}

	vkCmdEndRenderPass(graphicsCommandBuffer);
}

void VulkanApplication::sceneRenderPass(uint32_t imageIndex, VkCommandBuffer graphicsCommandBuffer)
{
	// Second render pass: Scene rendering
	VkRenderPassBeginInfo renderPassBeginInfo{};
	renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBeginInfo.renderPass = renderPass;
	renderPassBeginInfo.framebuffer = swapChainFramebuffers[imageIndex];
	renderPassBeginInfo.renderArea.offset = { 0, 0 };
	renderPassBeginInfo.renderArea.extent = swapChainExtent;

	std::array<VkClearValue, 2> clearValues{};

	clearValues[0].color = { clearColor.x, clearColor.y, clearColor.z, clearColor.w };
	clearValues[1].depthStencil = { 1.0f, 0 };

	renderPassBeginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
	renderPassBeginInfo.pClearValues = clearValues.data();

	// The final parameter controls how the drawing commands within the render pass will be provided. It can have one of two values:
	// VK_SUBPASS_CONTENTS_INLINE: The render pass commands will be embedded in the primary command buffer itself and no secondary command buffers will be executed.
	// VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS : The render pass commands will be executed from secondary command buffers.
	vkCmdBeginRenderPass(graphicsCommandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

	vkCmdBindPipeline(graphicsCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(swapChainExtent.width);
	viewport.height = static_cast<float>(swapChainExtent.height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	vkCmdSetViewport(graphicsCommandBuffer, 0, 1, &viewport);

	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = swapChainExtent;

	vkCmdSetScissor(graphicsCommandBuffer, 0, 1, &scissor);

	// 这里vertexBUffer是全局的顶点缓冲(整个.obj模型的所有submesh数据都存在它里面)
	VkBuffer vertexBuffers[]{ vertexBuffer.handle };
	VkDeviceSize offsets[]{ 0 };
	vkCmdBindVertexBuffers(graphicsCommandBuffer, 0, 1, vertexBuffers, offsets);

	// 这里的indexBUffer同样是全局的索引缓冲
	// 绘制时通过vkCmdDrawIndexd第三个参数indexCount配合第四个参数firstIndex来进行索引
	vkCmdBindIndexBuffer(graphicsCommandBuffer, indexBuffer.handle, 0, VK_INDEX_TYPE_UINT32);

	for (size_t i = 0; i < meshGeometries.size(); i++)
	{
		const auto& meshGeometry = meshGeometries[i];

		//VkBuffer vertexBuffers[]{ meshGeometry->vertexBuffer };

		if (meshGeometry->indexBuffer.handle != nullptr)
		{
			//vkCmdBindIndexBuffer(inCommandBuffer, meshGeometry->indexBuffer, 0, VK_INDEX_TYPE_UINT32);
		}

		MaterialUniformBufferObject materialUniformBufferObject;

		if (meshGeometry->hasTexture)
		{
			materialUniformBufferObject.diffuseColor = glm::vec4(meshGeometry->material->Kd, 1.0f);

			if (meshGeometry->hasAlphaTexture)
			{
				materialUniformBufferObject.diffuseColor = glm::vec4(meshGeometry->material->Kd, 2.0f);
			}

			if (meshGeometry->hasNormalTexture)
			{
				materialUniformBufferObject.diffuseColor = glm::vec4(meshGeometry->material->Kd, 3.0f);
			}
		}
		else
		{
			materialUniformBufferObject.diffuseColor = glm::vec4(meshGeometry->material->Kd, 0.0f);
		}

		materialUniformBufferObject.metallic = meshGeometry->material->metallic;
		materialUniformBufferObject.roughness = meshGeometry->material->roughness;
		materialUniformBufferObject.diffuseTextureIndex = meshGeometry->material->diffuseTextureIndex;
		materialUniformBufferObject.normalTextureIndex = meshGeometry->material->normalTextureIndex;
		materialUniformBufferObject.roughnessTextureIndex = meshGeometry->material->roughnessTextureIndex;
		materialUniformBufferObject.metallicTextureIndex = meshGeometry->material->metallicTextureIndex;
		materialUniformBufferObject.alphaTextureIndex = meshGeometry->material->alphaTextureIndex;

		updateMaterialUniformBuffer(currentFrame, static_cast<uint32_t>(i), materialUniformBufferObject);

		ObjectUniformBufferObject objectUniformBufferObject;
		objectUniformBufferObject.model = meshGeometry->transform;

		updateObjectUniformBuffer(currentFrame, static_cast<uint32_t>(i), objectUniformBufferObject);

		// From the Vulkan specification :
		// The descriptor set contents bound by a call to vkCmdBindDescriptorSets may be consumed during host execution of the command, 
		// or during shader execution of the resulting draws, or any time in between.Thus, the contents must not be altered(overwritten
		// by an update command, or freed) between when the command is recorded and when the command completes executing on the queue.
		//updateImageView(meshGeometry->textureImageView, meshGeometry->alphaTextureImageView, currentFrame);

		uint32_t dynamicOffsets[2] = { static_cast<uint32_t>(i) * static_cast<uint32_t>(objectUniformBufferAlignment), static_cast<uint32_t>(i) * static_cast<uint32_t>(materialUniformBufferAlignment) };
		vkCmdBindDescriptorSets(graphicsCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipelineLayout, 0, 1, &graphicsDescriptorSets[currentFrame], 2, dynamicOffsets);

		// The actual vkCmdDraw function is a bit anticlimactic, but it's so simple because of all the information we specified in advance. 
		// It has the following parameters, aside from the command buffer:
		// vertexCount: Even though we don't have a vertex buffer, we technically still have 3 vertices to draw.
		// instanceCount : Used for instanced rendering, use 1 if you're not doing that.
		// firstVertex : Used as an offset into the vertex buffer, defines the lowest value of gl_VertexIndex.
		// firstInstance : Used as an offset for instanced rendering, defines the lowest value of gl_InstanceIndex.

		// 第四个参数代表当前submesh的索引在全局索引缓冲中的起始位置
		vkCmdDrawIndexed(graphicsCommandBuffer, meshGeometry->indexCount, 1, meshGeometry->indexStartIndex, 0, 0);
	}

	//VkBuffer quadVertexBuffers[]{ quadVertexBuffer.handle };
	//VkDeviceSize quadOffsets[]{ 0 };
	//vkCmdBindVertexBuffers(graphicsCommandBuffer, 0, 1, quadVertexBuffers, quadOffsets);

	//vkCmdBindIndexBuffer(graphicsCommandBuffer, quadIndexBuffer.handle, 0, VK_INDEX_TYPE_UINT32);

	//uint32_t dynamicOffsets[2] = { 0, 0 };

	//vkCmdBindDescriptorSets(graphicsCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipelineLayout, 0, 1, &graphicsDescriptorSets[currentFrame], 2, dynamicOffsets);

	//vkCmdDrawIndexed(graphicsCommandBuffer, 6, 1, 0, 0, 0);

	//// Particles
	//vkCmdBindVertexBuffers(graphicsCommandBuffer, 0, 1, &shaderStorageBuffers[currentFrame].handle, offsets);

	//vkCmdBindPipeline(graphicsCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, particlePipeline);

	//vkCmdBindDescriptorSets(graphicsCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, particlePipelineLayout, 0, 1, &computeDescriptorSets[currentFrame], 0, nullptr);

	//vkCmdDraw(graphicsCommandBuffer, ParticleCount, 1, 0, 0);

	renderImGui();

	vkCmdEndRenderPass(graphicsCommandBuffer);
}

void VulkanApplication::bloomRenderPass(uint32_t imageIndex, VkCommandBuffer graphicsCommandBuffer)
{
	// Second render pass: Scene rendering
	VkRenderPassBeginInfo renderPassBeginInfo{};
	renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBeginInfo.renderPass = bloomFilterPass.renderPass;
	renderPassBeginInfo.framebuffer = bloomFilterPass.frameBuffer;
	renderPassBeginInfo.renderArea.offset = { 0, 0 };
	renderPassBeginInfo.renderArea.extent = swapChainExtent;

	std::array<VkClearValue, 2> clearValues{};

	clearValues[0].color = { clearColor.x, clearColor.y, clearColor.z, clearColor.w };
	clearValues[1].depthStencil = { 1.0f, 0 };

	renderPassBeginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
	renderPassBeginInfo.pClearValues = clearValues.data();

	// The final parameter controls how the drawing commands within the render pass will be provided. It can have one of two values:
	// VK_SUBPASS_CONTENTS_INLINE: The render pass commands will be embedded in the primary command buffer itself and no secondary command buffers will be executed.
	// VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS : The render pass commands will be executed from secondary command buffers.
	vkCmdBeginRenderPass(graphicsCommandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

	vkCmdBindPipeline(graphicsCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines.bloom[1]);

	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(swapChainExtent.width);
	viewport.height = static_cast<float>(swapChainExtent.height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	vkCmdSetViewport(graphicsCommandBuffer, 0, 1, &viewport);

	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = swapChainExtent;

	vkCmdSetScissor(graphicsCommandBuffer, 0, 1, &scissor);

	vkCmdBindDescriptorSets(graphicsCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayouts.bloom, 0, 1, &descriptorSets.bloom[currentFrame], 0, nullptr);

	vkCmdDraw(graphicsCommandBuffer, 3, 1, 0, 0);

	vkCmdEndRenderPass(graphicsCommandBuffer);
}

void VulkanApplication::screenQuadRenderPass(uint32_t imageIndex, VkCommandBuffer graphicsCommandBuffer)
{
	// Second render pass: Scene rendering
	VkRenderPassBeginInfo renderPassBeginInfo{};
	renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBeginInfo.renderPass = renderPass;
	renderPassBeginInfo.framebuffer = swapChainFramebuffers[imageIndex];
	renderPassBeginInfo.renderArea.offset = { 0, 0 };
	renderPassBeginInfo.renderArea.extent = swapChainExtent;

	std::array<VkClearValue, 2> clearValues{};

	clearValues[0].color = { clearColor.x, clearColor.y, clearColor.z, clearColor.w };
	clearValues[1].depthStencil = { 1.0f, 0 };

	renderPassBeginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
	renderPassBeginInfo.pClearValues = clearValues.data();

	// The final parameter controls how the drawing commands within the render pass will be provided. It can have one of two values:
	// VK_SUBPASS_CONTENTS_INLINE: The render pass commands will be embedded in the primary command buffer itself and no secondary command buffers will be executed.
	// VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS : The render pass commands will be executed from secondary command buffers.
	vkCmdBeginRenderPass(graphicsCommandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

	vkCmdBindDescriptorSets(graphicsCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayouts.screenQuad, 0, 1, &descriptorSets.screenQuad[currentFrame], 0, nullptr);
	
	if (tonemapping)
	{
		vkCmdBindPipeline(graphicsCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines.screenQuad);
	}
	else
	{
		vkCmdBindPipeline(graphicsCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines.screenQuadNoToneMapping);
	}

	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(swapChainExtent.width);
	viewport.height = static_cast<float>(swapChainExtent.height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	vkCmdSetViewport(graphicsCommandBuffer, 0, 1, &viewport);

	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = swapChainExtent;

	vkCmdSetScissor(graphicsCommandBuffer, 0, 1, &scissor);

	vkCmdDraw(graphicsCommandBuffer, 3, 1, 0, 0);

	if (bloom)
	{
		vkCmdBindPipeline(graphicsCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines.bloom[0]);

		vkCmdDraw(graphicsCommandBuffer, 3, 1, 0, 0);
	}

	renderImGui();

	vkCmdEndRenderPass(graphicsCommandBuffer);
}

void VulkanApplication::recordComputeCommandBuffer(VkCommandBuffer computeCommandBuffer)
{
	VkCommandBufferBeginInfo commandBufferBeginInfo{};
	commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	commandBufferBeginInfo.flags = 0;
	commandBufferBeginInfo.pInheritanceInfo = nullptr;

	VkCheck(vkBeginCommandBuffer(computeCommandBuffer, &commandBufferBeginInfo), "Failed to begin recording command buffer!");

	vkCmdBindPipeline(computeCommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, computePipeline);

	vkCmdBindDescriptorSets(computeCommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, computePipelineLayout, 0, 1, &computeDescriptorSets[currentFrame], 0, nullptr);

	vkCmdDispatch(computeCommandBuffer, ParticleCount / 256, 1, 1);

	VkCheck(vkEndCommandBuffer(computeCommandBuffer), "Failed to record command buffer!");
}

VkShaderModule VulkanApplication::createShaderModule(const std::string& path)
{
	auto shaderCode = readFile(path);
	return createShaderModule(shaderCode);
}

VkShaderModule VulkanApplication::createShaderModule(const std::vector<char>& shaderCode)
{
	VkShaderModuleCreateInfo shaderModuleCreateInfo{};
	shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	shaderModuleCreateInfo.codeSize = shaderCode.size();

	// The one catch is that the size of the byte code is specified in bytes, but the 
	// byte code pointer is a uint32_t pointer rather than a char pointer.Therefore we 
	// will need to cast the pointer with reinterpret_cast
	shaderModuleCreateInfo.pCode = reinterpret_cast<const uint32_t*>(shaderCode.data());

	VkShaderModule shaderModule;

	VkCheck(vkCreateShaderModule(device, &shaderModuleCreateInfo, nullptr, &shaderModule), "Failed to create shader module!");

	return shaderModule;
}

void VulkanApplication::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags memoryPropertyFlags, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
{
	VkBufferCreateInfo bufferCreateInfo{};
	bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.size = size;
	bufferCreateInfo.usage = usage;
	bufferCreateInfo.flags = 0;

	QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);

	uint32_t pQueueFamilyIndices[] = { queueFamilyIndices.graphicsAndComputeFamily.value(), queueFamilyIndices.transferFamily.value() };

	if (queueFamilyIndices.graphicsAndComputeFamily != queueFamilyIndices.transferFamily)
	{
		// Sharing between graphics queue family and transfer queue family
		bufferCreateInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;
		bufferCreateInfo.queueFamilyIndexCount = 2;
		bufferCreateInfo.pQueueFamilyIndices = pQueueFamilyIndices;
	}
	else
	{
		bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	}

	VkCheck(vkCreateBuffer(device, &bufferCreateInfo, allocator, &buffer), "Failed to create vertex buffer!");

	VkMemoryRequirements memoryRequirements;
	vkGetBufferMemoryRequirements(device, buffer, &memoryRequirements);

	VkMemoryAllocateInfo allocateInfo{};
	allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocateInfo.allocationSize = memoryRequirements.size;
	allocateInfo.memoryTypeIndex = findMemoryType(memoryRequirements.memoryTypeBits, memoryPropertyFlags);

	VkCheck(vkAllocateMemory(device, &allocateInfo, allocator, &bufferMemory), "Failed to allocate vertex buffer memory!");

	vkBindBufferMemory(device, buffer, bufferMemory, 0);
}

void VulkanApplication::createBuffer(Buffer& buffer)
{
	createBuffer(buffer.size, buffer.usage, buffer.memoryPropertyFlags, buffer.handle, buffer.memory);
}

void VulkanApplication::createBufferVma(Buffer& buffer, const std::string& name)
{
	VmaAllocationCreateInfo allocationCreateInfo{};
	allocationCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
	allocationCreateInfo.preferredFlags = buffer.memoryPropertyFlags;
	allocationCreateInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;

	char* nameString = const_cast<char*>(name.c_str());
	allocationCreateInfo.pUserData = static_cast<void*>(nameString);

	VkBufferCreateInfo bufferCreateInfo{};
	bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.size = buffer.size;
	bufferCreateInfo.usage = buffer.usage;

	vmaCreateBuffer(buffer.allocator, &bufferCreateInfo, &allocationCreateInfo, &buffer.handle, &buffer.allocation, nullptr);
	debugUtil.setObjectName(buffer.handle, name);
	VmaAllocationInfo allocationInfo;
	vmaGetAllocationInfo(buffer.allocator, buffer.allocation, &allocationInfo);
	vmaSetAllocationName(buffer.allocator, buffer.allocation, name.c_str());
}

void VulkanApplication::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
	VkCommandBuffer oneTimeCommandBuffer = beginSingleTimeCommands();

	VkBufferCopy copyRegion{};
	copyRegion.srcOffset = 0;
	copyRegion.dstOffset = 0;
	copyRegion.size = size;

	vkCmdCopyBuffer(oneTimeCommandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

	endSingleTimeCommands(oneTimeCommandBuffer);
}

void VulkanApplication::destroyBuffer(Buffer& buffer, bool mapped)
{
	if (useVma)
	{
		if (mapped)
		{
			vmaUnmapMemory(buffer.allocator, buffer.allocation);
		}
		vmaDestroyBuffer(buffer.allocator, buffer.handle, buffer.allocation);
	}
	else
	{
		vkDestroyBuffer(device, buffer.handle, allocator);
		if (mapped)
		{
			vkUnmapMemory(device, buffer.memory);
		}
		vkFreeMemory(device, buffer.memory, allocator);
	}
}

void VulkanApplication::destroyImage(Image& image)
{
	if (useVma)
	{
		vmaDestroyImage(image.allocator, image.handle, image.allocation);
	}
	else
	{
		vkDestroyImageView(device, image.view, allocator);
		vkDestroyImage(device, image.handle, allocator);
		vkFreeMemory(device, image.memory, allocator);
	}
}

void VulkanApplication::createImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags propertyFlags, VkImage& image, VkDeviceMemory& imageMemory)
{
	VkImageCreateInfo imageCreateInfo{};
	imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	imageCreateInfo.extent.width = static_cast<uint32_t>(width);
	imageCreateInfo.extent.height = static_cast<uint32_t>(height);
	imageCreateInfo.extent.depth = 1;
	imageCreateInfo.mipLevels = mipLevels;
	imageCreateInfo.arrayLayers = 1;
	imageCreateInfo.format = format;
	imageCreateInfo.tiling = tiling;
	imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageCreateInfo.usage = usage;
	imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageCreateInfo.samples = numSamples;
	imageCreateInfo.flags = 0;

	VkCheck(vkCreateImage(device, &imageCreateInfo, allocator, &image), "Faild to create image!");

	debugUtil.setObjectName(image, "image");

	VkMemoryRequirements memoryRequirements;
	vkGetImageMemoryRequirements(device, image, &memoryRequirements);

	VkMemoryAllocateInfo memoryAllocateInfo{};
	memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryAllocateInfo.allocationSize = memoryRequirements.size;
	memoryAllocateInfo.memoryTypeIndex = findMemoryType(memoryRequirements.memoryTypeBits, propertyFlags);

	VkCheck(vkAllocateMemory(device, &memoryAllocateInfo, allocator, &imageMemory), "Failed to allocate image memory!");

	debugUtil.setObjectName(imageMemory, "imageMemory");

	vkBindImageMemory(device, image, imageMemory, 0);
}

void VulkanApplication::createImage(Image& image)
{
	createImage(image.width, image.height, image.mipLevels, image.numSamples, image.format, 
				      image.tiling, image.usage, image.memoryPropertyFlags, image.handle, image.memory);
}

void VulkanApplication::createImageVma(Image& image, const std::string& name)
{
	VkImageCreateInfo imageCreateInfo{};
	imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	imageCreateInfo.extent.width = image.width;
	imageCreateInfo.extent.height = image.height;
	imageCreateInfo.extent.depth = 1;
	imageCreateInfo.mipLevels = image.mipLevels;
	imageCreateInfo.arrayLayers = 1;
	imageCreateInfo.format = image.format;
	imageCreateInfo.tiling = image.tiling;
	imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageCreateInfo.usage = image.usage;
	imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageCreateInfo.samples = image.numSamples;
	imageCreateInfo.flags = 0;

	VmaAllocationCreateInfo allocationCreateInfo{};
	allocationCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
	allocationCreateInfo.preferredFlags = image.memoryPropertyFlags;

	char* nameString = const_cast<char*>(name.c_str());
	allocationCreateInfo.pUserData = static_cast<void*>(nameString);
	//allocationCreateInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;

	vmaCreateImage(image.allocator, &imageCreateInfo, &allocationCreateInfo, &image.handle, &image.allocation, nullptr);

	debugUtil.setObjectName(image.handle, name);
	vmaSetAllocationName(image.allocator, image.allocation, name.c_str());
}

VkImageView VulkanApplication::createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels)
{
	VkImageViewCreateInfo imageViewCreateInfo{};
	imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	imageViewCreateInfo.image = image;
	imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	imageViewCreateInfo.format = format;
	imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
	imageViewCreateInfo.subresourceRange = {};
	imageViewCreateInfo.subresourceRange.aspectMask = aspectFlags;
	imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
	imageViewCreateInfo.subresourceRange.levelCount = mipLevels;
	imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
	imageViewCreateInfo.subresourceRange.layerCount = 1;

	VkImageView imageView;

	VkCheck(vkCreateImageView(device, &imageViewCreateInfo, nullptr, &imageView), "Failed to create image views!");

	debugUtil.setObjectName(imageView, "imageView");

	return imageView;
}

VkImageView VulkanApplication::createImageView(Image image, VkImageAspectFlags aspectFlags)
{
	return createImageView(image.handle, image.format, aspectFlags, image.mipLevels);
}

void VulkanApplication::generateTangents(SimpleModel& model)
{
	std::vector<tgen::VIndexT> vertexIndices;
	std::vector<tgen::VIndexT> uvIndices;

	for (size_t i = 0; i < model.indexCount; i++)
	{
		vertexIndices.push_back(model.indices[i]);
	}

	uvIndices = vertexIndices;

	std::vector<tgen::RealT> vertices;
	std::vector<tgen::RealT> normals;
	std::vector<tgen::RealT> uvs;

	for (size_t v = 0; v < model.vertices.size(); v++)
	{
		const auto& vertex = model.vertices[v];

		for (int i = 0; i < 3; i++)
		{
			vertices.push_back(vertex.position[i]);
		}

		for (int j = 0; j < 3; j++)
		{
			normals.push_back(vertex.normal[j]);
		}

		for (int k = 0; k < 2; k++)
		{
			uvs.push_back(vertex.texcoord[k]);
		}
	}

	std::vector<tgen::RealT> cornerTangents;
	std::vector<tgen::RealT> cornerBitangents;

	tgen::computeCornerTSpace(vertexIndices, uvIndices, vertices, uvs, cornerTangents, cornerBitangents);

	std::vector<tgen::RealT> vertexTangents;
	std::vector<tgen::RealT> vertexBitangents;

	tgen::computeVertexTSpace(uvIndices, cornerTangents, cornerBitangents, model.vertices.size(), vertexTangents, vertexBitangents);

	tgen::orthogonalizeTSpace(normals, vertexTangents, vertexBitangents);

	std::vector<tgen::RealT> tangents;

	tgen::computeTangent4D(normals, vertexTangents, vertexBitangents, tangents);
	
	for (size_t i = 0; i < model.vertices.size(); i++)
	{
		auto& vertex = model.vertices[i];

		vertex.tangent.x = static_cast<float>(tangents[i * 4]);
		vertex.tangent.y = static_cast<float>(tangents[i * 4 + 1]);
		vertex.tangent.z = static_cast<float>(tangents[i * 4 + 2]);
	}
}

void VulkanApplication::transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels)
{
	VkCommandBuffer oneTimeCommandBuffer = beginSingleTimeCommands();

	VkImageMemoryBarrier imageMemoryBarrier{};
	imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imageMemoryBarrier.oldLayout = oldLayout;
	imageMemoryBarrier.newLayout = newLayout;
	imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imageMemoryBarrier.image = image;
	imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imageMemoryBarrier.subresourceRange.baseMipLevel = 0;
	imageMemoryBarrier.subresourceRange.levelCount = mipLevels;
	imageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
	imageMemoryBarrier.subresourceRange.layerCount = 1;

	VkPipelineStageFlags sourceStage;
	VkPipelineStageFlags destinationStage;

	if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
	{
		imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

		if (hasStencilComponent(format))
		{
			imageMemoryBarrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
		}
	}
	else
	{
		imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	}

	if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
	{
		imageMemoryBarrier.srcAccessMask = 0;
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
	{
		imageMemoryBarrier.srcAccessMask = 0;
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	}
	else
	{
		throw std::invalid_argument("Unsupported layout transition!");
	}

	vkCmdPipelineBarrier(oneTimeCommandBuffer,
			  sourceStage,	// TODO
	          destinationStage,	// TODO
						 0,
						 0, nullptr,
						 0, nullptr,
						 1, &imageMemoryBarrier);

	endSingleTimeCommands(oneTimeCommandBuffer);
}

void VulkanApplication::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
{
	VkCommandBuffer oneTimeCommandBuffer = beginSingleTimeCommands();

	// Most of these fields are self - explanatory.The bufferOffset specifies the byte offset in the buffer at 
	// which the pixel values start. The bufferRowLength and bufferImageHeight fields specify how the pixels are 
	// laid out in memory.For example, you could have some padding bytes between rows of the image. Specifying 0 
	// for both indicates that the pixels are simply tightly packed like they are in our case. The imageSubresource, 
	// imageOffsetand imageExtent fields indicate to which part of the image we want to copy the pixels.
	VkBufferImageCopy copyRegion{};
	copyRegion.bufferOffset = 0;
	copyRegion.bufferRowLength = 0;
	copyRegion.bufferImageHeight = 0;

	copyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	copyRegion.imageSubresource.mipLevel = 0;
	copyRegion.imageSubresource.baseArrayLayer = 0;
	copyRegion.imageSubresource.layerCount = 1;

	copyRegion.imageOffset = { 0, 0, 0 };
	copyRegion.imageExtent = { width, height, 1 };

	vkCmdCopyBufferToImage(oneTimeCommandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);

	endSingleTimeCommands(oneTimeCommandBuffer);
}

void VulkanApplication::createVmaAllocator()
{
	VkPhysicalDeviceProperties props{};
	vkGetPhysicalDeviceProperties(physicalDevice, &props);

	VmaVulkanFunctions vulkanFunctions{};
	vulkanFunctions.vkGetInstanceProcAddr = vkGetInstanceProcAddr;
	vulkanFunctions.vkGetDeviceProcAddr = vkGetDeviceProcAddr;
	vulkanFunctions.vkGetPhysicalDeviceProperties = vkGetPhysicalDeviceProperties; 
	vulkanFunctions.vkGetPhysicalDeviceMemoryProperties = vkGetPhysicalDeviceMemoryProperties; 
	vulkanFunctions.vkAllocateMemory = vkAllocateMemory; 
	vulkanFunctions.vkFreeMemory = vkFreeMemory; 
	vulkanFunctions.vkMapMemory = vkMapMemory; 
	vulkanFunctions.vkUnmapMemory = vkUnmapMemory; 
	vulkanFunctions.vkFlushMappedMemoryRanges = vkFlushMappedMemoryRanges;
	vulkanFunctions.vkInvalidateMappedMemoryRanges = vkInvalidateMappedMemoryRanges; 
	vulkanFunctions.vkBindBufferMemory = vkBindBufferMemory;
	vulkanFunctions.vkBindImageMemory = vkBindImageMemory; 
	vulkanFunctions.vkGetBufferMemoryRequirements = vkGetBufferMemoryRequirements; 
	vulkanFunctions.vkGetImageMemoryRequirements = vkGetImageMemoryRequirements;
	vulkanFunctions.vkCreateBuffer = vkCreateBuffer;
	vulkanFunctions.vkDestroyBuffer = vkDestroyBuffer; 
	vulkanFunctions.vkGetDeviceBufferMemoryRequirements = vkGetDeviceBufferMemoryRequirements;
	vulkanFunctions.vkGetDeviceImageMemoryRequirements = vkGetDeviceImageMemoryRequirements;
	vulkanFunctions.vkCreateImage = vkCreateImage; 
	vulkanFunctions.vkDestroyImage = vkDestroyImage; 
	vulkanFunctions.vkCmdCopyBuffer = vkCmdCopyBuffer;
	vulkanFunctions.vkGetBufferMemoryRequirements2KHR = vkGetBufferMemoryRequirements2KHR; 
	vulkanFunctions.vkGetImageMemoryRequirements2KHR = vkGetImageMemoryRequirements2KHR; 
	vulkanFunctions.vkBindBufferMemory2KHR = vkBindBufferMemory2KHR; 
	vulkanFunctions.vkBindImageMemory2KHR = vkBindImageMemory2KHR; 
	vulkanFunctions.vkGetPhysicalDeviceMemoryProperties2KHR = vkGetPhysicalDeviceMemoryProperties2KHR;

	VmaAllocatorCreateInfo allocInfo{};
	allocInfo.physicalDevice = physicalDevice;
	allocInfo.device = device;
	allocInfo.instance = instance;
	allocInfo.pVulkanFunctions = &vulkanFunctions;
	allocInfo.vulkanApiVersion = VK_API_VERSION_1_3;

	VmaAllocator allocator = VK_NULL_HANDLE;

	//VkCheck(vmaCreateAllocator(&allocInfo, &allocator), "Unable to create allocator vmaCreateAllocator() returned %s");

	if (auto const res = vmaCreateAllocator(&allocInfo, &vmaAllocator); VK_SUCCESS != res)
	{
		throw labutils::Error("Unable to create allocator\n"
			"vmaCreateAllocator() returned %s", labutils::to_string(res).c_str()
		);
	}
}

VkCommandBuffer VulkanApplication::beginSingleTimeCommands()
{
	VkCommandBufferAllocateInfo commandBufferAllocateInfo{};
	commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	commandBufferAllocateInfo.commandPool = transferCommandPool;
	commandBufferAllocateInfo.commandBufferCount = 1;

	VkCommandBuffer oneTimeCommandBuffer;
	vkAllocateCommandBuffers(device, &commandBufferAllocateInfo, &oneTimeCommandBuffer);

	// We're only going to use the command buffer once and wait with returning from the function until 
	// the copy operation has finished executing. It's good practice to tell the driver about our intent 
	// using VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT.
	VkCommandBufferBeginInfo commandBufferBeginInfo{};
	commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(oneTimeCommandBuffer, &commandBufferBeginInfo);

	return oneTimeCommandBuffer;
}

void VulkanApplication::endSingleTimeCommands(VkCommandBuffer inCommandBuffer)
{
	vkEndCommandBuffer(inCommandBuffer);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &inCommandBuffer;

	vkQueueSubmit(transferQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(transferQueue);

	vkFreeCommandBuffers(device, transferCommandPool, 1, &inCommandBuffer);
}

void VulkanApplication::generateMipmaps(VkImage image, VkFormat imageFormat, int32_t width, int32_t height, uint32_t mipLevels)
{    
	// Check if image format supports linear blitting
	VkFormatProperties formatProperties;
	vkGetPhysicalDeviceFormatProperties(physicalDevice, imageFormat, &formatProperties);

	if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) 
	{
		THROW_ERROR("Texture image format does not support linear blitting!");
	}

	VkCommandBuffer commandBuffer = beginSingleTimeCommands();

	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.image = image;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;
	barrier.subresourceRange.levelCount = 1;

	int32_t mipWidth = width;
	int32_t mipHeight = height;

	for (uint32_t i = 1; i < mipLevels; i++) 
	{
		barrier.subresourceRange.baseMipLevel = i - 1;
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

		vkCmdPipelineBarrier(commandBuffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
			0, nullptr,
			0, nullptr,
			1, &barrier);

		VkImageBlit blit{};
		blit.srcOffsets[0] = { 0, 0, 0 };
		blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
		blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.srcSubresource.mipLevel = i - 1;
		blit.srcSubresource.baseArrayLayer = 0;
		blit.srcSubresource.layerCount = 1;
		blit.dstOffsets[0] = { 0, 0, 0 };
		blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
		blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.dstSubresource.mipLevel = i;
		blit.dstSubresource.baseArrayLayer = 0;
		blit.dstSubresource.layerCount = 1;

		vkCmdBlitImage(commandBuffer,
			image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1, &blit,
			VK_FILTER_LINEAR);

		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		vkCmdPipelineBarrier(commandBuffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
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
		VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
		0, nullptr,
		0, nullptr,
		1, &barrier);

	endSingleTimeCommands(commandBuffer);
}

std::vector<std::string> VulkanApplication::visit(std::string path)
{
	static std::vector<std::string> result;

	std::regex image(".*.*\\.(jpg|png|bmp)");	//包含字母z的所有jpg或png图片

	for (auto& fe : std::filesystem::directory_iterator(path))
	{
		const auto& fp = fe.path();

		if (!fp.has_extension())
		{
			visit(fp.string());
		}

		//std::wcout << fp.filename().wstring() << std::endl;

		auto temp = fp.filename();

		if (std::regex_match(temp.string(), image))
		{
			result.emplace_back(fp.string());
			//std::cout << temp << std::endl;
		}
		//replace_extension替换扩展名
		//stem去掉扩展名
	}

	return result;
}

void VulkanApplication::etc2Compress(const std::string& inputPath, const std::string& overrideOutputPath)
{
	int32_t width;
	int32_t height;
	int32_t comp;

	const uint8_t* imageData = stbi_load(inputPath.c_str(), &width, &height, &comp, 4);

	//float* imageData = stbi_loadf(inputPath.c_str(), &width, &height, &comp, 4);
	
	stbi_set_flip_vertically_on_load(true);
	stbi_ldr_to_hdr_scale(1.0f);

	std::vector<float> rgbaf;

	for (int i = 0; i != width * height * 4; i += 4)
	{
		rgbaf.push_back(imageData[i + 0] / 255.0f);
		rgbaf.push_back(imageData[i + 1] / 255.0f);
		rgbaf.push_back(imageData[i + 2] / 255.0f);
		rgbaf.push_back(imageData[i + 3] / 255.0f);
	}

	const auto etcFormat = Etc::Image::Format::RGB8;
	const auto errorMetric = Etc::ErrorMetric::BT709;

	Etc::Image image(rgbaf.data(), width, height, errorMetric);

	image.Encode(etcFormat, errorMetric, ETCCOMP_DEFAULT_EFFORT_LEVEL, std::thread::hardware_concurrency(), 1024);

	std::filesystem::path temp = inputPath;

	std::string outputPath = overrideOutputPath;

	if (outputPath.empty())
	{
		auto fileName = temp.replace_extension(".ktx").filename().string();
		auto directory = temp.remove_filename().string();

		outputPath = directory + "Compressed/" + fileName;
	}

	Etc::File etcFile(
		outputPath.c_str(),
		Etc::File::Format::KTX,
		etcFormat,
		image.GetEncodingBits(),
		image.GetEncodingBitsBytes(),
		image.GetSourceWidth(),
		image.GetSourceHeight(),
		image.GetExtendedWidth(),
		image.GetExtendedHeight()
	);
	etcFile.Write();
}

SimpleMaterialInfo VulkanApplication::bakedMaterial2SimpleMaterial(const BakedMaterialInfo& bakedMaterial, const std::vector<BakedTextureInfo>& bakedTextures)
{
	SimpleMaterialInfo material;

	auto textured = bakedMaterial.baseColorTextureId != 0xffffffff;

	if (textured)
	{
		const auto& baseColorTexture = bakedTextures[bakedMaterial.baseColorTextureId];

		material.diffuseTextureIndex = bakedMaterial.baseColorTextureId;
		material.diffuseTexturePath = baseColorTexture.path;
	}

	auto alphaTextured = bakedMaterial.alphaMaskTextureId != 0xffffffff;

	if (alphaTextured)
	{
		const auto& alphaTexture = bakedTextures[bakedMaterial.alphaMaskTextureId];

		material.alphaTextureIndex = bakedMaterial.alphaMaskTextureId;
		material.alphaTexturePath = alphaTexture.path;
	}

	auto normalTextured = bakedMaterial.normalMapTextureId != 0xffffffff;

	if (normalTextured)
	{
		const auto& normalTexture = bakedTextures[bakedMaterial.normalMapTextureId];

		material.normalTextureIndex = bakedMaterial.normalMapTextureId;
		material.normalTexturePath = normalTexture.path;
	}

	auto roughnessTextured = bakedMaterial.roughnessTextureId != 0xffffffff;

	if (roughnessTextured)
	{
		const auto& roughnessTexture = bakedTextures[bakedMaterial.roughnessTextureId];

		material.roughnessTextureIndex = bakedMaterial.roughnessTextureId;
		material.roughnessTexturePath = roughnessTexture.path;
	}

	auto metallicTextured = bakedMaterial.metalnessTextureId != 0xffffffff;

	if (metallicTextured)
	{
		const auto& metallicTexture = bakedTextures[bakedMaterial.metalnessTextureId];

		material.metallicTextureIndex = bakedMaterial.metalnessTextureId;
		material.metallicTexturePath = metallicTexture.path;
	}

	material.diffuseColor = bakedMaterial.baseColor;
	material.emissionColor = bakedMaterial.emissiveColor;
	material.metallic = bakedMaterial.metalness;
	material.roughness = bakedMaterial.roughness;

	return material;
}

SimpleModel VulkanApplication::bakedModel2SimpleModel(const BakedModel& bakedModel)
{
	SimpleModel model;

	const auto& bakedMaterials = bakedModel.materials;
	const auto& bakedMeshes = bakedModel.meshes;
	const auto& bakedextures = bakedModel.textures;

	std::size_t indexStartIndex = 0;

	uint32_t indexOffset = 0;
 		
	for (size_t i = 0; i < bakedMeshes.size(); i++)
	{
		const auto& bakedMesh = bakedMeshes[i];

		const auto& bakedMatrial = bakedMaterials[bakedMesh.materialId];

		SimpleMeshInfo mesh;

		for (size_t j = 0; j < bakedMesh.positions.size(); j++)
		{
			auto position = bakedMesh.positions[j];
			auto normal = bakedMesh.normals[j];
			auto texcoord = bakedMesh.texcoords[j];

			Vertex vertex{ position, normal, texcoord, glm::vec3(1.0f) };
			mesh.vertices.emplace_back(vertex);
		}

		mesh.indices = bakedMesh.indices;

		std::transform(mesh.indices.begin(), mesh.indices.end(), mesh.indices.begin(), [=](uint32_t index) { return index + static_cast<uint32_t>(indexOffset); });

		indexOffset += static_cast<uint32_t>(mesh.vertices.size());

		auto material =	bakedMaterial2SimpleMaterial(bakedMatrial, bakedextures);

		mesh.materialIndex = i;
		mesh.textured = !material.diffuseTexturePath.empty();
		mesh.alphaTextured = !material.alphaTexturePath.empty();
		mesh.normalTextured = !material.normalTexturePath.empty();
		mesh.roughnessTextured = !material.roughnessTexturePath.empty();
		mesh.metallicTextured = !material.metallicTexturePath.empty();

		mesh.vertexCount = bakedMesh.positions.size();
		mesh.indexStartIndex = indexStartIndex;
		mesh.indexCount = bakedMesh.indices.size();

		indexStartIndex += mesh.indexCount;

		model.indexCount += mesh.indexCount;

		model.vertices.insert(model.vertices.end(), mesh.vertices.begin(), mesh.vertices.end());
		model.indices.insert(model.indices.end(), mesh.indices.begin(), mesh.indices.end());

		model.meshes.emplace_back(mesh);
		model.materials.emplace_back(material);
	}

	return model;
}

void VulkanApplication::loadResources()
{
	//model.load(ResourceBase + "models/cube.obj");
	//model.load(ResourceBase + "models/texture_bunny.obj");
	//model.load(ResourceBase + "sponza_with_ship.obj");
	auto bakedSponzaModel = loadBakedModel(ResourceBase + "Models/sponza-pbr/sponza-pbr.comp5822mesh");
	auto bakedShipModel = loadBakedModel(ResourceBase + "Models/NewShip/ship.comp5822mesh");

	//sponza = loadSimpleWavefrontObj((ResourceBase + "models/sponza-pbr/sponza-pbr.obj").c_str());
	plane = loadSimpleWavefrontObj((ResourceBase + "Models/plane.obj").c_str());
	cube = loadSimpleWavefrontObj((ResourceBase + "Models/cube.obj").c_str());
	sphere = loadSimpleWavefrontObj((ResourceBase + "Models/sphere.obj").c_str());
	//objModel = loadSimpleWavefrontObj((ResourceBase + "models/plane.obj").c_str());
	//sponza = loadSimpleWavefrontObj((ResourceBase + "models/sponza_with_ship.obj").c_str());
	marry = loadSimpleWavefrontObj((ResourceBase + "Models/Marry.obj").c_str());
	disc = loadSimpleWavefrontObj((ResourceBase + "Models/Disc.obj").c_str());
	ship = loadSimpleWavefrontObj((ResourceBase + "Models/NewShip/NewShip.obj").c_str());

	sponza = bakedModel2SimpleModel(bakedSponzaModel);

	//ship = bakedModel2SimpleModel(bakedShipModel);

	auto transform = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 15.0f, -5.0f));

	cube.setTransform(transform);

	models.emplace_back(cube);

	transform = glm::translate(glm::mat4(1.0f), glm::vec3(-8.0f, 6.1f, -5.0f));
	transform = glm::scale(transform, glm::vec3(2.0f, 2.0f, 2.0f));

	marry.setTransform(transform);

	models.emplace_back(marry);
	
	for (uint32_t i = 0; i < LightCount; ++i)
	{
		lightPositions[i].y += 20.0f;
		lightPositions[i].z += -10.0f;

		transform = glm::translate(glm::mat4(1.0f), { lightPositions[i].x, lightPositions[i].y, lightPositions[i].z });

		sphere.setTransform(transform);

		sphere.materials[0].diffuseColor = glm::vec3(0.5f, 0.0f, 0.0f);

		models.emplace_back(sphere);
	}

	for (int row = 0; row < numOfRows; ++row)
	{
		auto metallic = (float)row / (float)numOfRows;
		for (int column = 0; column < numOfColumns; ++column)
		{
			// we clamp the roughness to 0.05 - 1.0 as perfectly smooth surfaces (roughness of 0.0) tend to look a bit off
			// on direct lighting.
			auto roughness = glm::clamp((float)column / (float)numOfColumns, 0.05f, 1.0f);

			transform = glm::mat4(1.0f);
			transform = glm::translate(transform, glm::vec3(
				(column - (numOfColumns / 2)) * spacing,
				(row - (numOfRows / 2)) * spacing + 20.0f,
				-10.0f
			));

			sphere.setTransform(transform);

			SimpleMaterialInfo material;

			material.diffuseColor = glm::vec3(0.5f, 0.0f, 0.0f);
			material.metallic = metallic;
			material.roughness = roughness;

			sphere.materials[0] = material;

			models.emplace_back(sphere);
		}
	}

	auto translation = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 6.0f, -4.5f));

	transform = glm::scale(translation, glm::vec3(5.0f));

	sponza.setTransform(transform);

	models.emplace_back(sponza);

	translation = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 10.0f, 0.0f));

	transform = glm::scale(translation, glm::vec3(0.05f));

	disc.setTransform(transform);

	models.emplace_back(disc);

	auto rotation = glm::rotate(translation, glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	rotation = glm::rotate(rotation, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));

	transform = glm::scale(rotation, glm::vec3(WindowWidth * 1.0f / WindowHeight * 5.0f, 1.0f, 5.0f));

	plane.setTransform(transform);

	//models.emplace_back(plane);

	translation = glm::translate(glm::mat4(1.0f), glm::vec3(-20.0f, 10.0f, -5.0f));

	rotation = glm::rotate(translation, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));

	transform = glm::scale(rotation, glm::vec3(0.5f));

	ship.setTransform(transform);

	models.emplace_back(ship);

	mergedModel = mergeModels(models);

	generateTangents(mergedModel);

	//createMeshGeometries(model);
}

void VulkanApplication::createTextureImageViews()
{
	for (const auto& path : textureImagePaths)
	{
		Image image;
		if (useVma)
		{
			image = createTextureImageVma(path);
		}
		else
		{
			image = createTextureImage(path);
		}
		image.view = createTextureImageView(image.handle);

		textureImages.push_back(image);
	}
}

void VulkanApplication::updateFPSCounter()
{
	static double previousSeconds = glfwGetTime();
	double currentSeconds = glfwGetTime();
	double elapsedSeconds = currentSeconds - previousSeconds;

	if (elapsedSeconds >= 0.25)
	{
		previousSeconds = currentSeconds;
		double fps = (double)frameCount / elapsedSeconds;
		auto temp = fmt::format("Vulkan [FPS: {0}] Triangles:{1} Camera:[x={2}, y={3}, z={4}, Yaw={5}, Pitch={6}]", static_cast<float>(fps), 
			mergedModel.indexCount / 3, camera.Position.x, camera.Position.y, camera.Position.z, camera.Yaw, camera.Pitch);

		glfwSetWindowTitle(window, temp.c_str());
		deltaTime = static_cast<float>(elapsedSeconds / frameCount);
		frameCount = 0;
	}

	frameCount++;
}

void VulkanApplication::updateGlobalUniformBuffer(uint32_t frameIndex)
{
	static auto startTime = std::chrono::high_resolution_clock::now();

	auto currentTime = std::chrono::high_resolution_clock::now();

	auto time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

	GlobalUniformBufferObject ubo{};
	ubo.view = camera.GetViewMatrix();
	ubo.projection = glm::perspective(glm::radians(45.0f), swapChainExtent.width / static_cast<float>(swapChainExtent.height), 0.1f, 100.0f);
	ubo.projection[1][1] *= -1.0f;
	ubo.cameraPosition = glm::vec4(camera.Position, 1.0f);

	Utils::memcpy(globalUniformBuffers[frameIndex].mappedData, sizeof(GlobalUniformBufferObject), &ubo, sizeof(GlobalUniformBufferObject));
}

void VulkanApplication::updateObjectUniformBuffer(uint32_t frameIndex, uint32_t index, const ObjectUniformBufferObject& objectUniformBufferObject)
{
	auto offset = static_cast<uint8_t*>(objectUniformBuffers[frameIndex].mappedData);
	offset += index * objectUniformBufferAlignment;
	Utils::memcpy(offset, sizeof(ObjectUniformBufferObject), &objectUniformBufferObject, sizeof(ObjectUniformBufferObject));
}

void VulkanApplication::updateMaterialUniformBuffer(uint32_t frameIndex, uint32_t index, const MaterialUniformBufferObject& materialUniformBufferObject)
{
	auto offset = static_cast<uint8_t*>(materialUniformBuffers[frameIndex].mappedData);
	offset += index * materialUniformBufferAlignment;
	Utils::memcpy(offset, sizeof(MaterialUniformBufferObject), &materialUniformBufferObject, sizeof(MaterialUniformBufferObject));
}

void VulkanApplication::updateLightUniformBuffer(uint32_t frameIndex)
{
	LightUniformBufferObject ubo{};

	ubo.turnOnLightCount = turnOnLightCount;

	for (uint32_t i = 0; i < LightCount; i++)
	{
		ubo.lightPositions[i] = lightPositions[i];
		ubo.lightColors[i] = lightColors[i];
	}

	Utils::memcpy(lightUniformBuffers[frameIndex].mappedData, sizeof(LightUniformBufferObject), &ubo, sizeof(LightUniformBufferObject));
}

void VulkanApplication::updateParticleUniformBuffer(uint32_t frameIndex)
{
	ParticleUniformBufferObject ubo;
	ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 10.0f, 0.0f));
	ubo.model = glm::rotate(ubo.model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	ubo.view = camera.GetViewMatrix();
	ubo.projection = glm::perspective(glm::radians(45.0f), swapChainExtent.width / static_cast<float>(swapChainExtent.height), 0.1f, 100.0f);
	ubo.projection[1][1] *= -1.0f;
	ubo.deltaTime = deltaTime * particleSpeed;

	Utils::memcpy(particleUniformBuffers[frameIndex].mappedData, sizeof(ParticleUniformBufferObject), &ubo, sizeof(ParticleUniformBufferObject));
}

void VulkanApplication::updateImageView(VkImageView imageView, VkImageView alphaImageView, uint32_t frameIndex)
{
	VkDescriptorBufferInfo globalBufferInfo{};
	globalBufferInfo.buffer = globalUniformBuffers[frameIndex].handle;
	globalBufferInfo.offset = 0;
	globalBufferInfo.range = sizeof(GlobalUniformBufferObject);

	VkDescriptorBufferInfo objectBufferInfo{};
	objectBufferInfo.buffer = objectUniformBuffers[frameIndex].handle;
	objectBufferInfo.offset = 0;
	objectBufferInfo.range = sizeof(ObjectUniformBufferObject);

	VkDescriptorBufferInfo materialBufferInfo{};
	materialBufferInfo.buffer = materialUniformBuffers[frameIndex].handle;
	materialBufferInfo.offset = 0;
	materialBufferInfo.range = sizeof(MaterialUniformBufferObject);

	VkDescriptorBufferInfo lightBufferInfo{};
	lightBufferInfo.buffer = lightUniformBuffers[frameIndex].handle;
	lightBufferInfo.offset = 0;
	lightBufferInfo.range = sizeof(LightUniformBufferObject);

	VkDescriptorImageInfo imageInfo{};

	imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageInfo.imageView = imageView;
	imageInfo.sampler = textureSampler;

	std::array<VkWriteDescriptorSet, 6> writeDescriptorSets{};

	writeDescriptorSets[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeDescriptorSets[0].dstSet = graphicsDescriptorSets[frameIndex];
	writeDescriptorSets[0].dstBinding = 0;
	writeDescriptorSets[0].dstArrayElement = 0;
	writeDescriptorSets[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	writeDescriptorSets[0].descriptorCount = 1;
	writeDescriptorSets[0].pBufferInfo = &globalBufferInfo;
	writeDescriptorSets[0].pImageInfo = nullptr;
	writeDescriptorSets[0].pTexelBufferView = nullptr;

	writeDescriptorSets[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeDescriptorSets[1].dstSet = graphicsDescriptorSets[frameIndex];
	writeDescriptorSets[1].dstBinding = 1;
	writeDescriptorSets[1].dstArrayElement = 0;
	writeDescriptorSets[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
	writeDescriptorSets[1].descriptorCount = 1;
	writeDescriptorSets[1].pBufferInfo = &objectBufferInfo;
	writeDescriptorSets[1].pImageInfo = nullptr;
	writeDescriptorSets[1].pTexelBufferView = nullptr;

	writeDescriptorSets[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeDescriptorSets[2].dstSet = graphicsDescriptorSets[frameIndex];
	writeDescriptorSets[2].dstBinding = 2;
	writeDescriptorSets[2].dstArrayElement = 0;
	writeDescriptorSets[2].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
	writeDescriptorSets[2].descriptorCount = 1;
	writeDescriptorSets[2].pBufferInfo = &materialBufferInfo;
	writeDescriptorSets[2].pImageInfo = nullptr;
	writeDescriptorSets[2].pTexelBufferView = nullptr;

	writeDescriptorSets[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeDescriptorSets[3].dstSet = graphicsDescriptorSets[frameIndex];
	writeDescriptorSets[3].dstBinding = 3;
	writeDescriptorSets[3].dstArrayElement = 0;
	writeDescriptorSets[3].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	writeDescriptorSets[3].descriptorCount = 1;
	writeDescriptorSets[3].pBufferInfo = &lightBufferInfo;
	writeDescriptorSets[3].pImageInfo = nullptr;
	writeDescriptorSets[3].pTexelBufferView = nullptr;

	writeDescriptorSets[4].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeDescriptorSets[4].dstSet = graphicsDescriptorSets[frameIndex];
	writeDescriptorSets[4].dstBinding = 4;
	writeDescriptorSets[4].dstArrayElement = 0;
	writeDescriptorSets[4].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	writeDescriptorSets[4].descriptorCount = 1;
	writeDescriptorSets[4].pBufferInfo = nullptr;
	writeDescriptorSets[4].pImageInfo = &imageInfo;
	writeDescriptorSets[4].pTexelBufferView = nullptr;

	VkDescriptorImageInfo alphaImageInfo{};
	alphaImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	alphaImageInfo.sampler = textureSampler;
	
	if (alphaImageView != VK_NULL_HANDLE)
	{
		alphaImageInfo.imageView = alphaImageView;
	}
	else
	{
		alphaImageInfo.imageView = textureImage.view;
	}

	writeDescriptorSets[5].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeDescriptorSets[5].dstSet = graphicsDescriptorSets[frameIndex];
	writeDescriptorSets[5].dstBinding = 5;
	writeDescriptorSets[5].dstArrayElement = 0;
	writeDescriptorSets[5].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	writeDescriptorSets[5].descriptorCount = 1;
	writeDescriptorSets[5].pBufferInfo = nullptr;
	writeDescriptorSets[5].pImageInfo = &alphaImageInfo;
	writeDescriptorSets[5].pTexelBufferView = nullptr;

	vkUpdateDescriptorSets(device, static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, nullptr);
}

void VulkanApplication::initOptick()
{
	OPTICK_THREAD("MainThread");
	OPTICK_START_CAPTURE();
}

void VulkanApplication::initWindow()
{
	glfwSetErrorCallback(
		[](int error, const char* description) 
		{
			fprintf(stderr, "Error: %s\n", description);
		});
	
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

	window = glfwCreateWindow(WindowWidth, WindowHeight, "Vulkan window", nullptr, nullptr);

	glfwSetWindowUserPointer(window, this);
	glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
	glfwSetMouseButtonCallback(window, mouseButtonCallback);
	glfwSetCursorPosCallback(window, mouseMoveCallback);
	glfwSetScrollCallback(window, mouseScrollCallback);

	glfwSetKeyCallback(window, keyCallback);
}

void VulkanApplication::initVulkan()
{
	volkInitialize();
	createInstance();
	setupDebugMessenger();
	createSurface();
	pickPhysicalDevice();
	createLogicalDevice();
	if (useVma)
	{
		createVmaAllocator();
		Buffer::allocator = vmaAllocator;
		Image::allocator = vmaAllocator;
	}
	createSwapChain();
	createImageViews();
	createRenderPass();
	
	{
		AriaCore::ScopedTimer timer("loadResources()");
		loadResources();
	}

	createGraphicsDescriptorSetLayout();
	createOffscreenDescriptorSetLayout();
	createComputeDescriptorSetLayout();
	createGraphicsCommandPool();
	createTransferCommandPool();
	prepareOffscreen();
	createGraphicsPipeline();
	createComputePipeline();
	createColorResources(colorImage, msaaSamples);
	createDepthResources(depthImage, msaaSamples);
	createFramebuffers();

	if (useVma)
	{
		textureImage = createTextureImageVma(ResourceBase + "Textures/Kanna.jpg", Channel::RGBAlpha);
	}
	else
	{
		textureImage = createTextureImage(ResourceBase + "Textures/Kanna.jpg", Channel::RGBAlpha);
	}

	textureImage.view = createTextureImageView(textureImage.handle);

	createTextureSampler();

	createTextureImageViews();

	createMeshGeometries(mergedModel);
	//createMeshGeometries(sphere);

	if (useVma)
	{
		vertexBuffer = createVertexBufferVma(mergedModel.vertices);
		indexBuffer = createIndexBufferVma(mergedModel.indices);

		quadVertexBuffer = createVertexBufferVma(quadVertices);
		quadIndexBuffer = createIndexBufferVma(quadIndices);
	}
	else
	{
		vertexBuffer = createVertexBuffer(mergedModel.vertices);
		indexBuffer = createIndexBuffer(mergedModel.indices);

		quadVertexBuffer = createVertexBuffer(quadVertices);
		quadIndexBuffer = createIndexBuffer(quadIndices);
	}

	debugUtil.setObjectName(vertexBuffer.handle, "vertexBuffer");
	debugUtil.setObjectName(indexBuffer.handle, "indexBuffer");

	if (useVma)
	{
		createGlobalUniformBuffersVma();
		createObjectUniformBuffersVma();
		createMaterialUniformBuffersVma();
		createLightUniformBuffersVma();
		createShaderStorageBuffersVma();
		createParticleUniformBuffersVma();
	}
	else
	{
		createGlobalUniformBuffers();
		createObjectUniformBuffers();
		createMaterialUniformBuffers();
		createLightUniformBuffers();
		createShaderStorageBuffers();
		createParticleUniformBuffers();
	}

	createDescriptorPool();
	createGraphicsDescriptorSets();
	createOffscreenDescriptorSets();
	createComputeDescriptorSets();
	createGraphicsCommandBuffers();
	createComputeCommandBuffers();
	createSyncObjects();
}

void VulkanApplication::initImGui()
{
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsLight();

	// Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForVulkan(window, true);
	ImGui_ImplVulkan_InitInfo initInfo = {};
	initInfo.Instance = instance;
	initInfo.PhysicalDevice = physicalDevice;
	initInfo.Device = device;

	QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);

	initInfo.QueueFamily = queueFamilyIndices.graphicsAndComputeFamily.value();
	initInfo.Queue = graphicsQueue;
	initInfo.PipelineCache = VK_NULL_HANDLE;
	initInfo.DescriptorPool = descriptorPool;
	initInfo.Subpass = 0;
	initInfo.MinImageCount = minImageCount;
	initInfo.ImageCount = imageCount;
	initInfo.MSAASamples = msaaSamples;
	initInfo.Allocator = allocator;
	initInfo.CheckVkResultFn = checkVkResult;
	ImGui_ImplVulkan_Init(&initInfo, renderPass);

	// Load Fonts
	// - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
	// - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
	// - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
	// - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
	// - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use Freetype for higher quality font rendering.
	// - Read 'docs/FONTS.md' for more instructions and details.
	// - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
	//io.Fonts->AddFontDefault();
	//io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf", 18.0f);
	//io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
	//io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
	//io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
	//io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
	io.Fonts->AddFontFromFileTTF((ResourceBase + "Fonts/ProggyClean.ttf").c_str(), 20.0f);
	//ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
	//IM_ASSERT(font != NULL);

	// Upload Fonts
	{
		// Use any command queue
		VkCheck(vkResetCommandPool(device, graphicsCommandPool, 0), "vkResetCommandPool failed!");

		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		VkCheck(vkBeginCommandBuffer(graphicsCommandBuffers[currentFrame], &beginInfo), "vkBeginCommandBuffer failed!");

		ImGui_ImplVulkan_CreateFontsTexture(graphicsCommandBuffers[currentFrame]);

		VkSubmitInfo endInfo = {};
		endInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		endInfo.commandBufferCount = 1;
		endInfo.pCommandBuffers = &graphicsCommandBuffers[currentFrame];
		VkCheck(vkEndCommandBuffer(graphicsCommandBuffers[currentFrame]), "vkEndCommandBuffer failed!");

		VkCheck(vkQueueSubmit(graphicsQueue, 1, &endInfo, VK_NULL_HANDLE), "vkQueueSubmit failed!");

		VkCheck(vkDeviceWaitIdle(device), "vkDeviceWaitIdle failed!");

		ImGui_ImplVulkan_DestroyFontUploadObjects();
	}
}

void VulkanApplication::updateImGui()
{
	// Start the Dear ImGui frame
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
}

void VulkanApplication::createImGuiWidgets()
{
	// 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
	if (showDemoWindow)
		ImGui::ShowDemoWindow(&showDemoWindow);

	// 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
	{
		static float f = 0.0f;
		static int counter = 0;

		ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

		ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
		ImGui::Checkbox("Demo Window", &showDemoWindow);      // Edit bools storing our window open/close state
		ImGui::Checkbox("Another Window", &showAnotherWindow);

		ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
		ImGui::ColorEdit3("clear color", (float*)&clearColor); // Edit 3 floats representing a color

		if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
			counter++;
		ImGui::SameLine();
		ImGui::Text("counter = %d", counter);

		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::End();
	}

	// 3. Show another simple window.
	if (showAnotherWindow)
	{
		ImGui::Begin("Another Window", &showAnotherWindow);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
		ImGui::Text("Hello from another window!");
		if (ImGui::Button("Close Me"))
			showAnotherWindow = false;
		ImGui::End();
	}

}

void VulkanApplication::renderImGui()
{
	// Rendering
	ImGui::Render();
	ImDrawData* drawData = ImGui::GetDrawData();
	const bool isMinimized = (drawData->DisplaySize.x <= 0.0f || drawData->DisplaySize.y <= 0.0f);
	// Record dear imgui primitives into command buffer
	ImGui_ImplVulkan_RenderDrawData(drawData, graphicsCommandBuffers[currentFrame]);
}

void VulkanApplication::shutdownImGui()
{
	ImGui_ImplVulkan_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

bool VulkanApplication::isDeviceSuitable(VkPhysicalDevice inDevice)
{
	VkPhysicalDeviceProperties deviceProperties;
	vkGetPhysicalDeviceProperties(inDevice, &deviceProperties);

	// Calculate required alignment based on minimum device offset alignment
	size_t minUniformBufferOffsetAlignment = deviceProperties.limits.minUniformBufferOffsetAlignment;
	materialUniformBufferAlignment = sizeof(MaterialUniformBufferObject);
	objectUniformBufferAlignment = sizeof(ObjectUniformBufferObject);

	if (minUniformBufferOffsetAlignment > 0)
	{
		materialUniformBufferAlignment = (materialUniformBufferAlignment + minUniformBufferOffsetAlignment - 1) & ~(minUniformBufferOffsetAlignment - 1);
		objectUniformBufferAlignment = (objectUniformBufferAlignment + minUniformBufferOffsetAlignment - 1) & ~(minUniformBufferOffsetAlignment - 1);
	}

	VkPhysicalDeviceFeatures physicalDeviceFeatures;
	vkGetPhysicalDeviceFeatures(inDevice, &physicalDeviceFeatures);

	VkPhysicalDeviceDescriptorIndexingFeaturesEXT indexingFeatures{};

	indexingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES_EXT;
	indexingFeatures.pNext = nullptr;

	VkPhysicalDeviceFeatures2 deviceFeatures{};
	deviceFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
	deviceFeatures.pNext = &indexingFeatures;
	vkGetPhysicalDeviceFeatures2(inDevice, &deviceFeatures);

	QueueFamilyIndices queueFamilyIndices = findQueueFamilies(inDevice);

	bool extensionsSupported = checkDeviceExtensionSupport(inDevice);

	bool swapChainAdequate = false;

	if (extensionsSupported)
	{
		SwapChainSupportDetails swapChainSupport = querySwapChainSupport(inDevice);

		swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
	}

	return deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && 
		   physicalDeviceFeatures.geometryShader &&
		   queueFamilyIndices.isComplete() && 
		   extensionsSupported &&  
		   swapChainAdequate && 
		   physicalDeviceFeatures.samplerAnisotropy && 
		   physicalDeviceFeatures.shaderSampledImageArrayDynamicIndexing &&
		   indexingFeatures.descriptorBindingPartiallyBound && 
		   indexingFeatures.runtimeDescriptorArray &&
		   indexingFeatures.shaderSampledImageArrayNonUniformIndexing &&
		   indexingFeatures.descriptorBindingVariableDescriptorCount;
}

int32_t VulkanApplication::rateDeviceSuitability(VkPhysicalDevice inDevice)
{
	int32_t score = 0;

	// Discrete GPUs have a significant performance advantage
	return score;
}

QueueFamilyIndices VulkanApplication::findQueueFamilies(VkPhysicalDevice inDevice)
{
	QueueFamilyIndices indices{};

	// Assign index to queue families that could be found
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(inDevice, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(inDevice, &queueFamilyCount, queueFamilies.data());

	int32_t index = 0;

	for (const auto& queueFamily : queueFamilies)
	{
		if ((queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) && (queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT))
		{
			indices.graphicsAndComputeFamily = index;
		}

		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(inDevice, index, surface, &presentSupport);

		if (presentSupport)
		{
			indices.presentFamily = index;
		}

		if (queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT)
		{
			indices.transferFamily = index;
		}

		if (indices.isComplete())
		{
			break;
		}

		index++;
	}

	return indices;
}

SwapChainSupportDetails VulkanApplication::querySwapChainSupport(VkPhysicalDevice inDevice)
{
	SwapChainSupportDetails swapchainDetails;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(inDevice, surface, &swapchainDetails.capabilities);

	uint32_t formatCount = 0;

	vkGetPhysicalDeviceSurfaceFormatsKHR(inDevice, surface, &formatCount, nullptr);

	if (formatCount != 0)
	{
		swapchainDetails.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(inDevice, surface, &formatCount, swapchainDetails.formats.data());
	}

	uint32_t presentModeCount = 0;

	vkGetPhysicalDeviceSurfacePresentModesKHR(inDevice, surface, &presentModeCount, nullptr);

	if (presentModeCount != 0)
	{
		swapchainDetails.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(inDevice, surface, &presentModeCount, swapchainDetails.presentModes.data());
	}

	return swapchainDetails;
}

VkSurfaceFormatKHR VulkanApplication::chooseSwapChainSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
	for (const auto& availableFormat : availableFormats)
	{
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		//if (availableFormat.format == VK_FORMAT_R8G8B8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			return availableFormat;
		}
	}

	return availableFormats[0];
}

VkPresentModeKHR VulkanApplication::chooseSwapChainPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
{
	for (const auto& availablePresentMode : availablePresentModes)
	{
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			return availablePresentMode;
		}
	}

	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D VulkanApplication::chooseSwapChainExtent(const VkSurfaceCapabilitiesKHR& capabilities)
{
	// GLFW uses two units when measuring sizes : pixels and screen coordinates.For example, the resolution{ WIDTH, HEIGHT } 
	// that we specified earlier when creating the window is measured in screen coordinates.But Vulkan works with pixels, 
	// so the swap chain extent must be specified in pixels as well.Unfortunately, if you are using a high DPI display(like Apple's Retina display), 
	// screen coordinates don't correspond to pixels.Instead, due to the higher pixel density, the resolution of the window in pixel will be larger 
	// than the resolution in screen coordinates.So if Vulkan doesn't fix the swap extent for us, we can't just use the original{ WIDTH, HEIGHT }. 
	// Instead, we must use glfwGetFramebufferSize to query the resolution of the window in pixel before matching it against the minimumand maximum image extent.
	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
	{
		return capabilities.currentExtent;
	}
	else
	{
		int32_t width = 0;
		int32_t height = 0;
		glfwGetFramebufferSize(window, &width, &height);
		
		VkExtent2D actualExtent =
		{
			static_cast<uint32_t>(width),
			static_cast<uint32_t>(height)
		};

		actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
		return actualExtent;
	}
}

uint32_t VulkanApplication::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags propertyFlags)
{
	VkPhysicalDeviceMemoryProperties memoryProperties;
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);

	for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++)
	{
		if ((typeFilter & (1 << i)) && (memoryProperties.memoryTypes[i].propertyFlags & propertyFlags) == propertyFlags)
		{
			return i;
		}
	}

	THROW_ERROR("Failed to find suitable memory type!");
}

VkFormat VulkanApplication::findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
{
	for (VkFormat format : candidates)
	{
		VkFormatProperties formatProperties;
		vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &formatProperties);

		if (tiling == VK_IMAGE_TILING_LINEAR && (formatProperties.linearTilingFeatures & features) == features)
		{
			return format;
		}
		else if (tiling == VK_IMAGE_TILING_OPTIMAL && (formatProperties.optimalTilingFeatures & features) == features)
		{
			return format;
		}
	}

	THROW_ERROR("Failed to find supported format!");
}

VkFormat VulkanApplication::findDepthFormat()
{
	return findSupportedFormat({ VK_FORMAT_D32_SFLOAT,
										   VK_FORMAT_D32_SFLOAT_S8_UINT,
										   VK_FORMAT_D24_UNORM_S8_UINT },
								           VK_IMAGE_TILING_OPTIMAL, 
							       VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

void VulkanApplication::mainLoop()
{
	static auto startTime = std::chrono::high_resolution_clock::now().time_since_epoch(); 
	static auto simulationTime = std::chrono::duration<float, std::chrono::seconds::period>(startTime).count();

	while (!glfwWindowShouldClose(window))
	{
		auto currentTime = std::chrono::high_resolution_clock::now().time_since_epoch();
		auto realTime = std::chrono::duration<float, std::chrono::seconds::period>(currentTime).count();

		//while (simulationTime < realTime)
		{
			simulationTime += FrameTime;
			processInput(deltaTime);
			update();
		}

		glfwPollEvents();
		updateFPSCounter();
		updateImGui();
		createImGuiWidgets();
		drawFrame();
	}

	OPTICK_STOP_CAPTURE();
	OPTICK_SAVE_CAPTURE("../Profiler/profiler_dump");

	vkDeviceWaitIdle(device);
}

void VulkanApplication::update()
{
	// We want to animate the particle system using the last frames time to get smooth, frame-rate independent animation
	double currentTime = glfwGetTime();
	lastFrameTime = static_cast<float>((currentTime - lastTime) * 1000.0);
	lastTime = currentTime;
}

void VulkanApplication::drawFrame()
{
	// Compute submission
	vkWaitForFences(device, 1, &computeInFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

	updateParticleUniformBuffer(currentFrame);

	vkResetFences(device, 1, &computeInFlightFences[currentFrame]);																																			

	vkResetCommandBuffer(computeCommandBuffers[currentFrame], /*VkCommandBufferResetFlagBits*/ 0);
	recordComputeCommandBuffer(computeCommandBuffers[currentFrame]);

	VkSubmitInfo computeSubmitInfo{};
	computeSubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	computeSubmitInfo.commandBufferCount = 1;
	computeSubmitInfo.pCommandBuffers = &computeCommandBuffers[currentFrame];
	computeSubmitInfo.signalSemaphoreCount = 1;
	computeSubmitInfo.pSignalSemaphores = &computeFinishedSemaphores[currentFrame];

	VkCheck(vkQueueSubmit(computeQueue, 1, &computeSubmitInfo, computeInFlightFences[currentFrame]), "Failed to submit compute command buffer!b");

	// The vkWaitForFences function takes an array of fences and waits on the host for either any or all of the fences 
	// to be signaled before returning. The VK_TRUE we pass here indicates that we want to wait for all fences, but in 
	// the case of a single one it doesn't matter. This function also has a timeout parameter that we set to the maximum
	// value of a 64 bit unsigned integer, UINT64_MAX, which effectively disables the timeout.
	vkWaitForFences(device, 1, &graphicsInFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

	uint32_t imageIndex = 0;

	auto result = vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

	if (result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		recreateSwapChain();
		return;
	}
	else
	{
		VkCheck(result, "vkAcquireNextImageKHR failed!");
	}

	// Only reset the fence if we are submitting work
	vkResetFences(device, 1, &graphicsInFlightFences[currentFrame]);

	vkResetCommandBuffer(graphicsCommandBuffers[currentFrame], 0);

	recordGraphicsCommandBuffer(graphicsCommandBuffers[currentFrame], imageIndex);

	updateGlobalUniformBuffer(currentFrame);
	updateLightUniformBuffer(currentFrame);
	
	VkSemaphore waitSemaphores[] = { computeFinishedSemaphores[currentFrame], imageAvailableSemaphores[currentFrame]};
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

	// The first three parameters specify which semaphores to wait on before execution begins and in which stage(s)
	// of the pipeline to wait. We want to wait with writing colors to the image until it's available, so we're specifying 
	// the stage of the graphics pipeline that writes to the color attachment. That means that theoretically the implementation 
	// can already start executing our vertex shader and such while the image is not yet available. Each entry in the waitStages 
	// array corresponds to the semaphore with the same index in pWaitSemaphores.
	VkSubmitInfo graphicsSubmitInfo{};
	graphicsSubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	graphicsSubmitInfo.waitSemaphoreCount = 2;
	graphicsSubmitInfo.pWaitSemaphores = waitSemaphores;
	graphicsSubmitInfo.pWaitDstStageMask = waitStages;
	graphicsSubmitInfo.commandBufferCount = 1;
	graphicsSubmitInfo.pCommandBuffers = &graphicsCommandBuffers[currentFrame];

	// The signalSemaphoreCount and pSignalSemaphores parameters specify which semaphores to signal once the command 
	// buffer(s) have finished execution. In our case we're using the renderFinishedSemaphore for that purpose.
	VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame]};
	graphicsSubmitInfo.signalSemaphoreCount = 1;
	graphicsSubmitInfo.pSignalSemaphores = signalSemaphores;

	VkCheck(vkQueueSubmit(graphicsQueue, 1, &graphicsSubmitInfo, graphicsInFlightFences[currentFrame]), "Failed to submit draw command buffer!b");

	// The first two parameters specify which semaphores to wait on before presentation can happen, just like VkSubmitInfo. 
	// Since we want to wait on the command buffer to finish execution, thus our triangle being drawn, we take the semaphores 
	// which will be signaled and wait on them, thus we use signalSemaphores.
	VkPresentInfoKHR presentInfo{};
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
		recreateSwapChain();
	}
	else
	{
		VkCheck(result, "vkQueuePresentKHR failed!");
	}

	currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void VulkanApplication::cleanup()
{
	shutdownImGui();

	cleanupSwapChain();

	vkDestroySampler(device, textureSampler, allocator);

	destroyImage(textureImage);
	vkDestroyImageView(device, textureImage.view, allocator);

	vkDestroySampler(device, offscreenPass.sampler, allocator);
	vkDestroySampler(device, bloomFilterPass.sampler, allocator);

	for (int i = 0; i < 2; i++)
	{
		destroyImage(offscreenPass.color[i]);
		vkDestroyImageView(device, offscreenPass.color[i].view, allocator);
	}

	destroyImage(offscreenPass.depth);
	vkDestroyImageView(device, offscreenPass.depth.view, allocator);

	vkDestroyPipeline(device, pipelines.offscreen, allocator);
	
	vkDestroyFramebuffer(device, offscreenPass.frameBuffer, allocator);

	vkDestroyRenderPass(device, offscreenPass.renderPass, allocator);

	destroyImage(bloomFilterPass.color[0]);
	vkDestroyImageView(device, bloomFilterPass.color[0].view, allocator);

	destroyImage(bloomFilterPass.depth);
	vkDestroyImageView(device, bloomFilterPass.depth.view, allocator);

	vkDestroyPipeline(device, pipelines.bloom[0], allocator);
	vkDestroyPipeline(device, pipelines.bloom[1], allocator);
	vkDestroyPipeline(device, pipelines.screenQuad, allocator);
	vkDestroyPipeline(device, pipelines.screenQuadNoToneMapping, allocator);

	vkDestroyFramebuffer(device, bloomFilterPass.frameBuffer, allocator);

	vkDestroyRenderPass(device, bloomFilterPass.renderPass, allocator);

	destroyBuffer(vertexBuffer);

	destroyBuffer(indexBuffer);

	destroyBuffer(quadVertexBuffer);

	destroyBuffer(quadIndexBuffer);

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		destroyBuffer(shaderStorageBuffers[i]);
	}

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		destroyBuffer(globalUniformBuffers[i], true);
	}

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		destroyBuffer(objectUniformBuffers[i], true);
	}

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		destroyBuffer(materialUniformBuffers[i], true);
	}

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		destroyBuffer(particleUniformBuffers[i], true);
	}

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		destroyBuffer(lightUniformBuffers[i], true);
	}

	for (size_t i = 0; i < textureImages.size(); i++)
	{
		destroyImage(textureImages[i]);
		vkDestroyImageView(device, textureImages[i].view, allocator);
	}

	for (size_t i = 0; i < meshGeometries.size(); i++)
	{
		const auto& meshGeometry = meshGeometries[i];

		destroyBuffer(meshGeometry->vertexBuffer);
		destroyBuffer(meshGeometry->indexBuffer);
	}

	vkDestroyDescriptorPool(device, descriptorPool, nullptr);

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		vkDestroySemaphore(device, imageAvailableSemaphores[i], allocator);
		vkDestroySemaphore(device, renderFinishedSemaphores[i], allocator);
		vkDestroySemaphore(device, computeFinishedSemaphores[i], allocator);
		vkDestroyFence(device, graphicsInFlightFences[i], allocator);
		vkDestroyFence(device, computeInFlightFences[i], allocator);
	}

	vkDestroyCommandPool(device, transferCommandPool, allocator);

	vkDestroyCommandPool(device, graphicsCommandPool, allocator);

	vkDestroyDescriptorSetLayout(device, computeDescriptorSetLayout, allocator);
	vkDestroyDescriptorSetLayout(device, graphicsDescriptorSetLayout, allocator);

	vkDestroyDescriptorSetLayout(device, descriptorSetLayouts.offscreen, allocator);
	vkDestroyDescriptorSetLayout(device, descriptorSetLayouts.bloom, allocator);

	vkDestroyPipeline(device, computePipeline, allocator);
	vkDestroyPipeline(device, particlePipeline, allocator);
	vkDestroyPipeline(device, graphicsPipeline, allocator);

	vkDestroyPipelineLayout(device, computePipelineLayout, allocator);
	vkDestroyPipelineLayout(device, particlePipelineLayout, allocator);
	vkDestroyPipelineLayout(device, graphicsPipelineLayout, allocator);
	vkDestroyPipelineLayout(device, pipelineLayouts.offscreen, allocator);
	vkDestroyPipelineLayout(device, pipelineLayouts.bloom, allocator);
	vkDestroyPipelineLayout(device, pipelineLayouts.screenQuad, allocator);

	vkDestroyRenderPass(device, renderPass, allocator);

	if (useVma)
	{
		vmaDestroyAllocator(vmaAllocator);
	}

	vkDestroyDevice(device, allocator);

	if (EnableValidationLayers)
	{
		destroyDebugUtilsMessengerEXT(instance, debugMessenger, allocator);
	}

	vkDestroySurfaceKHR(instance, surface, allocator);
	vkDestroyInstance(instance, allocator);

	glfwDestroyWindow(window);

	glfwTerminate();
}

void VulkanApplication::processInput(float deltaTime)
{
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
	{
		camera.SetMaxSpeed();
	}
	else if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
	{
		camera.SetMinSpeed();
	}
	else
	{
		camera.SetNormalSpeed();
	}

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
	{
		camera.ProcessKeyboard(FORWARD, deltaTime);
	}

	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
	{
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	}

	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
	{
		camera.ProcessKeyboard(LEFT, deltaTime);
	}

	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
	{
		camera.ProcessKeyboard(RIGHT, deltaTime);
	}

	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
	{
		camera.ProcessKeyboard(UP, deltaTime);
	}

	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
	{
		camera.ProcessKeyboard(DOWN, deltaTime);
	}
}

bool VulkanApplication::checkExtensionSupport()
{
	uint32_t extensionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

	fmt::print("{} extensions supported\n", extensionCount);

	std::vector<VkExtensionProperties> extensions(extensionCount);

	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

	std::cout << "Available extensions:\n";

	std::vector<std::string> extensionNames;

	for (const auto& extension : extensions)
	{
		std::cout << '\t' << extension.extensionName << '\n';
		extensionNames.emplace_back(extension.extensionName);
	}

	std::cout << std::endl;

	bool supported = false;

	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions = nullptr;

	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	for (uint32_t i = 0; i < glfwExtensionCount; i++)
	{
		auto begin = extensionNames.begin();
		auto end = extensionNames.end();

		if (std::find(begin, end, glfwExtensions[i]) != extensionNames.end())
		{
			supported = true;
		}
		else
		{
			supported = false;
		}
	}

	if (supported)
	{
		std::cout << "All GLFW required instance extensions are supported." << std::endl;
	}

	return true;
}

bool VulkanApplication::checkDeviceExtensionSupport(VkPhysicalDevice inDevice)
{
	uint32_t extensionCount = 0;
	vkEnumerateDeviceExtensionProperties(inDevice, nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);

	vkEnumerateDeviceExtensionProperties(inDevice, nullptr, &extensionCount, availableExtensions.data());

	std::set<std::string> requiredExtensions(DeviceExtensions.begin(), DeviceExtensions.end());

	for (const auto& extension : availableExtensions)
	{
		requiredExtensions.erase(extension.extensionName);
	}

	return requiredExtensions.empty();
}

bool VulkanApplication::checkValidationLayerSupport()
{
	uint32_t layerCount = 0;

	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (const char* layerName : ValidationLayers)
	{
		bool layerFound = false;
		
		for (const auto& layerProperties : availableLayers)
		{
			if (strcmp(layerName, layerProperties.layerName) == 0)
			{
				layerFound = true;
				break;
			}
		}

		if (!layerFound)
		{
			return false;
		}
	}

	return true;
}

std::vector<const char*> VulkanApplication::getRequiredExtensions()
{
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions = nullptr;

	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

	if (EnableValidationLayers)
	{
		extensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

	// 创建VkInstance的时候要加入VK_KHR_get_physical_device_properties2扩展，这个扩展说明文档见：
	// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VK_KHR_get_physical_device_properties2.html
	// 它引入了一些新函数：
	//vkGetPhysicalDeviceFeatures2KHR
	//vkGetPhysicalDeviceFormatProperties2KHR
	//vkGetPhysicalDeviceImageFormatProperties2KHR
	//vkGetPhysicalDeviceMemoryProperties2KHR
	//vkGetPhysicalDeviceProperties2KHR
	//vkGetPhysicalDeviceQueueFamilyProperties2KHR
	//vkGetPhysicalDeviceSparseImageFormatProperties2KHR
	// 这些函数对应volk.c中276~284行：
	//#if defined(VK_KHR_get_physical_device_properties2)
	//	vkGetPhysicalDeviceFeatures2KHR = (PFN_vkGetPhysicalDeviceFeatures2KHR)load(context, "vkGetPhysicalDeviceFeatures2KHR");
	//	vkGetPhysicalDeviceFormatProperties2KHR = (PFN_vkGetPhysicalDeviceFormatProperties2KHR)load(context, "vkGetPhysicalDeviceFormatProperties2KHR");
	//	vkGetPhysicalDeviceImageFormatProperties2KHR = (PFN_vkGetPhysicalDeviceImageFormatProperties2KHR)load(context, "vkGetPhysicalDeviceImageFormatProperties2KHR");
	//	vkGetPhysicalDeviceMemoryProperties2KHR = (PFN_vkGetPhysicalDeviceMemoryProperties2KHR)load(context, "vkGetPhysicalDeviceMemoryProperties2KHR");
	//	vkGetPhysicalDeviceProperties2KHR = (PFN_vkGetPhysicalDeviceProperties2KHR)load(context, "vkGetPhysicalDeviceProperties2KHR");
	//	vkGetPhysicalDeviceQueueFamilyProperties2KHR = (PFN_vkGetPhysicalDeviceQueueFamilyProperties2KHR)load(context, "vkGetPhysicalDeviceQueueFamilyProperties2KHR");
	//	vkGetPhysicalDeviceSparseImageFormatProperties2KHR = (PFN_vkGetPhysicalDeviceSparseImageFormatProperties2KHR)load(context, "vkGetPhysicalDeviceSparseImageFormatProperties2KHR");
	//#endif /* defined(VK_KHR_get_physical_device_properties2) */
	// 需要这个扩展的原因是初始化VMA(Vulkan Memory Allocator)时需要给VmaVulkanFunctions指定相关的
	// 几个函数，如果没有这个扩展，则这几个函数的指针为nullptr，调用vmaCreateAllocator就会崩溃
	extensions.emplace_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);

	return extensions;
}

VkSampleCountFlagBits VulkanApplication::getMaxUsableSampleCount()
{
	VkPhysicalDeviceProperties physicalDeviceProperties;
	vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);

	VkSampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts & physicalDeviceProperties.limits.framebufferDepthSampleCounts;

	if (counts & VK_SAMPLE_COUNT_64_BIT) { return VK_SAMPLE_COUNT_64_BIT; }
	if (counts & VK_SAMPLE_COUNT_32_BIT) { return VK_SAMPLE_COUNT_32_BIT; }
	if (counts & VK_SAMPLE_COUNT_16_BIT) { return VK_SAMPLE_COUNT_16_BIT; }
	if (counts & VK_SAMPLE_COUNT_8_BIT) { return VK_SAMPLE_COUNT_8_BIT; }
	if (counts & VK_SAMPLE_COUNT_4_BIT) { return VK_SAMPLE_COUNT_4_BIT; }
	if (counts & VK_SAMPLE_COUNT_2_BIT) { return VK_SAMPLE_COUNT_2_BIT; }

	return VK_SAMPLE_COUNT_1_BIT;
}

void VulkanApplication::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
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
	createInfo.pUserData = NULL; // Optional
}

VKAPI_ATTR VkBool32 VKAPI_CALL VulkanApplication::debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* userData)
{
	fmt::print("Validation layer: {0}\n", pCallbackData->pMessage);
	return VK_FALSE;
}

void VulkanApplication::framebufferResizeCallback(GLFWwindow* inWindow, int width, int height)
{
	auto app = reinterpret_cast<VulkanApplication*>(glfwGetWindowUserPointer(inWindow));

	app->framebufferResized = true;
}

void VulkanApplication::mouseMoveCallback(GLFWwindow* inWindow, double xpos, double ypos)
{
	auto deltaX = static_cast<float>(xpos - lastMousePosition.x);
	auto deltaY = static_cast<float>(lastMousePosition.y - ypos);

	if (rightMouseButtonDown)
	{
		auto app = reinterpret_cast<VulkanApplication*>(glfwGetWindowUserPointer(inWindow));

		app->camera.ProcessMouseMovement(deltaX, deltaY);
	}

	if (middleMouseButtonDown)
	{
		auto app = reinterpret_cast<VulkanApplication*>(glfwGetWindowUserPointer(inWindow));
		app->camera.ProcessKeyboard(LEFT, FrameTime * deltaX);
		app->camera.ProcessKeyboard(DOWN, FrameTime * deltaY);
	}

	lastMousePosition.x = static_cast<float>(xpos);
	lastMousePosition.y = static_cast<float>(ypos);
}

void VulkanApplication::mouseScrollCallback(GLFWwindow* inWindow, double xoffset, double yoffset)
{
	auto app = reinterpret_cast<VulkanApplication*>(glfwGetWindowUserPointer(inWindow));

	app->camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

void VulkanApplication::mouseButtonCallback(GLFWwindow* inWindow, int32_t button, int32_t action, int32_t mods)
{
	if (button == GLFW_MOUSE_BUTTON_2 && action == GLFW_PRESS)
	{
		rightMouseButtonDown = true;
	}

	if (button == GLFW_MOUSE_BUTTON_2 && action == GLFW_RELEASE)
	{
		rightMouseButtonDown = false;
	}

	if (button == GLFW_MOUSE_BUTTON_3 && action == GLFW_PRESS)
	{
		middleMouseButtonDown = true;
	}

	if (button == GLFW_MOUSE_BUTTON_3 && action == GLFW_RELEASE)
	{
		middleMouseButtonDown = false;
	}
}
