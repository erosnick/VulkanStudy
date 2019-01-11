#pragma once

#include <GLFW/glfw3.h>

#include <iostream>
#include <stdexcept>
#include <functional>

const int WIDTH = 800;
const int HEIGHT = 600;

class HelloTriangleApplication
{
public:

	// ������Ⱦѭ����
	void Run()
	{
		InitWindow();
		InitVulkan();
		MainLoop();
		Cleanup();
	}

	~HelloTriangleApplication()
	{
		Cleanup();
	}

private:

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
};