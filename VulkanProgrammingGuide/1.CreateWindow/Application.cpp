#include "Application.h"

Application::Application(int inWindowWidth, int inWindowHeight, const std::string title)
	: windowWidth(inWindowWidth), 
	  windowHeight(inWindowHeight),
	  windowTitle(title),
	  window(nullptr),
	  instance(nullptr),
	  device(nullptr),
	  physicalDevice(nullptr)
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
	queryInstanceLayers();
	queryInstanceExtensions();
	createInstance();
	createPhysicalDevice();
	queryDeviceLayers();
	queryDeviceExtensions();
	queryDeviceProperties();
	createLogicalDevice();
}

void Application::queryInstanceLayers()
{
	uint32_t instanceLayerPropertyCount = 0;
	std::vector<VkLayerProperties> instanceLayerProperties;

	if (!vkEnumerateInstanceLayerProperties(&instanceLayerPropertyCount, nullptr))
	{
		instanceLayerProperties.resize(instanceLayerPropertyCount);
		if (vkEnumerateInstanceLayerProperties(&instanceLayerPropertyCount, instanceLayerProperties.data()))
		{
			std::cout << "Enumerate instance layer properties failed." << std::endl;
		}
	}

	std::cout << "Available instance layers:" << std::endl;

	for (auto layerProperty : instanceLayerProperties)
	{
		std::cout << "\t" << layerProperty.layerName << std::endl;
	}

	std::cout << std::endl;
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

void Application::createInstance()
{
	const VkApplicationInfo applicationInfo = {
	
		VK_STRUCTURE_TYPE_APPLICATION_INFO,	// sType
		nullptr,							// pNext
		"Vulkan Application",				// pApplicationName
		VK_MAKE_VERSION(1, 0, 0),			// applicationVersion
		"No Engine",						// pEngineName
		VK_MAKE_VERSION(1, 0, 0),			// engineVersion
		VK_API_VERSION_1_1					// apiVersion
	};

	uint32_t GLFWExtensionCount = 0;
	const char** GLFWExtensions;

	GLFWExtensions = glfwGetRequiredInstanceExtensions(&GLFWExtensionCount);

	const VkInstanceCreateInfo instanceCreateInfo = {
	
		VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,	// sType
		nullptr,								// pNext
		0,										// flags
		&applicationInfo,						// pApplicationInfo
		0,										// enabledLayerCount
		nullptr,								// ppEnabledLayerNames
		GLFWExtensionCount,						// enabledExtensionCount
		GLFWExtensions							// ppEnabledExtensionNames
	};

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

void Application::queryDeviceExtensions()
{
	uint32_t deviceExtensionCount = 0;
	std::vector<VkExtensionProperties> deviceExtensions;
	if (!vkEnumerateInstanceExtensionProperties(nullptr, &deviceExtensionCount, nullptr))
	{
		deviceExtensions.resize(deviceExtensionCount);
		if (vkEnumerateInstanceExtensionProperties(nullptr, &deviceExtensionCount, deviceExtensions.data()))
		{
			std::cout << "Enumerate device extensions failed." << std::endl;
		}
	}

	std::cout << "Available device extensions:" << std::endl;

	for (const auto& extension : deviceExtensions)
	{
		std::cout << "\t" << extension.extensionName << std::endl;
	}

	std::cout << std::endl;
}

void Application::createPhysicalDevice()
{
	uint32_t physicalDeviceCount = 0;

	std::vector<VkPhysicalDevice> physicalDevices;

	if (!vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, nullptr))
	{
		physicalDevices.resize(physicalDeviceCount);
		if (vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, physicalDevices.data()))
		{
			std::cout << "Enumerate physical devices failed." << std::endl;
		}
	}

	physicalDevice = physicalDevices[0];
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

	vkGetPhysicalDeviceFormatProperties(physicalDevice, )
}

void Application::createLogicalDevice()
{
	VkPhysicalDeviceFeatures physicalDeviceFeatures;
	vkGetPhysicalDeviceFeatures(physicalDevice, &physicalDeviceFeatures);

	const VkDeviceQueueCreateInfo deviceQueueCreateInfo = {

	VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,	// sType
	nullptr,									// pNext
	0,											// flags
	0,											// queueFamilyIndex
	1,											// queueCount
	nullptr										// pQueuePriorities
	};

	const VkDeviceCreateInfo deviceCreateInfo = {

		VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,	// sType
		nullptr,								// pNext
		0,										// flags
		1,										// queueCreateInfoCount
		&deviceQueueCreateInfo,					// pQueueCreateInfos
		0,										// enabledLayerCount
		nullptr,								// ppEnabledLayerNames
		0,										// enabledExtensionCount
		nullptr,								// ppEnabledExtensionNames
		&physicalDeviceFeatures					// pEnabledFeatures
	};

	if (vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &device))
	{
		std::cout << "Create device failed." << std::endl;
	}
}

void Application::createBuffer()
{
	const VkBufferCreateInfo bufferCreateInfo = {

		0,
		1024 * 1024,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		VK_SHARING_MODE_EXCLUSIVE,
		0,
		nullptr
	};

	VkBuffer buffer = VK_NULL_HANDLE;
	vkCreateBuffer(device, &bufferCreateInfo, &buffer);
}

void Application::mainLoop()
{
	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
	}
}

void Application::shutDown()
{
	glfwDestroyWindow(window);

	glfwTerminate();

	vkDeviceWaitIdle(device);
	vkDestroyDevice(device, nullptr);
	vkDestroyInstance(instance, nullptr);
}

Application::~Application()
{
	shutDown();
}

void Application::run()
{
	createWindow(windowWidth, windowHeight, windowTitle);
	initializeVulkan();
	mainLoop();
}
