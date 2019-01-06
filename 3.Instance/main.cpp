#define GLFW_INCLUDE_VULKAN
#include "HelloTriangleApplication.h"
#include <Windows.h>

int main()
{
	AllocConsole();
	SetConsoleTitle(L"Debug Console");
	FILE* File;
	
	freopen_s(&File, "CONOUT$", "w", stdout);

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

	FreeConsole();

	return 0;
}