[TOC]

# Drawing a triangle

# 绘制一个三角形
## Setup
## 设置

### Base Code

### 基础代码

####  General structure

#### 一般结构

在上一章中，您已经创建了一个Vulkan项目，其中包含所有适当的内容配置，并使用示例代码对其进行了测试。这一章我们会从以下代码重新开始。

```c++
#include <vulkan/vulkan.h>

#include <iostream>
#include <stdexcept>
#include <functional>
#include <cstdlib>

class HelloTriangleApplication {
public:
    void run() {
        initVulkan();
        mainLoop();
        cleanup();
    }

private:
    void initVulkan() {

    }

    void mainLoop() {

    }

    void cleanup() {

    }
};

int main() {
    HelloTriangleApplication app;

    try {
        app.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
```

我们首先包含LunarG SDK中的Vulkan头文件，该头文件提供了函数，结构体和枚举。 stdexcept和iostream头文件
用于报告和传递错误信息。functional头文件将在**资源管理(resource management)**部分中用于lambda函数。cstdlib头文件提供**EXIT_SUCCESS**和**EXIT_FAILURE**宏。

程序本身包装在一个类中，我们将在其中存储Vulkan对象作为私有类成员，并添加函数以初始化每个成员，它们将
从initVulkan函数中被调用。一旦所有准备工作就绪，我们进入主循环开始渲染一帧。

我们将填充mainLoop函数，该函数包含一个循环，循环将反复执行直到窗即关闭。一旦窗口关闭mainLoop返回后，我们将确保在cleanup函数中清理所有资源。

#### Resource management

#### 资源管理

就像使用malloc分配的每个内存块都需要调用free一样，我们创建的每个Vulkan对象在我们不再需要它的时候需要显式销毁。在现代C ++代码中，可以通过<memory>头文件中的实用程序进行自动资源管理。但是在这个教程中我选择显式的分配和销毁Vulkan对象。毕竟，Vulkan的好处在于明确每项操作以免出错，因此最好显式对待对象的生命周期，以了解API
如何工作。

在学完这个教程之后，您可以通过重载std::shared_ptr来实现自动资源管理。对于较大的Vulkan程序，建议您先使用RAII，但是对于学习目的知道幕后的运作机制总是好的。

Vulkan对象要么直接通过vkCreateXXX这样的函数直接创建，要么通过其他对象用vkAllocateXXX这样的函数创建。在确保一个函数不会被使用之后，您需要使用对应的vkDestroyXXX和vkFreeXXX来进行销毁。这些函数的参数针对不同类型的对象有所不同，但是它们有一个共同的参数pAllocator。这是一个可选参数，允许您为自定义内存分配器指定回调。我们将忽略该参数，在教程中始终传递nullptr。

#### Integrating GLFW

#### 集成GLFW

如果您想使用Vulkan进行离屏渲染，则无需创建窗口即可完美工作，但实际展示一些东西会更加令人兴奋。首先用

```c++
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
```

替换

```c++
#include <vulkan/vulkan.h>	
```

这样，GLFW将包含其自己的定义并自动加载Vulkan头文件。添加一个initWindow函数，将它作为run中第一个调用的函数。我们将使用该函数初始化GLFW并创建一个窗口。

```c++
void run() {
    initWindow();
    initVulkan();
    mainLoop();
    cleanup();
}

private:
    void initWindow() {

    }
```

initWindow中的第一个调用应该是glfwInit（），它会初始化GLFW库。因为GLFW最初旨在创建OpenGL上下文，我们需要调用后续函数告诉它不要创建OpenGL上下文：

```c++
glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
```

由于处理窗口缩放需要特别注意，我们稍后再说，现在通过另一个调用将其禁用：

```c++
glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
```

现在剩下的就是创建实际的窗口。添加GLFWwindow* window作为私有类成员来引用它，然后用下面的函数来初始化窗口：

```c++
window = glfwCreateWindow(800, 600, "Vulkan", nullptr, nullptr);
```

前三个参数指定窗口的宽度，高度和标题。第四个参数允许您选择指定一个显示器来打开窗口，最后一个参数仅与OpenGL有关。

使用常量是个好主意，而不要使用硬编码的宽度和高度数字，因为将来我们会多次引用这些值。我在HelloTriangleApplication类定义上方添加了以下几行：

```c++
const int WIDTH = 800;
const int HEIGHT = 600;	
```

更新窗口创建代码：

```c++
window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
```

现在你的initWindow函数看起来应该是下面这个样子：

```c++
void initWindow() {
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
}
```

为了保持应用程序运行直到出现错误或关闭窗口，我们需要向mainLoop函数添加一个事件循环，如下所示：

```c++
void mainLoop() {
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
    }
}
```

该代码应该是不言自明的。它循环并检查事件，例如按X按钮直到用户关闭窗口。稍后我们将在循环中调用一个函数来渲染单个帧。

窗口关闭后，我们需要通过销毁资源来清理它，并结束GLFW本身。这将是我们的第一个清理代码：

```c++
void cleanup() {
    glfwDestroyWindow(window);

    glfwTerminate();
}
```

现在，运行程序时您应该会看到一个名为Vulkan的窗口。现在我们有了Vulkan应用程序的框架，让我们创建第一个Vulkan对象吧！

#### Instance

### 实例

#### Creating an instance

#### 创建一个实例

您需要做的第一件事是通过创建实例来初始化Vulkan库。实例是您的应用程序与Vulkan库之间的连接，创建该实例涉及向驱动程序指定有关您的应用程序的一些详细信息。

我们从添加一个createInstance函数，并在initVulkan函数中调用它开始：

```c++
void initVulkan() {
    createInstance();
}
```

此外添加一个类成员变量来保存实例句柄。

```c++
private:
VkInstance instance;
```

要创建实例，我们首先必须在结构中填充一些有关我们应用程序的信息。该数据在技术上是可选的，但可能会为驱动程序提供一些有用的信息，以针对我们的特定应用进行优化，例如，因为它使用了具有某些特殊行为的众所周知的图形引擎。该结构称为VKApplicationInfo。

```c++
void createInstance() {
    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Hello Triangle";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;
}
```

如前所述，Vulkan中的许多结构都要求您在sType成员中明确指定类型。VkApplicationInfo也是带有pNext成员的结构之一，该结构将来可以指向扩展信息。我们在这里使用默认初始化将其保留为nullptr。

Vulkan中的许多信息都是通过结构而不是函数传递的参数，我们需要再填充一个结构以提供足够的信息来创建实例。下一个结构体不是可选的，它告诉Vulkan驱动程序我们要使用的全局扩展和验证层。全局的意思是它们适用于整个程序，而不是特定的设备，将在接下来的几章中阐明。

```c++
VkInstanceCreateInfo createInfo = {};
createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
createInfo.pApplicationInfo = &appInfo;
```

前两个参数很简单。接下来的代码指定所需的全局扩展。如概述章节所述，Vulkan是一个与平台无关的API，这意味着您需要扩展才能与窗口系统交互。 GLFW有一个便利的内置函数，该函数返回它需要执行的扩展，并将其传递给结构体：

```c++
uint32_t glfwExtensionCount = 0;
const char** glfwExtensions;

glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

createInfo.enabledExtensionCount = glfwExtensionCount;
createInfo.ppEnabledExtensionNames = glfwExtensions;
```

结构的最后两个成员确定要启用的全局验证层。我们将在下一章更深入地讨论这些内容，因此暂时将它们留空。

```c++
createInfo.enabledLayerCount = 0;
```

现在，我们指定了Vulkan创建实例所需的一切，最后可以进行vkCreateInstance调用。

```c++
VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);
```

如您所见，Vulkan中对象创建函数参数遵循的一般模式是：

* 具有创建信息的结构指针
* 指向自定义分配器回调的指针，在本教程中始终为nullptr
* 指向存储新对象句柄的变量的指针

如果一切顺利，则实例的句柄将存储在VkInstance类成员中。几乎所有的Vulkan函数都返回VkResult类型的值，该值可以是VK_SUCCESS或错误代码。要检查实例是否创建成功，我们不需要存储结果，而是使用检查返回值的方法：

```c++
if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
    throw std::runtime_error("failed to create instance!");
}
```

现在运行程序确保实例创建成功。

#### Checking for extension support

#### 检查扩展支持

如果您查看vkCreateInstance文档，则会发现可能的错误代码之一是VK_ERROR_EXTENSION_NOT_PRESENT。我们可以简单地指定所需的扩展名，并在错误代码返回时终止。这对于诸如窗口系统界面之类的基本扩展是有意义的，但是如果您要检查可选功能该怎么办？

要在创建实例之前检索受支持的扩展列表，可以使用vkEnumerateInstanceExtensionProperties函数。它需要一个指向存储扩展数量的变量的指针和一个VkExtensionProperties数组来存储扩展的详细信息。它还带有一个可选的第一个参数，该参数允许我们按特定的验证层过滤扩展，我们暂时将其忽略。

要分配一个数组来保存扩展详细信息，我们首先需要知道有多少个扩展。您可以通过将后一个参数留空来仅请求扩展数：

```c++
uint32_t extensionCount = 0;
vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
```

现在声明一个数组来保存扩展细节：

```c++
std::vector<VkExtensionProperties> extensions(extensionCount);
```

最终我们可以查询扩展细节：

```c++
vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());
```

每个VkExtensionProperties结构都包含扩展名和版本。我们可以用一个简单的for循环列出它们(\t是制表符，用于缩进)。

```c++
std::cout << "available extensions:" << std::endl;

for (const auto& extension : extensions) {
    std::cout << "\t" << extension.extensionName << std::endl;
}
```

如果您想提供有关Vulkan支持的一些详细信息，可以将此代码添加到createInstance函数中。作为挑战，请尝试创建一个函数来检查glfwGetRequiredInstanceExtensions返回的所有扩展名是否包括在受支持的扩展名列表中。

#### Cleanup

#### 清理

VkInstance仅应在程序退出前销毁。可以在清理时使用vkDestroyInstance函数销毁它：

```c++
void cleanup() {
    vkDestroyInstance(instance, nullptr);

    glfwDestroyWindow(window);

    glfwTerminate();
}
```

vkDestroyInstance函数的参数很简单。如上一章所述，Vulkan中的分配和释放函数具有可选的分配器回调，我们将通过向其传递nullptr来忽略它。我们在后续章节中创建的所有其他Vulkan资源都应该在销毁实例前清理。

在实例创建之后，继续执行更复杂的步骤之前，是时候通过验证层来评估我们的调试选项了。

