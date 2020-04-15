#pragma once

#include <string>
#include <vector>
#include <optional>
#include <array>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtx/hash.hpp>

struct QueueFamilyIndices
{
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;
	std::optional<uint32_t> transferFamily;

	bool isComplete()
	{
		return graphicsFamily.has_value() && presentFamily.has_value() && transferFamily.has_value();
	}
};

struct SwapChainSupportDetails
{
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModeS;
};

const std::vector<const char*> validationLayers = { 
	
	"VK_LAYER_KHRONOS_validation",
	"VK_LAYER_LUNARG_standard_validation",
	//"VK_LAYER_RENDERDOC_Capture"
};

const std::vector<const char*> deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

struct Vertex
{
	glm::vec3 position;
	glm::vec3 color;
	glm::vec2 texCoord;
	glm::vec3 normal;

	static VkVertexInputBindingDescription getBindingDescription()
	{
		VkVertexInputBindingDescription bindingDescription = {};

		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(Vertex);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return bindingDescription;
	}

	static std::array<VkVertexInputAttributeDescription, 4> getAttributeDescription()
	{
		std::array<VkVertexInputAttributeDescription, 4> attributeDescriptions = {};

		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(Vertex, position);

		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Vertex, color);

		attributeDescriptions[2].binding = 0;
		attributeDescriptions[2].location = 2;
		attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

		attributeDescriptions[3].binding = 0;
		attributeDescriptions[3].location = 3;
		attributeDescriptions[3].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[3].offset = offsetof(Vertex, normal);

		return attributeDescriptions;
	}

	bool operator==(const Vertex& other) const
	{
		return position == other.position &&
			   color == other.color &&
			   texCoord == other.texCoord &&
			   normal == other.normal;
	}
};

namespace std
{
	template<> struct hash<Vertex>
	{
		size_t operator()(Vertex const& vertex) const
		{
			return ((hash<glm::vec3>()(vertex.position) ^
					(hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^
					(hash<glm::vec2>()(vertex.texCoord) << 1);
		}
	};
}

struct UniformBufferObject
{
	alignas(16) glm::mat4 model;
	alignas(16) glm::mat4 view;
	alignas(16) glm::mat4 projection;
};

struct LightDataBuffer
{
	alignas(16) glm::vec3 lightPosition;
};

struct Data
{
	glm::mat4 model;
	glm::float32 furLength;
	glm::float32 layer;
	glm::float32 gravity;
	glm::int32 layerIndex;
	glm::float32 time;
};

struct DynamicUniformBuffer
{
	Data* data = nullptr;
};

struct TextureAsset
{
	VkImage image;
	VkImageView imageView;
	VkDeviceMemory memory;
};

#ifdef NDEBUG
	const bool enableValidationLayers = true;
#else
	const bool enableValidationLayers = true;
#endif

class Application
{
public:

	Application(int inWindowWidth, int inWindowHeight, const std::string title);

	void createWindow(int inWindowWidth, int inWindowHeight, const std::string title);
	static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
	static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
	static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);

	void initializeVulkan();
	bool checkValidationLayerSupport();
	void queryInstanceExtensions();
	std::vector<const char*> getRequiredExtensions();
	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
	void setupDebugMessenger();
	void createSurface();
	void createInstance();
	void queryDeviceLayers();
	bool checkDeviceExtensionSupport(VkPhysicalDevice device);
	bool isDeviceSuitable(VkPhysicalDevice device);
	int rateDeviceSuitability(VkPhysicalDevice device);
	void pickPhysicalDevice();
	void queryDeviceProperties();
	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
	void createLogicalDevice();
	SwapChainSupportDetails querySwapCahinSupport(VkPhysicalDevice device);
	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
	void createSwapChain();
	void cleanupSwapChain();
	void recreateSwapChain();
	void createImageViews();
	VkShaderModule createShaderModule(const std::vector<char>& code);
	void createRenderPass();
	void createDescriptorSetLayout();
	VkPipelineShaderStageCreateInfo loadShader(const std::string file, VkShaderStageFlagBits stage);
	void createGraphicsPipeline();
	void createFramebuffers();
	void createCommandPool();
	VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
	VkFormat findDepthFormat();
	bool hasStencilComponent(VkFormat format);
	void createDepthResources();
	void createColorResources();
	void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, 
					 VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory, uint32_t mimLevels, VkSampleCountFlagBits numSamples = VK_SAMPLE_COUNT_1_BIT);
	void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels);
	void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
	void prepareTextureImages();
	void createTextureImage(std::string filePath, VkImage& image, VkDeviceMemory& imageMemory, bool generateMip, uint32_t& mipLevels);
	void createCustomTextureImage(uint32_t textureWidth, uint32_t textureHeight, VkImage& image, VkDeviceMemory& imageMemory, bool generateMip, uint32_t& mipLevels);
	void createCheckerboardTextureImage(uint32_t textureWidth, uint32_t textureHeight, VkImage& image, VkDeviceMemory& imageMemory, bool generateMip, uint32_t& mipLevels);
	VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels);
	void createTextureImageView();
	void createTextureSampler(VkSampler& sampler, uint32_t mipLevels);
	void prepareTextureSamplers();
	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
	void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
	void createBuffer(VkDeviceSize bufferSize, VkBufferUsageFlags usage, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
	VkCommandBuffer beginSingleTimeCommands(VkCommandPool commandPool);
	void endSingleTimeCommands(VkCommandBuffer commandBuffer, VkQueue queue, VkCommandPool commandPool);
	void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
	void loadModel();
	void createVertexBuffer(const std::vector<Vertex>& vertices, VkBuffer& vertexBuffer, VkDeviceMemory& vertexBufferMemory);
	void createIndexBuffer(const std::vector<uint32_t>& indices, VkBuffer& indexBuffer, VkDeviceMemory& indexBufferMemory);

	template<class T>
	void createBuffer(T dataArray, VkBufferUsageFlags usage, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
	{
		VkDeviceSize bufferSize = sizeof(dataArray[0]) * dataArray.size();

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;

		createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
								 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |	// RAM
								 VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
								 stagingBuffer, stagingBufferMemory);

		void* data = nullptr;
		vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy_s(data, (size_t)bufferSize, dataArray.data(), (size_t)bufferSize);
		vkUnmapMemory(device, stagingBufferMemory);

		createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT |
								 usage,
								 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,	// VRAM
								 buffer, bufferMemory);

		copyBuffer(stagingBuffer, buffer, bufferSize);

		vkDestroyBuffer(device, stagingBuffer, nullptr);
		vkFreeMemory(device, stagingBufferMemory, nullptr);
	}

	void prepareVertexBufferAndIndexBuffer();
	void prepareModelResources();
	void createAllInOneBuffer(const std::vector<Vertex>& vertices, 
							  const std::vector<uint32_t>& indeices,
							  VkBuffer& buffer, VkDeviceMemory& bufferMemory);
	void prepareGeometryBuffers();
	void prepareModelBuffers();
	void createUniformBuffers();
	void prepareDynamicUniformBuffers();
	void createDescriptorPool();
	void createDescriptorSets();
	void updateUniformBuffer(uint32_t currentImage);
	void setupViewport(size_t index);
	void createCommandBuffers();
	void createTransferCommandBuffers();
	void createSyncObjects();
	void generateMipmaps(VkImage image, VkFormat imageFormat, int32_t textureWidth, int32_t textureHeight, uint32_t mipLevels);
	VkSampleCountFlagBits getMaxUsableSampleCount();
	void drawFrame();
	void mainLoop();
	void cleanup();
	~Application();

	void run();

	float rotateAngle = 0.0f;
	const float cameraSpeed = 0.01f;
	glm::vec3 eyePosition = glm::vec3(0.0f, 0.0f, 0.75f);
	glm::vec3 center = glm::vec3(0.0f, 0.0f, 0.f);;
	glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
	DynamicUniformBuffer dynamicUniformBuffer;

protected:

	uint32_t windowWidth;
	uint32_t windowHeight;
	std::string windowTitle;
	GLFWwindow* window = nullptr;

	VkInstance instance = VK_NULL_HANDLE;
	VkDevice device = VK_NULL_HANDLE;
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;	// VkPhysicalDevice会在VkInstance销毁的时候销毁。

	VkDebugUtilsMessengerEXT debugMessenger = VK_NULL_HANDLE;

	VkQueue graphicsQueue = VK_NULL_HANDLE;
	VkQueue presentQueue = VK_NULL_HANDLE;
	VkQueue transferQueue = VK_NULL_HANDLE;
	VkSurfaceKHR surface = VK_NULL_HANDLE;
	VkSwapchainKHR swapChain = VK_NULL_HANDLE;
	std::vector<VkImage> swapChainImages;
	std::vector<VkImageView> swapChainImageViews;

	VkFormat swapChainImageFormat = VK_FORMAT_UNDEFINED;
	VkExtent2D swapChainExtent = { 0, 0 };
	VkRenderPass renderPass = VK_NULL_HANDLE;
	VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
	VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
	VkPipelineLayout basicPipelineLayout = VK_NULL_HANDLE;
	VkPipeline graphicsPipeline = VK_NULL_HANDLE;
	VkPipeline furGraphicPipeline = VK_NULL_HANDLE;
	VkPipeline furShadowGraphicPipeline = VK_NULL_HANDLE;
	VkPipeline normalDebugGraphicPipeline = VK_NULL_HANDLE;
	std::vector<VkFramebuffer> swapChainFramebuffers;
	VkCommandPool graphicsCommandPool = VK_NULL_HANDLE;
	VkCommandPool transferCommandPool = VK_NULL_HANDLE;
	std::vector<VkCommandBuffer> commandBuffers;
	VkSemaphore imageAvailableSemaphore = VK_NULL_HANDLE;
	VkSemaphore renderFinishedSemaphore = VK_NULL_HANDLE;
	// Each frame should have its own set of semaphores
	std::vector<VkSemaphore> imageAvailableSemaphores;
	std::vector<VkSemaphore> renderFinishedSemaphores;
	std::vector<VkFence> inFlightFences;
	std::vector<VkFence> imagesInFlight;
	size_t currentFrame = 0;

	bool framebufferResized = false;

	std::vector<Vertex> geometryVertices = {
		{{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
		{{ 0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
		{{ 0.5f,  0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}},
		{{-0.5f,  0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}},

		{{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
		{{ 0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
		{{ 0.5f,  0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}},
		{{-0.5f,  0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}}
	};

	std::vector<uint32_t> geometryIndices = {
		0, 1, 2, 2, 3, 0,
		4, 5, 6, 6, 7, 4
	};

	std::vector<Vertex> modelVertices;
	std::vector<uint32_t> modelIndices;

	VkBuffer geometryVertexBuffer = VK_NULL_HANDLE;
	VkDeviceMemory geometryVertexBufferMemory = VK_NULL_HANDLE;

	VkBuffer modelVertexBuffer = VK_NULL_HANDLE;
	VkDeviceMemory modelVertexBufferMemory = VK_NULL_HANDLE;

	VkBuffer modelIndexBuffer = VK_NULL_HANDLE;
	VkDeviceMemory modelIndexBufferMemory = VK_NULL_HANDLE;

	VkBuffer geometryIndexBuffer = VK_NULL_HANDLE;
	VkDeviceMemory geometryIndexBufferMemory = VK_NULL_HANDLE;

	VkBuffer allInOneBuffer = VK_NULL_HANDLE;
	VkDeviceMemory allInOneBufferMemory = VK_NULL_HANDLE;

	// Model Vertex & Index buffer
	VkBuffer modelAllInOneBuffer = VK_NULL_HANDLE;
	VkDeviceMemory modelAllInOneBufferMemory = VK_NULL_HANDLE;

	std::vector<VkBuffer> uniformBuffers;
	std::vector<VkDeviceMemory> uniformBufferMemorys;

	std::vector<VkBuffer> lightDataBuffers;
	std::vector<VkDeviceMemory> lightDataBufferMemorys;

	std::vector<VkBuffer> dynamicUniformBuffers;
	std::vector<VkDeviceMemory> dynamicUniformBufferMemorys;

	VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
	std::vector<VkDescriptorSet> descriptorSets;
	std::vector<VkDescriptorSet> testDescriptorSets;

	VkImage geometryTextureImage = VK_NULL_HANDLE;
	VkImageView geometryTextureImageView = VK_NULL_HANDLE;
	VkSampler geometryTextureSampler = VK_NULL_HANDLE;
	VkDeviceMemory geometryTextureDeviceMemory = VK_NULL_HANDLE;
	VkSampler testSampler = VK_NULL_HANDLE;
	
	VkImage depthImage = VK_NULL_HANDLE;
	VkImageView depthImageView = VK_NULL_HANDLE;
	VkDeviceMemory depthImageMemory = VK_NULL_HANDLE;

	VkImage modelTextureImage = VK_NULL_HANDLE;
	VkImageView modelTextureImageView = VK_NULL_HANDLE;
	VkSampler modelTextureSampler = VK_NULL_HANDLE;
	VkDeviceMemory modelTextureImageMemory = VK_NULL_HANDLE;

	std::vector<TextureAsset> textures;

	VkImage colorImage = VK_NULL_HANDLE;
	VkDeviceMemory colorImageMemory = VK_NULL_HANDLE;
	VkImageView colorImageView = VK_NULL_HANDLE;

	uint32_t geometryMipLevels = 0;
	uint32_t modelMipLevels = 0;
	uint32_t dynamicAlignment = 0;
	VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;
	std::vector<VkShaderModule> shaderModules;
};