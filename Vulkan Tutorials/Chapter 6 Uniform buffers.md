# Chapter 6 Uniform buffers

# 第6章2 Uniform缓冲

## Descriptor layout and buffer

## 描述符布局和缓冲

### Introduction

### 简介

现在，我们可以为每个顶点将任意属性传递给顶点着色器，但是全局变量呢？从本章开始，我们将继续介绍3D图形，这需要一个模型-视图-投影(model-view-projection)矩阵。我们可以将其包含为顶点数据，但这会浪费内存，并且每当变换发生更改时，都需要我们更新顶点缓冲区。而变换很容易每帧都发生改变。

在Vulkan中解决此问题的正确方法是使用资源描述符(resource descriptor)。描述符是着色器自由访问缓冲区和图像等资源的一种方式。我们将建立一个包含变换矩阵的缓冲区，并让顶点着色器通过描述符访问它们。描述符的使用包括三个部分：

* 在管线创建期间指定描述符布局(descriptor layout)
* 从描述符池分配描述符集(descriptor set)
* 在渲染期间绑定描述符集

描述符布局指定了管道将要访问的资源的类型，就像渲染通道指定了将要访问的附件的类型一样。描述符集指定将绑定到描述符的实际缓冲区或图像资源，就像帧缓冲区指定要绑定到渲染通道附件的实际图像视图一样。然后将描述符集绑定到绘制命令，就像顶点缓冲区和帧缓冲区一样。

描述符的类型很多，但是在本章中，我们将使用统一缓冲区对象（Uniform Buffer Object, UBO）。在以后的章节中，我们将介绍其他类型的描述符，但是基本过程是相同的。假设我们有想要顶点着色器包含的数据，以C风格结构体的样式，如下所示：

```c++
struct UniformBufferObject {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};
```

然后，我们可以将数据复制到VkBuffer并通过来自顶点着色器的统一缓冲区对象描述符进行访问，如下所示：

```c++
layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

void main() {
    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 0.0, 1.0);
    fragColor = inColor;
}
```

我们将每帧更新模型，视图和投影矩阵，以使上一章中的矩形进行3D旋转。

### Vertex shader

### 顶点着色器

修改顶点着色器以包括统一缓冲区对象，如上面指定的那样。我假设您熟悉MVP转换。如果不是这样，请参阅第一章中提到的[资源](http://opengl.datenwolf.net/gltut/html/index.html)。

```c++
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec3 inColor;

layout(location = 0) out vec3 fragColor;

void main() {
    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 0.0, 1.0);
    fragColor = inColor;
}
```

注意uniform声明，in和out声明的顺序无关紧要。binding指令类似于属性的location指令。我们将在描述符布局中引用此绑定。更改了带有gl_Position的行，以使用转换来计算裁剪坐标中的最终位置。与2D三角形不同，裁剪坐标的最后一个分量可能不是1，这在转换为屏幕上的最终归一化设备坐标时将导致除法。这在透视投影中用作透视除法，本质上使较近的对象看起来比较远的对象看起来更大。

### Descriptor set layout

### 描述符集合布局

下一步是在C ++端定义UBO，并在顶点着色器中向Vulkan告知此描述符。

```c++
struct UniformBufferObject {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};
```

我们可以使用GLM中的数据类型完全匹配着色器中的定义。矩阵中的数据与着色器期望的方式是二进制兼容的，因此以后我们可以将UniformBufferObject直接memcpy到VkBuffer。

我们需要提供有关着色器中用于管线创建的每个描述符绑定的详细信息，就像我们必须对每个顶点属性及其位置索引所做的一样。我们将建立一个新函数来定义所有这些信息，称为createDescriptorSetLayout。应该在创建管道之前就调用它，因为我们在那里需要它。

```c++
void initVulkan() {
    ...
    createDescriptorSetLayout();
    createGraphicsPipeline();
    ...
}

...

void createDescriptorSetLayout() {

}
```

每个绑定都需要通过VkDescriptorSetLayoutBinding结构进行描述。

```c++
void createDescriptorSetLayout() {
    VkDescriptorSetLayoutBinding uboLayoutBinding = {};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.descriptorCount = 1;
}
```

前两个字段指定在着色器中使用的binding和描述符的类型，描述符是一个统一的缓冲区对象。着色器变量可能表示一个统一缓冲区对象的数组，而descriptorCount指定数组中值的数量。例如，这可用于为骨骼动画指定骨骼中每个骨骼的变换。我们的MVP转换在单个统一缓冲区对象中，因此我们使用的descriptorCount为1。

```c++
uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
```

我们还需要指定在哪个着色器阶段引用描述符。 stageFlags字段可以是VkShaderStageFlagBits值或值VK_SHADER_STAGE_ALL_GRAPHICS的组合。在我们的例子中，我们仅引用来自顶点着色器的描述符。

```c++
uboLayoutBinding.pImmutableSamplers = nullptr; // 可选的
```

pImmutableSamplers字段仅与图像采样相关的描述符有关，我们将在后面介绍。您可以将其保留为默认值。

所有描述符绑定都组合到单个VkDescriptorSetLayout对象中。在pipelineLayout上方定义一个新的类成员：

```c++
VkDescriptorSetLayout descriptorSetLayout;
VkPipelineLayout pipelineLayout;
```

然后，我们可以使用vkCreateDescriptorSetLayout创建它。此函数接受带有绑定数组的简单VkDescriptorSetLayoutCreateInfo：

```c++
VkDescriptorSetLayoutCreateInfo layoutInfo = {};
layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
layoutInfo.bindingCount = 1;
layoutInfo.pBindings = &uboLayoutBinding;

if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
    throw std::runtime_error("failed to create descriptor set layout!");
}
```

我们需要在管道创建期间指定描述符集布局，以告诉Vulkan着色器将使用哪些描述符。描述符集布局在管道布局对象中指定。修改VkPipelineLayoutCreateInfo以引用布局对象：

```c++
VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
pipelineLayoutInfo.setLayoutCount = 1;
pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
```

您可能想知道为什么在这里可以指定多个描述符集布局，因为一个已经包含了所有绑定。在下一章中，我们将介绍描述符池和描述符集。

在我们可能会创建新的图形管道（即直到程序结束）之前，描述符布局应始终存在：

```c++
void cleanup() {
    cleanupSwapChain();

    vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);

    ...
}
```

### Uniform buffer

### Uniform缓冲

在下一章中，我们将指定包含着色器UBO数据的缓冲区，但是我们需要首先创建它。我们每帧将新数据复制到uniform缓冲区，因此拥有临时缓冲区实际上没有任何意义。在这种情况下，这只会增加额外的开销，并且有可能降低性能而不是提高性能。

我们应该有多个缓冲区，因为可能同时有多个帧在运行中，并且我们不希望在准备读取下一帧的同时更新该缓冲区，而下一帧仍在读取中！我们可以每帧或每个交换链图像有一个uniform缓冲区。但是，由于我们需要从每个交换链图像具有的命令缓冲区中引用uniform缓冲区，因此在每个交换链图像中也具有uniform缓冲区是最有意义的。

为此，请为unitformBuffers和uniformBuffersMemory添加新的类成员：

```c++
VkBuffer indexBuffer;
VkDeviceMemory indexBufferMemory;

std::vector<VkBuffer> uniformBuffers;
std::vector<VkDeviceMemory> uniformBuffersMemory;
```

同样，创建一个新函数createUniformBuffers，该函数在createIndexBuffer之后调用并分配缓冲区：

我们将编写一个单独的函数，以每帧新的变换来更新uniform缓冲区，因此此处将没有vkMapMemory。uniform数据将用于所有绘制调用，因此仅在我们停止渲染时才应销毁包含该数据的缓冲区。由于它还取决于交换链图像的数量，在重新创建后可能会更改，因此我们将在cleanupSwapChain中对其进行清理：

```c++
void cleanupSwapChain() {
    ...

    for (size_t i = 0; i < swapChainImages.size(); i++) {
        vkDestroyBuffer(device, uniformBuffers[i], nullptr);
        vkFreeMemory(device, uniformBuffersMemory[i], nullptr);
    }
}
```

这意味着我们还需要在recreateSwapChain中重新创建它：

```c++
void recreateSwapChain() {
    ...

    createFramebuffers();
    createUniformBuffers();
    createCommandBuffers();
}
```

### Updating uniform data

### 更新uniform数据

创建一个新函数updateUniformBuffer，并在知道要获取哪个交换链图像后立即从drawFrame函数添加对其的调用：

```c++
void drawFrame() {
    ...

    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

    ...

    updateUniformBuffer(imageIndex);

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    ...
}

...

void updateUniformBuffer(uint32_t currentImage) {

}
```

此函数将在每帧生成一个新的变换，以使几何体旋转。我们需要包括两个新的头文件才能实现此功能：

glm/gtc/matrix_transform.hpp头文件导出了可用于生成模型转换（如glm :: rotate），视图转换（如glm :: lookAt）和投影变换（如glm :: perspective）的函数。必须使用GLM_FORCE_RADIANS定义来确保glm :: rotate之类的函数使用弧度作为参数，以避免任何可能的混淆。

chrono标准库头文件提供了用于精确计时的函数。我们将使用它来确保几何体每秒旋转90度，而不管帧频如何。

```c++
void updateUniformBuffer(uint32_t currentImage) {
    static auto startTime = std::chrono::high_resolution_clock::now();

    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
}
```

updateUniformBuffer函数将以秒为单位计算自渲染以浮点精度开始以来的时间。

现在，我们将在uniform缓冲区对象中定义模型，视图和投影变换。使用时间变量，模型旋转将是围绕Z轴的简单旋转：

```c++
UniformBufferObject ubo = {};
ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
```

glm :: rotate函数将现有的变换，旋转角度和旋转轴作为参数。 glm :: mat4（1.0f）构造函数返回一个单位矩阵。使用旋转角度time * glm :: radians（90.0f）可以实现每秒旋转90度的目的。

```c++
ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
```

对于视图转换，我决定从上方以45度角查看几何。 glm :: lookAt函数将眼睛位置，中心位置和上轴作为参数。

```c++
ubo.proj = glm::perspective(glm::radians(45.0f), swapChainExtent.width / (float) swapChainExtent.height, 0.1f, 10.0f);
```

我选择使用具有45度垂直视场的透视投影。其他参数是长宽比，近视平面和远视平面。重要的是使用当前交换链尺寸来计算纵横比，以考虑调整大小后窗口的新宽度和高度。

```c++
ubo.proj[1][1] *= -1;
```

GLM最初是为OpenGL设计的，其中，裁剪坐标的Y坐标是反转的。最简单的补偿方法是在投影矩阵中翻转Y轴缩放比例上的符号。如果您不这样做，那么图像将被颠倒渲染。

现在已经定义了所有变换，因此我们可以将uniform缓冲区对象中的数据复制到当前的uniform缓冲区。这与我们使用顶点缓冲区的方式完全相同，不同之处在于没有临时缓冲区：

```c++
void* data;
vkMapMemory(device, uniformBuffersMemory[currentImage], 0, sizeof(ubo), 0, &data);
    memcpy(data, &ubo, sizeof(ubo));
vkUnmapMemory(device, uniformBuffersMemory[currentImage]);
```

以这种方式使用UBO并不是将频繁更改的值传递给着色器的最有效方法。将少量数据缓冲区传递给着色器的更有效方法是推送常量(push constants)。我们可能会在以后的章节中介绍这些内容。

在下一章中，我们将介绍描述符集，该描述符集会将VkBuffers实际上绑定到uniform缓冲区描述符，以便着色器可以访问此变换数据。

## Descriptor pool and sets

## 描述符池和集合

### Introduction
### 简介

上一章中的描述符布局描述了可以绑定的描述符的类型。在本章中，我们将为每个VkBuffer资源创建一个描述符集，以将其绑定到uniform缓冲区描述符。

### Descriptor pool

### 描述符池

描述符集不能直接创建，它们必须像命令缓冲区一样从池中分配。毫不奇怪，描述符集的等效项称为描述符池。我们将编写一个新函数createDescriptorPool进行设置。

```c++
void initVulkan() {
    ...
    createUniformBuffers();
    createDescriptorPool();
    ...
}

...

void createDescriptorPool() {

}
```

我们首先需要使用VkDescriptorPoolSize结构描述我们的描述符集将包含哪些描述符类型以及其中有多少个描述符类型。

```c++
VkDescriptorPoolSize poolSize = {};
poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
poolSize.descriptorCount = static_cast<uint32_t>(swapChainImages.size());
```

我们将为每个帧分配这些描述符之一。此描述符池大小结构由主VkDescriptorPoolCreateInfo引用：

```c++
VkDescriptorPoolCreateInfo poolInfo = {};
poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
poolInfo.poolSizeCount = 1;
poolInfo.pPoolSizes = &poolSize;
```

除了可用的单个描述符的最大数量外，我们还需要指定可以分配的最大描述符集数量：

```c++
poolInfo.maxSets = static_cast<uint32_t>(swapChainImages.size());;
```

该结构具有类似于命令池的可选标志，该标志确定是否可以释放各个描述符集：VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT。创建描述符集后，我们将不再碰它，因此我们不需要此标志。您可以将标志保留为默认值0。

```c++
kDescriptorPool descriptorPool;

...

if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
    throw std::runtime_error("failed to create descriptor pool!");
}    
```

添加一个新的类成员来存储描述符池的句柄，并调用vkCreateDescriptorPool来创建它。重新创建交换链时，应销毁描述符池，因为它取决于图像数：

```c++
void cleanupSwapChain() {
    ...

    for (size_t i = 0; i < swapChainImages.size(); i++) {
        vkDestroyBuffer(device, uniformBuffers[i], nullptr);
        vkFreeMemory(device, uniformBuffersMemory[i], nullptr);
    }

    vkDestroyDescriptorPool(device, descriptorPool, nullptr);
}
```

并在recreateSwapChain中重新创建：

```c++
void recreateSwapChain() {
    ...

    createUniformBuffers();
    createDescriptorPool();
    createCommandBuffers();
}
```

### Descriptor set

### 描述符集合

现在，我们可以自己分配描述符集。为此添加一个createDescriptorSets函数：

```c++
void initVulkan() {
    ...
    createDescriptorPool();
    createDescriptorSets();
    ...
}

void recreateSwapChain() {
    ...
    createDescriptorPool();
    createDescriptorSets();
    ...
}

...

void createDescriptorSets() {

}
```

使用VkDescriptorSetAllocateInfo结构描述描述符集分配。您需要指定要分配的描述符池，要分配的描述符集的数量以及基于它们的描述符布局：

```c++
std::vector<VkDescriptorSetLayout> layouts(swapChainImages.size(), descriptorSetLayout);
VkDescriptorSetAllocateInfo allocInfo = {};
allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
allocInfo.descriptorPool = descriptorPool;
allocInfo.descriptorSetCount = static_cast<uint32_t>(swapChainImages.size());
allocInfo.pSetLayouts = layouts.data();
```

在我们的例子中，我们将为每个交换链图像创建一个描述符集，所有描述符都具有相同的布局。不幸的是，我们确实需要布局的所有副本，因为下一个函数需要一个与集合数匹配的数组。

添加一个类成员来保存描述符集句柄，并使用vkAllocateDescriptorSets分配它们：

```c++
VkDescriptorPool descriptorPool;
std::vector<VkDescriptorSet> descriptorSets;

...

descriptorSets.resize(swapChainImages.size());
if (vkAllocateDescriptorSets(device, &allocInfo, descriptorSets.data()) != VK_SUCCESS) {
    throw std::runtime_error("failed to allocate descriptor sets!");
}
```

您不需要显式清理描述符集，因为在销毁描述符池时，它们将自动释放。对vkAllocateDescriptorSets的调用将分配描述符集，每个描述符集具有一个uniform缓冲区描述符。

现在已经分配了描述符集，但是仍然需要配置其中的描述符。现在，我们将添加一个循环以填充每个描述符：

```c++
for (size_t i = 0; i < swapChainImages.size(); i++) {

}
```

引用缓冲区的描述符（例如我们的uniform缓冲区描述符）使用VkDescriptorBufferInfo结构进行配置。此结构指定缓冲区以及其中包含描述符数据的区域。

```c++
for (size_t i = 0; i < swapChainImages.size(); i++) {
    VkDescriptorBufferInfo bufferInfo = {};
    bufferInfo.buffer = uniformBuffers[i];
    bufferInfo.offset = 0;
    bufferInfo.range = sizeof(UniformBufferObject);
}
```

如果要像在这种情况下那样覆盖整个缓冲区，那么也可以对范围使用VK_WHOLE_SIZE值。描述符的配置使用vkUpdateDescriptorSets函数进行更新，该函数将VkWriteDescriptorSet结构数组作为参数。

```c++
VkWriteDescriptorSet descriptorWrite = {};
descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
descriptorWrite.dstSet = descriptorSets[i];
descriptorWrite.dstBinding = 0;
descriptorWrite.dstArrayElement = 0;
```

前两个字段指定要更新的描述符集和绑定。我们为uniform缓冲区绑定索引指定了0。请记住，描述符可以是数组，因此我们还需要在数组中指定要更新的第一个索引。我们不使用数组，因此索引是0。

```c++
descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
descriptorWrite.descriptorCount = 1;
```

我们需要再次指定描述符的类型。可以从索引dstArrayElement开始一次更新数组中的多个描述符。 descriptorCount字段指定要更新的数组元素个数。

```c++
descriptorWrite.pBufferInfo = &bufferInfo;
descriptorWrite.pImageInfo = nullptr; // Optional
descriptorWrite.pTexelBufferView = nullptr; // Optional
```

最后一个字段引用具有descriptorCount结构的数组，该结构实际配置描述符。这取决于描述符的类型，您实际上需要使用这三个描述符中的哪一个。 pBufferInfo字段用于引用缓冲区数据的描述符，pImageInfo用于引用图像数据的描述符，pTexelBufferView用于引用缓冲区视图的描述符。我们的描述符基于缓冲区，因此我们使用的是pBufferInfo。

```c++
vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);
```

使用vkUpdateDescriptorSets应用更新。它接受两种数组作为参数：VkWriteDescriptorSet数组和VkCopyDescriptorSet数组。顾名思义，后者可用于相互复制描述符。

### Using descriptor sets

### 使用描述符集合

现在，我们需要更新createCommandBuffers函数，使用vkcmdBindDescriptorSets函数将每个交换链图像的正确描述符集实际绑定到着色器中的描述符。这需要在vkCmdDrawIndexed调用之前完成：

```c++
vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSets[i], 0, nullptr);
vkCmdDrawIndexed(commandBuffers[i], static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
```

与顶点和索引缓冲区不同，描述符集并不是图形管线所独有的。因此，我们需要指定是否要将描述符集绑定到图形或计算管道。下一个参数是描述符所基于的布局。接下来的三个参数指定第一个描述符集的索引，要绑定的集合的数量以及要绑定的集合的数组。我们待会儿再讲。最后两个参数指定用于动态描述符的偏移量数组。我们将在以后的章节中介绍这些内容。

如果您现在运行程序，那么您会发现不幸的是看不到任何内容。问题在于，由于我们在投影矩阵中进行了Y翻转，因此现在以逆时针顺序而不是顺时针顺序绘制了顶点。这将导致背面剔除，并阻止绘制任何几何图形。转到createGraphicsPipeline函数并在VkPipelineRasterizationStateCreateInfo中修改frontFace以更正此问题：

```c++
rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
```

再次运行程序，您现在应该看到以下内容：

![img](https://vulkan-tutorial.com/images/spinning_quad.png)

矩形已变成正方形，因为投影矩阵现在可以校正宽高比。 updateUniformBuffer负责屏幕大小调整，因此我们不需要重新创建recreateSwapChain中的描述符集。

### Alignment requirements

### 对齐要求

到目前为止，我们一笔带过的一件事是C++结构中的数据应该如何与着色器中的uniform定义完全匹配。似乎在两者中简单地使用相同的类型就足够了：

```c++
struct UniformBufferObject {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;
```

但是，这还不是全部。例如，尝试将struct和shader修改为如下所示：

```c++
struct UniformBufferObject {
    glm::vec2 foo;
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};

layout(binding = 0) uniform UniformBufferObject {
    vec2 foo;
    mat4 model;	// Offset 8
    mat4 view;  // Offset 72
    mat4 proj;	// Offset 136
} ubo;
```

重新编译着色器和程序并运行它，您会发现到目前为止工作正常的彩色正方形消失了！那是因为我们没有考虑对齐要求。

Vulkan希望结构中的数据以特定方式在内存中对齐，例如：

* 标量必须以N对齐（给定32位浮点数，= 4个字节）。
* vec2必须对齐2N（= 8字节）
* vec3或vec4必须对齐4N（= 16字节）
* 嵌套结构必须通过其成员的基本对齐方式对齐，并向上舍入为16的倍数。
* mat4矩阵必须与vec4具有相同的对齐方式。

您可以在[规范](https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/chap14.html#interfaces-resources-layout)中找到对齐要求的完整列表。

我们只有三个mat4字段的原始着色器已经满足对齐要求。由于每个mat4的大小为4 x 4 x 4 = 64字节，因此模型的偏移量为0，视图的偏移量为64，proj的偏移量为128。所有这些都是16的倍数，因此可以正常工作。

新结构以仅8个字节的vec2开头，因此破坏了所有偏移量。现在，模型的偏移量为8，查看的偏移量为72，项目的偏移量为136，都不是16的倍数。要解决此问题，我们可以使用C ++ 11中引入的alignas说明符：

```c++
struct UniformBufferObject {
    glm::vec2 foo;
    alignas(16) glm::mat4 model; // Offset now 16
    glm::mat4 view;				// Offset now 80
    glm::mat4 proj;				// Offset now 144
};
```

如果现在重新编译并运行程序，则应该看到着色器再次正确接收其矩阵值。

幸运的是，有一种方法大多数时候不必考虑这些对齐要求。我们可以在包含GLM之前定义GLM_FORCE_DEFAULT_ALIGNED_GENTYPES：

```c++
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include <glm/glm.hpp>
```

这将迫使GLM使用已经为我们指定对齐要求的vec2和mat4版本。如果添加此定义，则可以删除alignas指定符，并且程序仍然可以运行。

不幸的是，如果您开始使用嵌套结构，则此方法可能会失效。考虑C++代码中的以下定义：

```c++
struct Foo {
    glm::vec2 v;
};

struct UniformBufferObject {
    Foo f1;
    Foo f2;
};
```

以及以下着色器定义：

```c++
struct Foo {
    vec2 v;
};

layout(binding = 0) uniform UniformBufferObject {
    Foo f1;
    Foo f2;
} ubo;
```

在这种情况下，f2的偏移量为8，而f2的嵌套结构偏移量应该为16。在这种情况下，您必须自己指定对齐方式：

```c++
struct UniformBufferObject {
    Foo f1;
    alignas(16) Foo f2;
};
```

这些陷阱是始终明确对齐的一个很好的理由。这样，您将不会因对齐错误的奇怪症状而措手不及。

```c++
struct UniformBufferObject {
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
};
```

不要忘记删除foo字段后重新编译着色器。

### Multiple descriptor sets

### 多描述符集合

正如某些结构和函数调用所暗示的那样，实际上可以同时绑定多个描述符集。创建管道布局时，需要为每个描述符集指定一个描述符布局。着色器然后可以引用特定的描述符集，如下所示：

```c++
layout(set = 0, binding = 0) uniform UniformBufferObject { ... }
```

您可以使用此功能将每个对象不同的描述符和共享的描述符放到单独的描述符集合中。在这种情况下，您可以避免在绘制调用之间重新绑定大多数描述符，这可能会更高效。

