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

	// �����߼��豸��
	void CreateLogicalDevice();

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
};