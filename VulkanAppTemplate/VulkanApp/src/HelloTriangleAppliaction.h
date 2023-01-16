#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <vector>
#include <optional>

#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>

const uint32_t WindowWidth = 800;
const uint32_t WindowHeight = 600;

const std::vector<const char*> ValidationLayers = { "VK_LAYER_KHRONOS_validation" };
const std::vector<const char*> DeviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

const std::string ResourceBase = "../Assets/";

static ImGui_ImplVulkanH_Window g_MainWindowData;

#ifdef NDEBUG
const bool EnableValidationLayers = false;
#else
const bool EnableValidationLayers = true;
#endif // NDEBUG

#define THROW_ERROR(message) throw std::runtime_error(message);

#define VkCheck(result, message) if (result != VK_SUCCESS) { THROW_ERROR(message) }

// Our state
static bool showDemoWindow = true;
static bool showAnotherWindow = false;
static ImVec4 clearColor = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

static void check_vk_result(VkResult err)
{
	if (err == 0)
		return;
	fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
	if (err < 0)
		abort();
}

struct QueueFamilyIndices
{
	std::optional<uint32_t> graphicsFamily = 0;
	std::optional<uint32_t> presentFamily = 0;

	bool isComplete()
	{
		return graphicsFamily.has_value() && presentFamily.has_value();
	}
};

struct SwapChainSupportDetails
{
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};

class HelloTriangleApplicaton
{
public:
	HelloTriangleApplicaton();
	~HelloTriangleApplicaton();

	void run();
private:
	void static keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
private:
	void createInstance();
	void setupDebugMessenger();
	void createSurface();
	void pickPhysicalDevice();
	void createLogicalDevice();
	void createDescriptorPool();
	void createSwapChain();
	void createImageViews();
	void createRenderPass();
	void createGraphicsPipeline();
	void createFramebuffers();
	void createCommandPool();
	void createCommandBuffer();
	void createSyncObjects();
	void recordCommandBuffer(VkCommandBuffer inCommandBuffer, uint32_t imageIndex);

	VkShaderModule createShaderModule(const std::vector<char>& shaderCode);
	
	void initWindow();
	void initVulkan();
	void initImGui();

	void updateImGui();
	void createImGuiWidgets();
	void renderImGui();
	void cleanupImGui();

	bool isDeviceSuitable(VkPhysicalDevice inDevice);
	int32_t rateDeviceSuitability(VkPhysicalDevice inDevice);
	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice inDevice);
	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice inDevice);
	VkSurfaceFormatKHR chooseSwapChainSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR chooseSwapChainPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	VkExtent2D chooseSwapChainExtent(const VkSurfaceCapabilitiesKHR& capabilities);

	void mainLoop();
	void drawFrame();
	void cleanup();

	void processInput();

	bool checkExtensionSupport();
	bool checkDeviceExtensionSupport(VkPhysicalDevice inDevice);
	bool checkValidationLayerSupport();
	std::vector<const char*> getRequiredExtensions();

	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* userData);

private:
	struct GLFWwindow* window;

	VkAllocationCallbacks* allocator;

	VkInstance instance;
	VkPhysicalDevice physicalDevice;
	VkDescriptorPool descriptorPool;
	VkDevice device;
	VkQueue graphicsQueue;
	VkQueue presentQueue;
	VkSurfaceKHR surface;
	VkSwapchainKHR swapChain;
	VkRenderPass renderPass; 
	VkPipelineLayout pipelineLayout;
	VkPipeline graphicsPipeline;
	VkCommandPool commandPool;
	VkCommandBuffer commandBuffer;
	VkSemaphore imageAvailableSemaphore;
	VkSemaphore renderFinishedSemaphore;
	VkFence inFlightFence;
	std::vector<VkImage> swapChainImages;
	std::vector<VkImageView> swapChainImageViews;
	std::vector<VkFramebuffer> swapChainFramebuffers;
	VkFormat swapChainImageFormat;
	VkExtent2D swapChainExtent;
	VkDebugUtilsMessengerEXT debugMessenger;

	uint32_t minImageCount;
	uint32_t imageCount;
	VkClearValue clearColor;
};