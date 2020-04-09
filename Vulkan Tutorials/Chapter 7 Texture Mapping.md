# Chapter 7 Texture mapping

# 第7章 纹理映射

## Images

## 图像

### Introduction

### 简介

到目前为止，已使用每顶点颜色为几何图形着色，这是一种相当有限的方法。在本教程的这一部分中，我们将实现纹理映射以使几何看起来更有趣。这也将使我们在以后的章节中加载和绘制基本的3D模型。

向我们的应用程序添加纹理将涉及以下步骤：

* 创建由设备内存支持的图像对象
* 用图像文件中的像素填充
* 创建一个图像采样器
* 添加组合的图像采样器描述符以从纹理中采样颜色

我们之前已经使用过图像对象，但是这些对象是由交换链扩展自动创建的。这次我们必须自己创建一个。创建图像并将其填充数据类似于创建顶点缓冲区。我们将从创建临时资源并将其填充像素数据开始，然后将其复制到将用于渲染的最终图像对象。尽管可以为此目的创建暂存图像，但是Vulkan还允许您将像素从VkBuffer复制到图像，并且在某些硬件上，此API的速度实际上更快。我们将首先创建此缓冲区并用像素值填充它，然后创建图像来进行复制。创建图像与创建缓冲区没有太大区别。正如我们之前所见，它涉及到查询内存需求，分配设备内存并对其进行绑定。

但是，在处理图像时，我们需要注意一些其他事项。图像的布局可能会影响像素在内存中的组织方式。由于图形硬件的工作方式，例如，仅逐行存储像素可能不会导致最佳性能。对图像执行任何操作时，必须确保它们具有最适合在该操作中使用的布局。当指定渲染通道时，我们实际上已经看到了其中一些布局：

* VK_IMAGE_LAYOUT_PRESENT_SRC_KHR：最适合呈现
* VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL：最适合作为从片段着色器写入颜色的附件
* VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL：最适合作为传输操作中的源，例如vkCmdCopyImageToBuffer
* VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL：最适合作为传输操作中的目标，例如vkCmdCopyBufferToImage
* VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL：最适合从着色器采样

转换图像布局的最常见方法之一是管线屏障( *barrier*)。管线屏障主要用于同步对资源的访问，例如确保在读取图像之前已将其写入，但是它们也可以用于转换布局。在本章中，我们将了解如何将管线屏障用于此目的。使用VK_SHARING_MODE_EXCLUSIVE时，还可以使用屏障来转移队列家族的所有权。

### Image library

### 图像库

有许多库可用于加载图像，您甚至可以编写自己的代码来加载BMP和PPM等简单格式。在本教程中，我们将使用[stb集合](https://github.com/nothings/stb)中的stb_image库。这样做的好处是所有代码都在一个文件中，因此不需要任何棘手的构建配置。下载stb_image.h并将其存储在方便的位置，例如保存GLFW和GLM的目录。将位置添加到您的包含路径。

**Visual Studio**

将带有stb_image.h的目录添加到“Additional Include Directories”路径。

![img](https://vulkan-tutorial.com/images/include_dirs_stb.png)

**Makefile**

将带有stb_image.h的目录添加到GCC的包含目录中：

```makefile
VULKAN_SDK_PATH = /home/user/VulkanSDK/x.x.x.x/x86_64
STB_INCLUDE_PATH = /home/user/libraries/stb

...

CFLAGS = -std=c++17 -I$(VULKAN_SDK_PATH)/include -I$(STB_INCLUDE_PATH)
```

### Load an image

### 加载一张图像

像这样包含图像库：

```c++
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
```

头文件默认情况下仅定义函数的原型。一个代码文件需要包含带有STB_IMAGE_IMPLEMENTATION定义的头文件以包含函数主体，否则我们将获得链接错误。

```c++
void initVulkan() {
    ...
    createCommandPool();
    createTextureImage();
    createVertexBuffer();
    ...
}

...

void createTextureImage() {

}
```

创建一个新函数createTextureImage，在其中我们将加载图像并将其上传到Vulkan图像对象中。我们将使用命令缓冲区，因此应在createCommandPool之后调用它。

在shaders目录旁边创建一个新的目录textures来存储纹理图像。我们将从该目录中加载一个名为texture.jpg的图像。我选择使用以下[CC0许可](https://pixabay.com/en/statue-sculpture-fig-historically-1275469/)的图像，将其尺寸调整为512 x 512像素，但可以随意选择所需的任何图像。该库支持大多数常见的图像文件格式，例如JPEG，PNG，BMP和GIF。

![img](https://vulkan-tutorial.com/images/texture.jpg)

使用此库加载图像非常简单：

```c++
void createTextureImage() {
    int texWidth, texHeight, texChannels;
    stbi_uc* pixels = stbi_load("textures/texture.jpg", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    VkDeviceSize imageSize = texWidth * texHeight * 4;

    if (!pixels) {
        throw std::runtime_error("failed to load texture image!");
    }
}
```

stbi_load函数将文件路径和要加载的通道数作为参数。 STBI_rgb_alpha值强制将图像带Alpha通道加载，即使它没有也是如此，这对于将来与其他纹理的一致性非常有用。中间的三个参数是图像中通道的宽度，高度和实际数量的输出。返回的指针是像素值数组中的第一个元素。在STBI_rgb_alpha的情况下，像素以4字节为单位逐行排列，总共为texWidth * texHeight  * 4。

### Staging buffer

### 暂存缓冲

现在，我们将在主机可见内存中创建一个缓冲区，以便我们可以使用vkMapMemory并将像素复制到其中。将此临时缓冲区的变量添加到createTextureImage函数：

```c++
VkBuffer stagingBuffer;
VkDeviceMemory stagingBufferMemory;
```

缓冲区应位于主机可见的内存中，以便我们可以对其进行映射，并且应该可用作传输源，以便稍后可以将其复制到图像中：

```c++
createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);
```

然后，我们可以直接将从图像加载库中获得的像素值复制到缓冲区中：

```c++
void* data;
vkMapMemory(device, stagingBufferMemory, 0, imageSize, 0, &data);
    memcpy(data, pixels, static_cast<size_t>(imageSize));
vkUnmapMemory(device, stagingBufferMemory);
```

不要忘记现在清理原始像素阵列：

```c++
stbi_image_free(pixels);
```

### Texture image

### 纹理图像

尽管我们可以设置着色器来访问缓冲区中的像素值，但为此目的最好使用Vulkan中的图像对象。通过允许我们使用2D坐标，图像对象能更容易，更快地检索颜色。图像对象中的像素称为纹理像素，此后我们将使用该名称。添加以下新的类成员：

```c++
VkImage textureImage;
VkDeviceMemory textureImageMemory;
```

图像的参数在VkImageCreateInfo结构中指定：

```c++
VkImageCreateInfo imageInfo = {};
imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
imageInfo.imageType = VK_IMAGE_TYPE_2D;
imageInfo.extent.width = static_cast<uint32_t>(texWidth);
imageInfo.extent.height = static_cast<uint32_t>(texHeight);
imageInfo.extent.depth = 1;
imageInfo.mipLevels = 1;
imageInfo.arrayLayers = 1;
```

在imageType字段中指定的图像类型告诉Vulkan使用哪种坐标系来寻址图像中的纹理像素。可以创建1D，2D和3D图像。例如，一维图像可用于存储数据或渐变的数组，二维图像主要用于纹理，而三维图像可用于存储体素体积。范围字段指定图像的尺寸，基本上是每个轴上有多少个纹理像素。这就是为什么深度必须为1而不是0的原因。我们的纹理将不会是数组，并且我们现在不会使用mipmapping。

```c++
imageInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
```

Vulkan支持许多可能的图像格式，但是对于像素像素，我们应该使用与缓冲区中像素相同的格式，否则复制操作将失败。

```c++
imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
```

tiling段可以具有以下两个值之一：

* VK_IMAGE_TILING_LINEAR：像我们的像素数组一样，以行主序排列像素
* VK_IMAGE_TILING_OPTIMAL：以实现定义的顺序排列像素，以实现最佳访问

与图像的布局不同，平铺模式无法在以后更改。如果要能够直接访问图像内存中的纹理像素，则必须使用VK_IMAGE_TILING_LINEAR。我们将使用暂存缓冲区而不是暂存图像，因此这不是必需的。我们将使用VK_IMAGE_TILING_OPTIMAL从着色器进行高效访问。

```c++
imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
```

图像的initialLayout只有两个可能的值：

* VK_IMAGE_LAYOUT_UNDEFINED：GPU无法使用，并且第一个过渡将丢弃纹理像素。
* VK_IMAGE_LAYOUT_PREINITIALIZED：GPU无法使用，但第一次过渡将保留纹理像素。

在少数情况下，在第一个过渡期间必须保留纹理像素。一个示例是，如果您想结合使用VK_IMAGE_TILING_LINEAR布局将图像用作暂存图像。在这种情况下，您希望将texel数据上传到其中，然后将图像转换为传输源而不丢失数据。但是，在本例中，我们将首先将图像转换为传输目的地，然后从缓冲区对象将纹理像素数据复制到该图像，因此我们不需要此属性，可以安全地使用VK_IMAGE_LAYOUT_UNDEFINED。

```c++
imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
```

usage字段与缓冲区创建期间的语义相同。该图像将用作缓冲区副本的目的地，因此应将其设置为传输目的地。我们还希望能够从着色器访问图像来为网格着色，因此usage应包括VK_IMAGE_USAGE_SAMPLED_BIT。

```c++
imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
```

该图像将仅由一个队列族使用：该队列族支持图形（因此也支持）传输操作。

```c++
imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
imageInfo.flags = 0; // Optional
```

sample标志与多重采样有关。这仅与将用作附件的图像有关，因此只使用一个样本。与稀疏图像有关的图像有一些可选标志。稀疏图像是仅某些区域存在内存中的图像。例如，如果将3D纹理用于体素地形，则可以使用它来避免分配内存来存储大量的“空气”值。在本教程中，我们不会使用它，因此将其保留为默认值0。

```c++
if (vkCreateImage(device, &imageInfo, nullptr, &textureImage) != VK_SUCCESS) {
    throw std::runtime_error("failed to create image!");
}
```

使用vkCreateImage创建图像，该图像没有任何特别值得注意的参数。图形硬件可能不支持VK_FORMAT_R8G8B8A8_SRGB格式。您应该列出可接受的替代方案，并选择支持的最佳方案。但是，对这种特定格式的支持非常广泛，因此我们将跳过此步骤。使用不同的格式还需要烦人的转换。我们将在“深度缓冲”一章中回到这一点，在那里我们将实现这样的系统。

```c++
VkMemoryRequirements memRequirements;
vkGetImageMemoryRequirements(device, textureImage, &memRequirements);

VkMemoryAllocateInfo allocInfo = {};
allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
allocInfo.allocationSize = memRequirements.size;
allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

if (vkAllocateMemory(device, &allocInfo, nullptr, &textureImageMemory) != VK_SUCCESS) {
    throw std::runtime_error("failed to allocate image memory!");
}

vkBindImageMemory(device, textureImage, textureImageMemory, 0);
```

为图像分配内存的方式与为缓冲区分配内存的方式完全相同。使用vkGetImageMemoryRequirements代替vkGetBufferMemoryRequirements，并使用vkBindImageMemory代替vkBindBufferMemory。

该函数已经变得非常庞大，在以后的章节中将需要创建更多图像，因此我们应该像创建缓冲区一样将图像创建抽象为createImage函数。创建函数并将图像对象的创建和内存分配移至该函数：

```c++
void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory) {
    VkImageCreateInfo imageInfo = {};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateImage(device, &imageInfo, nullptr, &image) != VK_SUCCESS) {
        throw std::runtime_error("failed to create image!");
    }

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(device, image, &memRequirements);

    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate image memory!");
    }

    vkBindImageMemory(device, image, imageMemory, 0);
}
```

我已经设置了宽度，高度，格式，切片模式，用法和内存属性参数，因为在我们将在本教程中创建的所有图像之间，这些参数都会有所不同。

现在可以将createTextureImage函数简化为：

```c++
void createTextureImage() {
    int texWidth, texHeight, texChannels;
    stbi_uc* pixels = stbi_load("textures/texture.jpg", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    VkDeviceSize imageSize = texWidth * texHeight * 4;

    if (!pixels) {
        throw std::runtime_error("failed to load texture image!");
    }

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(device, stagingBufferMemory, 0, imageSize, 0, &data);
        memcpy(data, pixels, static_cast<size_t>(imageSize));
    vkUnmapMemory(device, stagingBufferMemory);

    stbi_image_free(pixels);

    createImage(texWidth, texHeight, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage, textureImageMemory);
}
```

### Layout transitions

### 布局过渡

我们现在要编写的函数涉及再次记录和执行命令缓冲区，因此现在是将该逻辑移入一两个辅助函数的好时机：

```c++
VkCommandBuffer beginSingleTimeCommands() {
    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
}

void endSingleTimeCommands(VkCommandBuffer commandBuffer) {
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(graphicsQueue);

    vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
}
```

这些功能的代码基于copyBuffer中的现有代码。现在，您可以将该函数简化为：

```c++
void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();

    VkBufferCopy copyRegion = {};
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    endSingleTimeCommands(commandBuffer);
}
```

如果我们仍在使用缓冲区，那么我们现在可以编写一个函数来记录并执行vkCmdCopyBufferToImage以完成作业，但是此命令要求图像首先位于正确的布局中。创建一个新函数来处理布局转换：

```c++
void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout) {
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();

    endSingleTimeCommands(commandBuffer);
}
```

执行布局转换的最常见方法之一是使用图像内存屏障。像这样的管线屏障通常用于同步对资源的访问，例如确保在对缓冲区的读取之前完成对缓冲区的写入，但是当使用VK_SHARING_MODE_EXCLUSIVE时，它也可以用于转换图像布局和转移队列家族所有权。对于缓冲区，存在等效的缓冲区内存屏障。

```c++
VkImageMemoryBarrier barrier = {};
barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
barrier.oldLayout = oldLayout;
barrier.newLayout = newLayout;
```

前两个字段指定布局转换。如果您不关心图像的现有内容，则可以将VK_IMAGE_LAYOUT_UNDEFINED用作oldLayout。

```c++
barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
```

如果使用障碍传递队列家族所有权，那么这两个字段应该是队列家族的索引。如果您不想这样做，则必须将它们设置为VK_QUEUE_FAMILY_IGNORED（不是默认值！）。

image和subresourceRange指定受影响的图像以及该图像的特定部分。我们的图像不是数组，没有mipmapping级别，因此仅指定了一个级别和一层。

```c++
barrier.srcAccessMask = 0; // TODO
barrier.dstAccessMask = 0; // TODO
```

屏障主要用于同步目的，因此您必须指定在屏障之前必须进行涉及资源的哪些操作类型，以及在屏障上要进行哪些涉及资源的操作。尽管已经使用vkQueueWaitIdle进行手动同步，但我们仍需要这样做。正确的值取决于新旧布局，因此一旦确定了要使用的过渡，我们将重新讨论。

```c++
vkCmdPipelineBarrier(
    commandBuffer,
    0 /* TODO */, 0 /* TODO */,
    0,
    0, nullptr,
    0, nullptr,
    1, &barrier
);
```

所有类型的管线壁垒均使用相同的函数提交。命令缓冲区之后的第一个参数指定应该在屏障之前执行的操作发生在哪个管线阶段。第二个参数指定操作将在屏障上等待的管线阶段。允许您在屏障之前和之后指定的管线阶段取决于屏障之前和之后如何使用资源。规范的[此表](https://www.khronos.org/registry/vulkan/specs/1.0/html/vkspec.html#synchronization-access-types-supported)中列出了允许的值。例如，如果您要从障碍后的uniform缓冲读取，则应指定VK_ACCESS_UNIFORM_READ_BIT的用法以及将从uniform读取的最早的着色器，作为管线阶段，例如VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT。为这种使用类型指定非着色器管线阶段是没有意义的，并且当您指定与使用类型不匹配的管线阶段时，验证层会警告您。

第三个参数是0或VK_DEPENDENCY_BY_REGION_BIT。后者将屏障变为每区域条件(per-region condition)。这意味着，例如，允许该实现从资源的到目前为止已写入的部分开始读取。

最后三对参数引用了三种可用类型的流水线屏障的数组：内存屏障，缓冲内存屏障和图像内存屏障，如我们在此处使用的那样。注意，我们还没有使用VkFormat参数，但是我们将在深度缓冲区一章中使用该参数进行特殊转换。

### Copying buffer to image

### 拷贝缓冲到图像

回到createTextureImage之前，我们将再编写一个辅助函数：copyBufferToImage：

```c++
void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();

    endSingleTimeCommands(commandBuffer);
}
```

就像缓冲区拷贝一样，您需要指定缓冲区的哪一部分将被复制到图像的哪一部分。这是通过VkBufferImageCopy结构实现的：

```c++
VkBufferImageCopy region = {};
region.bufferOffset = 0;
region.bufferRowLength = 0;
region.bufferImageHeight = 0;

region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
region.imageSubresource.mipLevel = 0;
region.imageSubresource.baseArrayLayer = 0;
region.imageSubresource.layerCount = 1;

region.imageOffset = {0, 0, 0};
region.imageExtent = {
    width,
    height,
    1
};
```

这些字段大多数都是不言自明的。 bufferOffset指定像素值开始处的缓冲区中的字节偏移量。 bufferRowLength和bufferImageHeight字段指定像素在内存中的布局方式。例如，您可能在图像的行之间有一些填充字节。两者均指定为0表示象我们本例一样，像素紧密地包装在一起。 imageSubresource，imageOffset和imageExtent字段指示我们要将像素复制到图像的哪一部分。

使用vkCmdCopyBufferToImage函数可以将缓冲区复制到图像：

```c++
vkCmdCopyBufferToImage(
    commandBuffer,
    buffer,
    image,
    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
    1,
    &region
);
```

第四个参数指示图像当前使用的布局。我在这里假设图像已经过渡到最适合像素复制的布局。现在，我们仅将一个像素块复制到整个图像，但是可以指定一个VkBufferImageCopy数组，在一次操作中执行多次从缓冲到图像的拷贝。

### Preparing the texture image

### 准备纹理图像

现在，我们拥有完成纹理图像设置所需的所有工具，因此我们将回到createTextureImage函数。我们在那里做的最后一件事是创建纹理图像。下一步是将暂存缓冲区复制到纹理图像。这涉及两个步骤：

* 将纹理图像过渡到VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
* 执行缓冲区到映像的复制操作

使用我们刚创建的函数很容易做到这一点：

```c++
transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
copyBufferToImage(stagingBuffer, textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
```

该图像是使用VK_IMAGE_LAYOUT_UNDEFINED布局创建的，因此在转换textureImage时应将其指定为旧布局。请记住，我们可以这样做是因为在执行复制操作之前我们并不关心其内容。

为了能够从着色器中的纹理图像开始采样，我们需要最后一个过渡来准备它以供着色器访问：

```c++
transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
```

### Transition barrier masks

### 过渡屏障掩码

如果您现在运行启用了验证层的应用程序，那么您将看到它抱怨TransitionImageLayout中的访问掩码和管道阶段无效。我们仍然需要根据过渡中的布局进行设置。

我们需要处理两个过渡：

* 未定义→传输目的地：不需要等待任何内容的传输写入
* 传输目标→着色器读取：着色器读取应等待传输写入，尤其是片段着色器中的着色器读取，因为这是我们要使用纹理的地方。

使用以下访问掩码和管线阶段来指定这些规则：

```c++
VkPipelineStageFlags sourceStage;
VkPipelineStageFlags destinationStage;

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
} else {
    throw std::invalid_argument("unsupported layout transition!");
}

vkCmdPipelineBarrier(
    commandBuffer,
    sourceStage, destinationStage,
    0,
    0, nullptr,
    0, nullptr,
    1, &barrier
);
```


如您在上述表中所见，传输写操作必须在管线传输阶段进行。由于写入不必等待任何内容，因此您可以指定一个空的访问掩码，并为屏障前操作指定最早的管线阶段VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT。应该注意的是，VK_PIPELINE_STAGE_TRANSFER_BIT不是图形和计算管线中的真实阶段。传输更像是伪阶段。有关更多信息和伪阶段的其他示例，请参见[文档](https://www.khronos.org/registry/vulkan/specs/1.1-extensions/man/html/VkPipelineStageFlagBits.html)。

该图像将在同一管线阶段中写入，然后由片段着色器读取，这就是为什么我们在片段着色器管线阶段指定着色器读取访问权限的原因。

如果将来需要进行更多转换，则将扩展该函数。该应用程序现在应该可以成功运行，尽管当然还没有任何视觉变化。

需要注意的一件事是，提交命令缓冲区会在开始时导致隐式的VK_ACCESS_HOST_WRITE_BIT同步。由于transitionImageLayout函数仅使用单个命令执行命令缓冲区，因此如果在布局转换中需要VK_ACCESS_HOST_WRITE_BIT依赖项，则可以使用此隐式同步并将srcAccessMask设置为0。是否要明确显示由您自己决定，但是我个人并不喜欢依赖这些类似OpenGL的“隐藏”操作。

实际上，有一种特殊的图像布局类型VK_IMAGE_LAYOUT_GENERAL可以支持所有操作。当然，它的问题在于，它不一定能为任何操作提供最佳性能。在某些特殊情况下，例如使用图像作为输入和输出，或者在离开预初始化的布局后读取图像，这是必需的。

到目前为止，所有提交命令的帮助函数都已设置为通过等待队列空闲而同步执行。对于实际应用，建议将这些操作组合在单个命令缓冲区中，并异步执行它们以提高吞吐量，尤其是createTextureImage函数中的过渡和复制。尝试通过创建一一个将命令记录到setupCommandBuffer的帮助函数，并添加一个flushSetupCommands函数来执行到目前为止已记录的命令来进行实验。最好在纹理映射工作后执行此操作，以检查纹理资源是否仍正确设置。

### Clean up

### 清理

图像现在包含纹理，但是我们仍然需要一种从图形管道访问纹理的方法。我们将在下一章中对此进行研究。

## Image view and sampler

## 图像视图和采样器

在本章中，我们将创建图形管道对图像进行采样所需的另外两个资源。第一个资源是我们在使用交换链图像时已经见过的资源，但是第二个资源是新资源-它涉及着色器如何从图像中读取纹理像素。

### Texture image view

### 纹理图像视图

之前我们已经看到，通过交换链图像和帧缓冲区，可以通过图像视图而不是直接访问图像。我们还需要为纹理图像创建这样的图像视图。

添加一个类成员以保存纹理图像的VkImageView，并在其中创建新函数createTextureImageView：

```c++
VkImageView textureImageView;

...

void initVulkan() {
    ...
    createTextureImage();
    createTextureImageView();
    createVertexBuffer();
    ...
}

...

void createTextureImageView() {

}
```

此函数的代码可以直接基于createImageViews。您只需要做的两个更改就是format和image：

```c++
VkImageViewCreateInfo viewInfo = {};
viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
viewInfo.image = textureImage;
viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
viewInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
viewInfo.subresourceRange.baseMipLevel = 0;
viewInfo.subresourceRange.levelCount = 1;
viewInfo.subresourceRange.baseArrayLayer = 0;
viewInfo.subresourceRange.layerCount = 1;
```

由于VK_COMPONENT_SWIZZLE_IDENTITY始终被定义为0，因此我省略了显式的viewInfo.components初始化。通过调用vkCreateImageView完成创建图像视图：

```c++
if (vkCreateImageView(device, &viewInfo, nullptr, &textureImageView) != VK_SUCCESS) {
    throw std::runtime_error("failed to create texture image view!");
}
```

由于从createImageViews复制了很多逻辑，因此您可能希望将其抽象为新的createImageView函数：

```c++
VkImageView createImageView(VkImage image, VkFormat format) {
    VkImageViewCreateInfo viewInfo = {};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    VkImageView imageView;
    if (vkCreateImageView(device, &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture image view!");
    }

    return imageView;
}
```

现在可以将createTextureImageView函数简化为：

```c++
void createTextureImageView() {
    textureImageView = createImageView(textureImage, VK_FORMAT_R8G8B8A8_SRGB);
}
```

并且createImageViews可以简化为：

```c++
void createImageViews() {
    swapChainImageViews.resize(swapChainImages.size());

    for (uint32_t i = 0; i < swapChainImages.size(); i++) {
        swapChainImageViews[i] = createImageView(swapChainImages[i], swapChainImageFormat);
    }
}
```

确保在销毁图像本身之前，在程序末尾销毁图像视图：

```c++
void cleanup() {
    cleanupSwapChain();

    vkDestroyImageView(device, textureImageView, nullptr);

    vkDestroyImage(device, textureImage, nullptr);
    vkFreeMemory(device, textureImageMemory, nullptr);
```

### Samplers

### 采样器

着色器可以直接从图像中读取纹理像素，但这在用作纹理时不是很常见。通常通过采样器访问纹理，采样器将应用过滤和转换以计算最终获取的颜色。

这些过滤器有助于处理过采样等问题。考虑纹理映射到片段比纹理像素多的几何体。如果仅在每个片段中使用最接近的纹理像素作为纹理坐标，那么您将获得类似于第一张图像的结果：

![img](https://vulkan-tutorial.com/images/texture_filtering.png)

如果通过线性插值将4个最接近的纹理像素合并在一起，则将获得更平滑的结果，如右图所示。当然，您的应用程序可能具有更适合左侧风格的艺术风格要求（请考虑Minecraft），但是在常规图形应用程序中，右侧是首选。从纹理读取颜色时，采样器对象会自动为您应用此过滤。

欠采样是相反的问题，在该纹理中，纹理像素多于片段。当以锐角采样高频模式（例如棋盘格纹理）时，将导致瑕疵：

![img](https://vulkan-tutorial.com/images/anisotropic_filtering.png)

如左图所示，纹理在远程上变成模糊一团。解决方案是各向异性过滤，也可以由采样器自动应用。

除了这些过滤器之外，采样器还可以处理转换。它决定了当您尝试通过其寻址模式读取图像外部的纹素时发生的情况。下图显示了一些可能性：

![img](https://vulkan-tutorial.com/images/texture_addressing.png)

现在，我们将创建一个函数createTextureSampler来设置此类采样器对象。稍后，我们将使用该采样器从着色器中的纹理读取颜色。

```c++
void initVulkan() {
    ...
    createTextureImage();
    createTextureImageView();
    createTextureSampler();
    ...
}

...

void createTextureSampler() {

}
```

采样器通过VkSamplerCreateInfo结构进行配置，该结构指定所有过滤器和转换。

```c++
VkSamplerCreateInfo samplerInfo = {};
samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
samplerInfo.magFilter = VK_FILTER_LINEAR;
samplerInfo.minFilter = VK_FILTER_LINEAR;
```

magFilter和minFilter字段指定如何对放大或缩小的纹理像素进行插值。放大与上述过采样问题有关，而缩小与欠采样有关。选择是VK_FILTER_NEAREST和VK_FILTER_LINEAR，与上图所示的模式相对应。

```c++
samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
```

可以使用addressMode字段为每个轴指定寻址模式。可用值在下面列出。上面的图片演示了其中的大多数。请注意，轴称为U，V和W，而不是X，Y和Z。这是纹理空间坐标的约定。

* VK_SAMPLER_ADDRESS_MODE_REPEAT：当超出图像尺寸时，重复纹理。
* VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT：类似于重复，但是当超出尺寸时会反转坐标以镜像图像。
* VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE：在图像尺寸之外，获取最靠近坐标的边缘的颜色。
* VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE：类似于钳制到边，而是使用与最接近边相反的边。
* VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER：采样超出图像尺寸时，返回纯色。

在这里使用哪种寻址模式并不重要，因为在本教程中我们不会在图像之外进行采样。但是，重复模式可能是最常见的模式，因为它可用于平铺地板和墙壁等纹理。

```c++
samplerInfo.anisotropyEnable = VK_TRUE;
samplerInfo.maxAnisotropy = 16;
```

这两个字段指定是否应使用各向异性过滤。除非考虑性能，否则没有理由不使用此功能。 maxAnisotropy字段限制了可用于计算最终颜色的纹理像素样本的数量。较低的值会导致更好的性能，但质量会降低。如今，没有可用的图形硬件使用超过16个样本，因为在此之后，差异可以忽略不计。

```c++
samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
```

borderColor字段指定在使用钳位边界寻址模式对图像进行采样时返回的颜色。可以以float或int格式返回黑色，白色或透明。您不能指定任意颜色。

```c++
samplerInfo.unnormalizedCoordinates = VK_FALSE;
```

unnormalizedCoordinates字段指定要用于处理图像中纹理像素的坐标系。如果此字段为VK_TRUE，则可以简单地使用[0，texWidth）和[0，texHeight）范围内的坐标。如果是VK_FALSE，则将使用所有轴上的[0，1）范围对纹理像素进行寻址。实际应用程序几乎总是使用归一化的坐标，因为这样就可以使用分辨率完全相同的不同分辨率的纹理。

```c++
samplerInfo.compareEnable = VK_FALSE;
samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
```

如果启用了比较功能，则将首先将纹理像素与一个值进行比较，并且该比较的结果将用于过滤操作中。这主要用于阴影贴图上的 [percentage-closer filtering](https://developer.nvidia.com/gpugems/GPUGems/gpugems_ch11.html)。我们将在以后的章节中对此进行介绍。

```c++
samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
samplerInfo.mipLodBias = 0.0f;
samplerInfo.minLod = 0.0f;
samplerInfo.maxLod = 0.0f;
```

所有这些字段都适用于mipmapping。我们将在后面的章节中看到mipmapping，但是基本上它是可以应用的另一种过滤器。

采样器的功能现已完全定义。添加一个类成员来保存采样器对象的句柄，并使用vkCreateSampler创建采样器：

```c++
VkImageView textureImageView;
VkSampler textureSampler;

...

void createTextureSampler() {
    ...

    if (vkCreateSampler(device, &samplerInfo, nullptr, &textureSampler) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture sampler!");
    }
}
```

请注意，采样器未在任何地方引用VkImage。采样器是一个独特的对象，提供了从纹理中提取颜色的接口。它可以应用于所需的任何图像，无论是1D，2D还是3D。这与许多较早的API不同，后者将纹理图像和过滤合并为一个状态。

当我们将不再访问图像时，请在程序结尾处销毁采样器：

```c++
void cleanup() {
    cleanupSwapChain();

    vkDestroySampler(device, textureSampler, nullptr);
    vkDestroyImageView(device, textureImageView, nullptr);

    ...
}
```

### Anisotropy device feature

### 各向异性设备特性

如果您现在运行程序，则会看到如下所示的验证层消息：

![img](https://vulkan-tutorial.com/images/validation_layer_anisotropy.png)

这是因为各向异性过滤实际上是一项可选的设备特性。我们需要更新createLogicalDevice函数来请求它：

```c++
VkPhysicalDeviceFeatures deviceFeatures = {};
deviceFeatures.samplerAnisotropy = VK_TRUE;
```

即使现代显卡不太可能不支持它，我们也应该更新isDeviceSuitable来检查它是否可用：

```c++
bool isDeviceSuitable(VkPhysicalDevice device) {
    ...

    VkPhysicalDeviceFeatures supportedFeatures;
    vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

    return indices.isComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
}
```

vkGetPhysicalDeviceFeatures重新设置VkPhysicalDeviceFeatures结构的用途，以指示支持哪些特性，而不是通过设置布尔值来请求。

除了加强各向异性过滤的可用性之外，还可以通过有条件地设置简单地不使用它：

```c++
samplerInfo.anisotropyEnable = VK_FALSE;
samplerInfo.maxAnisotropy = 1;
```

在下一章中，我们将图像和采样器对象暴露给着色器，以将纹理绘制到正方形上。

## Combined image sampler

## 合并的图像采样器

### Introduction

### 简介

我们在本教程的uniform缓冲区部分中首次查看了描述符。在本章中，我们将介绍一种新型的描述符：组合图像采样器。该描述符使着色器可以通过采样器对象（如上一章中创建的对象）访问图像资源。

我们将从修改描述符布局，描述符池和描述符集开始，以包括这样的组合图像采样器描述符。之后，我们将向顶点添加纹理坐标，并修改片段着色器以从纹理中读取颜色，而不仅仅是插入顶点颜色。

### Updating the descriptors

### 更新描述符

浏览到createDescriptorSetLayout函数，并为组合的图像采样器描述符添加VkDescriptorSetLayoutBinding。我们将其简单地放在统一缓冲区之后的绑定中：

```c++
VkDescriptorSetLayoutBinding samplerLayoutBinding = {};
samplerLayoutBinding.binding = 1;
samplerLayoutBinding.descriptorCount = 1;
samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
samplerLayoutBinding.pImmutableSamplers = nullptr;
samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

std::array<VkDescriptorSetLayoutBinding, 2> bindings = {uboLayoutBinding, samplerLayoutBinding};
VkDescriptorSetLayoutCreateInfo layoutInfo = {};
layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
layoutInfo.pBindings = bindings.data();
```

确保设置stageFlags以指示我们打算在片段着色器中使用组合的图像采样器描述符。那儿就是片段颜色确定的地方。可以在顶点着色器中使用纹理采样，例如通过高度图使顶点网格动态变形。

如果现在运行带有验证层的应用程序，那么您会发现它抱怨描述符池无法使用此布局分配描述符集，因为它没有任何组合的图像采样器描述符。转到createDescriptorPool函数并对其进行修改，以包括此描述符的VkDescriptorPoolSize：

```c++
std::array<VkDescriptorPoolSize, 2> poolSizes = {};
poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
poolSizes[0].descriptorCount = static_cast<uint32_t>(swapChainImages.size());
poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
poolSizes[1].descriptorCount = static_cast<uint32_t>(swapChainImages.size());

VkDescriptorPoolCreateInfo poolInfo = {};
poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
poolInfo.pPoolSizes = poolSizes.data();
poolInfo.maxSets = static_cast<uint32_t>(swapChainImages.size());
```

最后一步是将实际图像和采样器资源绑定到描述符集中的描述符。转到createDescriptorSets函数。

```c++
for (size_t i = 0; i < swapChainImages.size(); i++) {
    VkDescriptorBufferInfo bufferInfo = {};
    bufferInfo.buffer = uniformBuffers[i];
    bufferInfo.offset = 0;
    bufferInfo.range = sizeof(UniformBufferObject);

    VkDescriptorImageInfo imageInfo = {};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView = textureImageView;
    imageInfo.sampler = textureSampler;

    ...
}
```

必须在VkDescriptorImageInfo结构中指定用于组合图像采样器结构的资源，就像在VkDescriptorBufferInfo结构中指定用于uniform缓冲区描述符的缓冲区资源一样。这是上一章中的对象结合在一起的地方。

```c++
std::array<VkWriteDescriptorSet, 2> descriptorWrites = {};

descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
descriptorWrites[0].dstSet = descriptorSets[i];
descriptorWrites[0].dstBinding = 0;
descriptorWrites[0].dstArrayElement = 0;
descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
descriptorWrites[0].descriptorCount = 1;
descriptorWrites[0].pBufferInfo = &bufferInfo;

descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
descriptorWrites[1].dstSet = descriptorSets[i];
descriptorWrites[1].dstBinding = 1;
descriptorWrites[1].dstArrayElement = 0;
descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
descriptorWrites[1].descriptorCount = 1;
descriptorWrites[1].pImageInfo = &imageInfo;

vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
```

描述符必须使用此图像信息进行更新，就像缓冲区一样。这次我们使用的是pImageInfo数组而不是pBufferInfo。现在可以将着色器使用描述符了！

### Texture coordinates

### 纹理坐标

纹理映射有一个重要的要素仍然缺失，那就是每个顶点的实际坐标。坐标确定图像实际如何映射到几何。

```c++
struct Vertex {
    glm::vec2 pos;
    glm::vec3 color;
    glm::vec2 texCoord;

    static VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription bindingDescription = {};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Vertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions = {};

        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Vertex, pos);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex, color);

        attributeDescriptions[2].binding = 0;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

        return attributeDescriptions;
    }
};

```

修改“顶点”结构，使其包含用于纹理坐标的vec2。确保还添加了VkVertexInputAttributeDescription，以便我们可以将访问纹理坐标用作顶点着色器中的输入。为了将它们传递到片段着色器以便在正方形表面上进行插值，这是必需的。

```c++
const std::vector<Vertex> vertices = {
    {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
    {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
    {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
    {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}}
};
```

在本教程中，我将使用从左上角的0、0到右下角的1、1的坐标简单地用纹理填充正方形。随意尝试使用不同的坐标。尝试使用低于0或高于1的坐标来查看实际的寻址模式！

### Shaders

### 着色器

最后一步是修改着色器，以从纹理中采样颜色。我们首先需要修改顶点着色器，以将纹理坐标传递到片段着色器：

```c++
layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;

void main() {
    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 0.0, 1.0);
    fragColor = inColor;
    fragTexCoord = inTexCoord;
}
```

就像每个顶点的颜色一样，光栅化器会将fragTexCoord值平滑地插值到正方形区域上。我们可以通过使片段着色器将纹理坐标输出为颜色来形象化：

```c++
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = vec4(fragTexCoord, 0.0, 1.0);
}
```

您应该看到类似下图的内容。不要忘记重新编译着色器！

![img](https://vulkan-tutorial.com/images/texcoord_visualization.png)

绿色通道代表水平坐标，红色通道代表垂直坐标。黑色和黄色的角确认纹理坐标正确地从0、0到1、1插到正方形上。使用颜色可视化数据是与printf调试等效的着色器编程，因为缺少更好的选择！

组合图像采样器描述符在GLSL中由采样器统一表示。在片段着色器中添加对它的引用：

```c++
layout(binding = 1) uniform sampler2D texSampler;
```

对于其他类型的图像，有等效的sampler1D和sampler3D类型。确保在此处使用正确的绑定。

```glsl
void main() {
    outColor = texture(texSampler, fragTexCoord);
}
```

使用内置纹理功能对纹理进行采样。它以采样器和坐标为参数。采样器会自动处理后台的过滤和转换。现在，在运行应用程序时，您应该在正方形上看到纹理：

![img](https://vulkan-tutorial.com/images/texture_on_square.png)

尝试通过将纹理坐标缩放到大于1的值来尝试寻址模式。例如，当使用VK_SAMPLER_ADDRESS_MODE_REPEAT时，以下片段着色器在以下图像中生成结果：

```glsl
void main() {
    outColor = texture(texSampler, fragTexCoord * 2.0);
}
```

![img](https://vulkan-tutorial.com/images/texture_on_square_repeated.png)

您还可以使用顶点颜色来操纵纹理颜色：

```c++
void main() {
    outColor = vec4(fragColor * texture(texSampler, fragTexCoord).rgb, 1.0);
}
```

我将RGB和Alpha通道分开，以不缩放Alpha通道。

![img](https://vulkan-tutorial.com/images/texture_on_square_colorized.png)

您现在知道了如何在着色器中访问图像！当与写入帧缓冲区的图像结合使用时，这是一项非常强大的技术。您可以将这些图像用作输入，以在3D世界中实现出色的效果，例如后处理和相机显示。