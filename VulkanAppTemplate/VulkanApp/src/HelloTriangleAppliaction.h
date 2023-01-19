#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <vector>
#include <optional>
#include <array>

#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>

#include <glm/glm.hpp>

#include <fmt/format.h>

#include "Model.h"
#include "Camera.h"

const std::vector<Vertex> quadVertices = 
{
	{ { -0.5f, -0.5f, 0.0f }, { 0.0f, 0.0f, -1.0f }, { 0.0f, 1.0f }, { 1.0f, 0.0f, 0.0f } },
	{ {  0.5f, -0.5f, 0.0f }, { 0.0f, 0.0f, -1.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f, 0.0f } },
	{ {  0.5f,  0.5f, 0.0f }, { 0.0f, 0.0f, -1.0f }, { 1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f } },
	{ { -0.5f,  0.5f, 0.0f }, { 0.0f, 0.0f, -1.0f }, { 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f } },

	{ { -0.5f, -0.5f, -0.5f }, { 0.0f, 0.0f, -1.0f }, { 0.0f, 1.0f }, { 1.0f, 0.0f, 0.0f }},
	{ {  0.5f, -0.5f, -0.5f }, { 0.0f, 0.0f, -1.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f, 0.0f }},
	{ {  0.5f,  0.5f, -0.5f }, { 0.0f, 0.0f, -1.0f }, { 1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f }},
	{ { -0.5f,  0.5f, -0.5f }, { 0.0f, 0.0f, -1.0f }, { 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f }}
};

const std::vector<uint32_t> quadIndices =
{
	0, 1, 2,
	2, 3, 0,
	4, 5, 6,
	6, 7, 4
};

struct UniformBufferObject
{
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 projection;
};

const uint32_t WindowWidth = 1600;
const uint32_t WindowHeight = 900;

const std::vector<const char*> ValidationLayers = { "VK_LAYER_KHRONOS_validation" };
const std::vector<const char*> DeviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

const std::string ResourceBase = "../Assets/";

static ImGui_ImplVulkanH_Window g_MainWindowData;

const int32_t MAX_FRAMES_IN_FLIGHT = 2;

static bool rightMouseButtonDown = false;

static glm::vec2 lastMousePosition;

#ifdef NDEBUG
const bool EnableValidationLayers = false;
#else
const bool EnableValidationLayers = true;
#endif // NDEBUG

#define THROW_ERROR(message) throw std::runtime_error(message);

std::string VkResultToString(VkResult result);

#define VkCheck(result, message) if (result != VK_SUCCESS) { fmt::print("{}\n", VkResultToString(result)); THROW_ERROR(message) }

#define VectorSize(type, vector) sizeof(type) * vector.size()

// Our state
static bool showDemoWindow = true;
static bool showAnotherWindow = false;
static ImVec4 clearColor = ImVec4(0.4f, 0.6f, 0.9f, 1.0f);

static void checkVkResult(VkResult err)
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
	std::optional<uint32_t> transferFamily = 0;

	bool isComplete()
	{
		return graphicsFamily.has_value() && presentFamily.has_value() && transferFamily.has_value();
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
	void createSwapChain();
	void recreateSwapChain();
	void cleanupSwapChain();
	void createImageViews();
	void createRenderPass();
	void createDescriptorSetLayout();
	void createGraphicsPipeline();
	void createFramebuffers();
	void createGraphicsCommandPool();
	void createTransferCommandPool();
	void createDepthResources();
	void createTextureImage();
	void createTextureImageView();
	void createTextureSampler();
	void createVertexBuffer(const std::vector<Vertex>& vertices);
	void createIndexBuffer(const std::vector<uint32_t>& indices);
	void createUniformBuffers();
	void createCommandBuffers();
	void createDescriptorPool();
	void createDescriptorSets();
	void createSyncObjects();
	void recordCommandBuffer(VkCommandBuffer inCommandBuffer, uint32_t imageIndex);

	VkShaderModule createShaderModule(const std::vector<char>& shaderCode);

	void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags propertyFlags, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
	void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

	void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling,
					 VkImageUsageFlags usage, VkMemoryPropertyFlags propertyFlags, VkImage& image, VkDeviceMemory& imageMemory);

	VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);

	void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);

	void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

	VkCommandBuffer beginSingleTimeCommands();

	void endSingleTimeCommands(VkCommandBuffer inCommandBuffer);

	void loadResources();

	void updateFPSCounter();
	void updateUniformBuffer(uint32_t frameIndex);

	void initWindow();
	void initVulkan();
	void initImGui();

	void updateImGui();
	void createImGuiWidgets();
	void renderImGui();
	void shutdownImGui();

	bool isDeviceSuitable(VkPhysicalDevice inDevice);

	int32_t rateDeviceSuitability(VkPhysicalDevice inDevice);

	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice inDevice);

	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice inDevice);

	VkSurfaceFormatKHR chooseSwapChainSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);

	VkPresentModeKHR chooseSwapChainPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);

	VkExtent2D chooseSwapChainExtent(const VkSurfaceCapabilitiesKHR& capabilities);

	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags propertyFlags);

	VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

	VkFormat findDepthFormat();

	bool hasStencilComponent(VkFormat format) { return format == VK_FORMAT_D32_SFLOAT_S8_UINT || 
													   format == VK_FORMAT_D24_UNORM_S8_UINT; };

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

	static void framebufferResizeCallback(GLFWwindow* inWindow, int width, int height);
	static void mouseMoveCallback(GLFWwindow* inWindow, double xpos, double ypos);
	static void mouseScrollCallback(GLFWwindow* inWindow, double xoffset, double yoffset);
	static void mouseButtonCallback(GLFWwindow* inWindow, int32_t button, int32_t action, int32_t mods);

private:
	struct GLFWwindow* window;

	VkAllocationCallbacks* allocator;

	VkInstance instance;
	VkPhysicalDevice physicalDevice;
	VkDevice device;
	VkQueue graphicsQueue;
	VkQueue presentQueue;
	VkQueue transferQueue;
	VkSurfaceKHR surface;
	VkSwapchainKHR swapChain;
	VkRenderPass renderPass;
	VkDescriptorSetLayout descriptorSetLayout;
	VkPipelineLayout pipelineLayout;
	VkPipeline graphicsPipeline;
	VkCommandPool graphicsCommandPool;
	VkCommandPool transferCommandPool;
	VkBuffer vertexBuffer;
	VkDeviceMemory vertexBufferMemory;
	VkBuffer indexBuffer;
	VkDeviceMemory indexBufferMemory;
	VkDescriptorPool descriptorPool;
	VkImage textureImage;
	VkDeviceMemory textureImageMemory;
	VkImageView textureImageView;
	VkSampler textureSampler;
	VkImage depthImage;
	VkDeviceMemory depthImageMemory;
	VkImageView depthImageView;
	std::vector<VkDescriptorSet> descriptorSets;
	std::vector<VkBuffer> uniformBuffers;
	std::vector<VkDeviceMemory> uniformBuffersMemory;
	std::vector<void*> uniformBuffersMapped;
	std::vector<VkCommandBuffer> commandBuffers;
	std::vector<VkSemaphore> imageAvailableSemaphores;
	std::vector<VkSemaphore> renderFinishedSemaphores;
	std::vector<VkFence> inFlightFences;
	std::vector<VkImage> swapChainImages;
	std::vector<VkImageView> swapChainImageViews;
	std::vector<VkFramebuffer> swapChainFramebuffers;
	VkFormat swapChainImageFormat;
	VkExtent2D swapChainExtent;
	VkDebugUtilsMessengerEXT debugMessenger;

	uint32_t minImageCount;
	uint32_t imageCount;
	uint32_t currentFrame = 0;
	bool framebufferResized;

	float deltaTime;
	int32_t frameCount;

	Model model;
	Camera camera{{ 0.0f, 1.0f, 3.0f }};
};