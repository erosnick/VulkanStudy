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

	void Run();

	~HelloTriangleApplication();

private:

	void CreateInstance();

	void CheckExtensions();

	void InitWindow();

	void InitVulkan();

	void MainLoop();

	void Cleanup();

private:

	GLFWwindow* Window;
	VkInstance Instance;
};