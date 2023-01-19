#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_vulkan.h"

#include "HelloTriangleAppliaction.h"

#define IMGUI_UNLIMITED_FRAME_RATE
#ifdef _DEBUG
#define IMGUI_VULKAN_DEBUG_REPORT
#endif

int main() 
{
    HelloTriangleApplicaton app;

	try
	{
		app.run();
	}
	catch (const std::exception& e)
	{
		fmt::print("{}\n", e.what());
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}