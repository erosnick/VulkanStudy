#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include <vulkan/vulkan.h>

#include <iostream>
#include <vector>
class Application
{
public:

	Application(int inWindowWidth, int inWindowHeight, const std::string title);

	void createWindow(int inWindowWidth, int inWindowHeight, const std::string title);
	void initializeVulkan();
	void queryInstanceLayers();
	void queryInstanceExtensions();
	void createInstance();
	void queryDeviceLayers();
	void queryDeviceExtensions();
	void createPhysicalDevice();
	void queryDeviceProperties();
	void createLogicalDevice();
	void mainLoop();
	~Application();

	void run();

protected:

	int windowWidth;
	int windowHeight;
	std::string windowTitle;
	GLFWwindow* window;

	VkInstance instance;
	VkDevice device;
	VkPhysicalDevice physicalDevice;
};