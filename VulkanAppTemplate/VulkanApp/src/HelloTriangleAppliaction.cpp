#include "HelloTriangleAppliaction.h"

#include <map>
#include <set>
#include <cstdint>
#include <limits>
#include <algorithm>
#include <fstream>

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
		THROW_ERROR("Failed to open file" + filaName + "!");
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

HelloTriangleApplicaton::HelloTriangleApplicaton()
: window(nullptr),
  allocator(VK_NULL_HANDLE),
  instance(VK_NULL_HANDLE), 
  physicalDevice(VK_NULL_HANDLE),
  device(VK_NULL_HANDLE),
  graphicsQueue(VK_NULL_HANDLE),
  presentQueue(VK_NULL_HANDLE),
  surface(VK_NULL_HANDLE),
  swapChain(VK_NULL_HANDLE),
  renderPass(VK_NULL_HANDLE),
  pipelineLayout(VK_NULL_HANDLE),
  graphicsPipeline(VK_NULL_HANDLE),
  commandPool(VK_NULL_HANDLE),
  commandBuffer(VK_NULL_HANDLE),
  imageAvailableSemaphore(VK_NULL_HANDLE),
  renderFinishedSemaphore(VK_NULL_HANDLE),
  inFlightFence(VK_NULL_HANDLE),
  swapChainImageFormat(VK_FORMAT_UNDEFINED),
  swapChainExtent{ 0, 0 },
  debugMessenger(VK_NULL_HANDLE),
  minImageCount(0),
  imageCount(0)
{
}

HelloTriangleApplicaton::~HelloTriangleApplicaton()
{
}

void HelloTriangleApplicaton::run()
{
	initWindow();
	initVulkan();
	initImGui();
	mainLoop();
	cleanup();
}

void HelloTriangleApplicaton::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, true);
	}
}

void HelloTriangleApplicaton::createInstance()
{
	if (EnableValidationLayers && !checkValidationLayerSupport())
	{
		THROW_ERROR("Validation layers requested, but not available!");
	}

	VkApplicationInfo appInfo{};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pNext = nullptr;
	appInfo.pApplicationName = "Hello Triangle";
	appInfo.apiVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "No Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_3;

	VkInstanceCreateInfo instanceCreateInfo{};
	instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceCreateInfo.pApplicationInfo = &appInfo;

	auto extensions = getRequiredExtensions();

	extensions.emplace_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);

	instanceCreateInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;

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

	checkExtensionSupport();
}

void HelloTriangleApplicaton::setupDebugMessenger()
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

void HelloTriangleApplicaton::initWindow()
{
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	window = glfwCreateWindow(WindowWidth, WindowHeight, "Vulkan window", nullptr, nullptr);

	glfwSetKeyCallback(window, keyCallback);
}

void HelloTriangleApplicaton::initVulkan()
{
	createInstance();
	setupDebugMessenger();
	createSurface();
	pickPhysicalDevice();
	createLogicalDevice();
	createDescriptorPool();
	createSwapChain();
	createImageViews();
	createRenderPass();
	createGraphicsPipeline();
	createFramebuffers();
	createCommandPool();
	createCommandBuffer();
	createSyncObjects();
}

void HelloTriangleApplicaton::initImGui()
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
	initInfo.CheckVkResultFn = check_vk_result;
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
	//ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
	//IM_ASSERT(font != NULL);
	
	// Upload Fonts
	{
		// Use any command queue
		VkCheck(vkResetCommandPool(device, commandPool, 0), "vkResetCommandPool failed!");

		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		VkCheck(vkBeginCommandBuffer(commandBuffer, &beginInfo), "vkBeginCommandBuffer failed!");

		ImGui_ImplVulkan_CreateFontsTexture(commandBuffer);

		VkSubmitInfo endInfo = {};
		endInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		endInfo.commandBufferCount = 1;
		endInfo.pCommandBuffers = &commandBuffer;
		VkCheck(vkEndCommandBuffer(commandBuffer), "vkEndCommandBuffer failed!");
		
		VkCheck(vkQueueSubmit(graphicsQueue, 1, &endInfo, VK_NULL_HANDLE), "vkQueueSubmit failed!");

		VkCheck(vkDeviceWaitIdle(device), "vkDeviceWaitIdle failed!");

		ImGui_ImplVulkan_DestroyFontUploadObjects();
	}
}

void HelloTriangleApplicaton::updateImGui()
{
	// Start the Dear ImGui frame
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
}

void HelloTriangleApplicaton::createImGuiWidgets()
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

void HelloTriangleApplicaton::renderImGui()
{
	// Rendering
	ImGui::Render();
	ImDrawData* drawData = ImGui::GetDrawData();
	const bool isMinimized = (drawData->DisplaySize.x <= 0.0f || drawData->DisplaySize.y <= 0.0f);
	// Record dear imgui primitives into command buffer
	ImGui_ImplVulkan_RenderDrawData(drawData, commandBuffer);
}

void HelloTriangleApplicaton::cleanupImGui()
{
	ImGui_ImplVulkan_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

void HelloTriangleApplicaton::pickPhysicalDevice()
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

void HelloTriangleApplicaton::createLogicalDevice()
{
	QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

	std::set<uint32_t> uniqueQueueFamilies = { queueFamilyIndices.graphicsFamily.value(), queueFamilyIndices.presentFamily.value() };

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

	VkPhysicalDeviceFeatures deviceFeatures{};

	VkDeviceCreateInfo deviceCreateInfo{};
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();

	deviceCreateInfo.pEnabledFeatures = &deviceFeatures;

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
}

void HelloTriangleApplicaton::createDescriptorPool()
{
	// Create Descriptor Pool
	{
		VkDescriptorPoolSize poolSizes[] =
		{
			{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
		};
		VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {};
		descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		descriptorPoolCreateInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		descriptorPoolCreateInfo.maxSets = 1000 * IM_ARRAYSIZE(poolSizes);
		descriptorPoolCreateInfo.poolSizeCount = (uint32_t)IM_ARRAYSIZE(poolSizes);
		descriptorPoolCreateInfo.pPoolSizes = poolSizes;
		VkCheck(vkCreateDescriptorPool(device, &descriptorPoolCreateInfo, allocator, &descriptorPool), "vkCreateDescriptorPool failed!");
	}
}

void HelloTriangleApplicaton::createSwapChain()
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

void HelloTriangleApplicaton::createImageViews()
{
	swapChainImageViews.resize(swapChainImages.size());

	for (size_t i = 0; i < swapChainImages.size(); i++)
	{
		VkImageViewCreateInfo imageViewCreateInfo{};
		imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageViewCreateInfo.image = swapChainImages[i];
		imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		imageViewCreateInfo.format = swapChainImageFormat;
		imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
		imageViewCreateInfo.subresourceRange.levelCount = 1;
		imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
		imageViewCreateInfo.subresourceRange.layerCount = 1;

		VkCheck(vkCreateImageView(device, &imageViewCreateInfo, nullptr, &swapChainImageViews[i]), "Failed to create image views!");
	}
}

void HelloTriangleApplicaton::createRenderPass()
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

	// The following other types of attachments can be referenced by a subpass:
	// pInputAttachments: Attachments that are read from a shader
	// pResolveAttachments : Attachments used for multisampling color attachments
	// pDepthStencilAttachment : Attachment for depthand stencil data
	// pPreserveAttachments : Attachments that are not used by this subpass, but for which the data must be preserved

	VkRenderPassCreateInfo renderPassCreateInfo{};
	renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassCreateInfo.attachmentCount = 1;
	renderPassCreateInfo.pAttachments = &colorAttachment;
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
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;

	// The operations that should wait on this are in the color attachment stage and involve the writing of the color attachment. 
	// These settings will prevent the transition from happening until it's actually necessary (and allowed): when we want to start writing colors to it.
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = 0;

	renderPassCreateInfo.dependencyCount = 1;
	renderPassCreateInfo.pDependencies = &dependency;

	VkCheck(vkCreateRenderPass(device, &renderPassCreateInfo, nullptr, &renderPass), "Failed to create render pass!");
}

void HelloTriangleApplicaton::createGraphicsPipeline()
{
	auto vertexShaderCode = readFile(ResourceBase + "Shaders/shader.vert.spv");
	auto fragmentShaderCode = readFile(ResourceBase + "Shaders/shader.frag.spv");

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

	VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo{};
	vertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputStateCreateInfo.vertexBindingDescriptionCount = 0;
	vertexInputStateCreateInfo.pVertexBindingDescriptions = nullptr;
	vertexInputStateCreateInfo.vertexAttributeDescriptionCount = 0;
	vertexInputStateCreateInfo.pVertexAttributeDescriptions = nullptr;

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
	rasterizationStateCreateInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;

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
	pipelineLayoutCreateInfo.setLayoutCount = 0;
	pipelineLayoutCreateInfo.pSetLayouts = nullptr;
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
	graphicsPipelineCreateInfo.pDepthStencilState = nullptr;
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

void HelloTriangleApplicaton::createFramebuffers()
{
	swapChainFramebuffers.resize(swapChainImageViews.size());

	for (size_t i = 0; i < swapChainImageViews.size(); i++)
	{
		VkImageView attachments[] =
		{
			swapChainImageViews[i]
		};

		VkFramebufferCreateInfo framebufferCreateInfo{};
		framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferCreateInfo.renderPass = renderPass;
		framebufferCreateInfo.attachmentCount = 1;
		framebufferCreateInfo.pAttachments = attachments;
		framebufferCreateInfo.width = swapChainExtent.width;
		framebufferCreateInfo.height = swapChainExtent.height;
		framebufferCreateInfo.layers = 1;

		VkCheck(vkCreateFramebuffer(device, &framebufferCreateInfo, nullptr, &swapChainFramebuffers[i]), "Failed to create framebuffer!");
	}
}

void HelloTriangleApplicaton::createCommandPool()
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

	VkCheck(vkCreateCommandPool(device, &commandPoolCreateInfo, nullptr, &commandPool), "Failed to create command pool!");
}

void HelloTriangleApplicaton::createCommandBuffer()
{
	VkCommandBufferAllocateInfo commandBufferAllocateInfo{};
	commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	commandBufferAllocateInfo.commandPool = commandPool;

	// The level parameter specifies if the allocated command buffers are primary or secondary command buffers.
	// VK_COMMAND_BUFFER_LEVEL_PRIMARY: Can be submitted to a queue for execution, but cannot be called from other command buffers.
	// VK_COMMAND_BUFFER_LEVEL_SECONDARY : Cannot be submitted directly, but can be called from primary command buffers.
	commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	commandBufferAllocateInfo.commandBufferCount = 1;

	VkCheck(vkAllocateCommandBuffers(device, &commandBufferAllocateInfo, &commandBuffer), "lFaied to allocate command buffers!");
}

void HelloTriangleApplicaton::createSyncObjects()
{
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

	VkCheck(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &imageAvailableSemaphore), "Failed to create semaphores!");
	VkCheck(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &renderFinishedSemaphore), "Failed to create semaphores!");
	VkCheck(vkCreateFence(device, &fenceCreateInfo, nullptr, &inFlightFence), "Failed to create semaphores!");
}

void HelloTriangleApplicaton::recordCommandBuffer(VkCommandBuffer inCommandBuffer, uint32_t imageIndex)
{
	VkCommandBufferBeginInfo commandBufferBeginInfo{};
	commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	commandBufferBeginInfo.flags = 0;
	commandBufferBeginInfo.pInheritanceInfo = nullptr;

	VkCheck(vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo), "Failed to begin recording command buffer!");

	VkRenderPassBeginInfo renderPassBeginInfo{};
	renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBeginInfo.renderPass = renderPass;
	renderPassBeginInfo.framebuffer = swapChainFramebuffers[imageIndex];
	renderPassBeginInfo.renderArea.offset = { 0, 0 };
	renderPassBeginInfo.renderArea.extent = swapChainExtent;

	clearColor = { {{0.4f, 0.6f, 0.9f, 1.0f}} };
	renderPassBeginInfo.clearValueCount = 1;
	renderPassBeginInfo.pClearValues = &clearColor;

	// The final parameter controls how the drawing commands within the render pass will be provided. It can have one of two values:
	// VK_SUBPASS_CONTENTS_INLINE: The render pass commands will be embedded in the primary command buffer itselfand no secondary command buffers will be executed.
	// VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS : The render pass commands will be executed from secondary command buffers.
	vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(swapChainExtent.width);
	viewport.height = static_cast<float>(swapChainExtent.height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = swapChainExtent;
	
	vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

	// The actual vkCmdDraw function is a bit anticlimactic, but it's so simple because of all the information we specified in advance. 
	// It has the following parameters, aside from the command buffer:
	// vertexCount: Even though we don't have a vertex buffer, we technically still have 3 vertices to draw.
	// instanceCount : Used for instanced rendering, use 1 if you're not doing that.
	// firstVertex : Used as an offset into the vertex buffer, defines the lowest value of gl_VertexIndex.
	// firstInstance : Used as an offset for instanced rendering, defines the lowest value of gl_InstanceIndex.
	vkCmdDraw(commandBuffer, 3, 1, 0, 0);

	renderImGui();

	vkCmdEndRenderPass(commandBuffer);

	VkCheck(vkEndCommandBuffer(commandBuffer), "Failed to record command buffer!");
}

VkShaderModule HelloTriangleApplicaton::createShaderModule(const std::vector<char>& shaderCode)
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

void HelloTriangleApplicaton::createSurface()
{
	if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS)
	{
		THROW_ERROR("Failed to create window surface!");
	}
}

bool HelloTriangleApplicaton::isDeviceSuitable(VkPhysicalDevice inDevice)
{
	VkPhysicalDeviceProperties deviceProperties;
	vkGetPhysicalDeviceProperties(inDevice, &deviceProperties);

	VkPhysicalDeviceFeatures deviceFeatures;
	vkGetPhysicalDeviceFeatures(inDevice, &deviceFeatures);

	auto result = deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && deviceFeatures.geometryShader;

	QueueFamilyIndices queueFamilyIndices = findQueueFamilies(inDevice);

	bool extensionsSupported = checkDeviceExtensionSupport(inDevice);

	bool swapChainAdequate = false;

	if (extensionsSupported)
	{
		SwapChainSupportDetails swapChainSupport = querySwapChainSupport(inDevice);

		swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
	}

	return queueFamilyIndices.isComplete() && extensionsSupported && swapChainAdequate;
}

int32_t HelloTriangleApplicaton::rateDeviceSuitability(VkPhysicalDevice inDevice)
{
	int32_t score = 0;

	// Discrete GPUs have a significant performance advantage
	return score;
}

QueueFamilyIndices HelloTriangleApplicaton::findQueueFamilies(VkPhysicalDevice inDevice)
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

		if (indices.isComplete())
		{
			break;
		}

		index++;
	}

	return indices;
}

SwapChainSupportDetails HelloTriangleApplicaton::querySwapChainSupport(VkPhysicalDevice inDevice)
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

VkSurfaceFormatKHR HelloTriangleApplicaton::chooseSwapChainSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
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

VkPresentModeKHR HelloTriangleApplicaton::chooseSwapChainPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
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

VkExtent2D HelloTriangleApplicaton::chooseSwapChainExtent(const VkSurfaceCapabilitiesKHR& capabilities)
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

void HelloTriangleApplicaton::mainLoop()
{
	while (!glfwWindowShouldClose(window)) 
	{
		glfwPollEvents();
		updateImGui();
		createImGuiWidgets();
		drawFrame();
	}

	vkDeviceWaitIdle(device);
}

void HelloTriangleApplicaton::drawFrame()
{
	// The vkWaitForFences function takes an array of fences and waits on the host for either any or all of the fences 
	// to be signaled before returning. The VK_TRUE we pass here indicates that we want to wait for all fences, but in 
	// the case of a single one it doesn't matter. This function also has a timeout parameter that we set to the maximum
	// value of a 64 bit unsigned integer, UINT64_MAX, which effectively disables the timeout.
	vkWaitForFences(device, 1, &inFlightFence, VK_TRUE, UINT64_MAX);

	vkResetFences(device, 1, &inFlightFence);

	uint32_t imageIndex = 0;

	vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

	vkResetCommandBuffer(commandBuffer, 0);

	recordCommandBuffer(commandBuffer, imageIndex);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	
	VkSemaphore waitSemaphores[] = { imageAvailableSemaphore };
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
	submitInfo.pCommandBuffers = &commandBuffer;

	// The signalSemaphoreCount and pSignalSemaphores parameters specify which semaphores to signal once the command 
	// buffer(s) have finished execution. In our case we're using the renderFinishedSemaphore for that purpose.
	VkSemaphore signalSemaphores[] = { renderFinishedSemaphore };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	VkCheck(vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFence), "Failed to submit draw command buffer!b");

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

	vkQueuePresentKHR(presentQueue, &presentInfo);
}

void HelloTriangleApplicaton::cleanup()
{
	cleanupImGui();

	vkDestroySemaphore(device, imageAvailableSemaphore, allocator);
	vkDestroySemaphore(device, renderFinishedSemaphore, allocator);
	vkDestroyFence(device, inFlightFence, allocator);

	vkDestroyCommandPool(device, commandPool, allocator);

	for (auto& framebuffer : swapChainFramebuffers)
	{
		vkDestroyFramebuffer(device, framebuffer, allocator);
	}

	vkDestroyPipeline(device, graphicsPipeline, allocator);

	vkDestroyPipelineLayout(device, pipelineLayout, allocator);

	vkDestroyRenderPass(device, renderPass, allocator);

	for (auto& imageView : swapChainImageViews)
	{
		vkDestroyImageView(device, imageView, allocator);
	}

	vkDestroyDescriptorPool(device, descriptorPool, allocator);

	vkDestroySwapchainKHR(device, swapChain, allocator);

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

void HelloTriangleApplicaton::processInput()
{

}

bool HelloTriangleApplicaton::checkExtensionSupport()
{
	uint32_t extensionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

	std::cout << extensionCount << " extensions supported\n";

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

bool HelloTriangleApplicaton::checkDeviceExtensionSupport(VkPhysicalDevice inDevice)
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

bool HelloTriangleApplicaton::checkValidationLayerSupport()
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

std::vector<const char*> HelloTriangleApplicaton::getRequiredExtensions()
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

void HelloTriangleApplicaton::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
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

VKAPI_ATTR VkBool32 VKAPI_CALL HelloTriangleApplicaton::debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* userData)
{
	return VK_FALSE;
}
