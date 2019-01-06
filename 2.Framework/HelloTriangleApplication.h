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

	void InitWindow();

	void InitVulkan();

	void MainLoop();

	void Cleanup();

private:

	GLFWwindow* Window;
};