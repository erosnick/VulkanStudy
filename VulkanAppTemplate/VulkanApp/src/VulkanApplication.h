#pragma once

#define VK_NO_PROTOTYPES
#include <volk/volk.h>

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
#include "SimpleModel.h"
#include "DebugUtil.h"

#include <vk_mem_alloc.h>

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

constexpr uint32_t LightCount = 16;

static uint32_t turnOnLightCount = LightCount;

static glm::vec4 lightPositions[] = {
	glm::vec4(-10.0f,  10.0f, 10.0f, 1.0f),
	glm::vec4( 10.0f,  10.0f, 10.0f, 1.0f),
	glm::vec4(-10.0f, -10.0f, 10.0f, 1.0f),
	glm::vec4( 10.0f, -10.0f, 10.0f, 1.0f),
	glm::vec4(-30.0f,  10.0f, 10.0f, 1.0f),
	glm::vec4( 30.0f,  10.0f, 10.0f, 1.0f),
	glm::vec4(-30.0f, -10.0f, 10.0f, 1.0f),
	glm::vec4( 30.0f, -10.0f, 10.0f, 1.0f),
	glm::vec4(-10.0f,  10.0f, 0.0f, 1.0f),
	glm::vec4( 10.0f,  10.0f, 0.0f, 1.0f),
	glm::vec4(-10.0f, -10.0f, 0.0f, 1.0f),
	glm::vec4( 10.0f, -10.0f, 0.0f, 1.0f),
	glm::vec4(-30.0f,  10.0f, 0.0f, 1.0f),
	glm::vec4( 30.0f,  10.0f, 0.0f, 1.0f),
	glm::vec4(-30.0f, -10.0f, 0.0f, 1.0f),
	glm::vec4( 30.0f, -10.0f, 0.0f, 1.0f)
};
const glm::vec4 lightColors[] = {
	glm::vec4(300.0f, 300.0f, 300.0f, 1.0f),
	glm::vec4(300.0f, 300.0f, 300.0f, 1.0f),
	glm::vec4(300.0f, 300.0f, 300.0f, 1.0f),
	glm::vec4(300.0f, 300.0f, 300.0f, 1.0f),
	glm::vec4(300.0f, 300.0f, 300.0f, 1.0f),
	glm::vec4(300.0f, 300.0f, 300.0f, 1.0f),
	glm::vec4(300.0f, 300.0f, 300.0f, 1.0f),
	glm::vec4(300.0f, 300.0f, 300.0f, 1.0f),
	glm::vec4(300.0f, 300.0f, 300.0f, 1.0f),
	glm::vec4(300.0f, 300.0f, 300.0f, 1.0f),
	glm::vec4(300.0f, 300.0f, 300.0f, 1.0f),
	glm::vec4(300.0f, 300.0f, 300.0f, 1.0f),
	glm::vec4(300.0f, 300.0f, 300.0f, 1.0f),
	glm::vec4(300.0f, 300.0f, 300.0f, 1.0f),
	glm::vec4(300.0f, 300.0f, 300.0f, 1.0f),
	glm::vec4(300.0f, 300.0f, 300.0f, 1.0f)
};

constexpr uint32_t ParticleCount = 2048;

static float particleSpeed = 5000.0f;

struct Particle
{
	static VkVertexInputBindingDescription getBindingDescription() 
	{
		VkVertexInputBindingDescription bindingDescription{};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(Particle);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return bindingDescription;
	}

	static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions() 
	{
		std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};

		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(Particle, position);

		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32A32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Particle, color);

		return attributeDescriptions;
	}

	glm::vec2 position;
	glm::vec2 velocity;
	glm::vec4 color;
};

struct GlobalUniformBufferObject
{
	glm::mat4 view;
	glm::mat4 projection;
	glm::vec4 cameraPosition;
};

struct ObjectUniformBufferObject
{
	glm::mat4 model;
};

struct MaterialUniformBufferObject
{
	glm::vec4 albedo = glm::vec4(1.0f, 1.0f, 1.0f, 0.0f);
	float metallic = 0.0f;
	float roughness = 0.5f;
	float ao = 1.0f;
	int32_t diffuseTextureIndex = 0;
	int32_t alphaTextureIndex = 0;
};

struct LightUniformBufferObject
{
	glm::vec4 lightPositions[LightCount];
	glm::vec4 lightColors[LightCount];
	uint32_t turnOnLightCount = 0;
};

struct ParticleUniformBufferObject
{
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 projection;
	float deltaTime;
};

enum class Channel : int32_t
{
	Default = 0, // only used for desired_channels

	Grey = 1,
	GreyAlpha = 2,
	RGB = 3,
	RGBAlpha = 4
};

const uint32_t WindowWidth = 1600;
const uint32_t WindowHeight = 900;

const std::vector<const char*> ValidationLayers = { "VK_LAYER_KHRONOS_validation" };
const std::vector<const char*> DeviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_KHR_MAINTENANCE_4_EXTENSION_NAME };

const std::string ResourceBase = "../Assets/";

static ImGui_ImplVulkanH_Window g_MainWindowData;

const int32_t MAX_FRAMES_IN_FLIGHT = 2;

static bool rightMouseButtonDown = false;
static bool middleMouseButtonDown = false;

static glm::vec2 lastMousePosition;

const float FrameTime = 0.0166667f;
static float deltaTime = 0.0f;

static float lastFrameTime = 0.0f;

static double lastTime = 0.0;

const int32_t numOfRows = 7;
const int32_t numOfColumns = 7;
const float spacing = 2.5f;

const int32_t TextureUnits = 64;

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
static ImVec4 clearColor = ImVec4(0.1f, 0.1f, 0.1f, 1.0f);
//static ImVec4 clearColor = ImVec4(0.4f, 0.6f, 0.9f, 1.0f);

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
	std::optional<uint32_t> graphicsAndComputeFamily = 0;
	std::optional<uint32_t> presentFamily = 0;
	std::optional<uint32_t> transferFamily = 0;

	bool isComplete()
	{
		return graphicsAndComputeFamily.has_value() && presentFamily.has_value() && transferFamily.has_value();
	}
};

struct SwapChainSupportDetails
{
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};

struct MeshGeometry
{
	VkBuffer vertexBuffer = VK_NULL_HANDLE;
	VkDeviceMemory vertexBufferMemory = VK_NULL_HANDLE;
	VkBuffer indexBuffer = VK_NULL_HANDLE;
	VkDeviceMemory indexBufferMemory = VK_NULL_HANDLE;
	uint32_t textureImageViewIndex = 0;
	uint32_t vertexCount = 0;
	uint32_t indexCount = 0;
	uint32_t indexStartIndex = 0;
	bool hasTexture = false;
	bool hasAlphaTexture = false;
	bool dirty = true;
	std::shared_ptr<Material> material;
	glm::mat4 transform = glm::mat4(1.0f);
};

class VulkanApplication
{
public:
	VulkanApplication();
	~VulkanApplication();

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
	void createGraphicsDescriptorSetLayout();
	void createComputeDescriptorSetLayout();
	void createGraphicsPipeline();
	void createComputePipeline();
	void createFramebuffers();
	void createGraphicsCommandPool();
	void createTransferCommandPool();
	void createColorResources();
	void createDepthResources();
	VkImage createTextureImage(VkDeviceMemory& imageMemory, const std::string& path, Channel requireChannels = Channel::RGBAlpha);
	VkImageView createTextureImageView(VkImage image);
	void createTextureSampler();
	VkBuffer createVertexBuffer(const std::vector<Vertex>& vertices, VkDeviceMemory& vertexBufferMemory);
	VkBuffer createIndexBuffer(const std::vector<uint32_t>& indices, VkDeviceMemory& indexBufferMemory);
	std::unique_ptr<MeshGeometry> createMeshGeometry(const Mesh& mesh);
	std::unique_ptr<MeshGeometry> createMeshGeometry(const SimpleMeshInfo& mesh, const SimpleMaterialInfo& material);
	void createMeshGeometries(const Model& model);
	void createMeshGeometries(const SimpleModel& model);
	void createGlobalUniformBuffers();
	void createObjectUniformBuffers();
	void createMaterialUniformBuffers();
	void createLightUniformBuffers();
	void createShaderStorageBuffers();
	void createParticleUniformBuffers();
	void createGraphicsCommandBuffers();
	void createComputeCommandBuffers();
	void createDescriptorPool();
	void createGraphicsDescriptorSets();
	void createComputeDescriptorSets();
	void createSyncObjects();

	SimpleModel mergeModels(const std::vector<SimpleModel>& models);

	void recordGraphicsCommandBuffer(VkCommandBuffer graphicsCommandBuffer, uint32_t imageIndex);
	void recordComputeCommandBuffer(VkCommandBuffer computeCommandBuffer);

	VkShaderModule createShaderModule(const std::vector<char>& shaderCode);

	void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags propertyFlags, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
	void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

	void createImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling,
		VkImageUsageFlags usage, VkMemoryPropertyFlags propertyFlags, VkImage& image, VkDeviceMemory& imageMemory);

	VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels);

	void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels);

	void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

	void createVmaAllocator();

	VkCommandBuffer beginSingleTimeCommands();

	void endSingleTimeCommands(VkCommandBuffer inCommandBuffer);

	void generateMipmaps(VkImage image, VkFormat imageFormat, int32_t width, int32_t height, uint32_t mipLevels);

	void loadResources();

	void createTextureImageViews();

	void updateFPSCounter();
	void updateGlobalUniformBuffer(uint32_t frameIndex);
	void updateObjectUniformBuffer(uint32_t frameIndex, uint32_t index, const ObjectUniformBufferObject& objectUniformBufferObject);
	void updateMaterialUniformBuffer(uint32_t frameIndex, uint32_t index, const MaterialUniformBufferObject& materialUniformBufferObject);
	void updateLightUniformBuffer(uint32_t frameIndex);
	void updateParticleUniformBuffer(uint32_t frameIndex);

	void updateImageView(VkImageView imageView, VkImageView alphaImageView, uint32_t frameIndex);

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

	bool hasStencilComponent(VkFormat format) {
		return format == VK_FORMAT_D32_SFLOAT_S8_UINT ||
			format == VK_FORMAT_D24_UNORM_S8_UINT;
	};

	void mainLoop();
	void update();
	void drawFrame();
	void cleanup();

	void processInput(float deltaTime);

	bool checkExtensionSupport();
	bool checkDeviceExtensionSupport(VkPhysicalDevice inDevice);
	bool checkValidationLayerSupport();
	std::vector<const char*> getRequiredExtensions();

	VkSampleCountFlagBits getMaxUsableSampleCount();

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
	VkQueue computeQueue;
	VkQueue presentQueue;
	VkQueue transferQueue;
	VkSurfaceKHR surface;
	VkSwapchainKHR swapChain;
	VkRenderPass renderPass;
	VkDescriptorSetLayout graphicsDescriptorSetLayout;
	VkDescriptorSetLayout computeDescriptorSetLayout;
	VkPipelineLayout graphicsPipelineLayout;
	VkPipelineLayout particlePipelineLayout;
	VkPipelineLayout computePipelineLayout;
	VkPipeline graphicsPipeline;
	VkPipeline particlePipeline;
	VkPipeline computePipeline;
	VkCommandPool graphicsCommandPool;
	VkCommandPool transferCommandPool;
	VkDescriptorPool descriptorPool;
	VkImage textureImage;
	VkDeviceMemory textureImageMemory;
	VkImageView textureImageView;
	VkSampler textureSampler;
	VkImage depthImage;
	VkDeviceMemory depthImageMemory;
	VkImageView depthImageView;
	VkBuffer vertexBuffer;
	VkImage colorImage;
	VkDeviceMemory colorImageMemory;
	VkImageView colorImageView;
	VkDeviceMemory vertexBufferMemory;
	VkBuffer indexBuffer;
	VkDeviceMemory indexBufferMemory;
	std::vector<VkDescriptorSet> graphicsDescriptorSets;
	std::vector<VkDescriptorSet> computeDescriptorSets;
	std::vector<VkBuffer> globalUniformBuffers;
	std::vector<VkDeviceMemory> globalUniformBuffersMemory;
	std::vector<void*> globalUniformBuffersMapped;
	std::vector<VkBuffer> objectUniformBuffers;
	std::vector<VkDeviceMemory> objectUniformBuffersMemory;
	std::vector<void*> objectUniformBuffersMapped;
	std::vector<VkBuffer> materialUniformBuffers;
	std::vector<VkDeviceMemory> materialUniformBuffersMemory;
	std::vector<void*> materialUniformBuffersMapped;
	std::vector<VkBuffer> lightUniformBuffers;
	std::vector<VkDeviceMemory> lightUniformBuffersMemory;
	std::vector<void*> lightUniformBuffersMapped;
	std::vector<VkBuffer> particleUniformBuffers;
	std::vector<VkDeviceMemory> particleUniformBuffersMemory;
	std::vector<void*> particleUniformBuffersMapped;
	std::vector<VkCommandBuffer> graphicsCommandBuffers;
	std::vector<VkCommandBuffer> computeCommandBuffers;
	std::vector<VkSemaphore> imageAvailableSemaphores;
	std::vector<VkSemaphore> renderFinishedSemaphores;
	std::vector<VkSemaphore> computeFinishedSemaphores;
	std::vector<VkFence> computeInFlightFences;
	std::vector<VkFence> graphicsInFlightFences;
	std::vector<VkImage> swapChainImages;
	std::vector<VkImageView> swapChainImageViews;
	std::vector<VkFramebuffer> swapChainFramebuffers;
	std::vector<VkImage> textureImages;
	std::vector<VkDeviceMemory> textureImageMemories;
	std::vector<VkImageView> textureImageViews;
	std::vector<VkBuffer> shaderStorageBuffers;
	std::vector<VkDeviceMemory> shaderStorageBuffersMemory;
	VkFormat swapChainImageFormat;
	VkExtent2D swapChainExtent;
	VkDebugUtilsMessengerEXT debugMessenger;

	uint32_t minImageCount;
	uint32_t imageCount;
	uint32_t currentFrame = 0;
	bool framebufferResized;

	int32_t frameCount;

	Model model;
	//Camera camera{ glm::vec3(0.0f, 20.0f, 14.0f) };
	Camera camera{ glm::vec3(8.0f, 15.0f, -3.0f), glm::vec3(0.0f, 1.0f, 0.0f), -180.0f };
	//Camera camera{ glm::vec3(0.0f, 0.0f, 24.0f) };
	//Camera camera{ glm::vec3(0.0f, 0.0f, 5.0f) };

	std::vector<std::unique_ptr<MeshGeometry>> meshGeometries;
	std::vector<VkImageView> imageViews;

	SimpleModel sponza;
	SimpleModel cube;
	SimpleModel sphere;
	SimpleModel marry;
	SimpleModel mergedModel;
	
	std::vector<SimpleModel> models;

	size_t materialUniformBufferAlignment = 0;
	size_t objectUniformBufferAlignment = 0;
	uint32_t mipLevels = 1;

	bool anisotropyEnable = true;

	VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;

	std::vector<std::string> textureImagePaths;

	DebugUtil debugUtil;

	VmaAllocator vmaAllocator;
};