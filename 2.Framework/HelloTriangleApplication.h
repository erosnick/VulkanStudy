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

	// 启动渲染循环。
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

	// 初始化GLFW窗口系统。
	void InitWindow();

	// 初始化Vulkan库。
	void InitVulkan();

	// 渲染循环。
	void MainLoop();

	// 清理资源。
	void Cleanup();

private:

	GLFWwindow* Window;
};