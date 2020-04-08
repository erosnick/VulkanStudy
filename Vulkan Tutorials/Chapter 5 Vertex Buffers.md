[TOC]

# Chapter 5 Vertex buffers

# 第5章

## Vertex input description

## 顶点输入描述

### Introduction

### 简介

在接下来的几章中，我们将用内存中的顶点缓冲区替换顶点着色器中的硬编码顶点数据。我们将从最简单方法开始，创建CPU可见缓冲区并使用memcpy将顶点数据直接复制到其中，然后，我们将了解如何使用暂存缓冲区将顶点数据复制到高性能内存。

### Vertex shader

### 顶点着色器

首先将顶点着色器更改为不再将顶点数据包含在着色器代码本身中。顶点着色器使用in关键字从顶点缓冲区获取输入。

```c++
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec3 inColor;

layout(location = 0) out vec3 fragColor;

void main() {
    gl_Position = vec4(inPosition, 0.0, 1.0);
    fragColor = inColor;
}
```

inPosition和inColor变量是顶点属性。它们是在顶点缓冲区中为每个顶点指定的属性，就像我们使用两个数组为每个顶点手动指定位置和颜色一样。确保重新编译顶点着色器！

就像fragColor一样，layout（location = x）标注为输入分配索引，我们以后可以使用索引来引用它们。重要的是要知道某些类型（例如dvec3 64位向量）使用多个插槽。这意味着之后的索引必须至少提高2倍：

```c++
layout(location = 0) in dvec3 inPosition;
layout(location = 2) in vec3 inColor;
```

您可以在[OpenGL Wiki](https://www.khronos.org/opengl/wiki/Layout_Qualifier_(GLSL))中找到有关布局限定符的更多信息。

### Vertex data

### 顶点数据

我们正在将顶点数据从着色器代码移到程序代码的数组中。首先包括GLM库，该库为我们提供了与线性代数相关的类型，例如向量和矩阵。我们将使用这些类型来指定位置和颜色向量。

```c++
#include <glm/glm.hpp>
```

创建一个名为Vertex的新结构，该结构具有我们将在其中的顶点着色器中使用的两个属性：

```c++
struct Vertex {
    glm::vec2 pos;
    glm::vec3 color;
};
```

GLM方便地为我们提供了与着色器语言中使用的向量类型完全匹配的C ++类型。

```c++
const std::vector<Vertex> vertices = {
    {{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
    {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
    {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
};
```

现在，使用“Vertex”结构指定一个顶点数据数组。我们使用的位置和颜色值与以前完全相同，但是现在它们被合并为一个顶点数组。这称为交错(interleaving)顶点属性。

### Binding descriptions

### 绑定描述

下一步是告诉Vulkan一旦将数据格式上传到GPU内存后如何将其传递给顶点着色器。传达此信息需要两种类型的结构。

第一个结构是VkVertexInputBindingDescription，我们将向Vertex结构添加一个成员函数，以使用正确的数据填充该函数。

```c++
struct Vertex {
    glm::vec2 pos;
    glm::vec3 color;

    static VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription bindingDescription = {};

        return bindingDescription;
    }
};
```

顶点绑定描述了整个顶点从内存中加载数据的速率。它指定数据条目之间的字节数，以及是否在每个顶点之后或在每个实例之后移至下一个数据条目。

```c++
VkVertexInputBindingDescription bindingDescription = {};
bindingDescription.binding = 0;
bindingDescription.stride = sizeof(Vertex);
bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
```

我们所有的每个顶点数据都包装在一个数组中，因此我们只需要一个绑定。 binding参数指定绑定数组中绑定的索引。 stride参数指定从一个条目到下一个条目的字节数，inputRate参数可以具有以下值之一：

* VK_VERTEX_INPUT_RATE_VERTEX：在每个顶点之后移至下一个数据条目
* VK_VERTEX_INPUT_RATE_INSTANCE：每个实例后移至下一个数据条目

我们将不使用实例渲染，因此我们只使用每个顶点的数据。

### Attribute description

### 属性描述

描述如何处理顶点输入的第二个结构是VkVertexInputAttributeDescription。我们将向Vertex添加另一个辅助函数以填充这些结构。

```c++
#include <array>

...

static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions() {
    std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions = {};

    return attributeDescriptions;
}
```

如函数原型所示，将有两个这样的结构。属性描述结构描述了如何从源自绑定描述的大量顶点数据中提取顶点属性。我们具有位置和颜色这两个属性，因此我们需要两个属性描述结构。

```c++
attributeDescriptions[0].binding = 0;
attributeDescriptions[0].location = 0;
attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
attributeDescriptions[0].offset = offsetof(Vertex, pos);
```

binding参数告诉Vulkan每个顶点数据来自哪个绑定。 location参数引用顶点着色器中输入的location指令。位置为0的顶点着色器中的输入为position,，具有两个32位浮点分量。

format参数描述属性的数据类型。有点令人困惑，这些格式是使用与颜色格式相同的枚举指定的。以下着色器类型和格式通常一起使用：

* float：VK_FORMAT_R32_SFLOAT
* vec2：VK_FORMAT_R32G32_SFLOAT
* vec3：VK_FORMAT_R32G32B32_SFLOAT
* vec4：VK_FORMAT_R32G32B32A32_SFLOAT

如您所见，您应该使用颜色通道数量与着色器数据类型中的组件数量匹配的格式。允许使用比着色器中的组件数更多的通道，但是它们将被静默丢弃。如果通道数少于组件数，则BGA组件将使用默认值（0、0、1）。颜色类型（SFLOAT，UINT，SINT）和位宽也应与着色器输入的类型匹配。请参阅以下示例：

* ivec2：VK_FORMAT_R32G32_SINT，一个32位带符号整数的2分量向量
* uvec4：VK_FORMAT_R32G32B32A32_UINT，由32位无符号整数组成的4分量向量
* double：VK_FORMAT_R64_SFLOAT，双精度（64位）浮点型

format参数隐式定义属性数据的字节大小，而offset参数指定自要读取的每个顶点数据开始以来的字节数。绑定一次加载一个Vertex，并且position属性（pos）距离此结构的开头偏移0个字节。这是使用offsetof宏自动计算的。

```c++
attributeDescriptions[1].binding = 0;
attributeDescriptions[1].location = 1;
attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
attributeDescriptions[1].offset = offsetof(Vertex, color);
```

颜色属性的描述方式几乎相同。

### Pipeline vertex input

### 管线顶点输入

现在，我们需要通过引用createGraphicsPipeline中的结构来设置图形管道以接受这种格式的顶点数据。找到vertexInputInfo结构并对其进行修改以引用两个描述：

```c++
auto bindingDescription = Vertex::getBindingDescription();
auto attributeDescriptions = Vertex::getAttributeDescriptions();

vertexInputInfo.vertexBindingDescriptionCount = 1;
vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
```

现在，管线已准备好接受vertices容器格式的顶点数据，并将其传递给我们的顶点着色器。如果您现在在启用验证层的情况下运行该程序，则会看到它抱怨没有绑定到该绑定的顶点缓冲区。下一步是创建一个顶点缓冲区并将顶点数据移到该缓冲区，以便GPU能够访问它。

## Vertex buffer creation

## 顶点缓冲创建

### Introduction
### 简介

Vulkan中的缓冲区是内存区域，用于存储图形卡可以读取的任意数据。它们可以用来存储顶点数据，这将在本章中进行，但是它们也可以用于我们将在以后的章节中探讨的许多其他目的。与到目前为止我们一直在处理的Vulkan对象不同，缓冲区不会自动为其分配内存。前几章的工作表明，Vulkan API使程序员几乎可以控制所有事情，而内存管理就是其中之一。

### Buffer creation

### 缓冲创建

创建一个新函数createVertexBuffer并在createCommandBuffers之前从initVulkan调用它。

```c++
void initVulkan() {
    createInstance();
    setupDebugMessenger();
    createSurface();
    pickPhysicalDevice();
    createLogicalDevice();
    createSwapChain();
    createImageViews();
    createRenderPass();
    createGraphicsPipeline();
    createFramebuffers();
    createCommandPool();
    createVertexBuffer();
    createCommandBuffers();
    createSyncObjects();
}

...

void createVertexBuffer() {

}
```

创建缓冲区需要我们填充VkBufferCreateInfo结构。

```c++
VkBufferCreateInfo bufferInfo = {};
bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
bufferInfo.size = sizeof(vertices[0]) * vertices.size();
```

结构的第一个字段是size，它指定缓冲区的大小（以字节为单位）。使用sizeof可以轻松计算顶点数据的字节大小。

```c++
bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
```

第二个字段是usage，它指示缓冲区中的数据将用于哪个目的。可以使用按位或指定多个目的。我们的用例将是一个顶点缓冲区，我们将在以后的章节中介绍其他类型的用法。

```c++
bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
```

就像交换链中的图像一样，缓冲区也可以由特定的队列家族拥有，或同时在多个队列之间共享。该缓冲区将仅在图形队列中使用，因此我们可以用独占访问。

flags参数用于配置稀疏缓冲存储器，该参数现在不相关。我们将其保留为默认值0。

现在，我们可以使用vkCreateBuffer创建缓冲区。定义一个类成员来保存缓冲区句柄，并将其称为vertexBuffer。

缓冲区应可用于渲染命令，直到程序结束为止，并且它不依赖于交换链，因此我们将在原始cleanup函数中对其进行清理：

```c++
void cleanup() {
    cleanupSwapChain();

    vkDestroyBuffer(device, vertexBuffer, nullptr);

    ...
}
```

### Memory requirements

### 内存需求

缓冲区已创建，但实际上尚未分配任何内存。为缓冲区分配内存的第一步是使用适当命名的vkGetBufferMemoryRequirements函数查询其内存需求。

```c++
VkMemoryRequirements memRequirements;
vkGetBufferMemoryRequirements(device, vertexBuffer, &memRequirements);
```

VkMemoryRequirements结构具有三个字段：

* size：所需的内存量（以字节为单位），可能与bufferInfo.size不同。
* alignment：缓冲区从内存分配的区域开始的偏移量（以字节为单位），取决于bufferInfo.usage和bufferInfo.flags。
* memoryTypeBits：适用于缓冲区的内存类型的位字段。

图形卡可以提供不同类型的内存以进行分配。每种类型的内存在允许的操作和性能特征方面都不同。我们需要结合缓冲区的要求和我们自己的应用程序要求来找到要使用的正确类型的内存。为此，我们创建一个新函数findMemoryType。

```c++
uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {

}
```

首先，我们需要使用vkGetPhysicalDeviceMemoryProperties查询有关可用内存类型的信息。

```c++
VkPhysicalDeviceMemoryProperties memProperties;
vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);
```

VkPhysicalDeviceMemoryProperties结构具有两个数组memoryTypes和memoryHeaps。内存堆是不同的内存资源，例如专用VRAM和RAM中的交换空间（当VRAM用完时）。这些堆中存在不同类型的内存。现在，我们只关心内存的类型，而不关心它来自的堆，但是您可以想象这会影响性能。

首先让我们找到适合缓冲区本身的内存类型：

```c++
for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
    if (typeFilter & (1 << i)) {
        return i;
    }
}

throw std::runtime_error("failed to find suitable memory type!");
```

typeFilter参数将用于指定合适的内存类型的位字段。这意味着我们可以通过简单地遍历它们并检查相应的位是否设置为1来找到合适的内存类型的索引。

但是，我们不仅对适用于顶点缓冲区的内存类型感兴趣。我们还需要能够将顶点数据写入该内存。 memoryTypes数组由VkMemoryType结构组成，这些结构指定每种内存类型的堆和属性。这些属性定义了内存的特殊功能，例如能够对其进行映射，以便我们可以从CPU对其进行写入。该属性用VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT表示，但是我们还需要使用VK_MEMORY_PROPERTY_HOST_COHERENT_BIT属性。我们将在映射内存时看到原因。

现在，我们可以修改循环以也检查此属性的支持：

```c++
for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
    if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
        return i;
    }
}
```

我们可能有多个期望的属性，因此我们应该检查按位与的结果是否不仅为非零，还等于期望的属性位字段。如果存在适合该缓冲区的内存类型，并且该内存类型还具有我们需要的所有属性，则返回其索引，否则抛出异常。

### Memory allocation

### 内存分配

现在，我们有了一种确定正确的内存类型的方法，因此我们实际上可以通过填写VkMemoryAllocateInfo结构来分配内存。

```c++
VkMemoryAllocateInfo allocInfo = {};
allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
allocInfo.allocationSize = memRequirements.size;
allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
```

现在，内存分配就像指定大小和类型一样简单，这两者都是从顶点缓冲区的内存要求和所需属性中得出的。创建一个类成员以将句柄存储到内存中，并使用vkAllocateMemory对其进行分配。

```c++
VkBuffer vertexBuffer;
VkDeviceMemory vertexBufferMemory;

...

if (vkAllocateMemory(device, &allocInfo, nullptr, &vertexBufferMemory) != VK_SUCCESS) {
    throw std::runtime_error("failed to allocate vertex buffer memory!");
}
```

如果内存分配成功，那么我们现在可以使用vkBindBufferMemory将此内存与缓冲区关联：

```c++
vkBindBufferMemory(device, vertexBuffer, vertexBufferMemory, 0);
```

前三个参数是不言自明的，第四个参数是内存区域内的偏移量。由于此内存是专门为此顶点缓冲区分配的，因此偏移量是0。如果偏移量不为零，则要求它可以被memRequirements.alignment整除。

当然，就像C ++中的动态内存分配一样，应该在某个时候释放内存。一旦不再使用缓冲区，绑定到缓冲区对象的内存可能会释放，因此让我们在缓冲区被销毁后释放它：

```c++
void cleanup() {
    cleanupSwapChain();

    vkDestroyBuffer(device, vertexBuffer, nullptr);
    vkFreeMemory(device, vertexBufferMemory, nullptr);
```

### Filling the vertex buffer

### 填充顶点缓冲

现在是时候将顶点数据复制到缓冲区了。这是通过使用vkMapMemory将缓冲存储器映射到CPU可访问的内存来完成的。

```c++
void* data;
vkMapMemory(device, vertexBufferMemory, 0, bufferInfo.size, 0, &data);
```

此功能使我们可以访问由偏移量和大小定义的指定内存资源的区域。这里的偏移量和大小分别为0和bufferInfo.size。也可以指定特殊值VK_WHOLE_SIZE来映射所有内存。倒数第二个参数可用于指定标志，但是当前API中没有可用的标志。必须将其设置为值0。最后一个参数指定指向映射内存的指针的输出。

```c++
void* data;
vkMapMemory(device, vertexBufferMemory, 0, bufferInfo.size, 0, &data);
    memcpy(data, vertices.data(), (size_t) bufferInfo.size);
vkUnmapMemory(device, vertexBufferMemory);
```

现在，您可以简单地将顶点数据memcpy到映射的内存，然后使用vkUnmapMemory再次取消映射。不幸的是，比如由于缓存，驱动程序可能不会立即将数据复制到缓冲内存中。也有可能在映射的内存中尚不可见对缓冲区的写入。有两种方法可以解决该问题：

* 使用主机相关的内存堆，用VK_MEMORY_PROPERTY_HOST_COHERENT_BIT表示(`VK_MEMORY_PROPERTY_HOST_COHERENT_BIT` bit specifies that the host cache management commands [vkFlushMappedMemoryRanges](https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/vkFlushMappedMemoryRanges.html) and [vkInvalidateMappedMemoryRanges](https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/vkInvalidateMappedMemoryRanges.html) are not needed to flush host writes to the device or make device writes visible to the host, respectively.)
* 写入映射的内存后，调用vkFlushMappedMemoryRanges，并在从映射的内存中读取之前，调用vkInvalidateMappedMemoryRanges。

我们采用第一种方法，该方法可确保映射的内存始终与分配的内存的内容匹配。请记住，与显式刷新相比，这可能会导致性能稍差，但是我们将在下一章中了解为什么这无关紧要。

刷新内存范围或使用相关的内存堆意味着驱动程序将意识到我们对缓冲区的写入，但是这并不意味着它们实际上在GPU上是可见的。数据传输到GPU的操作是在后台发生的，并且该规范仅告诉我们，在下次调用vkQueueSubmit时，它可以保证已完成。

### Binding the vertex buffer

### 绑定顶点缓冲

现在剩下的就是在渲染操作期间绑定顶点缓冲区。我们将扩展createCommandBuffers函数来做到这一点。

```c++
vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

VkBuffer vertexBuffers[] = {vertexBuffer};
VkDeviceSize offsets[] = {0};
vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, vertexBuffers, offsets);

vkCmdDraw(commandBuffers[i], static_cast<uint32_t>(vertices.size()), 1, 0, 0);
```

vkCmdBindVertexBuffers函数用于将顶点缓冲区绑定到绑定，就像我们在上一章中设置的那样。除了命令缓冲区外，前两个参数还指定了偏移量和绑定数量，我们将为其指定顶点缓冲区。最后两个参数指定要绑定的顶点缓冲区的数组以及开始从中读取顶点数据的字节偏移量。您还应该更改对vkCmdDraw的调用，以传递缓冲区中的顶点数量，而不是硬编码的数量3。

现在运行程序，您应该再次看到熟悉的三角形：

![img](https://vulkan-tutorial.com/images/triangle.png)

尝试通过修改顶点数组将顶部顶点的颜色更改为白色：

```c++
const std::vector<Vertex> vertices = {
    {{0.0f, -0.5f}, {1.0f, 1.0f, 1.0f}},
    {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
    {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
};
```

再次运行该程序，您应该看到以下内容：

![img](https://vulkan-tutorial.com/images/triangle_white.png)

在下一章中，我们将介绍另一种方法。

### Staging buffer

### 暂存缓冲

#### Introduction

#### 简介

现在，我们拥有的顶点缓冲区可以正常工作，但是允许我们从CPU访问它的内存类型可能不是图形卡本身读取的最佳内存类型。最佳内存具有VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT标志，通常是无法被CPU访问的专用图形卡显存。在本章中，我们将创建两个顶点缓冲区。 CPU可访问内存中的一个暂存缓冲区，用于将数据从顶点数组上传，最后一个顶点缓冲区在设备本地内存(显存)中。然后，我们将使用缓冲区复制命令将数据从额极端缓冲区移动到实际的顶点缓冲区。

#### Transfer queue

#### 传输队列

缓冲区复制命令需要支持传输操作的队列家族，该家族使用VK_QUEUE_TRANSFER_BIT表示。好消息是，任何具有VK_QUEUE_GRAPHICS_BIT或VK_QUEUE_COMPUTE_BIT功能的队列家族都已隐式支持VK_QUEUE_TRANSFER_BIT操作。在这种情况下，不需要实现在queueFlags中明确列出它。

如果您喜欢挑战，那么仍然可以尝试使用其他专门用于传输操作的队列家族。它将要求您对程序进行以下修改：

* 修改QueueFamilyIndices和findQueueFamilies以使用VK_QUEUE_TRANSFER位而不是VK_QUEUE_GRAPHICS_BIT显式查找队列家族。
* 修改createLogicalDevice以请求传输队列的句柄
* 为在传输队列系列上提交的命令缓冲区创建第二个命令池
* 将资源的共享模式更改为VK_SHARING_MODE_CONCURRENT并指定图形和传输队列家族
* 将任何传输命令（例如vkCmdCopyBuffer（将在本章中使用））提交到传输队列而不是图形队列

这是不少的工作，但是它将教您很多有关队列家族之间如何共享资源的知识。

#### Abstracting buffer creation

#### 抽象化缓冲创建

因为我们将在本章中创建多个缓冲区，所以将缓冲区创建移至辅助函数是一个好主意。创建一个新函数createBuffer并将createVertexBuffer中的代码（映射除外）移至该函数。

```c++
void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) {
    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to create buffer!");
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate buffer memory!");
    }

    vkBindBufferMemory(device, buffer, bufferMemory, 0);
}
```

确保添加有关缓冲区大小，内存属性和使用情况的参数，以便我们可以使用此功能创建许多不同类型的缓冲区。最后两个参数是向其写入句柄的输出变量。

现在，您可以从createVertexBuffer中删除缓冲区创建和内存分配代码，而只需调用createBuffer即可：

```c++
void createVertexBuffer() {
    VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();
    createBuffer(bufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, vertexBuffer, vertexBufferMemory);

    void* data;
    vkMapMemory(device, vertexBufferMemory, 0, bufferSize, 0, &data);
        memcpy(data, vertices.data(), (size_t) bufferSize);
    vkUnmapMemory(device, vertexBufferMemory);
}
```

运行您的程序，以确保顶点缓冲区仍然正常工作。

#### Using a staging buffer

#### 使用暂存缓冲

现在，我们将createVertexBuffer更改为仅将主机可见缓冲区用作临时缓冲区，将设备本地缓冲区用作实际顶点缓冲区。

```c++
void createVertexBuffer() {
    VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
        memcpy(data, vertices.data(), (size_t) bufferSize);
    vkUnmapMemory(device, stagingBufferMemory);

    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer, vertexBufferMemory);
}
```

现在，我们使用带有stagingBufferMemory的新stagingBuffer来映射和复制顶点数据。在本章中，我们将使用两个新的缓冲区使用标志：

* VK_BUFFER_USAGE_TRANSFER_SRC_BIT：缓冲区可用作内存传输操作中的源。
* VK_BUFFER_USAGE_TRANSFER_DST_BIT：缓冲区可用作内存传输操作中的目标。

现在，vertexBuffer是从设备本地的内存类型分配的，这通常意味着我们无法使用vkMapMemory。但是，我们可以将数据从stagingBuffer复制到vertexBuffer。我们必须指出，我们打算通过为stagingBuffer指定传输源标志，为vertexBuffer指定传输目标标志以及顶点缓冲区用途标志来做到这一点。

现在，我们将编写一个将内容从一个缓冲区复制到另一个缓冲区的函数，称为copyBuffer。

```c++
void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {

}
```

就像绘图命令一样，使用命令缓冲区执行内存传输操作。因此，我们必须首先分配一个临时命令缓冲区。您可能希望为这类短期缓冲区创建一个单独的命令池，因为该实现可能能够应用内存分配优化。在这种情况下，您应在命令池生成期间使用VK_COMMAND_POOL_CREATE_TRANSIENT_BIT标志。

```c++
void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);
}
```

并立即开始记录命令缓冲区：

```c++
VkCommandBufferBeginInfo beginInfo = {};
beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

vkBeginCommandBuffer(commandBuffer, &beginInfo);
```

我们将只使用一次命令缓冲区，然后等待从函数返回，直到复制操作完成执行。最好使用VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT向驱动介绍我们的意图。

```c++
VkBufferCopy copyRegion = {};
copyRegion.srcOffset = 0; // Optional
copyRegion.dstOffset = 0; // Optional
copyRegion.size = size;
vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
```

使用vkCmdCopyBuffer命令传输缓冲区的内容。它以源缓冲区和目标缓冲区作为参数，并复制一个区域数组。区域在VkBufferCopy结构中定义，并由源缓冲区偏移量，目标缓冲区偏移量和大小组成。与vkMapMemory命令不同，此处无法指定VK_WHOLE_SIZE。

```c++
vkEndCommandBuffer(commandBuffer);
```

该命令缓冲区仅包含复制命令，因此我们可以在此之后立即停止记录。现在执行命令缓冲区以完成传输：

```c++
VkSubmitInfo submitInfo = {};
submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
submitInfo.commandBufferCount = 1;
submitInfo.pCommandBuffers = &commandBuffer;

vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
vkQueueWaitIdle(graphicsQueue);
```

与绘制命令不同，这次没有任何事件需要等待。我们只想立即在缓冲区上执行传输。还有两种可能的方法来等待此传输完成。我们可以使用栅栏并等待vkWaitForFences，或者简单地等待传输队列通过vkQueueWaitIdle变为空闲。栅栏允许您同时安排多个传输并等待所有传输完成，而不必一次执行一个。这可以为驱动提供更多优化的机会。

```c++
vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
```

不要忘记清理用于传输操作的命令缓冲区。

现在，我们可以从createVertexBuffer函数调用copyBuffer，以将顶点数据移至设备本地缓冲区：

```c++
createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer, vertexBufferMemory);

copyBuffer(stagingBuffer, vertexBuffer, bufferSize);
```

将数据从登台缓冲区复制到设备缓冲区后，我们应该清理它：

```c++
 ...

    copyBuffer(stagingBuffer, vertexBuffer, bufferSize);

    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);
}
```

运行程序以确认您再次看到了熟悉的三角形。这种改进可能暂时不可见，但是现在它的顶点数据正在从高性能内存中加载。当我们要开始渲染更复杂的几何图形时，这将很重要。

#### Conclusion

#### 总结

应该注意的是，在实际应用中，您不应为每个单独的缓冲区实际调用vkAllocateMemory。最大同时内存分配数量受maxMemoryAllocationCount物理设备限制的限制，即使在像NVIDIA GTX 1080这样的高端硬件上，该限制也可能低至4096。同时为大量对象分配内存正确的方式是创建一个自定义分配器，该分配器使用我们在许多函数中看到的偏移量参数在多个不同对象之间单个分配。

您既可以自己实现这样的分配器，也可以使用GPUOpen计划提供的[VulkanMemoryAllocator](https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator)库。但是，对于本教程而言，可以为每个资源使用单独的分配是可以的，因为目前我们还无法接近任何这些限制。

### Index buffer

### 索引缓冲

#### Introduction

#### 简介

您将在实际应用程序中渲染的3D网格通常会在多个三角形之间共享顶点。即使像绘制矩形这样的简单操作也已经发生了：

![img](https://vulkan-tutorial.com/images/vertex_vs_index.svg)

绘制矩形需要两个三角形，这意味着我们需要一个具有6个顶点的顶点缓冲区。问题在于两个顶点的数据需要复制，从而导致50％的冗余。它只会随着更复杂的网格而变得更糟，在这些网格中，顶点平均以3个三角形重用。解决此问题的方法是使用索引缓冲区。

索引缓冲区本质上是指向顶点缓冲区的指针数组。它允许您对顶点数据进行重新排序，并将现有数据重用于多个顶点。上图说明了如果我们有一个包含四个唯一顶点中的每个顶点的顶点缓冲区，则该矩形的索引缓冲区将是什么样。前三个索引定义右上三角形，后三个索引定义左下三角形的顶点。

#### Index buffer creation

#### 索引缓冲创建

在本章中，我们将修改顶点数据并添加索引数据以绘制一个矩形，如图所示。修改顶点数据以代表四个角：

```c++
const std::vector<Vertex> vertices = {
    {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
    {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
    {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
    {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}
};
```

左上角为红色，右上角为绿色，右下角为蓝色，左下角为白色。我们将添加一个新的数组索引来表示索引缓冲区的内容。它应与图中的索引匹配，以绘制右上三角形和左下三角形。

```c++
const std::vector<uint16_t> indices = {
    0, 1, 2, 2, 3, 0
};
```

可以将uint16_t或uint32_t用于索引缓冲区，具体取决于顶点中的条目数。我们现在可以使用uint16_t，因为我们使用的少于65535个唯一顶点。

就像顶点数据一样，索引需要上载到VkBuffer中，GPU才能访问它们。定义两个新的类成员以保存索引缓冲区的资源：

```c++
VkBuffer vertexBuffer;
VkDeviceMemory vertexBufferMemory;
VkBuffer indexBuffer;
VkDeviceMemory indexBufferMemory;
```

我们现在要添加的createIndexBuffer函数与createVertexBuffer几乎相同：

```c++
void initVulkan() {
    ...
    createVertexBuffer();
    createIndexBuffer();
    ...
}

void createIndexBuffer() {
    VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, indices.data(), (size_t) bufferSize);
    vkUnmapMemory(device, stagingBufferMemory);

    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer, indexBufferMemory);

    copyBuffer(stagingBuffer, indexBuffer, bufferSize);

    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);
```

只有两个显着差异。现在，bufferSize等于索引数乘以索引类型（uint16_t或uint32_t）的大小。 indexBuffer的用法应为VK_BUFFER_USAGE_INDEX_BUFFER_BIT，而不是VK_BUFFER_USAGE_VERTEX_BUFFER_BIT，这是有道理的。除此之外，过程完全相同。我们创建一个暂存缓冲区，将索引的内容复制到该缓冲区，然后将其复制到最终的设备本地索引缓冲区。

索引缓冲区应在程序结束时清理，就像顶点缓冲区一样：

```c++
void cleanup() {
    cleanupSwapChain();

    vkDestroyBuffer(device, indexBuffer, nullptr);
    vkFreeMemory(device, indexBufferMemory, nullptr);

    vkDestroyBuffer(device, vertexBuffer, nullptr);
    vkFreeMemory(device, vertexBufferMemory, nullptr);

    ...
}
```

#### Using an index buffer

#### 使用索引缓冲

使用索引缓冲区进行绘制涉及对createCommandBuffers的两项更改。我们首先需要绑定索引缓冲区，就像对顶点缓冲区一样。区别在于只能有一个索引缓冲区。不幸的是，不可能为每个顶点属性使用不同的索引，因此即使只有一个属性发生变化，我们仍然必须完全复制顶点数据。

```c++
vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, vertexBuffers, offsets);

vkCmdBindIndexBuffer(commandBuffers[i], indexBuffer, 0, VK_INDEX_TYPE_UINT16);
```

索引缓冲区与vkCmdBindIndexBuffer绑定，后者具有索引缓冲区，其中的字节偏移量以及索引数据的类型作为参数。如前所述，可能的类型为VK_INDEX_TYPE_UINT16和VK_INDEX_TYPE_UINT32。

只是绑定索引缓冲区并不会改变任何东西，我们还需要更改绘图命令以告诉Vulkan使用索引缓冲区。删除vkCmdDraw行，并将其替换为vkCmdDrawIndexed：

```c++
vkCmdDrawIndexed(commandBuffers[i], static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
```

对该函数的调用与vkCmdDraw非常相似。前两个参数指定索引数和实例数。我们不使用实例化，因此只需指定1个实例。索引数表示将传递到顶点缓冲区的顶点数。下一个参数指定索引缓冲区的偏移量，使用1值将导致图形卡从第二个索引处开始读取。倒数第二个参数指定要添加到索引缓冲区中索引的偏移量。 最后一个参数指定实例化的偏移量，我们没有使用它。

现在运行程序，您应该看到以下内容：

![img](https://vulkan-tutorial.com/images/indexed_rectangle.png)

现在，您知道了如何通过重新使用带有索引缓冲区的顶点来节省内存。在以后的章节中，我们将加载复杂的3D模型，这一点将变得尤为重要。

上一章已经提到，您应该从单个内存分配中分配多个资源（例如缓冲区），但实际上您应该更进一步。驱动程序开发人员建议您也将多个缓冲区（例如顶点和索引缓冲区）存储到单个VkBuffer中，并在vkCmdBindVertexBuffers等命令中使用偏移量。这样做的好处是，在这种情况下，您的数据更接近缓存，因为它们之间的距离更近。如果在相同的渲染操作期间未使用多个资源，则甚至有可能对多个资源重用同一块内存，当然，只要刷新它们的数据即可。这称为别名，某些Vulkan函数具有显式标志来指定您要执行此操作。

译注：1.对于传输队列章节提到的挑战的尝试

> 如果您喜欢挑战，那么仍然可以尝试使用其他专门用于传输操作的队列家族。它将要求您对程序进行以下修改：
>
> * 修改QueueFamilyIndices和findQueueFamilies以使用VK_QUEUE_TRANSFER位而不是VK_QUEUE_GRAPHICS_BIT显式查找队列家族。
> * 修改createLogicalDevice以请求传输队列的句柄
> * 为在传输队列系列上提交的命令缓冲区创建第二个命令池
> * 将资源的共享模式更改为VK_SHARING_MODE_CONCURRENT并指定图形和传输队列家族
> * 将任何传输命令（例如vkCmdCopyBuffer（将在本章中使用））提交到传输队列而不是图形队列

1.在QueueFamilyIndices结构体中添加一项：

```c++
struct QueueFamilyIndices
{
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;
	std::optional<uint32_t> transferFamily;

	bool isComplete()
	{
		return graphicsFamily.has_value() && presentFamily.has_value() && transferFamily.has_value();
	}
};
```

2.修改findQueueFamilies函数：

```c++
QueueFamilyIndices Application::findQueueFamilies(VkPhysicalDevice device)
{
	QueueFamilyIndices indices;

	uint32_t queueFamilyCount = 0;

	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

	int i = 0;

	for (const auto queueFamily : queueFamilies)
	{
		if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			indices.graphicsFamily = i;
		}

		if (queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT && i != 0)
		{
			indices.transferFamily = i;
		}

		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

		if (presentSupport)
		{
			indices.presentFamily = i;
		}

		if (indices.isComplete())
		{
			break;
		}

		i++;
	}

	return indices;
}
```

从实验中可以发现支持VK_QUEUE_TRANSFER_BIT的队列家族不止一个，实际上支持VK_QUEUE_GRAPHICS_BIT的队列家族同时也是支持VK_QUEUE_TRANSFER_BIT的，但是我们这里的目的是使用两个队列家族，因此就要找到另一个队列家族。

3.修改createLogicalDevice函数来获得传输队列的句柄transferQueue：

```
void Application::createLogicalDevice()
{
	QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value(), indices.transferFamily.value() };

	float queuePriority = 1.0f;
	for (uint32_t queueFamily : uniqueQueueFamilies)
	{
		VkDeviceQueueCreateInfo queueCreateInfo = {};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfos.push_back(queueCreateInfo);
	}

	VkPhysicalDeviceFeatures physicalDeviceFeatures = {};
	vkGetPhysicalDeviceFeatures(physicalDevice, &physicalDeviceFeatures);

	VkDeviceCreateInfo deviceCreateInfo = {};

	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();


	vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
	vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
	vkGetDeviceQueue(device, indices.transferFamily.value(), 0, &transferQueue);
}
```

4.创建第二个命令池transferCommandPool：

```c++
void Application::createCommandPool()
{
	QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);

	VkCommandPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

	if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create command pool!");
	}

	VkCommandPoolCreateInfo transformPoolInfo = {};
	transformPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	transformPoolInfo.queueFamilyIndex = queueFamilyIndices.transferFamily.value();

	if (vkCreateCommandPool(device, &transformPoolInfo, nullptr, &transferCommandPool) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create transfer command pool!");
	}
}

```

5.修改createSwapChain函数，将资源的共享模式改为VK_SHARING_MODE_CONCURRENT，以实现在多个队列之间的共享：

```c++
QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.transferFamily.value() };

if (indices.graphicsFamily != indices.transferFamily)
{
	createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
	createInfo.queueFamilyIndexCount = 2;
	createInfo.pQueueFamilyIndices = queueFamilyIndices;
}
else
{
	createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	createInfo.queueFamilyIndexCount = 0;
	createInfo.pQueueFamilyIndices = nullptr;
}
```
6.修改copyBuffer函数，将传输命令提交到新的transferQueue：

```c++
void Application::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
	VkCommandBufferAllocateInfo allocateInfo = {};
	allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocateInfo.commandPool = transferCommandPool;
	allocateInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(device, &allocateInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	VkBufferCopy copyRegion = {};
	copyRegion.srcOffset = 0;
	copyRegion.dstOffset = 0;
	copyRegion.size = size;

	vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	vkQueueSubmit(transferQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(transferQueue);

	vkFreeCommandBuffers(device, transferCommandPool, 1, &commandBuffer);
}
```

2.合并Vertex Buffer和Index Buffer到一个VkBuffer

1.创建两个新的变量

```c++
	VkBuffer allInOneBuffer;
	VkDeviceMemory allInOneBufferMemory;
```

allInOneBuffer会保存Vertex Buffer和Index Buffer的数据。

2.创建一个新的函数createAllInOneBuffer

```c++
void Application::createAllInOneBuffer()
{
	VkDeviceSize vertexBufferSize = sizeof(vertices[0]) * vertices.size();
	VkDeviceSize indexBufferSize = sizeof(indices[0]) * indices.size();

	VkDeviceSize allInOneBufferSize = vertexBufferSize + indexBufferSize;

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	createBuffer(allInOneBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
									 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
									 VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
									 stagingBuffer, stagingBufferMemory);

	void* data = nullptr;
	vkMapMemory(device, stagingBufferMemory, 0, allInOneBufferSize, 0, &data);
	memcpy_s(data, (size_t)vertexBufferSize, vertices.data(), (size_t)vertexBufferSize);
	memcpy_s((Vertex*)data + vertices.size(), (size_t)indexBufferSize, indices.data(), (size_t)indexBufferSize);

	vkUnmapMemory(device, stagingBufferMemory);

	createBuffer(allInOneBufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT |
									 VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
									 VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
									 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
									 allInOneBuffer, allInOneBufferMemory);

	copyBuffer(stagingBuffer, allInOneBuffer, allInOneBufferSize);
	vkDestroyBuffer(device, stagingBuffer, nullptr);
	vkFreeMemory(device, stagingBufferMemory, nullptr);
}
```

这个函数和之前的createVertexBuffer和createIndexBuffer差不多，不同点是，现在我们将vertices和indices的数据都拷贝到allInOneBuffer中，并且在创建allInOneBuffer的时候，usage参数同时指定VK_BUFFER_USAGE_VERTEX_BUFFER_BIT和VK_BUFFER_USAGE_INDEX_BUFFER_BIT。

3.绑定新的allInOneBuffer

修改createCommandBuffer时，将allInOneBuffer进行绑定。

```c++
vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, vertexBuffers, offsets);

VkDeviceSize offset = sizeof(vertices[0]) * vertices.size();

vkCmdBindIndexBuffer(commandBuffers[i], allInOneBuffer, offset, VK_INDEX_TYPE_UINT16);
```

