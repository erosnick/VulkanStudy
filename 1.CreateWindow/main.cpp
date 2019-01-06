#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include <iostream>

int main()
{
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	GLFWwindow* Window = glfwCreateWindow(800, 600, "Vulkan windoww", nullptr, nullptr);

	uint32_t ExtensionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &ExtensionCount, nullptr);

	std::cout << ExtensionCount << "extensions supported" << std::endl;

	glm::mat4 Matrix;
	glm::vec4 Vec;

	auto Test = Matrix * Vec;

	while (!glfwWindowShouldClose(Window))
	{
		glfwPollEvents();
	}

	glfwDestroyWindow(Window);

	glfwTerminate();

	return 0;
}