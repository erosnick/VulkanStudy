#pragma once

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include <iostream>
#include <stdexcept>
#include <functional>

const int WIDTH = 800;
const int HEIGHT = 600;

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

	// ��ʼ��GLFW����ϵͳ��
	void InitWindow();

	// ��ʼ��Vulkan�⡣
	void InitVulkan();

	// ��Ⱦѭ����
	void MainLoop();

	// ������Դ��
	void Cleanup();

private:

	GLFWwindow* Window;
	VkInstance Instance;
};