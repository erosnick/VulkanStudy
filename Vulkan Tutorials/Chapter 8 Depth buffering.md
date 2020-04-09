# Chapter 8 Depth buffering

# 第8章 深度缓冲

## Introduction

## 简介

到目前为止，我们使用过的几何体已投影到3D中，但它仍然完全平坦。在本章中，我们将向position添加一个Z坐标以准备3D网格。我们将使用第三个坐标在当前正方形上放置一个正方形，以查看未按深度对几何进行排序时出现的问题。

## 3D Geometry

## 3D几何体

更改“顶点”结构以使用3D向量作为位置，并在相应的VkVertexInputAttributeDescription中更新格式：

```c++
struct Vertex {
    glm::vec3 pos;
    glm::vec3 color;
    glm::vec2 texCoord;

    ...

    static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions = {};

        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Vertex, pos);

        ...
    }
};
```

接下来，更新顶点着色器以接受和变换3D坐标作为输入。不要忘了之后重新编译它！

```c++
layout(location = 0) in vec3 inPosition;

...

void main() {
    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 1.0);
    fragColor = inColor;
    fragTexCoord = inTexCoord;
}
```

最后，更新顶点容器以包含Z坐标：

```c++
const std::vector<Vertex> vertices = {
    {{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
    {{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
    {{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
    {{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}}
};
```

如果现在运行您的应用程序，那么您应该会看到与以前完全相同的结果。现在是时候添加一些额外的几何图形以使场景更有趣了，并演示本章要解决的问题。复制顶点以定义当前位置正下方的正方形的位置，如下所示：

![img](https://vulkan-tutorial.com/images/extra_square.svg)

使用-0.5f的Z坐标，并为额外的正方形添加适当的索引：

```c++
const std::vector<Vertex> vertices = {
    {{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
    {{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
    {{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
    {{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},

    {{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
    {{0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
    {{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
    {{-0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}}
};

const std::vector<uint16_t> indices = {
    0, 1, 2, 2, 3, 0,
    4, 5, 6, 6, 7, 4
};
```

立即运行程序，您将看到类似于Escher的插图：

![img](https://vulkan-tutorial.com/images/depth_issues.png)

问题在于，下部正方形的片段被绘制在上部正方形的片段上，这仅仅是因为它位于索引数组的后面。有两种解决方法：

* 从后到前按深度对所有绘制调用进行排序
* 使用深度缓冲区进行深度测试

第一种方法通常用于绘制透明对象，因为与顺序无关的透明性(Order Independent Transparency, OIT)是很难解决的挑战。但是，使用深度缓冲区通常可以解决按深度对片段排序的问题。深度缓冲区是一个附加附件，用于存储每个位置的深度，就像颜色附件存储每个位置的颜色一样。每次光栅化器生成一个片段时，深度测试都会检查新片段是否比前一个片段更近。如果不是，则丢弃新片段。通过深度测试的片段会将其自己的深度写入深度缓冲区。可以从片段着色器操纵此值，就像可以操纵颜色输出一样。

```c++
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
```

默认情况下，由GLM生成的透视投影矩阵将使用-1.0至1.0的OpenGL深度范围。我们需要使用GLM_FORCE_DEPTH_ZERO_TO_ONE定义将其配置为使用0.0到1.0的Vulkan范围。

## Depth image and view

## 深度图形和视图

深度附件是基于图像的，就像颜色附件一样。区别在于交换链不会自动为我们创建深度图像。我们只需要一个深度图像，因为一次只运行一个绘制操作。深度图像将再次需要三重资源：图像，内存和图像视图。

```c++
VkImage depthImage;
VkDeviceMemory depthImageMemory;
VkImageView depthImageView;
```

创建一个新函数createDepthResources来设置以下资源：

```c++
void initVulkan() {
    ...
    createCommandPool();
    createDepthResources();
    createTextureImage();
    ...
}

...

void createDepthResources() {

}
```

创建深度图像非常简单。它应该具有与颜色附件相同的分辨率，该分辨率由交换链范围定义，适合深度附件的图像用法，最佳平铺和设备本地内存。唯一的问题是：深度图像的正确格式是什么？该格式必须包含一个深度分量，在VK_FORMAT_中用 _D??\_表示。

与纹理图像不同，我们不一定需要特定的格式，因为我们不会直接从程序访问深度纹理像素。它仅需要具有合理的精度，至少在实际应用中是24位。有几种符合此要求的格式：

* VK_FORMAT_D32_SFLOAT：32位浮点深度
* VK_FORMAT_D32_SFLOAT_S8_UINT：用于深度的32位带符号浮点数和8位模板组件
* VK_FORMAT_D24_UNORM_S8_UINT：深度为24位浮点和8位模板组件

模板组件用于模板测试，这是可以与深度测试结合使用的附加测试。我们将在以后的章节中对此进行介绍。

我们可以简单地选择VK_FORMAT_D32_SFLOAT格式，因为对它的支持非常普遍（请参阅硬件数据库），但是最好在应用程序中添加一些额外的灵活性。我们将编写一个函数findSupportedFormat，该函数以从最理想到最不理想的顺序获取候选格式的列表，并检查哪个是第一个受支持的格式：

```c++
VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) {

}
```

格式的支持取决于切片模式和用法，因此我们还必须将它们作为参数包括在内。可以使用vkGetPhysicalDeviceFormatProperties函数查询格式的支持：

```c++
for (VkFormat format : candidates) {
    VkFormatProperties props;
    vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);
}
```

VkFormatProperties结构包含三个字段：

* linearTilingFeatures：线性平铺支持的用例
* optimumTilingFeatures：最佳切片支持的用例
* bufferFeatures：缓冲区支持的用例

这里只有前两个是相关的，我们检查的取决于函数的tiling参数：

```c++
if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
    return format;
} else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
    return format;
}
```

如果没有一种候选格式支持所需的用法，那么我们可以返回一个特殊值或简单地引发一个异常：

```c++
VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) {
    for (VkFormat format : candidates) {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);

        if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
            return format;
        } else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
            return format;
        }
    }

    throw std::runtime_error("failed to find supported format!");
}
```

现在，我们将使用此函数来创建findDepthFormat辅助函数，以选择具有支持用作深度附件的深度组件的格式：

```c++
VkFormat findDepthFormat() {
    return findSupportedFormat(
        {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
    );
}
```

在这种情况下，请确保使用VK_FORMAT_FEATURE_标志而不是VK_IMAGE_USAGE_。所有这些候选格式均包含深度组件，但后两种还包含模板组件。我们暂时不会使用它，但是在使用这些格式的图像上执行布局转换时，确实需要考虑到这一点。添加一个简单的辅助函数，该函数可以告诉我们所选择的深度格式是否包含模板组件：

```c++
bool hasStencilComponent(VkFormat format) {
    return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}
```

调用该函数以从createDepthResources查找深度格式：

```c++
VkFormat depthFormat = findDepthFormat();
```

现在，我们具有调用createImage和createImageView辅助函数所需的所有信息：

```c++
createImage(swapChainExtent.width, swapChainExtent.height, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depthImage, depthImageMemory);
depthImageView = createImageView(depthImage, depthFormat);
```

但是，createImageView函数当前假定子资源始终是VK_IMAGE_ASPECT_COLOR_BIT，因此我们需要将该字段转换为参数：

```c++
VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags) {
    ...
    viewInfo.subresourceRange.aspectMask = aspectFlags;
    ...
}
```

更新对此函数的所有调用以使用正确的aspect：

```c++
swapChainImageViews[i] = createImageView(swapChainImages[i], swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
...
depthImageView = createImageView(depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
...
textureImageView = createImageView(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);
```

这就是创建深度图像的过程。我们不需要映射它或将其他图像复制到它，因为我们将在渲染过程开始时将其清除，例如颜色附件。

### Explicity transitioning the depth image

### 显示转换深度图像

我们不需要将图像的布局显式过渡到深度附件，因为我们将在渲染过程中进行处理。但是，为了完整起见，我仍将在本节中描述该过程。如果愿意，可以跳过它。

在createDepthResources函数的末尾调用transitionImageLayout，如下所示：

```c++
transitionImageLayout(depthImage, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
```

未定义的布局可用作初始布局，因为没有重要的现有深度图像内容。我们需要更新transitionImageLayout中的一些逻辑以使用正确的子资源方面：

```c++
if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

    if (hasStencilComponent(format)) {
        barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
    }
} else {
    barrier.subresourceRange.aspectMas
```

尽管我们没有使用模板组件，但确实需要将其包括在深度图像的布局过渡中。

最后，添加正确的访问掩码和管线阶段：

```c++
if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

    sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
} else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
} else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
} else {
    throw std::invalid_argument("unsupported layout transition!");
}
```

将读取深度缓冲区以执行深度测试以查看片段是否可见，并在绘制新片段时写入深度缓冲区。读取发生在VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT阶段，写入发生在VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT。您应该选择与指定操作匹配的最早的管道阶段，以便在需要时可以用作深度附件。

## Render pass

## 渲染通道

现在，我们将修改createRenderPass以包括深度附件。首先指定VkAttachmentDescription：

format应与深度图像本身相同。这次我们不在乎存储深度数据（storeOp），因为在绘制完成后将不再使用它。这可以允许硬件执行其他优化。就像颜色缓冲区一样，我们不需要关心先前的深度内容，因此可以将VK_IMAGE_LAYOUT_UNDEFINED用作initialLayout。

```c++
VkAttachmentReference depthAttachmentRef = {};
depthAttachmentRef.attachment = 1;
depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
```

为第一个（也是唯一一个）子通道添加对附件的引用：

```c++
VkSubpassDescription subpass = {};
subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
subpass.colorAttachmentCount = 1;
subpass.pColorAttachments = &colorAttachmentRef;
subpass.pDepthStencilAttachment = &depthAttachmentRef;
```

与颜色附件不同，子通道只能使用单个深度（+模板）附件。在多个缓冲区上进行深度测试并没有任何意义。

```c++
std::array<VkAttachmentDescription, 2> attachments = {colorAttachment, depthAttachment};
VkRenderPassCreateInfo renderPassInfo = {};
renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
renderPassInfo.pAttachments = attachments.data();
renderPassInfo.subpassCount = 1;
renderPassInfo.pSubpasses = &subpass;
renderPassInfo.dependencyCount = 1;
renderPassInfo.pDependencies = &dependency;
```

最后，更新VkRenderPassCreateInfo结构以引用两个附件。

## Framebuffer

## 帧缓冲

下一步是修改帧缓冲区的创建，以将深度图像绑定到深度附件。转到createFramebuffers并将深度图像视图指定为第二个附件：

```c++
std::array<VkImageView, 2> attachments = {
    swapChainImageViews[i],
    depthImageView
};

VkFramebufferCreateInfo framebufferInfo = {};
framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
framebufferInfo.renderPass = renderPass;
framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
framebufferInfo.pAttachments = attachments.data();
framebufferInfo.width = swapChainExtent.width;
framebufferInfo.height = swapChainExtent.height;
framebufferInfo.layers = 1;
```

每个交换链图像的颜色附件都不同，但是由于我们的信号量，所有对象都可以使用相同的深度图像，因为同一时间只能运行一个子通道。

您还需要移动对createFramebuffers的调用，以确保在实际创建深度图像视图之后调用该调用：

```c++
void initVulkan() {
    ...
    createDepthResources();
    createFramebuffers();
    ...
}
```

## Clear values

## 清除值

因为我们现在有多个带有VK_ATTACHMENT_LOAD_OP_CLEAR的附件，所以我们还需要指定多个清除值。转到createCommandBuffers并创建一个VkClearValue结构数组：

```c++
std::array<VkClearValue, 2> clearValues = {};
clearValues[0].color = {0.0f, 0.0f, 0.0f, 1.0f};
clearValues[1].depthStencil = {1.0f, 0};

renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
renderPassInfo.pClearValues = clearValues.data();
```

在Vulkan中，深度缓冲区中的深度范围为0.0到1.0，其中1.0位于远视平面，而0.0位于近视平面。深度缓冲区中每个点的初始值应为最远的深度，即1.0。

请注意，clearValues的顺序应与附件的顺序相同。

## Depth and stencil state

## 深度和模板状态

深度附件现在可以使用了，但是深度测试仍需要在图形管道中启用。通过VkPipelineDepthStencilStateCreateInfo结构进行配置：

```c++
VkPipelineDepthStencilStateCreateInfo depthStencil = {};
depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
depthStencil.depthTestEnable = VK_TRUE;
depthStencil.depthWriteEnable = VK_TRUE;
```

depthTestEnable字段指定是否应将新片段的深度与深度缓冲区进行比较，以查看是否应将其丢弃。 depthWriteEnable字段指定是否应将通过深度测试的新片段深度实际写入深度缓冲区。这对于绘制透明对象很有用。应该将它们与先前渲染的不透明对象进行比较，但不要使更远的透明对象无法绘制。

```c++
depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
```

depthCompareOp字段指定为保留或丢弃片段而执行的比较。我们使用较低深度=更近的约定，因此新片段的深度应较小。

```c++
depthStencil.depthBoundsTestEnable = VK_FALSE;
depthStencil.minDepthBounds = 0.0f; // Optional
depthStencil.maxDepthBounds = 1.0f; // Optional
```

depthBoundsTestEnable，minDepthBounds和maxDepthBounds字段用于可选的深度限制测试。基本上，这仅允许您保留落入指定深度范围内的片段。我们将不会使用此功能。

```c++
depthStencil.stencilTestEnable = VK_FALSE;
depthStencil.front = {}; // Optional
depthStencil.back = {}; // Optional
```

最后三个字段用于配置模板缓冲区操作，本教程中也不会使用。如果要使用这些操作，则必须确保深度/模板图像的格式包含模板组件。

```c++
pipelineInfo.pDepthStencilState =＆depthStencil;
```

更新VkGraphicsPipelineCreateInfo结构以引用我们刚刚填充的深度模板状态。如果渲染通道包含深度模板附件，则必须始终指定深度模板状态。

如果现在运行程序，则应该看到几何片段的顺序正确：

![img](https://vulkan-tutorial.com/images/depth_correct.png)

## Handling window resize

## 处理窗口缩放

调整窗口大小以匹配新的颜色附件分辨率时，深度缓冲区的分辨率应更改。在这种情况下，扩展recreateSwapChain函数以重新创建深度资源：

```c++
void recreateSwapChain() {
    int width = 0, height = 0;
    while (width == 0 || height == 0) {
        glfwGetFramebufferSize(window, &width, &height);
        glfwWaitEvents();
    }

    vkDeviceWaitIdle(device);

    cleanupSwapChain();

    createSwapChain();
    createImageViews();
    createRenderPass();
    createGraphicsPipeline();
    createDepthResources();
    createFramebuffers();
    createUniformBuffers();
    createDescriptorPool();
    createDescriptorSets();
    createCommandBuffers();
}
```

清除操作应在交换链清除函数中进行：

```c++
void cleanupSwapChain() {
    vkDestroyImageView(device, depthImageView, nullptr);
    vkDestroyImage(device, depthImage, nullptr);
    vkFreeMemory(device, depthImageMemory, nullptr);

    ...
}
```

恭喜，您的应用程序现在终于可以渲染任意3D几何并使其看起来正确了。我们将在下一章中通过绘制纹理模型来进行尝试！