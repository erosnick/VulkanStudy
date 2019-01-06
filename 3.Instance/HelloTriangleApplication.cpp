#include "HelloTriangleApplication.h"
#include <vector>

void HelloTriangleApplication::CreateInstance()
{
	VkApplicationInfo ApplicationInfo = {};

	ApplicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	ApplicationInfo.pNext = nullptr;
	ApplicationInfo.pApplicationName = "Hello Triangle";
	ApplicationInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	ApplicationInfo.pEngineName = "No Engine";
	ApplicationInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	ApplicationInfo.apiVersion = VK_API_VERSION_1_1;

	VkInstanceCreateInfo CreateInfo = {};
	CreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	CreateInfo.pApplicationInfo = &ApplicationInfo;

	uint32_t GLFWExtensionCount = 0;
	const char** GLFWExtensions;

	GLFWExtensions = glfwGetRequiredInstanceExtensions(&GLFWExtensionCount);

	CreateInfo.enabledExtensionCount = GLFWExtensionCount;
	CreateInfo.ppEnabledExtensionNames = GLFWExtensions;
	CreateInfo.enabledLayerCount = 0;

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
	vkDestroyInstance(Instance, nullptr);
	glfwDestroyWindow(Window);
	glfwTerminate();
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