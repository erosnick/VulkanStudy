#include "VulkanApplication.h"

#include <map>
#include <set>
#include <cstdint>
#include <limits>
#include <algorithm>
#include <fstream>
#include <chrono>

#include "glm.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "LoadModelObj.h"
#include "GeometryGenerator.h"

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
	
	auto fileSize = file.tellg();
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
  presentQueue(VK_NULL_HANDLE),
  transferQueue(VK_NULL_HANDLE),
  surface(VK_NULL_HANDLE),
  swapChain(VK_NULL_HANDLE),
  renderPass(VK_NULL_HANDLE),
  descriptorSetLayout(VK_NULL_HANDLE),
  pipelineLayout(VK_NULL_HANDLE),
  graphicsPipeline(VK_NULL_HANDLE),
  graphicsCommandPool(VK_NULL_HANDLE),
  transferCommandPool(VK_NULL_HANDLE),
  descriptorPool(VK_NULL_HANDLE),
  textureImage(VK_NULL_HANDLE),
  textureImageMemory(VK_NULL_HANDLE),
  textureImageView(VK_NULL_HANDLE),
  textureSampler(VK_NULL_HANDLE),
  depthImage(VK_NULL_HANDLE),
  depthImageMemory(VK_NULL_HANDLE),
  depthImageView(VK_NULL_HANDLE),
  descriptorSets{VK_NULL_HANDLE},
  commandBuffers{ VK_NULL_HANDLE },
  imageAvailableSemaphores{ VK_NULL_HANDLE },
  renderFinishedSemaphores{ VK_NULL_HANDLE },
  inFlightFences{ VK_NULL_HANDLE },
  swapChainImageFormat(VK_FORMAT_UNDEFINED),
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
	appInfo.apiVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "Vulkan Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
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

	std::set<uint32_t> uniqueQueueFamilies = { queueFamilyIndices.graphicsFamily.value(), queueFamilyIndices.presentFamily.value(), queueFamilyIndices.transferFamily.value() };

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

	VkCheck(vkCreateDevice(physicalDevice, &deviceCreateInfo, allocator, &device), "Failed to create logical device!");

	vkGetDeviceQueue(device, queueFamilyIndices.graphicsFamily.value(), 0, &graphicsQueue);
	vkGetDeviceQueue(device, queueFamilyIndices.presentFamily.value(), 0, &presentQueue);
	vkGetDeviceQueue(device, queueFamilyIndices.transferFamily.value(), 0, &transferQueue);
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

	uint32_t pQueueFamilyIndices[] = { queueFamilyIndices.graphicsFamily.value(), queueFamilyIndices.presentFamily.value() };

	if (queueFamilyIndices.graphicsFamily != queueFamilyIndices.presentFamily)
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
	createDepthResources();
	createFramebuffers();
}

void VulkanApplication::cleanupSwapChain()
{
	vkDestroyImageView(device, depthImageView, allocator);
	vkDestroyImage(device, depthImage, allocator);
	vkFreeMemory(device, depthImageMemory, allocator);

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
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	// A single render pass can consist of multiple subpasses. Subpasses are subsequent rendering operations 
	// that depend on the contents of framebuffers in previous passes, for example a sequence of post-processing 
	// effects that are applied one after another. If you group these rendering operations into one render pass, 
	// then Vulkan is able to reorder the operations and conserve memory bandwidth for possibly better performance.

	// Every subpass references one or more of the attachments that we've described using the structure in the previous sections. 
	// These references are themselves VkAttachmentReference structs that look like this:
	VkAttachmentReference colorAttachmentReference{};
	colorAttachmentReference.attachment = 0;
	colorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentDescription depthAttachment{};
	depthAttachment.format = findDepthFormat();
	depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
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
}

void VulkanApplication::createDescriptorSetLayout()
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

	VkDescriptorSetLayoutBinding samplerLayoutBingding{};
	samplerLayoutBingding.binding = 3;
	samplerLayoutBingding.descriptorCount = 1;
	samplerLayoutBingding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerLayoutBingding.pImmutableSamplers = nullptr;
	samplerLayoutBingding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	VkDescriptorSetLayoutBinding alphaSamplerLayoutBingding{};
	alphaSamplerLayoutBingding.binding = 4;
	alphaSamplerLayoutBingding.descriptorCount = 1;
	alphaSamplerLayoutBingding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	alphaSamplerLayoutBingding.pImmutableSamplers = nullptr;
	alphaSamplerLayoutBingding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	std::array<VkDescriptorSetLayoutBinding, 5> descriptorSetLayoutBindings{ globalUniformBufferLayoutBinding, objectUniformBufferLayoutBinding, materialUniformBufferLayoutBinding, samplerLayoutBingding, alphaSamplerLayoutBingding };

	VkDescriptorSetLayoutCreateInfo descriptorSetlayoutCreateInfo{};
	descriptorSetlayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptorSetlayoutCreateInfo.bindingCount = static_cast<uint32_t>(descriptorSetLayoutBindings.size());
	descriptorSetlayoutCreateInfo.pBindings = descriptorSetLayoutBindings.data();

	VkCheck(vkCreateDescriptorSetLayout(device, &descriptorSetlayoutCreateInfo, 
										allocator, &descriptorSetLayout), "Failed to create descriptor set layout");
}

void VulkanApplication::createGraphicsPipeline()
{
	auto vertexShaderCode = readFile(ResourceBase + "shaders/shader.vert.spv");
	auto fragmentShaderCode = readFile(ResourceBase + "shaders/shader.frag.spv");

	VkShaderModule vertexShaderModule = createShaderModule(vertexShaderCode);
	VkShaderModule fragmentShaderModule = createShaderModule(fragmentShaderCode);

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

	bool usedynamicStates = true;

	std::vector<VkDynamicState> dynamicStates =
	{
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};

	VkPipelineDynamicStateCreateInfo dynamicStatesCreateInfo{};

	dynamicStatesCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicStatesCreateInfo.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
	dynamicStatesCreateInfo.pDynamicStates = dynamicStates.data();

	auto bindingDescription = Vertex::getBindingDescription();
	auto attributeDescriptions = Vertex::getAttributeDescriptions();

	VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo{};
	vertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputStateCreateInfo.vertexBindingDescriptionCount = 1;
	vertexInputStateCreateInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
	vertexInputStateCreateInfo.pVertexBindingDescriptions = &bindingDescription;
	vertexInputStateCreateInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

	VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo{};
	inputAssemblyStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssemblyStateCreateInfo.primitiveRestartEnable = VK_FALSE;

	VkPipelineViewportStateCreateInfo viewportStateCreateInfo{};
	viewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportStateCreateInfo.viewportCount = 1;
	viewportStateCreateInfo.scissorCount = 1;

	VkPipelineRasterizationStateCreateInfo rasterizationStateCreateInfo{};
	rasterizationStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;

	// If depthClampEnable is set to VK_TRUE, then fragments that are beyond the near and far planes 
	// are clamped to them as opposed to discarding them.This is useful in some special cases like 
	// shadow maps.Using this requires enabling a GPU feature.
	rasterizationStateCreateInfo.depthClampEnable = VK_FALSE;
	rasterizationStateCreateInfo.rasterizerDiscardEnable = VK_FALSE;

	// Using any mode other than fill requires enabling a GPU feature.
	rasterizationStateCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;

	// The lineWidth member is straightforward, it describes the thickness of lines in 
	// terms of number of fragments. The maximum line width that is supported depends on the
	// hardware and any line thicker than 1.0f requires you to enable the wideLines GPU feature.
	rasterizationStateCreateInfo.lineWidth = 1.0f;

	rasterizationStateCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizationStateCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

	// The rasterizer can alter the depth values by adding a constant value or biasing them based on a fragment's slope. 
	// This is sometimes used for shadow mapping, but we won't be using it. Just set depthBiasEnable to VK_FALSE.
	rasterizationStateCreateInfo.depthBiasEnable = VK_FALSE;
	rasterizationStateCreateInfo.depthBiasConstantFactor = 0.0f;
	rasterizationStateCreateInfo.depthBiasClamp = 0.0f;
	rasterizationStateCreateInfo.depthBiasSlopeFactor = 0.0f;

	VkPipelineMultisampleStateCreateInfo multisampleStateCreateInfo{};
	multisampleStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampleStateCreateInfo.sampleShadingEnable = VK_FALSE;
	multisampleStateCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampleStateCreateInfo.minSampleShading = 1.0f;
	multisampleStateCreateInfo.pSampleMask = nullptr;
	multisampleStateCreateInfo.alphaToCoverageEnable = VK_FALSE;
	multisampleStateCreateInfo.alphaToOneEnable = VK_FALSE;

	VkPipelineColorBlendAttachmentState colorBlendAttachment{};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | 
										  VK_COLOR_COMPONENT_G_BIT | 
										  VK_COLOR_COMPONENT_B_BIT |
										  VK_COLOR_COMPONENT_A_BIT;

	bool blendEnable = false;

	if (blendEnable)
	{
		colorBlendAttachment.blendEnable = VK_TRUE;
		colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
		colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
	}
	else
	{
		colorBlendAttachment.blendEnable = VK_FALSE;
		colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
		colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
		colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
		colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
	}

	VkPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo{};
	colorBlendStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlendStateCreateInfo.logicOpEnable = VK_FALSE;
	colorBlendStateCreateInfo.logicOp = VK_LOGIC_OP_COPY;
	colorBlendStateCreateInfo.attachmentCount = 1;
	colorBlendStateCreateInfo.pAttachments = &colorBlendAttachment;
	colorBlendStateCreateInfo.blendConstants[0] = 0.0f;
	colorBlendStateCreateInfo.blendConstants[1] = 0.0f;
	colorBlendStateCreateInfo.blendConstants[2] = 0.0f;
	colorBlendStateCreateInfo.blendConstants[3] = 0.0f;

	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
	pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutCreateInfo.setLayoutCount = 1;
	pipelineLayoutCreateInfo.pSetLayouts = &descriptorSetLayout;
	pipelineLayoutCreateInfo.pushConstantRangeCount = 0;
	pipelineLayoutCreateInfo.pPushConstantRanges = nullptr;

	VkCheck(vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout), "Failed to create pipeline layout!");

	VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo{};
	graphicsPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	graphicsPipelineCreateInfo.stageCount = 2;
	graphicsPipelineCreateInfo.pStages = shaderStages;

	graphicsPipelineCreateInfo.pVertexInputState = &vertexInputStateCreateInfo;
	graphicsPipelineCreateInfo.pInputAssemblyState = &inputAssemblyStateCreateInfo;
	graphicsPipelineCreateInfo.pViewportState = &viewportStateCreateInfo;
	graphicsPipelineCreateInfo.pRasterizationState = &rasterizationStateCreateInfo;
	graphicsPipelineCreateInfo.pMultisampleState = &multisampleStateCreateInfo;

	VkPipelineDepthStencilStateCreateInfo depthStencilStateCreateInfo{};
	depthStencilStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencilStateCreateInfo.depthTestEnable = VK_TRUE;
	depthStencilStateCreateInfo.depthWriteEnable = VK_TRUE;
	depthStencilStateCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS;
	depthStencilStateCreateInfo.depthBoundsTestEnable = VK_FALSE;
	depthStencilStateCreateInfo.minDepthBounds = 0.0f;
	depthStencilStateCreateInfo.maxDepthBounds = 1.0f;
	depthStencilStateCreateInfo.stencilTestEnable = VK_FALSE;
	depthStencilStateCreateInfo.front = {};
	depthStencilStateCreateInfo.back = {};

	graphicsPipelineCreateInfo.pDepthStencilState = &depthStencilStateCreateInfo;
	graphicsPipelineCreateInfo.pColorBlendState = &colorBlendStateCreateInfo;
	graphicsPipelineCreateInfo.pDynamicState = &dynamicStatesCreateInfo;
	graphicsPipelineCreateInfo.layout = pipelineLayout;
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

	vkDestroyShaderModule(device, fragmentShaderModule, nullptr);
	vkDestroyShaderModule(device, vertexShaderModule, nullptr);
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
	commandPoolCreateInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

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
	commandPoolCreateInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

	VkCheck(vkCreateCommandPool(device, &commandPoolCreateInfo, nullptr, &transferCommandPool), "Failed to create command pool!");
}

void VulkanApplication::createDepthResources()
{
	VkFormat depthFormat = findDepthFormat();

	createImage(swapChainExtent.width, swapChainExtent.height, 1, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depthImage, depthImageMemory);

	depthImageView = createImageView(depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, 1);

	transitionImageLayout(depthImage, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 1);
}

void VulkanApplication::createFramebuffers()
{
	swapChainFramebuffers.resize(swapChainImageViews.size());

	for (size_t i = 0; i < swapChainImageViews.size(); i++)
	{
		std::array<VkImageView, 2> attachments =
		{
			swapChainImageViews[i],
			depthImageView
		};

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

VkImage VulkanApplication::createTextureImage(VkDeviceMemory& imageMemory, const std::string& path, Channel requireChannels)
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

	VkImage textureImage;
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;

	createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
							VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

	void* data = nullptr;

	vkMapMemory(device, stagingBufferMemory, 0, imageSize, 0, &data);
	memcpy_s(data, static_cast<size_t>(imageSize), pixels, static_cast<size_t>(imageSize));
	vkUnmapMemory(device, stagingBufferMemory);
	
	STBI_FREE(pixels);

	createImage(width, height, mipLevels, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL,
					           VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
							   VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage, imageMemory);

	transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mipLevels);
	copyBufferToImage(stagingBuffer, textureImage, static_cast<uint32_t>(width), static_cast<uint32_t>(height));
	
	// transitioned to VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL while generating mipmaps
	//transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, mipLevels);

	generateMipmaps(textureImage, VK_FORMAT_R8G8B8A8_SRGB, width, height, mipLevels);

	vkDestroyBuffer(device, stagingBuffer, allocator);
	vkFreeMemory(device, stagingBufferMemory, allocator);

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

VkBuffer VulkanApplication::createVertexBuffer(const std::vector<Vertex>& vertices, VkDeviceMemory& vertexBufferMemory)
{
	VkDeviceSize bufferSize = VectorSize(Vertex, vertices);

	VkBuffer vertexBuffer;
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;

	createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
				 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | 
							 VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
							 stagingBuffer, stagingBufferMemory);

	void* data = nullptr;
	vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);

	memcpy_s(data, bufferSize, vertices.data(), bufferSize);

	vkUnmapMemory(device, stagingBufferMemory);

	createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
				 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
					      vertexBuffer, vertexBufferMemory);

	copyBuffer(stagingBuffer, vertexBuffer, bufferSize);

	vkDestroyBuffer(device, stagingBuffer, allocator);
	vkFreeMemory(device, stagingBufferMemory, allocator);

	return vertexBuffer;
}

VkBuffer VulkanApplication::createIndexBuffer(const std::vector<uint32_t>& indices, VkDeviceMemory& indexBufferMemory)
{
	VkDeviceSize bufferSize = VectorSize(uint32_t, indices);

	VkBuffer indexBuffer;
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;

	createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
						  stagingBuffer, stagingBufferMemory);

	void* data = nullptr;
	vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy_s(data, bufferSize, indices.data(), bufferSize);
	vkUnmapMemory(device, stagingBufferMemory);

	createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
				 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer, indexBufferMemory);

	copyBuffer(stagingBuffer, indexBuffer, bufferSize);
	
	vkDestroyBuffer(device, stagingBuffer, allocator);
	vkFreeMemory(device, stagingBufferMemory, allocator);

	return indexBuffer;
}

std::unique_ptr<MeshGeometry> VulkanApplication::createMeshGeometry(const Mesh& mesh)
{
	auto meshGeometry = std::make_unique<MeshGeometry>();

	meshGeometry->vertexBuffer = createVertexBuffer(mesh.getVertices(), meshGeometry->vertexBufferMemory);
	meshGeometry->indexBuffer = createIndexBuffer(mesh.getIndices(), meshGeometry->indexBufferMemory);
	meshGeometry->vertexCount = static_cast<uint32_t>(mesh.getVertices().size());
	meshGeometry->indexCount = static_cast<uint32_t>(mesh.getIndices().size());
	meshGeometry->material = mesh.getMaterial();

	if (mesh.getMaterial()->hasTexture())
	{
		meshGeometry->textureImage = createTextureImage(meshGeometry->textureImageMemory, ResourceBase + mesh.getMaterial()->diffuseTexturePath);
		meshGeometry->textureImageView = createTextureImageView(meshGeometry->textureImage);
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

	meshGeometry->vertexBuffer = createVertexBuffer(mesh.vertices, meshGeometry->vertexBufferMemory);
	meshGeometry->indexBuffer = createIndexBuffer(mesh.indices, meshGeometry->indexBufferMemory);
	meshGeometry->vertexCount = static_cast<uint32_t>(mesh.vertices.size());
	meshGeometry->indexStartIndex = static_cast<uint32_t>(mesh.indexStartIndex);
	meshGeometry->indexCount = static_cast<uint32_t>(mesh.indices.size());
	meshGeometry->material->diffuseTexturePath = material.diffuseTexturePath;
	meshGeometry->material->alphaTexturePath = material.alphaTexturePath;
	meshGeometry->material->Kd = material.diffuseColor;

	if (textured)
	{
		meshGeometry->textureImage = createTextureImage(meshGeometry->textureImageMemory, material.diffuseTexturePath);
		meshGeometry->textureImageView = createTextureImageView(meshGeometry->textureImage);
		meshGeometry->hasTexture = true;

		if (mesh.alphaTextured)
		{
			meshGeometry->alphaTextureImage = createTextureImage(meshGeometry->alphaTextureImageMemory, material.alphaTexturePath);
			meshGeometry->alphaTextureImageView = createTextureImageView(meshGeometry->alphaTextureImage);
			meshGeometry->hasAlphaTexture = true;
		}
	}

	return meshGeometry;
}

void VulkanApplication::createMeshGeometries(const Model& model)
{
	for (auto i = 0; i < model.meshes.size(); i++)
	{
		auto meshGeometry = createMeshGeometry(model.meshes[i]);

		meshGeometries.emplace_back(std::move(meshGeometry));
	}
}

void VulkanApplication::createMeshGeometries(const SimpleModel& model)
{
	for (auto i = 0; i < model.meshes.size(); i++)
	{
		auto meshGeometry = createMeshGeometry(model.meshes[i], model.materials[model.meshes[i].materialIndex]);

		meshGeometries.emplace_back(std::move(meshGeometry));
	}
}

void VulkanApplication::createGlobalUniformBuffers()
{
	VkDeviceSize bufferSize = sizeof(GlobalUniformBufferObject);

	globalUniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
	globalUniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);
	globalUniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
					 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			globalUniformBuffers[i], globalUniformBuffersMemory[i]);

		vkMapMemory(device, globalUniformBuffersMemory[i], 0, bufferSize, 0, &globalUniformBuffersMapped[i]);
	}
}

void VulkanApplication::createObjectUniformBuffers()
{
	VkDeviceSize bufferSize = objectUniformBufferAlignment * meshGeometries.size();

	objectUniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
	objectUniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);
	objectUniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			objectUniformBuffers[i], objectUniformBuffersMemory[i]);

		vkMapMemory(device, objectUniformBuffersMemory[i], 0, bufferSize, 0, &objectUniformBuffersMapped[i]);
	}
}

void VulkanApplication::createMaterialUniformBuffers()
{
	VkDeviceSize bufferSize = materialUniformBufferAlignment * meshGeometries.size();

	materialUniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
	materialUniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);
	materialUniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			materialUniformBuffers[i], materialUniformBuffersMemory[i]);

		vkMapMemory(device, materialUniformBuffersMemory[i], 0, bufferSize, 0, &materialUniformBuffersMapped[i]);
	}
}

void VulkanApplication::createCommandBuffers()
{
	commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

	VkCommandBufferAllocateInfo commandBufferAllocateInfo{};
	commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	commandBufferAllocateInfo.commandPool = graphicsCommandPool;

	// The level parameter specifies if the allocated command buffers are primary or secondary command buffers.
	// VK_COMMAND_BUFFER_LEVEL_PRIMARY: Can be submitted to a queue for execution, but cannot be called from other command buffers.
	// VK_COMMAND_BUFFER_LEVEL_SECONDARY : Cannot be submitted directly, but can be called from primary command buffers.
	commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	commandBufferAllocateInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

	VkCheck(vkAllocateCommandBuffers(device, &commandBufferAllocateInfo, commandBuffers.data()), "lFaied to allocate command buffers!");
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
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, MAX_FRAMES_IN_FLIGHT },
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

void VulkanApplication::createDescriptorSets()
{
	std::vector<VkDescriptorSetLayout> descriptorSetLayouts(MAX_FRAMES_IN_FLIGHT, descriptorSetLayout);

	VkDescriptorSetAllocateInfo descriptorSetAllocateInfo{};
	descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descriptorSetAllocateInfo.descriptorPool = descriptorPool;
	descriptorSetAllocateInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
	descriptorSetAllocateInfo.pSetLayouts = descriptorSetLayouts.data();

	descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);

	VkCheck(vkAllocateDescriptorSets(device, &descriptorSetAllocateInfo, descriptorSets.data()), "Failed to allocate descriptor sets!");

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		VkDescriptorBufferInfo globalBufferBufferInfo{};
		globalBufferBufferInfo.buffer = globalUniformBuffers[i];
		globalBufferBufferInfo.offset = 0;
		globalBufferBufferInfo.range = sizeof(GlobalUniformBufferObject);

		VkDescriptorBufferInfo objectBufferBufferInfo{};
		objectBufferBufferInfo.buffer = objectUniformBuffers[i];
		objectBufferBufferInfo.offset = 0;
		objectBufferBufferInfo.range = sizeof(ObjectUniformBufferObject);

		VkDescriptorBufferInfo materialBufferInfo{};
		materialBufferInfo.buffer = materialUniformBuffers[i];
		materialBufferInfo.offset = 0;
		materialBufferInfo.range = sizeof(MaterialUniformBufferObject);

		VkDescriptorImageInfo imageInfo{};

		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.imageView = textureImageView;
		imageInfo.sampler = textureSampler;

		std::array<VkWriteDescriptorSet, 5> writeDescriptorSets{};

		writeDescriptorSets[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeDescriptorSets[0].dstSet = descriptorSets[i];
		writeDescriptorSets[0].dstBinding = 0;
		writeDescriptorSets[0].dstArrayElement = 0;
		writeDescriptorSets[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		writeDescriptorSets[0].descriptorCount = 1;
		writeDescriptorSets[0].pBufferInfo = &globalBufferBufferInfo;
		writeDescriptorSets[0].pImageInfo = nullptr;
		writeDescriptorSets[0].pTexelBufferView = nullptr;

		writeDescriptorSets[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeDescriptorSets[1].dstSet = descriptorSets[i];
		writeDescriptorSets[1].dstBinding = 1;
		writeDescriptorSets[1].dstArrayElement = 0;
		writeDescriptorSets[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
		writeDescriptorSets[1].descriptorCount = 1;
		writeDescriptorSets[1].pBufferInfo = &objectBufferBufferInfo;
		writeDescriptorSets[1].pImageInfo = nullptr;
		writeDescriptorSets[1].pTexelBufferView = nullptr;

		writeDescriptorSets[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeDescriptorSets[2].dstSet = descriptorSets[i];
		writeDescriptorSets[2].dstBinding = 2;
		writeDescriptorSets[2].dstArrayElement = 0;
		writeDescriptorSets[2].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
		writeDescriptorSets[2].descriptorCount = 1;
		writeDescriptorSets[2].pBufferInfo = &materialBufferInfo;
		writeDescriptorSets[2].pImageInfo = nullptr;
		writeDescriptorSets[2].pTexelBufferView = nullptr;

		writeDescriptorSets[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeDescriptorSets[3].dstSet = descriptorSets[i];
		writeDescriptorSets[3].dstBinding = 3;
		writeDescriptorSets[3].dstArrayElement = 0;
		writeDescriptorSets[3].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		writeDescriptorSets[3].descriptorCount = 1;
		writeDescriptorSets[3].pBufferInfo = nullptr;
		writeDescriptorSets[3].pImageInfo = &imageInfo;
		writeDescriptorSets[3].pTexelBufferView = nullptr;

		writeDescriptorSets[4].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeDescriptorSets[4].dstSet = descriptorSets[i];
		writeDescriptorSets[4].dstBinding = 4;
		writeDescriptorSets[4].dstArrayElement = 0;
		writeDescriptorSets[4].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		writeDescriptorSets[4].descriptorCount = 1;
		writeDescriptorSets[4].pBufferInfo = nullptr;
		writeDescriptorSets[4].pImageInfo = &imageInfo;
		writeDescriptorSets[4].pTexelBufferView = nullptr;

		vkUpdateDescriptorSets(device, static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, nullptr);
	}
}

void VulkanApplication::createSyncObjects()
{
	imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

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

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		VkCheck(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &imageAvailableSemaphores[i]), "Failed to create semaphores!");
		VkCheck(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &renderFinishedSemaphores[i]), "Failed to create semaphores!");
		VkCheck(vkCreateFence(device, &fenceCreateInfo, nullptr, &inFlightFences[i]), "Failed to create semaphores!");
	}
}

void VulkanApplication::recordCommandBuffer(VkCommandBuffer inCommandBuffer, uint32_t imageIndex)
{
	VkCommandBufferBeginInfo commandBufferBeginInfo{};
	commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	commandBufferBeginInfo.flags = 0;
	commandBufferBeginInfo.pInheritanceInfo = nullptr;

	VkCheck(vkBeginCommandBuffer(inCommandBuffer, &commandBufferBeginInfo), "Failed to begin recording command buffer!");

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
	// VK_SUBPASS_CONTENTS_INLINE: The render pass commands will be embedded in the primary command buffer itselfand no secondary command buffers will be executed.
	// VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS : The render pass commands will be executed from secondary command buffers.
	vkCmdBeginRenderPass(inCommandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

	vkCmdBindPipeline(inCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(swapChainExtent.width);
	viewport.height = static_cast<float>(swapChainExtent.height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	vkCmdSetViewport(inCommandBuffer, 0, 1, &viewport);

	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = swapChainExtent;
	
	vkCmdSetScissor(inCommandBuffer, 0, 1, &scissor);

	// 这里vertexBUffer是全局的顶点缓冲(整个.obj模型的所有submesh数据都存在它里面)
	VkBuffer vertexBuffers[]{ vertexBuffer };
	VkDeviceSize offsets[]{ 0 };
	vkCmdBindVertexBuffers(inCommandBuffer, 0, 1, vertexBuffers, offsets);

	// 这里的indexBUffer同样是全局的索引缓冲
	// 绘制时通过vkCmdDrawIndexd第三个参数indexCount配合第四个参数firstIndex来进行索引
	vkCmdBindIndexBuffer(inCommandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);

	for (auto i = 0; i < meshGeometries.size(); i++)
	{
		const auto& meshGeometry = meshGeometries[i];

		//VkBuffer vertexBuffers[]{ meshGeometry->vertexBuffer };

		if (meshGeometry->indexBuffer != nullptr)
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
		}
		else
		{
			materialUniformBufferObject.diffuseColor = glm::vec4(meshGeometry->material->Kd, 0.0f);
		}

		updateMaterialUniformBuffer(currentFrame, i, materialUniformBufferObject);

		ObjectUniformBufferObject objectUniformBufferObject;
		objectUniformBufferObject.model = meshGeometry->model;

		updateObjectUniformBuffer(currentFrame, i, objectUniformBufferObject);

		updateImageView(meshGeometry->textureImageView, meshGeometry->alphaTextureImageView, currentFrame);

		uint32_t dynamicOffsets[2] = { i * static_cast<uint32_t>(objectUniformBufferAlignment), i * static_cast<uint32_t>(materialUniformBufferAlignment) };
		vkCmdBindDescriptorSets(inCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSets[currentFrame], 2, dynamicOffsets);

		// The actual vkCmdDraw function is a bit anticlimactic, but it's so simple because of all the information we specified in advance. 
		// It has the following parameters, aside from the command buffer:
		// vertexCount: Even though we don't have a vertex buffer, we technically still have 3 vertices to draw.
		// instanceCount : Used for instanced rendering, use 1 if you're not doing that.
		// firstVertex : Used as an offset into the vertex buffer, defines the lowest value of gl_VertexIndex.
		// firstInstance : Used as an offset for instanced rendering, defines the lowest value of gl_InstanceIndex.

		// 第四个参数代表当前submesh的索引在全局索引缓冲中的起始位置
		vkCmdDrawIndexed(inCommandBuffer, meshGeometry->indexCount, 1, meshGeometry->indexStartIndex, 0, 0);
	}

	renderImGui();

	vkCmdEndRenderPass(inCommandBuffer);

	VkCheck(vkEndCommandBuffer(inCommandBuffer), "Failed to record command buffer!");
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

void VulkanApplication::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags propertyFlags, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
{
	VkBufferCreateInfo bufferCreateInfo{};
	bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.size = size;
	bufferCreateInfo.usage = usage;
	bufferCreateInfo.flags = 0;

	QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);

	uint32_t pQueueFamilyIndices[] = { queueFamilyIndices.graphicsFamily.value(), queueFamilyIndices.transferFamily.value() };

	if (queueFamilyIndices.graphicsFamily != queueFamilyIndices.transferFamily)
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
	allocateInfo.memoryTypeIndex = findMemoryType(memoryRequirements.memoryTypeBits,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	VkCheck(vkAllocateMemory(device, &allocateInfo, allocator, &bufferMemory), "Failed to allocate vertex buffer memory!");

	vkBindBufferMemory(device, buffer, bufferMemory, 0);
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

void VulkanApplication::createImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags propertyFlags, VkImage& image, VkDeviceMemory& imageMemory)
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
	imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageCreateInfo.flags = 0;

	VkCheck(vkCreateImage(device, &imageCreateInfo, allocator, &image), "Faild to create image!");

	VkMemoryRequirements memoryRequirements;
	vkGetImageMemoryRequirements(device, image, &memoryRequirements);

	VkMemoryAllocateInfo memoryAllocateInfo{};
	memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryAllocateInfo.allocationSize = memoryRequirements.size;
	memoryAllocateInfo.memoryTypeIndex = findMemoryType(memoryRequirements.memoryTypeBits, propertyFlags);

	VkCheck(vkAllocateMemory(device, &memoryAllocateInfo, allocator, &imageMemory), "Failed to allocate image memory!");

	vkBindImageMemory(device, image, imageMemory, 0);
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
	imageViewCreateInfo.subresourceRange.aspectMask = aspectFlags;
	imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
	imageViewCreateInfo.subresourceRange.levelCount = mipLevels;
	imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
	imageViewCreateInfo.subresourceRange.layerCount = 1;

	VkImageView imageView;

	VkCheck(vkCreateImageView(device, &imageViewCreateInfo, nullptr, &imageView), "Failed to create image views!");

	return imageView;
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

void VulkanApplication::loadResources()
{
	//model.load(ResourceBase + "models/cube.obj");
	//model.load(ResourceBase + "models/texture_bunny.obj");
	//model.load(ResourceBase + "sponza_with_ship.obj");

	cube = loadSimpleWavefrontObj((ResourceBase + "models/cube.obj").c_str());
	sphere = loadSimpleWavefrontObj((ResourceBase + "models/sphere.obj").c_str());
	//objModel = loadSimpleWavefrontObj((ResourceBase + "models/plane.obj").c_str());
	sponza = loadSimpleWavefrontObj((ResourceBase + "models/sponza_with_ship.obj").c_str());

	vertexBuffer = createVertexBuffer(sponza.vertices, vertexBufferMemory);
	indexBuffer = createIndexBuffer(sponza.indices, indexBufferMemory);

	//model.mesh.vertices = quadVertices;
	//model.mesh.indices = quadIndices;
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
		auto temp = fmt::format("Vulkan [FPS: {0}]", static_cast<float>(fps));
		glfwSetWindowTitle(window, temp.c_str());
		frameCount = 0;
	}

	frameCount++;
}

void VulkanApplication::updateGlobaltUniformBuffer(uint32_t frameIndex)
{
	static auto startTime = std::chrono::high_resolution_clock::now();

	auto currentTime = std::chrono::high_resolution_clock::now();

	auto time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

	GlobalUniformBufferObject ubo{};
	ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	ubo.model = glm::mat4(1.0f);
	ubo.view = camera.GetViewMatrix();
	ubo.projection = glm::perspective(glm::radians(45.0f), swapChainExtent.width / static_cast<float>(swapChainExtent.height), 0.1f, 100.0f);
	ubo.projection[1][1] *= -1.0f;

	memcpy_s(globalUniformBuffersMapped[frameIndex], sizeof(ubo), &ubo, sizeof(ubo));
}

void VulkanApplication::updateObjectUniformBuffer(uint32_t frameIndex, uint32_t index, const ObjectUniformBufferObject& objectUniformBufferObject)
{
	auto offset = static_cast<uint8_t*>(objectUniformBuffersMapped[frameIndex]);
	offset += index * objectUniformBufferAlignment;
	memcpy_s(offset, sizeof(ObjectUniformBufferObject), &objectUniformBufferObject, sizeof(ObjectUniformBufferObject));
}

void VulkanApplication::updateMaterialUniformBuffer(uint32_t frameIndex, uint32_t index, const MaterialUniformBufferObject& materialUniformBufferObject)
{
	auto offset = static_cast<uint8_t*>(materialUniformBuffersMapped[frameIndex]);
	offset += index * materialUniformBufferAlignment;
	memcpy_s(offset, sizeof(MaterialUniformBufferObject), &materialUniformBufferObject, sizeof(MaterialUniformBufferObject));
}

void VulkanApplication::updateImageView(VkImageView imageView, VkImageView alphaImageView, uint32_t frameIndex)
{
	VkDescriptorBufferInfo globalBufferBufferInfo{};
	globalBufferBufferInfo.buffer = globalUniformBuffers[frameIndex];
	globalBufferBufferInfo.offset = 0;
	globalBufferBufferInfo.range = sizeof(GlobalUniformBufferObject);

	VkDescriptorBufferInfo objectBufferBufferInfo{};
	objectBufferBufferInfo.buffer = objectUniformBuffers[frameIndex];
	objectBufferBufferInfo.offset = 0;
	objectBufferBufferInfo.range = sizeof(ObjectUniformBufferObject);

	VkDescriptorBufferInfo materialBufferInfo{};
	materialBufferInfo.buffer = materialUniformBuffers[frameIndex];
	materialBufferInfo.offset = 0;
	materialBufferInfo.range = sizeof(MaterialUniformBufferObject);

	VkDescriptorImageInfo imageInfo{};

	imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageInfo.imageView = imageView;
	imageInfo.sampler = textureSampler;

	std::array<VkWriteDescriptorSet, 5> writeDescriptorSets{};

	writeDescriptorSets[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeDescriptorSets[0].dstSet = descriptorSets[frameIndex];
	writeDescriptorSets[0].dstBinding = 0;
	writeDescriptorSets[0].dstArrayElement = 0;
	writeDescriptorSets[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	writeDescriptorSets[0].descriptorCount = 1;
	writeDescriptorSets[0].pBufferInfo = &globalBufferBufferInfo;
	writeDescriptorSets[0].pImageInfo = nullptr;
	writeDescriptorSets[0].pTexelBufferView = nullptr;

	writeDescriptorSets[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeDescriptorSets[1].dstSet = descriptorSets[frameIndex];
	writeDescriptorSets[1].dstBinding = 1;
	writeDescriptorSets[1].dstArrayElement = 0;
	writeDescriptorSets[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
	writeDescriptorSets[1].descriptorCount = 1;
	writeDescriptorSets[1].pBufferInfo = &objectBufferBufferInfo;
	writeDescriptorSets[1].pImageInfo = nullptr;
	writeDescriptorSets[1].pTexelBufferView = nullptr;

	writeDescriptorSets[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeDescriptorSets[2].dstSet = descriptorSets[frameIndex];
	writeDescriptorSets[2].dstBinding = 2;
	writeDescriptorSets[2].dstArrayElement = 0;
	writeDescriptorSets[2].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
	writeDescriptorSets[2].descriptorCount = 1;
	writeDescriptorSets[2].pBufferInfo = &materialBufferInfo;
	writeDescriptorSets[2].pImageInfo = nullptr;
	writeDescriptorSets[2].pTexelBufferView = nullptr;

	writeDescriptorSets[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeDescriptorSets[3].dstSet = descriptorSets[frameIndex];
	writeDescriptorSets[3].dstBinding = 3;
	writeDescriptorSets[3].dstArrayElement = 0;
	writeDescriptorSets[3].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	writeDescriptorSets[3].descriptorCount = 1;
	writeDescriptorSets[3].pBufferInfo = nullptr;
	writeDescriptorSets[3].pImageInfo = &imageInfo;
	writeDescriptorSets[3].pTexelBufferView = nullptr;

	VkDescriptorImageInfo alphaImageInfo{};
	alphaImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	alphaImageInfo.sampler = textureSampler;
	
	if (alphaImageView != VK_NULL_HANDLE)
	{
		alphaImageInfo.imageView = alphaImageView;
	}
	else
	{
		alphaImageInfo.imageView = textureImageView;
	}

	writeDescriptorSets[4].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeDescriptorSets[4].dstSet = descriptorSets[frameIndex];
	writeDescriptorSets[4].dstBinding = 4;
	writeDescriptorSets[4].dstArrayElement = 0;
	writeDescriptorSets[4].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	writeDescriptorSets[4].descriptorCount = 1;
	writeDescriptorSets[4].pBufferInfo = nullptr;
	writeDescriptorSets[4].pImageInfo = &alphaImageInfo;
	writeDescriptorSets[4].pTexelBufferView = nullptr;

	vkUpdateDescriptorSets(device, static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, nullptr);
}

void VulkanApplication::initWindow()
{
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
	createSwapChain();
	createImageViews();
	createRenderPass();
	createDescriptorSetLayout();
	createGraphicsPipeline();
	createGraphicsCommandPool();
	createTransferCommandPool();
	createDepthResources();
	createFramebuffers();

	textureImage =  createTextureImage(textureImageMemory, ResourceBase + "Textures/Kanna.jpg", Channel::RGBAlpha);
	textureImageView = createTextureImageView(textureImage);

	createTextureSampler();
	loadResources();

	//createMeshGeometries(model);
	createMeshGeometries(sponza);
	createMeshGeometries(sphere);

	createGlobalUniformBuffers();
	createObjectUniformBuffers();
	createMaterialUniformBuffers();
	createDescriptorPool();
	createDescriptorSets();
	createCommandBuffers();
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

	initInfo.QueueFamily = queueFamilyIndices.graphicsFamily.value();
	initInfo.Queue = graphicsQueue;
	initInfo.PipelineCache = VK_NULL_HANDLE;
	initInfo.DescriptorPool = descriptorPool;
	initInfo.Subpass = 0;
	initInfo.MinImageCount = minImageCount;
	initInfo.ImageCount = imageCount;
	initInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
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
		VkCheck(vkBeginCommandBuffer(commandBuffers[currentFrame], &beginInfo), "vkBeginCommandBuffer failed!");

		ImGui_ImplVulkan_CreateFontsTexture(commandBuffers[currentFrame]);

		VkSubmitInfo endInfo = {};
		endInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		endInfo.commandBufferCount = 1;
		endInfo.pCommandBuffers = &commandBuffers[currentFrame];
		VkCheck(vkEndCommandBuffer(commandBuffers[currentFrame]), "vkEndCommandBuffer failed!");

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
	ImGui_ImplVulkan_RenderDrawData(drawData, commandBuffers[currentFrame]);
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

	auto result = deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && physicalDeviceFeatures.geometryShader;

	QueueFamilyIndices queueFamilyIndices = findQueueFamilies(inDevice);

	bool extensionsSupported = checkDeviceExtensionSupport(inDevice);

	bool swapChainAdequate = false;

	if (extensionsSupported)
	{
		SwapChainSupportDetails swapChainSupport = querySwapChainSupport(inDevice);

		swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
	}

	return queueFamilyIndices.isComplete() && extensionsSupported && swapChainAdequate && physicalDeviceFeatures.samplerAnisotropy;
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
		if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			indices.graphicsFamily = index;
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

		while (simulationTime < realTime)
		{
			simulationTime += FrameTime;
			processInput(FrameTime);
		}

		glfwPollEvents();
		updateFPSCounter();
		updateImGui();
		createImGuiWidgets();
		drawFrame();
	}

	vkDeviceWaitIdle(device);
}

void VulkanApplication::drawFrame()
{
	// The vkWaitForFences function takes an array of fences and waits on the host for either any or all of the fences 
	// to be signaled before returning. The VK_TRUE we pass here indicates that we want to wait for all fences, but in 
	// the case of a single one it doesn't matter. This function also has a timeout parameter that we set to the maximum
	// value of a 64 bit unsigned integer, UINT64_MAX, which effectively disables the timeout.
	vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

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
	vkResetFences(device, 1, &inFlightFences[currentFrame]);

	vkResetCommandBuffer(commandBuffers[currentFrame], 0);

	recordCommandBuffer(commandBuffers[currentFrame], imageIndex);

	updateGlobaltUniformBuffer(currentFrame);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	
	VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrame]};
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

	// The first three parameters specify which semaphores to wait on before execution begins and in which stage(s)
	// of the pipeline to wait. We want to wait with writing colors to the image until it's available, so we're specifying 
	// the stage of the graphics pipeline that writes to the color attachment. That means that theoretically the implementation 
	// can already start executing our vertex shader and such while the image is not yet available. Each entry in the waitStages 
	// array corresponds to the semaphore with the same index in pWaitSemaphores.
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;

	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffers[currentFrame];

	// The signalSemaphoreCount and pSignalSemaphores parameters specify which semaphores to signal once the command 
	// buffer(s) have finished execution. In our case we're using the renderFinishedSemaphore for that purpose.
	VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame]};
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	VkCheck(vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]), "Failed to submit draw command buffer!b");

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

	vkDestroyImageView(device, textureImageView, allocator);

	vkDestroyImage(device, textureImage, allocator);
	vkFreeMemory(device, textureImageMemory, allocator);

	vkDestroyBuffer(device, vertexBuffer, allocator);
	vkUnmapMemory(device, vertexBufferMemory);
	vkFreeMemory(device, vertexBufferMemory, allocator);

	vkDestroyBuffer(device, indexBuffer, allocator);
	vkUnmapMemory(device, indexBufferMemory);
	vkFreeMemory(device, indexBufferMemory, allocator);

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		vkDestroyBuffer(device, globalUniformBuffers[i], allocator);
		vkUnmapMemory(device, globalUniformBuffersMemory[i]);
		vkFreeMemory(device, globalUniformBuffersMemory[i], allocator);
	}

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		vkDestroyBuffer(device, objectUniformBuffers[i], allocator);
		vkUnmapMemory(device, objectUniformBuffersMemory[i]);
		vkFreeMemory(device, objectUniformBuffersMemory[i], allocator);
	}

	vkDestroyDescriptorPool(device, descriptorPool, nullptr);

	for (auto i = 0; i < meshGeometries.size(); i++)
	{
		const auto& meshGeometry = meshGeometries[i];

		vkDestroyBuffer(device, meshGeometry->indexBuffer, allocator);

		vkFreeMemory(device, meshGeometry->indexBufferMemory, allocator);

		vkDestroyBuffer(device, meshGeometry->vertexBuffer, allocator);

		vkFreeMemory(device, meshGeometry->vertexBufferMemory, allocator);

		vkDestroyImageView(device, meshGeometry->textureImageView, allocator);
		vkDestroyImage(device, meshGeometry->textureImage, allocator);
		vkFreeMemory(device, meshGeometry->textureImageMemory, allocator);
	}

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		vkDestroySemaphore(device, imageAvailableSemaphores[i], allocator);
		vkDestroySemaphore(device, renderFinishedSemaphores[i], allocator);
		vkDestroyFence(device, inFlightFences[i], allocator);
	}

	vkDestroyCommandPool(device, transferCommandPool, allocator);

	vkDestroyCommandPool(device, graphicsCommandPool, allocator);

	vkDestroyDescriptorSetLayout(device, descriptorSetLayout, allocator);

	vkDestroyPipeline(device, graphicsPipeline, allocator);

	vkDestroyPipelineLayout(device, pipelineLayout, allocator);

	vkDestroyRenderPass(device, renderPass, allocator);

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

	return extensions;
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
