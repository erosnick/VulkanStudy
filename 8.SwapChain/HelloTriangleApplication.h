#pragma once

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <stdexcept>
#include <functional>
#include <vector>

const int WIDTH = 800;
const int HEIGHT = 600;

// Vulkan API����ƺ����Ǿ�����С����������Ķ��⿪������ν���⿪���������ָ����Ⱦ��������㡣
// ����һ������ı��־���Ĭ�������£�Vulkan API�Ĵ������֧�ַǳ����ޡ���ʹ��������ȷ��ֵ����
// ����Ҫ�Ĳ�������Ϊ��ָ�룬Ҳ��������ȷ�Ĵ����߼�������ֱ�ӵ��±�������δ������쳣��Ϊ��֮����
// ����������ΪVulkanҪ��ÿһ�����趨�嶼�ǳ���ȷ�����º��������С��������ʹ���µ�GPU���ܣ�
// �����������߼��豸����ʱ��������

// Validation Layers�ǿ�ѡ��������Թ��ڵ�Vulkan�����е��ã��Իص������Ĳ�����Validation Layers�ĳ��������龰�У�
// 1.���ݹ淶��������ֵ������ȷ���Ƿ������Ԥ�ڲ��������
// 2.���ٶ���Ĵ��������٣��Բ����Ƿ������Դ��й¶
// 3.�����̵߳ĵ�������ȷ���߳�ִ�й����еİ�ȫ��
// 4.��ÿ�κ���������ʹ�õĲ�����¼����׼������У����г�����Vulkan��Ҫ����

// Vulkan SDK�ṩ�ı�׼��ϲ㡣
const std::vector<const char*> ValidationLayers = { "VK_LAYER_LUNARG_standard_validation" };

// Swap Chain��չ��
const std::vector<const char*> DeviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

#ifdef NDEBUG
	const bool EnableValidationLayers = false;
#else
	const bool EnableValidationLayers = true;
#endif

struct QueueFamilyIndices
{
	int GraphicsFamily = -1;
	int PresentFamily = -1;

	bool IsComplete()
	{
		return GraphicsFamily >= 0 && PresentFamily >= 0;
	}
};

// ����SwapChain��������ϸ��Ϣ��
struct SwapChainSupprotDetails
{
	VkSurfaceCapabilitiesKHR Capabilities;
	std::vector<VkSurfaceFormatKHR> Formats;
	std::vector<VkPresentModeKHR> PresentModes;
};

class HelloTriangleApplication
{
public:

	// ������Ⱦѭ����
	void Run();

	~HelloTriangleApplication();

private:

	// ����VkInstance��
	void CreateInstance();

	// �����֧�ֵ���չ�����
	void CheckExtensions();

	// ���Device��չ��֧�������
	bool CheckDeviceExtensionSupport(VkPhysicalDevice Device);

	// ���Validation Layer��֧�������
	bool CheckValidationLayerSupport();

	// ��ȡ��Ҫ����չ��
	std::vector<const char*> GetRequiredExtensions();

	// ��������Validdation Layer�Ļص�������
	void SetupDebugCallback();

	void CreateSurface();

	// Validation Layer�Ļص�������ֻ�����������ڳ�����ʱ��û㱨��Ϣ��
	static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugReportFlagsEXT Flags,
		VkDebugReportObjectTypeEXT ObjectType,
		uint64_t Object,
		size_t Location,
		int32_t Code,
		const char* LayerPrefix,
		const char* Message,
		void* UserData)
	{
		std::cerr << "Validation layer: " << Message << std::endl;

		return VK_FALSE;
	}

	// ��ʼ��GLFW����ϵͳ��
	void InitWindow();

	// ��ʼ��Vulkan�⡣
	void InitVulkan();

	// ��ȡ���õ������豸(�Կ�)��
	void PickPhysicalDevice();

	// ��������豸�Ƿ����á�
	bool IsDeviceSuitable(VkPhysicalDevice Device);

	// ��ȡ���м��塣
	QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice Device);

	// ��ȡSwapChain֧��ϸ�ڡ�
	SwapChainSupprotDetails QuerySwapChainSupport(VkPhysicalDevice Device);

	// ѡ��Surface�ĸ�ʽ��
	// ÿ��VkSurfaceFormatKHR�ṹ������һ��format��һ��colorSpace��Ա��format��Ա����ָ��ɫ��ͨ�������͡����磬VK_FORMAT_B8G8R8A8_UNORM����������ʹ��B, G, R��alpha�����ͨ������ÿһ��ͨ��Ϊ�޷���8bit������ÿ�������ܼ�32bits��colorSpace��Ա����SRGB��ɫ�ռ��Ƿ�ͨ��VK_COLOR_SPACE_SRGB_NONLINEAR_KHR��־֧��
	VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& AvailableForamt);

	// ѡ�񽻻����ĳ���ģʽ��
	// Presentationģʽ���ڽ������Ƿǳ���Ҫ�ģ���Ϊ������������Ļ����ͼ�����������Vulkan�����ĸ�ģʽ����ʹ��:

	// 1.VK_PRESENT_MODE_IMMEDIATE_KHR: Ӧ�ó����ύ��ͼ���������䵽��Ļ���֣�����ģʽ���ܻ����˺��Ч����

	// 2.VK_PRESENT_MODE_FIFO_KHR : ������������һ�����У�����ʾ������Ҫˢ�µ�ʱ����ʾ�豸�Ӷ��е�ǰ���ȡͼ��
	// ���ҳ�����Ⱦ��ɵ�ͼ�������еĺ��档������������ĳ����ȴ������ֹ�ģ����Ƶ��Ϸ�Ĵ�ֱͬ�������ơ���ʾ�豸��ˢ��ʱ�̱���Ϊ����ֱ�жϡ���

	// 3.VK_PRESENT_MODE_FIFO_RELAXED_KHR : ��ģʽ����һ��ģʽ���в�ͬ�ĵط�Ϊ�����Ӧ�ó�������ӳ٣����������һ����ֱͬ���ź�ʱ���п��ˣ�
	// ������ȴ���һ����ֱͬ���źţ����ǽ�ͼ��ֱ�Ӵ��͡����������ܵ��¿ɼ���˺��Ч����

	// 4.VK_PRESENT_MODE_MAILBOX_KHR : ���ǵڶ���ģʽ�ı��֡�����������������ʱ��ѡ���µ��滻�ɵ�ͼ�񣬴Ӷ��������Ӧ�ó�������Ρ�
	// ����ģʽͨ������ʵ�����ػ����������׼�Ĵ�ֱͬ��˫������ȣ���������Ч�����ӳٴ�����˺��Ч����
	VkPresentModeKHR ChooseSwapPresentaMode(const std::vector<VkPresentModeKHR> AvailablePresentModes);

	// ��ȡ�������Ľ�����Χ��
	// ������Χ��ָ������ͼ��ķֱ��ʣ����������ǵ������ǻ��ƴ���ķֱ��ʡ�
	// �ֱ��ʵķ�Χ��������VkSurfaceCapabilitiesKHR�ṹ���С�
	// Vulkan��������ͨ������currentExtent��Ա��width��height��ƥ�䴰��ķֱ��ʡ�
	// Ȼ����һЩ�������������ͬ�����ã���ζ�Ž�currentExtent��width��height����Ϊ�������ֵ��ʾ:uint32_t�����ֵ��
	// ����������£����ǲο�����minImageExtent��maxImageExtentѡ����ƥ��ķֱ��ʡ�
	VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& Capabilities);

	// �����߼��豸��
	void CreateLogicalDevice();

	// ������������
	void CreateSwapChain();

	// ��Ⱦѭ����
	void MainLoop();

	// ������Դ��
	void Cleanup();

private:

	GLFWwindow* Window;
	VkSurfaceKHR Surface;
	VkInstance Instance;
	VkDebugReportCallbackEXT Callback;
	VkPhysicalDevice PhysicalDevice;
	VkDevice LogicalDevice;
	VkQueue GraphicQueue;
	VkQueue PresentQueue;
	VkSwapchainKHR SwapChain;
	std::vector<VkImage> SwapChainImages;
	VkFormat SwapChainImageFormat;
	VkExtent2D SwapChainExtent;
};