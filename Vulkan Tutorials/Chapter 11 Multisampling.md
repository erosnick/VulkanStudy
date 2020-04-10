# Chapter 11 Multisampling

# 第11章 多重采样

## Introduction

## 简介

现在，我们的程序可以为纹理加载多个LOD，从而在渲染远离观察者的对象时可以修复瑕疵。现在，图像变得更加平滑，但是在仔细检查时，您会注意到绘制的几何形状边缘出现锯齿状图案。这在我们的早期程序之一渲染的四边形中尤其明显：

![img](https://vulkan-tutorial.com/images/texcoord_visualization.png)

这种不希望的效果称为“锯齿”，这是由于可用于渲染的像素数量有限而导致的。由于市面上没有无限分辨率的显示器，因此在某种程度上它总是可见的。有很多方法可以解决此问题，在本章中，我们将重点介绍一种较流行的方法：[多重采样抗锯齿](https://en.wikipedia.org/wiki/Multisample_anti-aliasing)（MSAA）。

在通常的渲染中，像素颜色是基于单个采样点确定的，该采样点在大多数情况下是屏幕上目标像素的中心。如果画线的一部分通过某个像素但不覆盖采样点，则该像素将留为空白，从而导致锯齿状的“楼梯”效果。

![img](https://vulkan-tutorial.com/images/aliasing.png)

MSAA的作用是每个像素使用多个采样点（因此得名）来确定其最终颜色。正如人们可能期望的那样，更多的样本会导致更好的结果，但是在计算上也更加昂贵。

![img](https://vulkan-tutorial.com/images/antialiasing.png)

在我们的实现中，我们将重点放在使用最大可用采样数上。取决于您的应用，这可能并不总是最好的方法，如果最终结果满足您的质量要求，则可以使用较少的采样以提高性能，这可能更好。

## Getting available sample count

## 获取可用的采样数量

首先确定硬件可以使用多少个采样。大多数现代GPU至少支持8个采样，但并不能保证每个地方都相同。我们将通过添加一个新的类成员来跟踪它：

默认情况下，我们每个像素仅使用一个样本，这相当于没有多重采样，在这种情况下，最终图像将保持不变。可以从与我们选择的物理设备关联的VkPhysicalDeviceProperty中提取确切的最大样本数。我们使用了深度缓冲区，因此我们必须考虑颜色和深度的采样数。这两个都支持（＆）的最高样本数将是我们可以支持的最大样本数。添加一个将为我们获取此信息的函数：

```c++
VkSampleCountFlagBits getMaxUsableSampleCount() {
    VkPhysicalDeviceProperties physicalDeviceProperties;
    vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);

    VkSampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts & physicalDeviceProperties.limits.framebufferDepthSampleCounts;
    if (counts & VK_SAMPLE_COUNT_64_BIT) { return VK_SAMPLE_COUNT_64_BIT; }
    if (counts & VK_SAMPLE_COUNT_32_BIT) { return VK_SAMPLE_COUNT_32_BIT; }
    if (counts & VK_SAMPLE_COUNT_16_BIT) { return VK_SAMPLE_COUNT_16_BIT; }
    if (counts & VK_SAMPLE_COUNT_8_BIT) { return VK_SAMPLE_COUNT_8_BIT; }
    if (counts & VK_SAMPLE_COUNT_4_BIT) { return VK_SAMPLE_COUNT_4_BIT; }
    if (counts & VK_SAMPLE_COUNT_2_BIT) { return VK_SAMPLE_COUNT_2_BIT; }

    return VK_SAMPLE_COUNT_1_BIT;
}
```

现在，我们将在物理设备选择过程中使用此功能来设置msaaSamples变量。为此，我们必须稍微修改pickPhysicalDevice函数：

```c++
void pickPhysicalDevice() {
    ...
    for (const auto& device : devices) {
        if (isDeviceSuitable(device)) {
            physicalDevice = device;
            msaaSamples = getMaxUsableSampleCount();
            break;
        }
    }
    ...
}
```

## Setting up a render target

## 设置一个渲染目标

在MSAA中，每个像素都在屏幕外缓冲区中采样，然后渲染到屏幕上。这个新缓冲区与我们一直在渲染的常规图像略有不同-它们必须能够为每个像素存储一个以上的样本。创建多采样缓冲区后，必须将其解析为默认的帧缓冲区（每个像素仅存储一个样本）。这就是为什么我们必须创建一个额外的渲染目标并修改我们当前的绘制过程的原因。我们只需要一个渲染目标，因为一次只有一个绘制操作处于活动状态，就像深度缓冲区一样。添加以下类成员：

```c++
...
VkImage colorImage;
VkDeviceMemory colorImageMemory;
VkImageView colorImageView;
...
```

此新图像将必须存储每个像素所需的样本数，因此我们需要在图像创建过程中将此数字传递给VkImageCreateInfo。通过添加numSamples参数来修改createImage函数：

```c++
void createImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory) {
    ...
    imageInfo.samples = numSamples;
    ...
```

现在，使用VK_SAMPLE_COUNT_1_BIT更新对该函数的所有调用-随着实现的进行，我们将用适当的值替换该调用：

```c++
createImage(swapChainExtent.width, swapChainExtent.height, 1, VK_SAMPLE_COUNT_1_BIT, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depthImage, depthImageMemory);
...
createImage(texWidth, texHeight, mipLevels, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage, textureImageMemory);
```

现在，我们将创建一个多采样颜色缓冲区。添加一个createColorResources函数，并注意我们在此处使用msaaSamples作为createImage的函数参数。我们还仅使用一个Mip级别，因为Vulkan规范会在图像每个像素具有多个样本的情况下强制执行此操作。另外，此颜色缓冲区不需要mipmap，因为它不会用作纹理：

```
void createColorResources() {
    VkFormat colorFormat = swapChainImageFormat;

    createImage(swapChainExtent.width, swapChainExtent.height, 1, msaaSamples, colorFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, colorImage, colorImageMemory);
    colorImageView = createImageView(colorImage, colorFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
}
```

为了保持一致性，请在createDepthResources之前立即调用该函数：

```c++
void initVulkan() {
    ...
    createColorResources();
    createDepthResources();
    ...
}
```

现在我们已经有了一个多采样的颜色缓冲区，是时候照顾深度了。修改createDepthResources并更新深度缓冲区使用的样本数：

```c++
void createDepthResources() {
    ...
    createImage(swapChainExtent.width, swapChainExtent.height, 1, msaaSamples, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depthImage, depthImageMemory);
    ...
```

现在，我们已经创建了几个新的Vulkan资源，因此请不要忘记在必要时释放它们：

```c++
void cleanupSwapChain() {
    vkDestroyImageView(device, colorImageView, nullptr);
    vkDestroyImage(device, colorImage, nullptr);
    vkFreeMemory(device, colorImageMemory, nullptr);
    ...
}
```

并更新recreateSwapChain，以便在调整窗口大小时可以正确的分辨率重新创建新的彩色图像：

```c++
void recreateSwapChain() {
    ...
    createGraphicsPipeline();
    createColorResources();
    createDepthResources();
    ...
}
```

我们完成了最初的MSAA设置，现在我们需要在图形管线，帧缓冲区，渲染过程中开始使用此新资源，然后查看结果！

## Adding new attachments

## 添加新的附件

让我们先处理渲染过程。修改createRenderPass并更新颜色和深度附件创建信息结构：

```c++
void createRenderPass() {
    ...
    colorAttachment.samples = msaaSamples;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    ...
    depthAttachment.samples = msaaSamples;
    ...
```

您会注意到，我们已经将finalLayout从VK_IMAGE_LAYOUT_PRESENT_SRC_KHR更改为VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL。那是因为不能直接显示多重采样的图像。我们首先需要将它们解析为常规图像。此要求不适用于深度缓冲区，因为它不会在任何时候显示。因此，我们只需要为颜色添加一个新的附件，即所谓的“解析附件”：

```c++
 ...
    VkAttachmentDescription colorAttachmentResolve = {};
    colorAttachmentResolve.format = swapChainImageFormat;
    colorAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    ...
```

现在必须指示渲染过程将多重采样的彩色图像解析为常规附件。创建一个新的附件引用，该引用将指向用作解析目标的颜色缓冲区：

```c++
 ...
    VkAttachmentReference colorAttachmentResolveRef = {};
    colorAttachmentResolveRef.attachment = 2;
    colorAttachmentResolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    ...
```

将pResolveAttachments子传递结构成员设置为指向新创建的附件引用。这足以让渲染过程定义一个多重采样解析操作，该操作让我们可以将图像渲染到屏幕上：

```c++
   ...
    subpass.pResolveAttachments = &colorAttachmentResolveRef;
    ...
```

现在，使用新的颜色附件更新渲染过程信息结构：

```c++
    ...
    std::array<VkAttachmentDescription, 3> attachments = {colorAttachment, depthAttachment, colorAttachmentResolve};
    ...
```

放置好渲染通道后，修改createFrameBuffers并将新的图像视图添加到列表中：

```c++
void createFrameBuffers() {
        ...
        std::array<VkImageView, 3> attachments = {
            colorImageView,
            depthImageView,
            swapChainImageViews[i]
        };
        ...
}
```

最后，通过修改createGraphicsPipeline来告诉新创建的管道使用多个采样：

```c++
void createGraphicsPipeline() {
    ...
    multisampling.rasterizationSamples = msaaSamples;
    ...
}
```

现在运行程序，您应该看到以下内容：

![img](https://vulkan-tutorial.com/images/multisampling.png)

就像mipmapping一样，差异可能不会立即显现。仔细观察，您会发现屋顶的边缘不再像锯齿一样，整个图像看起来比原始图像更平滑。

![img](https://vulkan-tutorial.com/images/multisampling_comparison.png)

当靠近边缘之一看时，差异更明显：

![img](https://vulkan-tutorial.com/images/multisampling_comparison2.png)

## Quality improvements

## 质量提升

我们当前的MSAA实施存在某些局限性，可能会影响更详细场景中输出图像的质量。例如，我们目前尚未解决由着色器混叠引起的潜在问题，即MSAA仅使几何图形的边缘平滑而不对内部填充平滑。当您在屏幕上渲染平滑多边形时，可能会导致出现这种情况，但是如果所应用的纹理包含高对比度的颜色，则仍会出现锯齿。解决此问题的一种方法是启用“采样着色”，这将进一步改善图像质量，尽管会增加性能成本：

```c++

void createLogicalDevice() {
    ...
    deviceFeatures.sampleRateShading = VK_TRUE; // enable sample shading feature for the device
    ...
}

void createGraphicsPipeline() {
    ...
    multisampling.sampleShadingEnable = VK_TRUE; // enable sample shading in the pipeline
    multisampling.minSampleShading = .2f; // min fraction for sample shading; closer to one is smoother
    ...
}
```

在此示例中，我们将禁用样本明暗处理，但在某些情况下，质量改善可能是显而易见的：

![img](https://vulkan-tutorial.com/images/sample_shading.png)

## Conclusion

## 总结

到目前为止，已经花费了很多工作，但是现在您终于有了Vulkan程序的良好基础。您现在掌握的Vulkan基本原理知识应该足以开始探索更多功能，例如：

- Push constants
- Instanced rendering
- Dynamic uniforms
- Separate images and sampler descriptors
- Pipeline cache
- Multi-threaded command buffer generation
- Multiple subpasses
- Compute shaders

当前程序可以通过多种方式扩展，例如添加Blinn-Phong照明，后处理效果和阴影贴图。您应该能够从其他API的教程中了解这些效果的工作原理，因为尽管Vulkan明确明确，但许多概念仍然起作用。