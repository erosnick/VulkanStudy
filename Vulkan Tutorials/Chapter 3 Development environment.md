[TOC]

# Development environment

# 开发环境

在本章中，我们将设置用于开发Vulkan应用程序的环境，并安装一些有用的库，除了编译器以外，它们与Windows，Linux和MacOS兼容，但是安装它们的方式略有不同，这就是为什么在此单独描述它们的原因。

##  Windows

## Windows

如果您是针对Windows开发的，那么我将假定您使用的是Visual Studio 2017编译代码。您也可以使用Visual Studio 2013或2015，但步骤可能有所不同。

## Vulkan SDK

## Vulkan SDK

开发Vulkan应用程序所需的最重要的组件是SDK。它包括t头文件，标准验证层，调试工具以及用于Vulkan函数的加载器。加载器在运行时从驱动中寻找函数，类似于OpenGL的GLEW。

可以用LunarG网站页面底部的按钮下载该SDK，不需要创建帐户，但是注册账户能让你访问一些可能对你有用的其他文档。

![img](https://vulkan-tutorial.com/images/vulkan_sdk_download_buttons.png)

继续安装并注意SDK的安装位置。我们要做的第一件事是验证你的显卡和驱动程序支持Vulkan。转到安装SDK的目录，打开Bin目录并运行cube.exe演示。你应该看到以下内容：

![img](https://vulkan-tutorial.com/images/cube_demo.png)

如果您收到错误消息，请确保您的驱动程序是最新的，包括Vulkan运行时，并且支持您的图形卡。简介章节中提供了主要显卡供应商的驱动程序链接。

## GLFW

## GLFW

如前所述，Vulkan本身是与平台无关的API，并非包括用于创建窗口以显示渲染结果的工具。为了从Vulkan的跨平台优势中收益以及避免Win32的可怕，我们将使用GLFW库创建一个窗口，它支持Windows，Linux和MacOS。还有其他可用于此目的的库，例如SDL，但是GLFW的优势在于，除了窗口创建外，它还可以抽象出Vulkan中其他一些平台特定的东西。

你可以在官方网站上找到最新版本的GLFW。在本教程中我们将使用64位二进制文件，你当然也可以选择32位版本。在这种情况下，请确保链接到以下版本的Vulkan SDK二进制文件：Lib32目录而不是Lib。下载后，解压缩档案到一个方便的位置。我选择在以下目录中创建一个Libraries目录Visual Studio目录下的文件。不用担心没有libvc-2017文件夹，libvc-2015那个是兼容VS2017的。

![img](https://vulkan-tutorial.com/images/glfw_directory.png)

## GLM

## GLM

与DirectX 12不同，Vulkan不包含用于线性代数运算的库，因此我们必须下载一个，GLM是一个不错的库，专为
与图形API一起使用设计，也通常与OpenGL一起使用。

GLM是只有头文件的库，因此只需下载最新版本并进行存储在方便的位置。您的目录结构应类似于下面这样：

![img](https://vulkan-tutorial.com/images/library_directory.png)

## Setting up Visual Studio

## 设置Visual Studio

现在，你已经安装了所有依赖项，我们可以为Vulkan设置一个基本的Visual Studio项目，并编写一些代码以确保一切正常。

启动Visual Studio并通过输入名称并按确定来创建新的Windows桌面向导项目。

![img](https://vulkan-tutorial.com/images/vs_new_cpp_project.png)

确保选择了Console Application（.exe）作为应用程序类型，以便我们有一个向其打印调试消息的位置，并选中“ Empty Project”以防止Visual Studio添加样板代码。

![img](https://vulkan-tutorial.com/images/vs_application_settings.png)

按OK创建项目并添加C ++源文件。你应该已经知道怎么做，但是为了完整起见，此处包括了这些步骤。

![img](https://vulkan-tutorial.com/images/vs_new_item.png)

![img](https://vulkan-tutorial.com/images/vs_new_source_file.png)

现在，将以下代码添加到文件中。不必担心现在就尝试了解它。我们只是确保你可以编译和运行Vulkan应用程序。在下一章中，我们将从头开始。

```c++
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include <iostream>

int main() {
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(800, 600, "Vulkan window", nullptr, nullptr);

    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

    std::cout << extensionCount << " extensions supported" << std::endl;

    glm::mat4 matrix;
    glm::vec4 vec;
    auto test = matrix * vec;

    while(!glfwWindowShouldClose(window)) {
        glfwPollEvents();
    }

    glfwDestroyWindow(window);

    glfwTerminate();

    return 0;
}
```

现在让我们配置项目以消除错误。打开项目属性对话框，并确保选择“All Configurations”，因为大多数设置都适用于“Debug”和“Release”模式。

![img](https://vulkan-tutorial.com/images/vs_open_project_properties.png)

![img](https://vulkan-tutorial.com/images/vs_include_dirs.png)

转到C++ -> General -> Additional Include Directories并点击下拉列表中的<Edit...>。

![img](https://vulkan-tutorial.com/images/vs_cpp_general.png)

为Vulkan, GLFW和GLM添加头文件目录。

![img](https://vulkan-tutorial.com/images/vs_include_dirs.png)

接下来，打开Linker->General下的库目录编辑器。

![img](https://vulkan-tutorial.com/images/vs_link_settings.png)

添加Vulkan和GLFW的库路径。

![img](https://vulkan-tutorial.com/images/vs_link_dirs.png)

转到Linker->Input，点击Additional Dependencies下拉列表中的<Edit...>。

![img](https://vulkan-tutorial.com/images/vs_link_input.png)

输入Vulkan和GLFW库的名字。

![img](https://vulkan-tutorial.com/images/vs_dependencies.png)

最后改变编译器来支持C++17特性。

![img](https://vulkan-tutorial.com/images/vs_cpp17.png)

现在，您可以关闭项目属性对话框。如果您做对了所有事情，那么您应该再也看不到代码中突出显示了任何错误。

![img](https://vulkan-tutorial.com/images/vs_build_mode.png)

按F5编译并运行项目，您将看到命令提示符，并弹出如下窗口：

![img](https://vulkan-tutorial.com/images/vs_test_window.png)

扩展名的数量应为非零。恭喜，你已经准备好与Vulkan一起玩耍了。