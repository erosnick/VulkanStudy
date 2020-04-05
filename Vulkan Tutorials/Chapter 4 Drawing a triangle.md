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

### Instance

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

### Validation layers

### 验证层

#### What are validation layers?

#### 什么是验证层？

Vulkan API设计围绕最小化驱动开支的理念，表现之一就是默认的错误检查很有限。诸如给枚举变量赋了错误的值以及传递空指针给必须的参数这样简单的错误通常也没有明确的处理，只会导致崩溃和不确定的行为。

由于Vulkan要求您对所做的一切都非常明确，因此很容易犯很多小错误，例如使用新的GPU功能而忘记在逻辑设备创建时请求它。

但是，这并不意味着不能将这些检查添加到API。 Vulkan为此引入了一种优雅的系统，称为验证层。验证层是可插入Vulkan函数调用中以应用其他操作的可选组件。验证层的常见操作有：

* 根据技术规范检查参数值来检测错误
* 追踪资源的创建和销毁来寻找资源泄露
* 通过追踪线程的起源来检查线程安全性
* 记录每次调用及其参数到标准输出
* 追踪Vulkan调用用于剖析和回放

这里有一个在诊断层中函数实现的例子：

```c++
VkResult vkCreateInstance(
    const VkInstanceCreateInfo* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkInstance* instance) {

    if (pCreateInfo == nullptr || instance == nullptr) {
        log("Null pointer passed to required parameter!");
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    return real_vkCreateInstance(pCreateInfo, pAllocator, instance);
}
```

这些验证层能够自由的堆叠来包含你感兴趣的所有调试功能性。您只需为调试版本启用验证层，而对发行版本完全禁用它们，两全其美(gives you the best of both worlds)！

Vulkan没有内置任何验证层，但是LunarG Vulkan SDK提供了一组不错的层来检查常见错误。它们也是完全开源的，因此您可以看看他们检查哪些类型的错误并作出处理。使用验证层是避免意外依赖未定义行为，导致程序在不同的驱动上停止工作的最佳方法。

验证层只有在安装到系统后才能使用。例如，LunarG验证层仅在安装了Vulkan SDK的PC上可用。

Vulkan中以前有两种不同类型的验证层：实例和设备特定。这个想法是实例层将仅检查与实例之类的全局Vulkan对象有关的调用，而设备特定层将仅检查与特定GPU相关的调用。设备特定的层现在已经被弃用了，也就是说实例验证层应用于所有的Vulkan调用。规范文档仍然建议您在设备级别启用验证层，以实现兼容性，这是某些实现所需的。我们只需在逻辑设备级别指定与实例相同的层，稍后我们将介绍。

#### Using validation layers

#### 使用验证层

在本节中，我们将看到如何启用Vulkan SDK提供的标准诊断层。与扩展一样，验证层也需要通过指定其名称来启用。所有有用的标准验证都打包到SDK中包含的一层中，该层称为VK_LAYER_KHRONOS_validation。

首先，我们将两个配置变量添加到程序中，以指定要启用的层以及是否启用它们。我选择将该值基于是否在调试模式下编译程序。 NDEBUG宏是C ++标准的一部分，表示“非调试”。

```c++
const int WIDTH = 800;
const int HEIGHT = 600;

const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif
```

我们将添加一个新函数checkValidationLayerSupport，以检查所有请求的图层是否可用。首先使用vkEnumerateInstanceLayerProperties函数列出所有可用层。它的用法与实例创建一章中讨论的vkEnumerateInstanceExtensionProperties的用法相同。

```c++
bool checkValidationLayerSupport() {
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    return false;
}
```

接下来，检查validationLayers中的所有层是否都存在于availableLayers列表中。您可能需要包含<cstring>来使用strcmp。

```c++
for (const char* layerName : validationLayers) {
    bool layerFound = false;

    for (const auto& layerProperties : availableLayers) {
        if (strcmp(layerName, layerProperties.layerName) == 0) {
            layerFound = true;
            break;
        }
    }

    if (!layerFound) {
        return false;
    }
}

return true;
```

现在我们可以在createInstance函数中使用它：

```c++
void createInstance() {
    if (enableValidationLayers && !checkValidationLayerSupport()) {
        throw std::runtime_error("validation layers requested, but not available!");
    }

    ...
}
```

现在，以调试模式运行该程序，并确保不会发生该错误。如果出错了，请查看常见问题解答。

最后，修改VkInstanceCreateInfo结构实例以包括验证层名称（如果已启用）：

```c++
if (enableValidationLayers) {
    createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
    createInfo.ppEnabledLayerNames = validationLayers.data();
} else {
    createInfo.enabledLayerCount = 0;
}
```

译注：在我自己的机器上(GTX 1060，Vulkan SDK 1.2.131.2)支持的Layer如下：

VK_LAYER_NV_optimus
VK_LAYER_RENDERDOC_Capture
VK_LAYER_VALVE_steam_overlay
VK_LAYER_VALVE_steam_fossilize
VK_LAYER_EOS_Overlay
VK_LAYER_EOS_Overlay
VK_LAYER_ROCKSTAR_GAMES_social_club
VK_LAYER_LUNARG_api_dump
VK_LAYER_LUNARG_device_simulation
VK_LAYER_KHRONOS_validation
VK_LAYER_LUNARG_monitor
VK_LAYER_LUNARG_screenshot
VK_LAYER_LUNARG_standard_validation
VK_LAYER_LUNARG_vktrace

#### Message callback

#### 消息回调

验证层默认情况下会将调试消息打印到标准输出，但是我们也可以通过在程序中提供显式回调来自行处理它们。这也使您可以决定要查看哪种消息，因为并非所有消息都一定是（致命的）错误。如果您现在不想这样做，则可以跳到本章的最后一部分。

为了在程序中设置一个回调来处理消息和相关的细节，我们必须使用VK_EXT_debug_utils扩展来设置带有回调的调试程序。

我们首先将创建一个getRequiredExtensions函数，该函数将根据是否启用验证层来返回所需的扩展列表：

```c++
std::vector<const char*> getRequiredExtensions() {
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    if (enableValidationLayers) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return extensions;
}
```

始终需要GLFW指定的扩展名，但是有条件地添加了调试消息扩展名。请注意，我在这里使用了VK_EXT_DEBUG_UTILS_EXTENSION_NAME宏，它等于文字字符串“ VK_EXT_debug_utils”。使用此宏可以避免输入错误。

现在，我们可以在createInstance中使用此函数：

```c++
auto extensions = getRequiredExtensions();
createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
createInfo.ppEnabledExtensionNames = extensions.data();
```

运行程序以确保您没有收到VK_ERROR_EXTENSION_NOT_PRESENT错误。我们实际上并不需要检查此扩展的存在，因为验证层的可用性应隐含此扩展。

现在，让我们看看调试回调函数长啥样。使用PFN_vkDebugUtilsMessengerCallbackEXT原型添加一个名为debugCallback的新静态成员函数。 VKAPI_ATTR和VKAPI_CALL确保该函数具有Vulkan调用i的正确签名。

```c++
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData) {

    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

    return VK_FALSE;
}
```

第一个参数指定消息的严重性，它是以下标志之一：

- `VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT`: 诊断信息
- `VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT`: 提示信息，比如资源的创建
- `VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT`: 不一定是错误行为的信息，但很可能是应用程序中的Debug
- `VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT`: 无效会导致崩溃行为的信息

枚举值设置的方式让你可以使用比较操作来检查消息的严重等级，比如：

```c++
if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
    // Message is important enough to show
    // 消息足够重要需要被显示
}
```

messageType参数可以具有以下值：

- `VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT`: 发生了与规格或性能无关的事件
- `VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT`: 发生了违反规范或表明可能的错误的事情
- `VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT`:非最优化的Vulkan实践

pCallbackData参数引用VkDebugUtilsMessengerCallbackDataEXT结构，其中包含消息本身的详细信息，最重要的成员是：

- `pMessage`: 以\0结尾的调试信息
- `pObjects`: 和当前消息有关的Vulkan对象句柄数组
- `objectCount`: 数组中对象的数量

最后，pUserData参数包含在回调设置期间指定的指针，并允许您将自己的数据传递给它。

回调返回一个布尔值，该布尔值指示是否应终止触发验证层消息的Vulkan调用。如果回调返回true，则调用将中止，并出现VK_ERROR_VALIDATION_FAILED_EXT错误。通常，这仅用于测试验证层本身，因此您应始终返回VK_FALSE。

现在剩下的就是告诉Vulkan回调函数的信息。也许有些令人惊讶，甚至Vulkan中的debug回调也使用需要显式创建和销毁的句柄进行管理。这样的回调是调试消息传递器的一部分，您可以根据需要拥有任意数量的回调。在实例下直接为此句柄添加一个类成员：

```c++
VkDebugUtilsMessengerEXT debugMessenger;
```

现在添加一个函数setupDebugMessenger，该函数将在initVulkan中createInstance函数之后调用：

```c++
void initVulkan() {
    createInstance();
    setupDebugMessenger();
}

void setupDebugMessenger() {
    if (!enableValidationLayers) return;

}
```

我们需要在结构中填写有关消息传递器和其回调的详细信息：

```c++
VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
createInfo.pfnUserCallback = debugCallback;
createInfo.pUserData = nullptr; // Optional
```

messageSeverity字段允许您指定要调用回调的所有严重性类型。我在这里指定了所有类型，除了VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT以外，都是为了接收有关可能出现的问题的通知，同时省略了详细的常规调试信息。

类似地，messageType字段可让您过滤回调通知的消息类型。我在这里启用了所有类型。如果它们对您没有用，您可以随时禁用它们。

最后，pfnUserCallback字段指定指向回调函数的指针。您可以选择将指针传递给pUserData字段，该指针将通过pUserData参数传递给回调函数。例如，您可以使用它来传递一个指向HelloTriangleApplication类的指针。

请注意，还有更多方法可以配置验证层消息和调试回调，但对于本教程来说是个不错的起点。请参见[扩展规范](https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#VK_EXT_debug_utils)获取更多信息。

该结构应该传递给vkCreateDebugUtilsMessengerEXT函数以创建VkDebugUtilsMessengerEXT对象。不幸的是，由于此功能是扩展功能，因此不会自动加载。我们必须使用vkGetInstanceProcAddr自己查找其地址。我们将创建自己的代理函数在后台处理。我已经在HelloTriangleApplication类定义的上方添加了它。

```c++
VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}
```

如果无法加载，则vkGetInstanceProcAddr函数将返回nullptr。现在，我们可以调用此函数来创建扩展对象：

```c++
if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
    throw std::runtime_error("failed to set up debug messenger!");
}
```

倒数第二个参数还是我们设置为nullptr的可选分配器回调，除了参数非常简单之外。由于调试传递器特定于我们的Vulkan实例及其层，因此需要将其明确指定为第一个参数。稍后您还将在其他子对象中看到此模式。

还需要通过调用vkDestroyDebugUtilsMessengerEXT来清理VkDebugUtilsMessengerEXT对象。与vkCreateDebugUtilsMessengerEXT类似，该函数需要显式加载。

在CreateDebugUtilsMessengerEXT下面创建另一个代理函数：

```c++
void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}
```

确保此函数是静态类函数或该类之外的函数。然后我们可以在清理函数中调用它：

```c++
void cleanup() {
    if (enableValidationLayers) {
        DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
    }

    vkDestroyInstance(instance, nullptr);

    glfwDestroyWindow(window);

    glfwTerminate();
}
```

#### Debugging instance creation and destruction

#### 调试实例的创建和销毁

尽管我们现在已经在程序中添加了带有验证层的调试，但是我们还没有涵盖所有内容。 vkCreateDebugUtilsMessengerEXT调用要求已创建有效实例，并且必须在销毁实例之前调用vkDestroyDebugUtilsMessengerEXT。这使我们无法调试vkCreateInstance和vkDestroyInstance调用中的任何问题。

但是，如果您仔细阅读[扩展文档](https://github.com/KhronosGroup/Vulkan-Docs/blob/master/appendices/VK_EXT_debug_utils.txt#L120)，就会发现有一种方法可以专门为这两个函数调用创建一个单独的调试传递器实用程序。它只需要将一个VkDebugUtilsMessengerCreateInfoEXT结构体指针传递给VkInstanceCreateInfo的pNext扩展字段。首先将VkDebugUtilsMessengerCreateInfoEXT结构体的填充提取到一个单独的函数中：

```c++
void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
    createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;
}

...

void setupDebugMessenger() {
    if (!enableValidationLayers) return;

    VkDebugUtilsMessengerCreateInfoEXT createInfo;
    populateDebugMessengerCreateInfo(createInfo);

    if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
        throw std::runtime_error("failed to set up debug messenger!");
    }
}
```

现在我们可以在createInstance函数中重用它：

debugCreateInfo变量位于if语句之外，以确保在vkCreateInstance调用之前不会将其销毁。通过以这种方式创建一个附加的调试消息传递器，它将在vkCreateInstance和vkDestroyInstance期间自动使用并在此之后进行清理。

#### Testing

#### 测试

现在，让我们有意犯一个错误，以查看运行中的验证层。暂时删除清理函数中对DestroyDebugUtilsMessengerEXT的调用，然后运行程序。退出后，您应该会看到以下内容：

![img](https://vulkan-tutorial.com/images/validation_layer_test.png)

如果要查看哪个调用触发了消息，则可以在消息回调中添加一个断点并查看堆栈跟踪。

#### Configuration

#### 配置

除了VkDebugUtilsMessengerCreateInfoEXT结构中指定的标志外，验证层的行为还有更多的设置。浏览到Vulkan SDK，然后转到Config目录。在那里，您将找到一个vk_layer_settings.txt文件，该文件说明了如何配置调试层。

要为自己的应用程序配置层设置，请将文件复制到项目的Debug和Release目录，然后按照说明进行操作以设置所需的行为。但是，在本教程的其余部分中，我将假定您使用的是默认设置。

在本教程中，我将犯一些故意的错误，以向您展示验证层在捕获它们方面的帮助，并告诉您准确了解您对Vulkan所做的工作有多重要。现在是时候查看系统中的Vulkan设备了。

### Physical devices and queue families

### 物理设备和队列家族

#### Selecting a physical device

#### 选择一个物理设备

通过VkInstance初始化Vulkan库后，我们需要在系统中寻找并选择支持所需功能的图形卡。实际上，我们可以选择任意数量的图形卡并同时使用它们，但是在本教程中，我们将只使用符合我们需求的第一张图形卡。

我们将添加一个函数pickPhysicalDevice，并在initVulkan函数中添加对其的调用。

```c++
void initVulkan() {
    createInstance();
    setupDebugMessenger();
    pickPhysicalDevice();
}

void pickPhysicalDevice() {

}
```

我们最终选择的图形卡将存储在VkPhysicalDevice句柄中，该句柄作为新的类成员添加。销毁VkInstance时，该对象将被隐式销毁，因此我们无需在清除函数中做任何新的事情。

列出图形卡与列出扩展名非常相似，从查询数量开始。

```c++
uint32_t deviceCount = 0;
vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
```

如果有设备支持Vulkan，那么就没有往下继续的必要了。

```c++
if (deviceCount == 0) {
    throw std::runtime_error("failed to find GPUs with Vulkan support!");
}
```

否则，我们可以分配一个数组来容纳所有VkPhysicalDevice句柄。

```c++
std::vector<VkPhysicalDevice> devices(deviceCount);
vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());
```

现在，我们需要评估它们中的每一个，并检查它们是否适合我们要执行的操作，因为并非所有图形卡都是相同的。为此，我们将引入一个新函数：

```c++
bool isDeviceSuitable(VkPhysicalDevice device) {
    return true;
}
```

并且我们将检查是否有任何物理设备满足我们要添加到该函数的要求。

```c++
for (const auto& device : devices) {
    if (isDeviceSuitable(device)) {
        physicalDevice = device;
        break;
    }
}

if (physicalDevice == VK_NULL_HANDLE) {
    throw std::runtime_error("failed to find a suitable GPU!");
```

下一节将介绍我们将在isDeviceSuitable函数中检查的第一个需求。在后面的章节中，我们将开始使用更多的Vulkan功能，我们还将对该功能进行扩展以包括更多的检查、

#### Base device suitability checks

#### 基本设备适用性检查

为了评估设备的适用性，我们可以从查询一些细节开始。可以使用vkGetPhysicalDeviceProperties查询基本设备属性，例如名称，类型和受支持的Vulkan版本。

```c++
VkPhysicalDeviceProperties deviceProperties;
vkGetPhysicalDeviceProperties(device, &deviceProperties);
```

可以使用vkGetPhysicalDeviceFeatures查询是否支持可选功能，例如纹理压缩，64位浮点和多视口渲染（对VR有用）：

```c++
VkPhysicalDeviceFeatures deviceFeatures;
vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
```

还可以从设备查询更多详细信息，我们将在后面讨论有关设备内存和队列家族的信息（请参阅下一节）。

例如，假设我们的应用程序仅可用于支持几何着色器的专用图形卡。然后，isDeviceSuitable函数将如下所示：

```c++
bool isDeviceSuitable(VkPhysicalDevice device) {
    VkPhysicalDeviceProperties deviceProperties;
    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceProperties(device, &deviceProperties);
    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

    return deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
           deviceFeatures.geometryShader;
}
```

您不仅可以检查设备是否适合并选择第一个设备，还可以给每个设备评分并选择最高的设备。这样，您可以通过给它更高的分数来偏爱专用的图形卡，但是如果那是唯一可用的显卡，则可以使用集成的GPU。您可以实现如下所示的内容：

```c++
#include <map>

...

void pickPhysicalDevice() {
    ...

    // Use an ordered map to automatically sort candidates by increasing score
    std::multimap<int, VkPhysicalDevice> candidates;

    for (const auto& device : devices) {
        int score = rateDeviceSuitability(device);
        candidates.insert(std::make_pair(score, device));
    }

    // Check if the best candidate is suitable at all
    if (candidates.rbegin()->first > 0) {
        physicalDevice = candidates.rbegin()->second;
    } else {
        throw std::runtime_error("failed to find a suitable GPU!");
    }
}

int rateDeviceSuitability(VkPhysicalDevice device) {
    ...

    int score = 0;

    // Discrete GPUs have a significant performance advantage
    if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
        score += 1000;
    }

    // Maximum possible size of textures affects graphics quality
    score += deviceProperties.limits.maxImageDimension2D;

    // Application can't function without geometry shaders
    if (!deviceFeatures.geometryShader) {
        return 0;
    }

    return score;
}
```

您无需在本教程中实现所有这些功能，但可以使您了解如何设计设备选择过程。当然，您也可以只显示选择的名称并允许用户选择。

因为我们才刚刚起步，所以Vulkan支持是我们唯一需要的，因此我们将满足于任何GPU：

```c++
bool isDeviceSuitable(VkPhysicalDevice device) {
    return true;
}
```

在下一节中，我们将讨论要检查的第一个实际必需功能。

#### Queue families

#### 队列家族

在此之前，我们已经简短地提到过Vulkan中几乎所有的操作，从绘图到上传纹理的任何操作，都需要将命令提交到队列中。有不同类型的队列，它们来自不同的队列家族，并且每个队列家族仅允许一个命令的子集。例如，可能有一个仅允许处理计算命令的队列家族，或者仅允许一个与内存传输相关的命令的队列家族。

我们需要检查设备支持哪些队列家族，以及哪些队列家族支持我们要使用的命令。为此，我们将添加一个新函数findQueueFamilies，以查找我们需要的所有队列家族。

```c++
uint32_t findQueueFamilies(VkPhysicalDevice device) {
    // Logic to find graphics queue family
}
```

但是，在下一章中，我们已经要寻找另一个队列，因此最好为此做准备并将索引包装到一个结构体中：

```c++
struct QueueFamilyIndices {
    uint32_t graphicsFamily;
};

QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) {
    QueueFamilyIndices indices;
    // Logic to find queue family indices to populate struct with
    return indices;
}
```

但是，如果没有可用的队列家族怎么办？我们可以在findQueueFamilies中引发异常，但是此功能实际上并不是做出设备适用性决策的正确位置。例如，我们可能更想要带有专用传输队列家族的设备，但不是必须的。因此，我们需要某种方式来指示是否找到了特定的队列家族。

实际上不可能使用一个魔术值来指示队列家族的不存在，因为从理论上讲uint32_t的任何值都可以是有效的队列家族索引，包括0。幸运的是，C ++ 17引入了一种数据结构来区分值是否存在：

```c++
#include <optional>

...

std::optional<uint32_t> graphicsFamily;

std::cout << std::boolalpha << graphicsFamily.has_value() << std::endl; // false

graphicsFamily = 0;

std::cout << std::boolalpha << graphicsFamily.has_value() << std::endl; // true
```

std :: optional是一个包装，在您为其分配值之前，它不包含任何值。您可以随时通过调用其has_value（）成员函数查询其是否包含值。这意味着我们可以将逻辑更改为：

```c++
#include <optional>

...

struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
};

QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) {
    QueueFamilyIndices indices;
    // Assign index to queue families that could be found
    return indices;
}
```

现在，我们可以开始实际实现findQueueFamilies了：

```c++
QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) {
    QueueFamilyIndices indices;

    ...

    return indices;
}
```

检索队列族列表的过程正是您所期望的，使用vkGetPhysicalDeviceQueueFamilyProperties：

```c++
uint32_t queueFamilyCount = 0;
vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());
```

VkQueueFamilyProperties结构包含有关队列家族的一些详细信息，包括支持的操作类型以及可以基于该系列创建的队列数量。我们需要找到至少一个支持VK_QUEUE_GRAPHICS_BIT的队列家族。

现在，我们有了队列家族查找功能，可以将其用作isDeviceSuitable函数中的检查，以确保设备可以处理我们要使用的命令:

```c++
bool isDeviceSuitable(VkPhysicalDevice device) {
    QueueFamilyIndices indices = findQueueFamilies(device);

    return indices.graphicsFamily.has_value();
}
```

为了使此操作更加方便，我们还将对结构本身添加一个通用检查：

```c++
struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;

    bool isComplete() {
        return graphicsFamily.has_value();
    }
};

...

bool isDeviceSuitable(VkPhysicalDevice device) {
    QueueFamilyIndices indices = findQueueFamilies(device);

    return indices.isComplete();
```

现在，我们还可以使用它来提前退出findQueueFamilies：

```c++
for (const auto& queueFamily : queueFamilies) {
    ...

    if (indices.isComplete()) {
        break;
    }

    i++;
}
```

很好，这就是我们现在找到合适的物理设备所需的一切！下一步是创建逻辑设备以与其进行交互。

### Logical device and queues

### 逻辑设备和队列

#### Introduction

#### 简介

选择要使用的物理设备后，我们需要设置一个逻辑设备以与其连接。逻辑设备创建过程类似于实例创建过程，并描述了我们要使用的功能。现在，我们已经查询了哪些队列家族可用，因此还需要指定要创建的队列。如果您有不同的要求，甚至可以从同一物理设备创建多个逻辑设备。

首先添加一个新的类成员来存储逻辑设备句柄。

```c++
VkDevice device;
```

接下来，添加一个从initVulkan调用的createLogicalDevice函数。

```c++
void initVulkan() {
    createInstance();
    setupDebugMessenger();
    pickPhysicalDevice();
    createLogicalDevice();
}

void createLogicalDevice() {

}
```

#### Specifying the queues to be created

#### 指定要创建的队列

逻辑设备的创建涉及再次在结构体中指定一堆详细信息，其中第一个是VkDeviceQueueCreateInfo。此结构体描述了单个队列家族所需的队列数。现在，我们只对具有图形功能的队列感兴趣。

```c++
QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

VkDeviceQueueCreateInfo queueCreateInfo = {};
queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
queueCreateInfo.queueFamilyIndex = indices.graphicsFamily.value();
queueCreateInfo.queueCount = 1;
```

当前可用的驱动程序只允许您为每个队列系列创建少量队列，而您实际上并不需要多个。这是因为您可以在多个线程上创建所有命令缓冲区，然后通过一次低开销调用一次在主线程上全部提交它们。

Vulkan允许您使用0.0到1.0之间的浮点数为队列分配优先级，以影响命令缓冲区执行的调度。即使只有一个队列，这也是必需的：

```c++
float queuePriority = 1.0f;
queueCreateInfo.pQueuePriorities = &queuePriority;
```

#### Specifying used device features

#### 指定使用的设备特性

接下来要指定的信息是我们将要使用的设备特性集。这些是我们在上一章中查询的vkGetPhysicalDeviceFeatures支持的功能，例如几何着色器。现在，我们不需要任何特殊的东西，因此我们可以简单地定义它，并将所有内容保留VK_FALSE。一旦我们开始使用Vulkan做更多有趣的事情，我们将回到这个结构体。

```c++
VkPhysicalDeviceFeatures deviceFeatures = {};
```

#### Creating the logical device

#### 创建逻辑设备

有了前面两个结构体，我们可以开始填充最主要的VkDeviceCreateInfo结构体了。

```c++
VkDeviceCreateInfo createInfo = {};
createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
```

首先添加指向队列创建信息和设备特性结构体的指针：

```c++
createInfo.pQueueCreateInfos = &queueCreateInfo;
createInfo.queueCreateInfoCount = 1;

createInfo.pEnabledFeatures = &deviceFeatures;
```

其余信息与VkInstanceCreateInfo结构体相似，并且要求您指定扩展和验证层。所不同的是，这一次这些特定于设备。

设备特定扩展的一个示例是VK_KHR_swapchain，它允许您将渲染的图像从该设备呈现到Windows。系统中可能有缺少此功能的Vulkan设备，例如因为它们仅支持计算操作。我们将在交换链这一章中回到这个扩展。

Vulkan的之前实现在实例和特定于设备的验证层之间进行了区分，但是已经不再如此。这意味着最新的实现会忽略VkDeviceCreateInfo的enabledLayerCount和ppEnabledLayerNames字段。但是，仍然将它们设置为与较早的实现兼容是一个好主意：

```c++
createInfo.enabledExtensionCount = 0;

if (enableValidationLayers) {
    createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
    createInfo.ppEnabledLayerNames = validationLayers.data();
} else {
    createInfo.enabledLayerCount = 0;
}
```

我们现在不需要任何设备特定的扩展。

好了，我们现在准备通过调用vkCreateDevice函数来实例化逻辑设备。

```c++
if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
    throw std::runtime_error("failed to create logical device!");
}
```

这些参数是与之交互的物理设备，我们刚刚指定的队列和使用情况信息，可选的分配回调指针以及指向用于存储逻辑设备句柄的变量的指针。类似于实例创建函数，此调用可以返回错误基于启用不存在的扩展或指定不支持的功能。

应该使用vkDestroyDevice函数在清除时销毁该设备：

```c++
void cleanup() {
    vkDestroyDevice(device, nullptr);
    ...
}
```

逻辑设备不直接与实例交互，这就是为什么不将其作为参数包含在内的原因。

#### Retrieving queue handles

#### 获取队列句柄

队列是与逻辑设备一起自动创建的，但是我们尚无与之交互的句柄。首先添加一个类成员以存储图形队列的句柄：

```c++
VkQueue graphicsQueue;
```

设备销毁时，将隐式清理设备队列，因此我们无需执行任何清理操作。

我们可以使用vkGetDeviceQueue函数来检索每个队列家族的队列句柄。参数是逻辑设备，队列家族，队列索引和指向变量的指针，该变量用于存储队列句柄。由于我们仅从该家族创建单个队列，因此我们将使用索引0。

```c++
vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
```

有了逻辑设备和队列句柄，我们现在就可以真正开始使用图形卡执行操作了！在接下来的几章中，我们将设置资源以将结果呈现给窗口系统。

## Presentation

## 呈现

### Window surface

### 窗口表面

由于Vulkan是与平台无关的API，因此它无法直接与窗口系统交互。为了在Vulkan和窗口系统之间建立连接以将结果呈现给屏幕，我们需要使用WSI（Window System Integration窗口系统集成）扩展。在本章中，我们将讨论第一个，即VK_KHR_surface。它暴露了一个VkSurfaceKHR对象，该对象表示要呈现渲染图像的抽象表面类型。程序中的表面将由我们已经使用GLFW打开的窗口支持。

VK_KHR_surface扩展是实例级别的扩展，我们实际上已经启用了它，因为它包含在glfwGetRequiredInstanceExtensions返回的列表中。该列表还包括其他一些WSI扩展，我们将在接下来的两章中使用。

窗口表面需要在实例创建之后立即创建，因为它实际上会影响物理设备的选择。我们之所以推迟这样做，是因为窗口表面是渲染目标和表示的较大主题的一部分，对此的解释可能会使基本设置显得混乱。还应注意，如果仅需要离屏渲染，则窗口表面在Vulkan中是完全可选的组件。 Vulkan允许您做到这一点，而无需创建不可见窗口（OpenGL必需）之类的技巧。

#### Window surface creation

#### 窗口表面选择

首先添加一个表面类成员。

```c++
VkSurfaceKHR surface;
```

尽管VkSurfaceKHR对象及其用法与平台无关，它的创建却和平台有关，因为它取决于窗口系统的详细信息。例如，它需要Windows上的HWND和HMODULE句柄。因此，该扩展有特定于平台的扩展，在Windows上称为VK_KHR_win32_surface，它也自动包含在glfwGetRequiredInstanceExtensions的列表中。

我将演示如何使用该特定于平台的扩展来在Windows上创建表面，但是在本教程中我们实际上不会使用它。使用诸如GLFW之类的库然后继续使用平台特定的代码毫无意义。 GLFW实际上有一个glfwCreateWindowSurface函数，可以为我们处理平台差异。尽管如此，在我们开始依赖它之前，先看看它在幕后做什么是一件好事。

因为窗口表面是Vulkan对象，所以它带有需要填充的VkWin32SurfaceCreateInfoKHR结构。它具有两个重要参数：hwnd和hinstance。这些是窗口的句柄，

glfwGetWin32Window函数用于从GLFW窗口对象获取原始HWND。调用GetModuleHandle返回当前进程的HINSTANCE句柄。

之后，可以使用vkCreateWin32SurfaceKHR创建表面，该表面包括实例的参数，表面创建详细信息，自定义分配器以及要存储在其中的表面句柄的变量。从技术上讲，这是WSI扩展功能，但它是如此常用标准Vulkan加载程序包括它，因此与其他扩展不同，您无需显式加载它。

```c++
if (vkCreateWin32SurfaceKHR(instance, &createInfo, nullptr, &surface) != VK_SUCCESS) {
    throw std::runtime_error("failed to create window surface!");
}
```

该过程与其他平台（例如Linux）相似，其中vkCreateXcbSurfaceKHR将XCB连接和窗口作为X11的创建详细信息。

glfwCreateWindowSurface函数针对每个平台使用不同的实现来精确执行此操作。现在，我们将其集成到我们的程序中。在实例创建和setupDebugMessenger之后立即添加要从initVulkan调用的函数createSurface。

```c++
void initVulkan() {
    createInstance();
    setupDebugMessenger();
    createSurface();
    pickPhysicalDevice();
    createLogicalDevice();
}

void createSurface() {

}
```

GLFW调用采用简单的参数而不是结构，这使函数的实现非常简单：

```c++
void createSurface() {
    if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
        throw std::runtime_error("failed to create window surface!");
    }
}
```

参数是VkInstance，GLFW窗口指针，自定义分配器和指向VkSurfaceKHR变量的指针。它只是通过相关平台调用中的VkResult传递。 GLFW没有提供销毁表面的特殊函数，但是可以通过原始API轻松完成：

```c++
void cleanup() {
        ...
        vkDestroySurfaceKHR(instance, surface, nullptr);
        vkDestroyInstance(instance, nullptr);
        ...
    }
```

确保在实例之前已销毁表面。

#### Querying for presentation support

#### 查询呈现支持

尽管Vulkan实现可能支持窗口系统集成，但这并不意味着系统中的每个设备都支持它。因此，我们需要扩展isDeviceSuitable以确保设备可以将图像呈现到我们创建的表面上。由于呈现是特定于队列的功能，因此问题实际上出在寻找支持呈现到我们创建的表面的队列家族。

```c++
struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete() {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};
```

接下来，我们将修改findQueueFamilies函数以查找具有呈现到我们的窗口表面的功能的队列家族。用来检查的函数是vkGetPhysicalDeviceSurfaceSupportKHR，它将物理设备，队列家族索引和表面作为参数。在与VK_QUEUE_GRAPHICS_BIT相同的循环中添加对它的调用：

```c++
VkBool32 presentSupport = false;
vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
```

然后，只需检查布尔值并存储表示系列队列索引：

```c++
if (presentSupport) {
    indices.presentFamily = i;
}
```

请注意，最终它们很可能为同一个队列家族，但是在整个程序中，我们将它们视为独立的队列，以采用统一的方法。但是，您可以添加逻辑以明确地使用支持在同一队列中进行绘图和演示的物理设备，以提高性能。

#### Creating the presentation queue

#### 创建呈现队列

剩下的一件事是修改逻辑设备创建过程以创建呈现队列并获得VkQueue句柄。为该句柄添加一个成员变量：

```c++
VkQueue presentQueue;
```

接下来，我们需要多个VkDeviceQueueCreateInfo结构体来创建两个家族的队列。一种优雅的做法是创建一组唯一的队列家族，它是必需的队列所需的。

```c++

...

QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};

float queuePriority = 1.0f;
for (uint32_t queueFamily : uniqueQueueFamilies) {
    VkDeviceQueueCreateInfo queueCreateInfo = {};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = queueFamily;
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = &queuePriority;
    queueCreateInfos.push_back(queueCreateInfo);
}
```

并修改VkDeviceCreateInfo以指向上面创建的queueCreateInfos容器：

如果队列族相同，那么我们只需传递一次索引。最后，添加一个调用以检索队列句柄：

```c++
vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
```

如果队列系列相同，则两个句柄现在很可能具有相同的值。在下一章中，我们将研究交换链以及它们如何使我们能够在表面呈现图像。

### Swap Chain

### 交换链

Vulkan没有“默认帧缓冲区”的概念，因此它需要一种基础结构，该基础结构拥有我们将渲染到的缓冲区，然后才能在屏幕上可视化它们。该基础结构被称为交换链，它必须在Vulkan中显式创建。交换链本质上是等待显示在屏幕上的图像队列。我们的应用程序将获取要绘制的图像，然后将其返回到队列。队列的工作方式以及从队列中显示图像的条件取决于交换链的设置方式，但是交换链的一般目的是使图像的显示与屏幕的刷新率同步。

#### Checking for swap chain support

#### 检查交换链支持

由于各种原因，并非所有的图形卡都能够将图像直接显示在屏幕上，例如，因为它们是为服务器设计的，并且没有任何显示输出。其次，由于图像表示与窗口系统以及与窗口相关的表面紧密相关，因此它实际上不是Vulkan核心的一部分。在查询其支持后，必须启用VK_KHR_swapchain设备扩展。

为此，我们将首先扩展isDeviceSuitable函数，以检查是否支持此扩展。前面我们已经看到了如何列出VkPhysicalDevice支持的扩展，因此这样做应该非常简单。请注意，Vulkan头文件提供了一个很棒的宏VK_KHR_SWAPCHAIN_EXTENSION_NAME，该宏定义为VK_KHR_swapchain。使用此宏的优点是编译器将捕获拼写错误。

首先声明所需设备扩展的列表，类似于要启用的验证层的列表。

```c++
const std::vector<const char*> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};
```

接下来，创建一个从isDeviceSuitable调用的新功能checkDeviceExtensionSupport作为附加检查：

```c++
bool isDeviceSuitable(VkPhysicalDevice device) {
    QueueFamilyIndices indices = findQueueFamilies(device);

    bool extensionsSupported = checkDeviceExtensionSupport(device);

    return indices.isComplete() && extensionsSupported;
}

bool checkDeviceExtensionSupport(VkPhysicalDevice device) {
    return true;
```

修改函数的主体以枚举扩展名，并检查所有必需的扩展名是否在其中。

```c++
bool checkDeviceExtensionSupport(VkPhysicalDevice device) {
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

    std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

    for (const auto& extension : availableExtensions) {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}
```

我选择在这里使用一组字符串来表示未确认的必需扩展名。这样，我们可以在枚举可用扩展名的序列时轻松地将其勾掉。当然，您也可以像在checkValidationLayerSupport中那样使用嵌套循环。性能差异无关紧要。现在运行代码，并验证您的图形卡确实能够创建交换链。应当注意，如上一章所述，表示队列的可用性意味着必须支持交换链扩展。但是，明确说明仍然是很好的，并且必须明确启用扩展。

#### Enabling device extensions

#### 启用设备扩展

使用交换链要求首先启用VK_KHR_swapchain扩展。启用扩展只需要对逻辑设备创建结构进行少量更改：

```c++
createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
createInfo.ppEnabledExtensionNames = deviceExtensions.data();
```

#### Querying details of swap chain support

#### 查询交换链支持细节

仅检查交换链是否可用还不够，因为它实际上可能与我们的窗户表面不兼容。创建交换链还涉及比实例和设备创建更多的设置，因此我们需要查询更多详细信息，然后才能继续。

我们基本上需要检查三种属性：

* 基本表面功能（交换链中图像的最小/最大数量，图像的最小/最大宽度和高度）
* 表面格式（像素格式，色彩空间）
* 可用的呈现模式

与findQueueFamilies类似，一旦查询到这些细节，我们将使用一个结构来传递这些细节。前面提到的三种类型的属性以下列结构体和结构体列表的形式出现：

```c++
struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};
```

现在，我们将创建一个新函数querySwapChainSupport，它将填充此结构。

```c++
SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device) {
    SwapChainSupportDetails details;

    return details;
}
```

本节介绍如何查询包含此信息的结构。下一节将讨论这些结构的含义以及它们包含的确切数据。

让我们从基本的表面功能开始。这些属性易于查询，并返回到单个VkSurfaceCapabilitiesKHR结构中。

```c++
vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);
```

确定支持的功能时，此函数将指定的VkPhysicalDevice和VkSurfaceKHR窗口表面考虑在内。所有支持查询函数都将这两个作为第一个参数，因为它们是交换链的核心组件。

下一步是查询支持的表面格式。因为这是一个结构列表，所以它遵循两个函数调用的熟悉习惯：

```c++
uint32_t formatCount;
vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

if (formatCount != 0) {
    details.formats.resize(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
}
```

确保调整容器的大小以容纳所有可用格式。最后，使用vkGetPhysicalDeviceSurfacePresentModesKHR查询支持的表示模式的方式完全相同：

```c++
uint32_t presentModeCount;
vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

if (presentModeCount != 0) {
    details.presentModes.resize(presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
}
```

现在所有细节都在结构体中，因此让我们再次扩展isDeviceSuitable以利用此函数来验证交换链支持是否足够。如果在我们拥有的窗口表面范围内至少存在一种支持的图像格式和一种支持的呈现模式，那么交换链支持对于本教程就足够了。

```c++
bool swapChainAdequate = false;
bool swapChainAdequate = false;
if (extensionsSupported) {
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
    swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
}
```

重要的是，我们仅在验证扩展可用后才尝试查询交换链支持。函数的最后一行更改为：

```c++
return indices.isComplete() && extensionsSupported && swapChainAdequate;
```

#### Choosing the right settings for the swap chain

#### 为交换链选择正确的配置

如果满足swapChainAdequate条件，则支持肯定足够，但可能仍存在许多不同的最优模式。现在，我们将编写一些函数来找到正确的设置，以实现最佳的交换链。可以通过三种类型的设置来确定：

* 表面格式（颜色深度）
* 呈现模式（将图像“交换”到屏幕的条件）
* 交换尺寸（交换链中图像的分辨率）

对于这些设置中的每一个，我们都将牢记一个理想的值，如果可用，我们将与之保持一致，否则，我们将创建一些逻辑来寻找下一个最好的选择。

##### Surface format

##### 表面格式

表面格式设定函数一开始是这样的，稍后，我们将SwapChainSupportDetails结构的formats成员作为参数传递。

```c++
VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {

}
```

每个VkSurfaceFormatKHR条目均包含format和colorSpace成员。format成员指定颜色通道和类型。例如，VK_FORMAT_B8G8R8A8_SRGB表示我们以8位无符号整数的顺序存储B，G，R和alpha通道，每个像素总共32位。 colorSpace成员使用VK_COLOR_SPACE_SRGB_NONLINEAR_KHR标志指示是否支持SRGB颜色空间。请注意，在规范的旧版本中，此标志以前称为VK_COLORSPACE_SRGB_NONLINEAR_KHR。

对于色彩空间，我们将使用SRGB（如果可用），因为它可以产生更准确的感知颜色。它还几乎是图像的标准颜色空间，例如稍后将要使用的纹理。因此，我们还应该使用SRGB颜色格式，其中最常见的一种是VK_FORMAT_B8G8R8A8_SRGB。

让我们浏览一下列表，看看是否可以使用首选组合：

```c++
for (const auto& availableFormat : availableFormats) {
    if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
        return availableFormat;
    }
}
```

如果仍然失败，那么我们可以根据它们的“良好”程度开始对可用格式进行排名，但是在大多数情况下，只需要使用指定的第一种格式就可以了。

```c++
VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
    for (const auto& availableFormat : availableFormats) {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return availableFormat;
        }
    }

    return availableFormats[0];
}
```

##### Presentation mode

##### 呈现模式

呈现模式可以说是交换链最重要的设置，因为它代表了在屏幕上显示图像的实际条件。 Vulkan有四种可能的模式：

* VK_PRESENT_MODE_IMMEDIATE_KHR：您的应用程序提交的图像会立即传输到屏幕上，这可能会导致撕裂。
* VK_PRESENT_MODE_FIFO_KHR：交换链是一个队列，当刷新显示时，显示器从队列的前面获取图像，并且程序将渲染的图像插入队列的后面。如果队列已满，则程序必须等待。这与现代游戏中的垂直同步最为相似。刷新显示的那一刻被称为“垂直空白”。
* VK_PRESENT_MODE_FIFO_RELAXED_KHR：仅当应用程序延迟并且队列在最后一个垂直空白处为空时，此模式才与前一个模式不同。当图像最终到达时，将立即传输图像，而不是等待下一个垂直空白。这可能会导致可见的撕裂。
* VK_PRESENT_MODE_MAILBOX_KHR：这是第二种模式的另一种变化。当队列已满时，不会阻塞应用程序，而是将已经排队的图像替换为更新的图像。此模式可用于实现三重缓冲，与使用双缓冲的标准垂直同步相比，它可以避免撕裂，并显着减少了延迟问题。

只有VK_PRESENT_MODE_FIFO_KHR模式确保是可用，因此我们再次必须编写一个函数来寻找可用的最佳模式：

```c++
VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
    return VK_PRESENT_MODE_FIFO_KHR;
}
```

我个人认为三重缓冲是一个非常好的折衷方案。通过渲染尽可能新的新图像直至垂直空白，它使我们避免了撕裂，同时仍保持相当低的延迟。因此，让我们浏览一下列表以查看是否可用：

```c++
VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
    for (const auto& availablePresentMode : availablePresentModes) {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return availablePresentMode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}
```

##### Swap extent

##### 交换尺寸

剩下仅一个主要属性，为此我们将添加最后一个函数：

```c++
VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {

}
```

交换尺寸是交换链图像的分辨率，它几乎始终等于我们要绘制到的窗口的分辨率。可能的分辨率尺寸在VkSurfaceCapabilitiesKHR结构中定义。 Vulkan告诉我们通过在currentExtent成员中设置宽度和高度来匹配窗口的分辨率。但是，某些窗口管理器确实允许我们在这里有所不同，这可以通过将currentExtent中的宽度和高度设置为特殊值来表示：uint32_t的最大值。在这种情况下，我们将选择与minImageExtent和maxImageExtent边界内的窗口最匹配的分辨率。

```c++
#include <cstdint> // Necessary for UINT32_MAX

...

VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
    if (capabilities.currentExtent.width != UINT32_MAX) {
        return capabilities.currentExtent;
    } else {
        VkExtent2D actualExtent = {WIDTH, HEIGHT};

        actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
        actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

        return actualExtent;
    }
}
```

此处使用max和min函数将WIDTH和HEIGHT的值限制在实现支持的允许的最小和最大范围之间。确保包括<algorithm>头文件以使用它们。

#### Creating the swap chain

#### 创建交换链

现在，我们拥有所有这些帮助程序功能来帮助我们在运行时进行选择，我们终于有了创建能够工作的交换链所需的所有信息。

创建一个createSwapChain函数，该函数从这些调用的结果开始，并确保在创建逻辑设备后从initVulkan

```c++
void initVulkan() {
    createInstance();
    setupDebugMessenger();
    createSurface();
    pickPhysicalDevice();
    createLogicalDevice();
    createSwapChain();
}

void createSwapChain() {
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);

    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);
```

除了这些属性外，我们还必须确定交换链中要包含多少个图像。该实现指定其运行所需的最小数量：

```c++
uint32_t imageCount = swapChainSupport.capabilities.minImageCount;
```

但是，只用最小值意味着我们有时可能需要等待驱动程序完成内部操作，然后才能获取要渲染的另一张图像。因此，建议您至少多请求一张图片：

```c++
uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
```

在执行此操作时，我们还应确保不超过最大图像数，其中0是一个特殊值，表示没有最大图像数：

```c++
if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
    imageCount = swapChainSupport.capabilities.maxImageCount;
}
```

与Vulkan对象的传统一样，创建交换链对象需要填充大型结构体。它的开头我们非常熟悉：

```c++
VkSwapchainCreateInfoKHR createInfo = {};
createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
createInfo.surface = surface;
```

在指定交换链应绑定到哪个表面之后，将指定交换链图像的详细信息：

```c++
createInfo.minImageCount = imageCount;
createInfo.imageFormat = surfaceFormat.format;
createInfo.imageColorSpace = surfaceFormat.colorSpace;
createInfo.imageExtent = extent;
createInfo.imageArrayLayers = 1;
createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
```

imageArrayLayers指定每个图像组成的层数。除非您正在开发立体3D应用程序，否则始终为1。 imageUsage位字段指定我们将对交换链中的图像执行哪种操作。在本教程中，我们将直接对其进行渲染，这意味着它们将用作颜色附件。也有可能首先将图像渲染为单独的图像，以执行诸如后处理之类的操作。在这种情况下，您可以改用VK_IMAGE_USAGE_TRANSFER_DST_BIT之类的值，并使用内存操作将渲染的图像传输到交换链图像。

```c++
QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

if (indices.graphicsFamily != indices.presentFamily) {
    createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    createInfo.queueFamilyIndexCount = 2;
    createInfo.pQueueFamilyIndices = queueFamilyIndices;
} else {
    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.queueFamilyIndexCount = 0; // Optional
    createInfo.pQueueFamilyIndices = nullptr; // Optional
}
```

接下来，我们需要指定如何处理将在多个队列系列中使用的交换链图像。如果在我们的应用程序中图形队列家族与呈现队列不同。我们将从图形队列中绘制交换链中的图像，然后将其提交到演示队列中。有两种方法可以处理从多个队列访问的图像：

VK_SHARING_MODE_EXCLUSIVE：图像一次由一个队列族拥有，并且必须在另一个队列族中使用它之前显式转移所有权。此选项提供最佳性能。
VK_SHARING_MODE_CONCURRENT：可以在多个队列族之间使用图像，而无需显式所有权转移。

如果队列系列不同，那么在本教程中我们将使用并发模式以避免制作所有权相关章节，因为涉及一些概念，这些概念在以后会得到更好的解释。并发模式要求您预先使用queueFamilyIndexCount和pQueueFamilyIndices参数指定要在哪个队列家族所有权之间进行共享。如果图形队列家族和演示队列家族相同（在大多数硬件上都是这种情况），那么我们应该使用独占模式，因为并发模式要求您至少指定两个不同的队列家族。

```c++
createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
```

如果支持的话，则可以指定对交换链中的图像进行某种变换（capabilities中的supportedTransforms），例如顺时针旋转90度或水平翻转。如果您不希望进行任何转换，只需指定currentTransform即可。

```c++
createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
```

compositeAlpha字段指定是否应将alpha通道用于与窗口系统中的其他窗口混合。您几乎总是想简单地忽略Alpha通道，因此使用VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR。

```c++
createInfo.presentMode = presentMode;
createInfo.clipped = VK_TRUE;
```

presentMode成员不言自明。如果将裁剪的成员设置为VK_TRUE，则意味着我们不在乎被遮盖的像素的颜色，例如，因为在它们前面有另一个窗口。除非您真的需要能够读回这些像素并获得可预测的结果，否则通过启用裁剪将获得最佳性能。

```c++
createInfo.oldSwapchain = VK_NULL_HANDLE;
```

剩下的最后一个字段是oldSwapChain。使用Vulkan时，您的交换链可能在应用程序运行时无效或未优化，例如，因为调整了窗口大小。在这种情况下，实际上需要从头开始重新创建交换链，并且必须在该字段中指定对旧交换链的引用。这是一个复杂的主题，我们将在以后的章节中进一步了解。现在，我们假设我们只会创建一个交换链。

现在添加一个类成员来存储VkSwapchainKHR对象：

```c++
VkSwapchainKHR swapChain;
```

现在，创建交换链只需简单调用vkCreateSwapchainKHR：

```c++
if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS) {
    throw std::runtime_error("failed to create swap chain!");
}
```

参数是逻辑设备，交换链创建信息，可选的自定义分配器以及指向用于存储句柄的变量的指针。应该在设备之前使用vkDestroySwapchainKHR对其进行清理：

```c++
void cleanup() {
    vkDestroySwapchainKHR(device, swapChain, nullptr);
    ...
}
```

现在运行该应用程序，以确保成功创建交换链！如果此时您在vkCreateSwapchainKHR中遇到访问冲突错误，或在SteamOverlayVulkanLayer.dll层中看到类似“Failed to find 'vkGetInstanceProcAddress' in layer SteamOverlayVulkanLayer.dll”的消息，则请参阅关于Steam Overlay的[FAQ](https://vulkan-tutorial.com/en/FAQ)条目。

尝试在启用启用验证层的情况下删除createInfo.imageExtent = extent；。您将看到验证层之一立即发现错误，并显示一条有用的消息：

![img](https://vulkan-tutorial.com/images/swap_chain_validation_layer.png)

#### Retrieving the swap chain images

#### 获取交换链图像

现在已经创建了交换链，因此剩下的只是获取其中的VkImage的句柄。在后面的章节中，我们将在渲染操作中引用这些内容。添加一个类成员来存储句柄：

```c++
std::vector<VkImage> swapChainImages;
```

图像是由交换链的实现创建的，一旦交换链被破坏，它们将被自动清理，因此我们不需要添加任何清理代码。

我在vkCreateSwapchainKHR调用之后添加了代码，在createSwapChain函数末尾获取句柄。这个过程与我们其他时候从Vulkan获取对象数组非常相似。请记住，我们仅在交换链中指定了最小数量的图像，因此实现允许创建具有更多图像的交换链。这就是为什么我们将首先使用vkGetSwapchainImagesKHR查询图像的最终数量，然后调整容器的大小，最后再次调用它以获取句柄。

```c++
vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
swapChainImages.resize(imageCount);
vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());
```

最后一件事，将我们为交换链图像选择的格式和尺寸存储在成员变量中。在以后的章节中，我们将需要它们。

```c++
VkSwapchainKHR swapChain;
std::vector<VkImage> swapChainImages;
VkFormat swapChainImageFormat;
VkExtent2D swapChainExtent;

...

swapChainImageFormat = surfaceFormat.format;
swapChainExtent = extent;
```

现在，我们有了一组可以绘制到窗口上并可以呈现给窗口的图像。下一章将开始介绍如何将图像设置为渲染目标，然后开始研究实际的图形管道和绘图命令！

### Image views

### 图像视图

要在渲染管线中使用任何VkImage（包括交换链中的VkImage），我们必须创建一个VkImageView对象。图像视图描述了如何访问图像以及访问图像的哪一部分，例如，是否应将其视为2D深度纹理而没有任何mipmap。

在本章中，我们将编写一个createImageViews函数，该函数为交换链中的每个图像创建一个基本图像视图，以便以后可以将它们用作颜色目标。

首先添加一个类成员来存储图像视图：

```c++
std::vector<VkImageView> swapChainImageViews;
```

创建createImageViews函数，并在交换链创建后立即调用它。

```c++
void initVulkan() {
    createInstance();
    setupDebugMessenger();
    createSurface();
    pickPhysicalDevice();
    createLogicalDevice();
    createSwapChain();
    createImageViews();
}

void createImageViews() {

}
```

我们需要做的第一件事是调整列表的大小以适合我们将要创建的所有图像视图：

接下来，设置遍历所有交换链映像的循环。

```c++
for (size_t i = 0; i < swapChainImages.size(); i++) {

}
```

在VkImageViewCreateInfo结构中指定用于创建图像视图的参数。前几个参数很简单。

```c++
VkImageViewCreateInfo createInfo = {};
createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
createInfo.image = swapChainImages[i];
```

viewType和format字段指定应如何解释图像数据。 viewType参数允许您将图像视为1D纹理，2D纹理，3D纹理和立方体贴图。

```c++
createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
createInfo.format = swapChainImageFormat;
```

components字段可让您调整颜色通道。例如，您可以将所有通道映射到红色以获得单色纹理。您还可以将常数0和1映射到通道。在我们的情况下将使用默认映射。

```c++
createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
```

subresourceRange字段描述图像的用途以及应该访问图像的哪一部分。我们的图像将用作颜色目标，而没有任何mipmap或多层。

```c++
createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
createInfo.subresourceRange.baseMipLevel = 0;
createInfo.subresourceRange.levelCount = 1;
createInfo.subresourceRange.baseArrayLayer = 0;
createInfo.subresourceRange.layerCount = 1;
```

如果您正在使用立体3D应用程序，则将创建一个具有多层的交换链。然后，您可以通过访问不同的层为每个图像创建代表左眼和右眼视图的多个图像视图。

现在，创建图像视图只需调用vkCreateImageView：

```c++
if (vkCreateImageView(device, &createInfo, nullptr, &swapChainImageViews[i]) != VK_SUCCESS) {
    throw std::runtime_error("failed to create image views!");
}
```

与图像不同，图像视图是由我们明确创建的，因此我们需要添加一个类似的循环以在程序结束时销毁它们：

```c++
void cleanup() {
    for (auto imageView : swapChainImageViews) {
        vkDestroyImageView(device, imageView, nullptr);
    }

    ...
}
```

图像视图足以开始使用图像作为纹理，但是还没有准备好用作渲染目标。这需要一个间接的步骤，称为帧缓冲区。但是首先我们必须设置图形管綫。

## Graphics pipeline basics

## 图形管线基础

### Introduction

### 简介

在接下来的几章中，我们将建立一个图形管线，该管线被配置为绘制第一个三角形。图形管线是一系列操作，这些操作将网格的顶点和纹理一直带到渲染目标中的像素。简化的概述如下所示：

![img](https://vulkan-tutorial.com/images/vulkan_simplified_pipeline.svg)

输入装配器从您指定的缓冲区中收集原始顶点数据，并且还可以使用索引缓冲区来重复某些元素，而不必复制顶点数据本身。

顶点着色器针对每个顶点运行，并且通常应用变换以将顶点位置从模型空间转换到屏幕空间。它还将每个顶点的数据向下传递到管线。

镶嵌着色器可让您根据某些规则细分几何形状，以提高网格质量。这通常用于使诸如砖墙和楼梯的表面使得在它们附近时看起来不太平坦。

几何着色器在每个图元（三角形，直线，点）上运行，并且可以丢弃它或输出比输入时更多的图元。这与镶嵌细分着色器相似，但更加灵活。但是，它在当今的应用程序中使用不多，因为除英特尔的集成GPU外，它在大多数图形卡的性能都不太好。

光栅化阶段将图元离散为片段。这些是它们填充在帧缓冲区上的像素元素。如图所示，所有落在屏幕外部的片段都将被丢弃，并且顶点着色器输出的属性将被差值入到这些片段之间。通常，由于进行深度测试，其他原始片段背后的片段也会在此处被丢弃。

片段着色器将为每个到这个阶段尚存的片段调用，并确定将片段写入哪个帧缓冲区以及使用哪个颜色和深度值。它可以使用来自顶点着色器的插值数据来执行此操作，其中可以包括诸如纹理坐标和光照法线之类的内容。

颜色混合阶段应用操作来混合映射到帧缓冲区中同一像素的不同片段。片段可以根据透明度简单地相互覆盖，累加或混合。

绿色的阶段称为固定功能阶段。这些阶段允许您使用参数来调整其操作，但是它们的工作方式是预定义的。

另一方面，橙色的阶段是可编程的，这意味着您可以将自己的代码上传到图形卡以完全应用所需的操作。例如，这使您可以使用片段着色器，以实现从纹理化和光照到光线追踪器的所有功能。这些程序同时在许多GPU内核上运行，以并行处理许多对象，例如顶点和片段。

如果您以前使用过OpenGL和Direct3D等较旧的API，那么您将习惯于通过调用glBlendFunc和OMSetBlendState随意更改任何管线设置。 Vulkan中的图形管线几乎是不可改变的，因此，如果要更改着色器，绑定不同的帧缓冲区或更改混合功能，则必须从头开始重新创建管线。缺点是您必须创建许多管线，这些管线代表要在渲染操作中使用的状态的所有不同组合。但是，由于您将预先知道管线中要执行的所有操作，因此驱动程序可以对其进行更好的优化。

根据您的计划，某些可编程阶段是可选的。例如，如果仅绘制简单几何图形，则可以禁用镶嵌和几何图形阶段。如果仅对深度值感兴趣，则可以禁用片段着色器阶段，这对于生成阴影贴图很有用。

在下一章中，我们将首先创建在屏幕上放置三角形所需的两个可编程阶段：顶点着色器和片段着色器。固定功能配置，例如混合模式，视口，栅格化将在随后的章节中进行设置。在Vulkan中设置图形管线的最后一部分涉及输入和输出帧缓冲区的规范。

创建一个createGraphicsPipeline函数，该函数将在initVulkan中的createImageViews之后立即调用。在以下各章中，我们将专注于这个函数。

```c++
void initVulkan() {
    createInstance();
    setupDebugMessenger();
    createSurface();
    pickPhysicalDevice();
    createLogicalDevice();
    createSwapChain();
    createImageViews();
    createGraphicsPipeline();
}

...

void createGraphicsPipeline() {

}
```

### Shader module

### 着色器模块

与早期的API不同，Vulkan中的着色器代码必须以字节码格式指定，而不是像GLSL和HLSL这样的人类可读语法。这种字节码格式称为[SPIR-V](https://www.khronos.org/spir)(Standard Portable Intermediate Representation)，旨在与Vulkan和OpenCL（均使用Khronos API）一起使用。它是一种可用于编写图形和计算着色器的格式，但在本教程中，我们将重点介绍Vulkan图形管线中使用的着色器。

使用字节码格式的优势在于，由GPU供应商编写的将着色器代码转换为本地代码的编译器的复杂度大大降低。过去已经表明，使用诸如GLSL之类的人类可读语法，一些GPU供应商对标准的解释相当灵活。如果您碰巧使用其中一个供应商提供的GPU编写非平凡的着色器，则可能会冒着其他供应商的驱动程序由于语法错误而拒绝代码的风险，或者更糟糕的是，由于编译器错误，着色器将以不同的方式运行。使用像SPIR-V这样简单的字节码格式有希望避免这些问题。

但是，这并不意味着我们需要手动编写此字节码。 Khronos发布了他们自己的独立于供应商的编译器，该编译器将GLSL编译为SPIR-V。该编译器旨在验证您的着色器代码完全符合标准，并生成一个SPIR-V二进制文件，您可以将其与程序一起提供。您也可以将此编译器作为库包含在内，以在运行时生成SPIR-V，但在本教程中我们不会这样做。尽管我们可以直接通过glslangValidator.exe使用此编译器，但我们将改为使用Google的glslc.exe。 glslc的优点在于，它使用与GCC和Clang等知名编译器相同的参数格式，并包含一些额外的功能（如include）。它们都已包含在Vulkan SDK中，因此您无需下载任何其他内容。

GLSL是一种具有C样式语法的着色语言。用它编写的程序具有为每个对象调用的main函数。 GLSL使用全局变量来处理输入和输出，而不是使用输入参数和返回值作为输出。该语言包括许多有助于图形编程的功能，例如内置矢量和矩阵基本类型。包括叉乘，矩阵向量乘积以及向量0反射等运算函数。向量类型称为vec，带有表示元素数量的数字。例如，将3D位置存储在vec3中。可以通过.x之类的成员访问单个组件，但也可以同时从多个组件创建一个新的向量。例如，表达式vec3（1.0，2.0，3.0）.xy将产生vec2。向量的构造函数也可以采用向量对象和标量值的组合。例如，可以使用vec3（vec2（1.0，2.0），3.0）构造vec3。

如前一章所述，我们需要编写一个顶点着色器和一个片段着色器以在屏幕上显示一个三角形。接下来的两节将介绍其中每一个的GLSL代码，然后，我将向您展示如何生成两个SPIR-V二进制文件并将其加载到程序中。

#### Vertex Shader

#### 顶点着色器

顶点着色器处理每个传入的顶点。它以其属性（例如世界位置，颜色，法线和纹理坐标）作为输入。输出是裁剪坐标系中的最终位置以及需要传递到片段着色器的属性，例如颜色和纹理坐标。然后，这些值将被光栅化器插值到片段上，以生成平滑的渐变。

裁剪坐标是来自顶点着色器的四维矢量，随后通过将整个矢量除以其最后一个分量，将其转换为归一化的设备坐标。这些规范化的设备坐标是齐次坐标，它们将帧缓冲区映射到一個[-1, 1] x [-1, 1]的坐标系，如下所示：

![img](https://vulkan-tutorial.com/images/normalized_device_coordinates.svg)

**如果您以前涉足过计算机图形学，则应该已经熟悉这些内容。如果您以前使用过OpenGL，则会注意到Y坐标的符号现在被翻转了，Z坐标使用与Direct3D相同的范围，从0到1**。

对于第一个三角形，我们将不应用任何变换，我们将三个顶点的位置直接指定为归一化设备坐标即可创建以下形状：

![img](https://vulkan-tutorial.com/images/triangle_coordinates.svg)

我们可以在顶点着色器中将片段坐标最后一位设置为1来直接输出归一化设备坐标。这样，将片段坐标转换为归一化设备坐标的透视触发不会改变任何东西。

通常，这些坐标将存储在顶点缓冲区中，但是在Vulkan中创建顶点缓冲区并将其填充数据并非易事。因此，我决定将其推迟到直到看到屏幕上显示一个三角形为止。同时，我们将做一些非常规的事情：将坐标直接包含在顶点着色器中。代码如下：

```c++
#version 450

vec2 positions[3] = vec2[](
    vec2(0.0, -0.5),
    vec2(0.5, 0.5),
    vec2(-0.5, 0.5)
);

void main() {
    gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
}
```

每个顶点都会调用main函数。内置的gl_VertexIndex变量包含当前顶点的索引。这通常是顶点缓冲区的索引，但在我们的情况下，它将是顶点数据的硬编码数组的索引。可从着色器中的常数数组访问每个顶点的位置，并将其与虚拟z和w分量组合以在裁剪坐标中生成一个位置。内置变量gl_Position用作输出。

#### Fragment shader

#### 片段着色器

由顶点着色器的位置形成的三角形用片段填充屏幕上的一个区域。在这些片段上调用片段着色器以为帧缓冲区（或多个帧缓冲区）生成颜色和深度。一个简单的片段着色器，为整个三角形输出红色，如下所示：

```c++
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec4 outColor;

void main() {
    outColor = vec4(1.0, 0.0, 0.0, 1.0);
}
```

就像顶点着色器一样，为每个片段调用main函数。 GLSL中的颜色是4成分向量，其中R，G，B和alpha通道在[0，1]范围内。与顶点着色器中的gl_Position不同，没有内置变量可为当前片段输出颜色。您必须为每个帧缓冲区指定自己的输出变量，其中layout（location = 0）修饰符指定帧缓冲区的索引。将红色写入此outColor变量，该变量链接到索引为0的第一个（也是唯一一个）帧缓冲区。

#### Per-vertex colors

#### 每顶点颜色

将整个三角形设为红色不是很有趣，下面的内容看起来不是会更好吗？

![img](https://vulkan-tutorial.com/images/triangle_coordinates_colors.png)

为此，我们必须对两个着色器进行一些更改。首先，我们需要为三个顶点分别指定不同的颜色。顶点着色器现在应该包括一个颜色数组，就像其位置一样：

```c++
vec3 colors[3] = vec3[](
    vec3(1.0, 0.0, 0.0),
    vec3(0.0, 1.0, 0.0),
    vec3(0.0, 0.0, 1.0)
);
```

现在我们只需要将这些每个顶点的颜色传递到片段着色器，以便它可以将其插值输出到帧缓冲区。将输出的颜色添加到顶点着色器，并在main函数中写入：

```c++
layout(location = 0) out vec3 fragColor;

void main() {
    gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
    fragColor = colors[gl_VertexIndex];
}
```

接下来，我们需要在片段着色器中添加一个匹配的输入：

```c++
layout(location = 0) in vec3 fragColor;

void main() {
    outColor = vec4(fragColor, 1.0);
}
```

输入变量不一定必须使用相同的名称，它们将使用location指令指定的索引链接在一起。main被修改以输出颜色以及alpha值。如上图所示，将自动为三个顶点之间的片段内插fragColor的值，从而产生平滑的渐变。

#### Compiling the shaders

#### 编译着色器

在项目的根目录中创建一个名为shader的目录，并将顶点着色器存储在该目录中的shader.vert文件中，并将片段着色器存储在该目录中的shader.frag文件中。 GLSL着色器没有官方扩展名，但是这两个通常用于区分它们。

shader.vert的内容应为：

```c++
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec3 fragColor;

vec2 positions[3] = vec2[](
    vec2(0.0, -0.5),
    vec2(0.5, 0.5),
    vec2(-0.5, 0.5)
);

vec3 colors[3] = vec3[](
    vec3(1.0, 0.0, 0.0),
    vec3(0.0, 1.0, 0.0),
    vec3(0.0, 0.0, 1.0)
);

void main() {
    gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
    fragColor = colors[gl_VertexIndex];
}
```

并且shader.frag的内容应为：

```c++
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 fragColor;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = vec4(fragColor, 1.0);
}
```

现在，我们将使用glslc程序将它们编译为SPIR-V字节码。

**Windows**

创建具有以下内容的compile.bat文件：

```bash
C:/VulkanSDK/x.x.x.x/Bin32/glslc.exe shader.vert -o vert.spv
C:/VulkanSDK/x.x.x.x/Bin32/glslc.exe shader.frag -o frag.spv
pause
```

用您安装Vulkan SDK的路径替换glslc.exe的路径。双击运行该文件。

**End of platform-specific instructions**

**平台特定说明的结尾**

这两个命令告诉编译器读取GLSL源文件，并使用-o（输出）标志输出SPIR-V字节码文件。

如果您的着色器包含语法错误，则编译器将按照您的预期告诉您行号和问题。例如，尝试省略分号，然后再次运行编译脚本。也可以尝试在不带任何参数的情况下运行编译器，以查看其支持哪些类型的标志。例如，它还可以将字节码输出为人类可读的格式，以便您可以准确地看到着色器正在做什么以及在此阶段已应用的任何优化。

在命令行上编译着色器是最直接的选项之一，也是我们在本教程中将要使用的选项，但是也可以直接从您自己的代码中编译着色器。 Vulkan SDK包含libshaderc，这是一个从程序内部将GLSL代码编译为SPIR-V的库。

#### Load a shader

#### 加载一个着色器

现在我们有了生产SPIR-V着色器的方法，是时候将它们加载到我们的程序中，以便将它们插入到图形管线中了。首先，我们将编写一个简单的辅助函数，以从文件中加载二进制数据。

```c++
#include <fstream>

...

static std::vector<char> readFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        throw std::runtime_error("failed to open file!");
    }
}
```

readFile函数将从指定文件中读取所有字节，并将它们返回到由std :: vector管理的字节数组中。我们首先打开带有两个标志的文件：

ate：从文件末尾开始阅读
binary：将文件读取为二进制文件（避免文本转换）

从文件末尾开始读取的好处是，我们可以使用读取位置来确定文件的大小并分配缓冲区：

```c++
size_t fileSize = (size_t) file.tellg();
std::vector<char> buffer(fileSize);
```

之后，我们可以返回文件的开头并立即读取所有字节：

最后关闭文件并返回字节：

```c++
file.close();

return buffer;
```

现在，我们将从createGraphicsPipeline调用此函数以加载两个着色器的字节码：

```c++
void createGraphicsPipeline() {
    auto vertShaderCode = readFile("shaders/vert.spv");
    auto fragShaderCode = readFile("shaders/frag.spv");
}
```

通过打印缓冲区的大小并检查它们是否与实际文件大小（以字节为单位）匹配，以确保正确加载了着色器。请注意，由于该代码是二进制代码，因此不需要以null结尾的代码，我们稍后将对其大小进行明确说明。

#### Creating shader modeule

#### 创建着色器模块

在将代码传递到管线之前，我们必须将其包装在VkShaderModule对象中。让我们创建一个辅助函数createShaderModule来做到这一点。

该函数将使用字节码作为参数的缓冲区，并从中创建一个VkShaderModule。

创建着色器模块很简单，我们只需要使用字节码及其长度指定指向缓冲区的指针即可。此信息在VkShaderModuleCreateInfo结构中指定。一个陷阱是字节码的大小以字节为单位指定，但是字节码指针是uint32_t指针而不是char指针。因此，我们将需要使用reinterpret_cast转换指针，如下所示。当执行这样的转换时，还需要确保数据满足uint32_t的对齐要求。对我们来说幸运的是，数据存储在std :: vector中，其中默认分配器已经确保数据满足最坏情况下的对齐要求。

```c++
VkShaderModuleCreateInfo createInfo = {};
createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
createInfo.codeSize = code.size();
```

然后可以通过调用vkCreateShaderModule来创建VkShaderModule：

```c++
VkShaderModule shaderModule;
if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
    throw std::runtime_error("failed to create shader module!");
}
```

这些参数与以前的对象创建函数中的参数相同：逻辑设备，创建信息结构的指针，指向自定义分配器的可选指针以及处理输出变量。创建着色器模块后，可以立即释放带有代码的缓冲区。不要忘记返回创建的着色器模块：

```c++
return shaderModule;
```

着色器模块只是我们先前从文件及其中定义的函数加载的着色器字节码的轻量级包装。在创建图形管线之前，以供GPU执行的SPIR-V字节码到机器代码的编译和链接不会发生。这意味着我们可以在管道创建完成后立即销毁着色器模块，这就是为什么我们要在createGraphicsPipeline函数而不是类成员中将它们设为局部变量：

```c++
void createGraphicsPipeline() {
    auto vertShaderCode = readFile("shaders/vert.spv");
    auto fragShaderCode = readFile("shaders/frag.spv");

    VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
    VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);
```

然后，通过两个vkDestroyShaderModule调用，在函数的末尾进行清理。本章中其余代码的AL1将插入这两行调用之前。

```c++
    ...
    vkDestroyShaderModule(device, fragShaderModule, nullptr);
    vkDestroyShaderModule(device, vertShaderModule, nullptr);
}
```

#### Shader stage creation

#### 着色器阶段创建

要实际使用着色器，我们需要通过VkPipelineShaderStageCreateInfo结构将它们分配到特定的管线阶段，这是实际管线创建过程的一部分。

我们将再次在createGraphicsPipeline函数中填充顶点着色器的结构。

```c++
VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
```

除了必需的sType成员之外，第一步是告诉Vulkan将在哪个管线阶段使用着色器。上一章中描述的每个可编程阶段都有一个枚举值。

接下来的两个成员指定包含代码的着色器模块以及要调用的函数（称为入口点）。这意味着可以将多个片段着色器组合到一个着色器模块中，并使用不同的入口点来区分它们的行为。但是，在这种情况下，我们将使用标准的main。

还有一个（可选）成员pSpecializationInfo，我们这里不使用它，但是值得讨论。它允许您指定着色器常量的值。您可以使用单个着色器模块，可以通过在其中创建的常量指定不同的值来配置其行为。这比在渲染时使用变量配置着色器更高效，因为编译器可以进行优化，例如消除依赖于这些值的if语句。如果没有这样的常量，则可以将成员设置为nullptr，这是我们的结构初始化自动完成的。

修改结构以适合片段着色器很容易：

```c++
VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
fragShaderStageInfo.module = fragShaderModule;
fragShaderStageInfo.pName = "main";
```

通过定义一个包含这两个结构的数组来完成，我们稍后将在实际的管线创建步骤中来引用它们。

```c++
VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};
```

这就是描述管线的可编程阶段的全部内容。在下一章中，我们将介绍固定功能阶段。

### Fixed functions

### 固定功能

较旧的图形API为图形管线的大多数阶段提供了默认状态。在Vulkan中，您必须明确说明所有内容，从视口大小到颜色混合功能。在本章中，我们将填写所有结构以配置这些固定功能的操作。

#### Vertex input

#### 顶点输入

VkPipelineVertexInputStateCreateInfo结构描述将传递到顶点着色器的顶点数据的格式。它大致以两种方式对此进行了描述：

* 绑定：数据之间的间距以及数据是按顶点还是按实例（请参见实例化）
* 属性描述：传递给顶点着色器的属性的类型，该属性绑定以从中以及在哪个偏移处加载它们

因为我们要直接在顶点着色器中对顶点数据进行硬编码，所以我们将填充此结构以指定目前没有要加载的顶点数据。我们将在“顶点缓冲区”一章中重新介绍它。

```c++
VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
vertexInputInfo.vertexBindingDescriptionCount = 0;
vertexInputInfo.pVertexBindingDescriptions = nullptr; // Optional
vertexInputInfo.vertexAttributeDescriptionCount = 0;
vertexInputInfo.pVertexAttributeDescriptions = nullptr; // Optional
```

pVertexBindingDescriptions和pVertexAttributeDescriptions成员指向一个结构数组，这些结构描述了上述用于加载顶点数据的详细信息，将此结构添加到createGraphicsPipeline函数的shaderStages数组之后。

#### Input assembly

#### 输入装配器

VkPipelineInputAssemblyStateCreateInfo结构描述了两件事：将从顶点绘制哪种几何形状以及是否应启用图元重启。前者在拓扑成员中指定，并且可以具有如下值：

* VK_PRIMITIVE_TOPOLOGY_POINT_LIST：来自顶点的点
* VK_PRIMITIVE_TOPOLOGY_LINE_LIST：每2个顶点一条线，不重复使用顶点
* VK_PRIMITIVE_TOPOLOGY_LINE_STRIP：每行的终点用作下一行的起点
* VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST：每3个顶点一个三角形，不重复使用顶点
* VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP：每个三角形的第二个和第三个顶点用作下一个三角形的前两个顶点
  通常，顶点是按索引从顶点缓冲区按顺序加载的，但是使用元素缓冲区，您可以指定要使用的索引。这使您可以执行优化，例如重用顶点。如果将primaryRestartEnable成员设置为VK_TRUE，则可以通过使用特殊索引0xFFFF或0xFFFFFFFF在_STRIP拓扑模式下分解直线和三角形。

我们打算在本教程中绘制三角形，因此我们将使用以下结构数据：

```c++
VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
inputAssembly.primitiveRestartEnable = VK_FALSE;
```

#### Viewports and scissors

#### 视口和剪裁器

视口基本上描述了将输出渲染到的帧缓冲区的区域。这几乎总是（0，0）到（宽度，高度），在本教程中也是如此。

```c++
VkViewport viewport = {};
viewport.x = 0.0f;
viewport.y = 0.0f;
viewport.width = (float) swapChainExtent.width;
viewport.height = (float) swapChainExtent.height;
viewport.minDepth = 0.0f;
viewport.maxDepth = 1.0f;
```

请记住，交换链及其图像的大小可能与窗口的宽度和高度不同。交换链图像稍后将用作帧缓冲区，因此我们应保持其大小。

minDepth和maxDepth值指定要用于帧缓冲区的深度值的范围。这些值必须在[0.0f，1.0f]范围内，但minDepth可能高于maxDepth。如果您没有做任何特别的事情，那么您应该坚持使用0.0f和1.0f的标准值。

视口定义了从图像到帧缓冲区的转换，而剪裁矩形定义了实际将存储像素的区域。剪裁矩形之外的所有像素将被光栅化器丢弃。它们的作用就像过滤器而不是转换。区别说明如下。请注意，只要它大于视口，左侧的剪裁矩形就是产生该图像的多种可能性之一。

![img](https://vulkan-tutorial.com/images/viewports_scissors.png)

在本教程中，我们只想绘制整个帧缓冲区，因此我们将指定一个完全覆盖它的剪裁矩形：

```c++
VkRect2D scissor = {};
scissor.offset = {0, 0};
scissor.extent = swapChainExtent;
```

现在，需要使用VkPipelineViewportStateCreateInfo结构将此视口和剪裁矩形组合为视口状态。在某些图形卡上可以使用多个视口和剪裁矩形，因此其成员引用了它们的数组。使用多个视口需要启用GPU功能（请参阅逻辑设备创建）。

```c++
VkPipelineViewportStateCreateInfo viewportState = {};
viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
viewportState.viewportCount = 1;
viewportState.pViewports = &viewport;
viewportState.scissorCount = 1;
viewportState.pScissors = &scissor;
```



#### Rasterizer

#### 光栅化器

#### Multisampling

#### 多重采样

#### Depth and stencil testing

#### 深度和模板测试

#### Color Blending

#### 颜色混合

#### Dynamic state

#### 动态状态

#### Pipeline layout

#### 管线布局

#### Conclusion

#### 总结

