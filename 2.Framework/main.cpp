#define GLFW_INCLUDE_VULKAN
#include "HelloTriangleApplication.h"

int main()
{
	HelloTriangleApplication App;

	try
	{
		App.Run();
	}
	catch (const std::runtime_error& e)
	{
		std::cerr << e.what() << std::endl;
		return -1;
	}

	return 0;
}