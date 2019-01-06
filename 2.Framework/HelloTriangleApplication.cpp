#include "HelloTriangleApplication.h"

void HelloTriangleApplication::InitWindow()
{
	glfwInit();

	// ���GLFW��ΪOpenGL���������ģ�
	// ����������������Ҫ��������Ҫ����OpenGL��صĳ�ʼ��������
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	Window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
}

void HelloTriangleApplication::InitVulkan()
{

}

void HelloTriangleApplication::MainLoop()
{
	while (!glfwWindowShouldClose(Window))
	{
		glfwPollEvents();
	}
}

void HelloTriangleApplication::Cleanup()
{
	glfwDestroyWindow(Window);
	glfwTerminate();
}
