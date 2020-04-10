# Chapter 10 Generating Mipmaps

# 第10章 生成Mipmaps

## Introduction

## 简介

我们的程序现在可以加载和渲染3D模型。在本章中，我们将添加另一个功能，即mipmap生成。 Mipmaps在游戏和渲染软件中广泛使用，Vulkan使我们可以完全控制它们的创建方式。

Mipmap是图像的预先计算的缩小版本。每个新图像的宽度和高度是前一个图像的一半。 Mipmap用作LOD(Level Of Detail)的一种形式。距离相机较远的物体将从较小的Mip图像中采样其纹理。使用较小的图像可提高渲染速度，并避免出现[摩尔纹](https://en.wikipedia.org/wiki/Moir%C3%A9_pattern)等瑕疵。 mipmap的示例如下：

![img](https://vulkan-tutorial.com/images/mipmaps_example.jpg)

## Image Creation

## 图形创建

在Vulkan中，每个Mip图像都存储在VkImage的不同Mip级别中。 Mip级别0是原始图像，级别0之后的Mip级别通常称为Mip链。

创建VkImage时指定了Mip级别数。到目前为止，我们始终将此值设置为1。我们需要根据图像的尺寸计算出Mip级别的数量。首先，添加一个类成员以存储此数字：

```c++
...
uint32_t mipLevels;
VkImage textureImage;
...
```

将纹理加载到createTextureImage中后，即可找到mipLevels的值：

```c++
int texWidth, texHeight, texChannels;
stbi_uc* pixels = stbi_load(TEXTURE_PATH.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
...
mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;

```

这将计算Mip链中的级别数。 max函数选择最大尺寸。 log2函数计算该尺寸可以除以2的次数。floor函数处理最大尺寸不是2的幂的情况。添加1以便原始图像具有mip级别。

要使用此值，我们需要更改createImage，createImageView和transitionImageLayout函数，以允许我们指定Mip级别的数量。向函数添加一个mipLevels参数：

```c++
void createImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory) {
    ...
    imageInfo.mipLevels = mipLevels;
    ...
}
```

```c++
VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels) {
    ...
    viewInfo.subresourceRange.levelCount = mipLevels;
    ...
```

```c++
void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels) {
    ...
    barrier.subresourceRange.levelCount = mipLevels;
    ...
```

更新对这些函数的所有调用以使用正确的值：

```c++
createImage(swapChainExtent.width, swapChainExtent.height, 1, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depthImage, depthImageMemory);
...
createImage(texWidth, texHeight, mipLevels, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage, textureImageMemory);
```

```c++
swapChainImageViews[i] = createImageView(swapChainImages[i], swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
...
depthImageView = createImageView(depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, 1);
...
textureImageView = createImageView(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, mipLevels);
```

```c++
transitionImageLayout(depthImage, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 1);
...
transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mipLevels);
```

## Generating Mipmaps

## 生成Mipmaps

现在，我们的纹理图像具有多个Mip级别，但是暂存缓冲区只能用于填充Mip级别0。其他级别仍未定义。要填充这些级别，我们需要从我们拥有的单个级别生成数据。我们将使用vkCmdBlitImage命令。此命令执行复制，缩放和过滤操作。我们将多次调用以将数据混合到纹理图像的每个级别。

VkCmdBlit被认为是传输操作，因此我们必须通知Vulkan我们打算将纹理图像用作传输的源和目标。将VK_IMAGE_USAGE_TRANSFER_SRC_BIT添加到createTextureImage中纹理图像的使用标志中：

```c++
...
createImage(texWidth, texHeight, mipLevels, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage, textureImageMemory);
...
```

像其他图像操作一样，vkCmdBlitImage取决于其所操作图像的布局。我们可以将整个图像过渡到VK_IMAGE_LAYOUT_GENERAL，但这很可能会很慢。为了获得最佳性能，源图像应该是VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL，而目标图像应该是VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL。 Vulkan允许我们独立转换图像的每个Mip级别。每个blit一次只能处理两个mip级别，因此我们可以将每个级别转换为blits命令之间的最佳布局。

transitionImageLayout仅在整个图像上执行布局转换，因此我们需要再编写一些管线屏障命令。在createTextureImage中删除到VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL的现有过渡：

```c++
...
transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mipLevels);
    copyBufferToImage(stagingBuffer, textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
//transitioned to VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL while generating mipmaps
...
```

这会将纹理图像的每个级别保留在VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL中。从blit命令读取完成后，每个级别将转换为VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL。

现在，我们将编写生成mipmap的函数：

```
void generateMipmaps(VkImage image, int32_t texWidth, int32_t texHeight, uint32_t mipLevels) {
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();
    
    VkImageMemoryBarrier barrier = {};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.image = image;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.subresourceRange.levelCount = 1;
    
    endSingleTimeCommands(commandBuffer);
}
```

我们将进行几次转换，因此我们将重用此VkImageMemoryBarrier。上面设置的字段将对所有障碍保持不变。对于每个转换，subresourceRange.miplevel，oldLayout，newLayout，srcAccessMask和dstAccessMask都会更改。

```c++
int32_t mipWidth = texWidth;
int32_t mipHeight = texHeight;

for (uint32_t i = 1; i < mipLevels; i++) {

}
```

此循环将记录每个VkCmdBlitImage命令。请注意，循环变量从1开始，而不是0。

```c++
barrier.subresourceRange.baseMipLevel = i - 1;
barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

vkCmdPipelineBarrier(commandBuffer,
    VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
    0, nullptr,
    0, nullptr,
    1, &barrier);
```

首先，我们将级别i-1转换为VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL。此过渡将等待从上一个blit命令或从vkCmdCopyBufferToImage填充级别i-1。当前的blit命令将等待此过渡。

```c++
VkImageBlit blit = {};
blit.srcOffsets[0] = { 0, 0, 0 };
blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
blit.srcSubresource.mipLevel = i - 1;
blit.srcSubresource.baseArrayLayer = 0;
blit.srcSubresource.layerCount = 1;
blit.dstOffsets[0] = { 0, 0, 0 };
blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
blit.dstSubresource.mipLevel = i;
blit.dstSubresource.baseArrayLayer = 0;
blit.dstSubresource.layerCount = 1;
```

接下来，我们指定将在blit操作中使用的区域。源Mip级别为i-1，目标Mip级别为i。 srcOffsets数组的两个元素确定了将对数据进行blit的3D区域。 dstOffsets确定将数据拖入的区域。 dstOffsets [1]的X和Y尺寸被二除，因为每个Mip级别是前一个级别的一半。 srcOffsets [1]和dstOffsets [1]的Z尺寸必须为1，因为2D图像的深度为1。

```c++
vkCmdBlitImage(commandBuffer,
    image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
    image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
    1, &blit,
    VK_FILTER_LINEAR);
```

现在，我们记录blit命令。请注意，srcImage和dstImage参数均使用textureImage。这是因为我们在同一张图片的不同级别之间blitting。源Mip级别刚刚过渡到VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL，而目标级别仍从保留在来自createTextureImage的VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL中。

最后一个参数允许我们指定要在blit中使用的VkFilter。我们这里有与制作VkSampler时相同的过滤选项。我们使用VK_FILTER_LINEAR启用插值。

```c++
barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

vkCmdPipelineBarrier(commandBuffer,
    VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
    0, nullptr,
    0, nullptr,
    1, &barrier);
```

此屏障将mip级别从i-1转换为VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL。此过渡等待当前的blit命令完成。所有采样操作将等待此转换完成。

```c++
    ...
    if (mipWidth > 1) mipWidth /= 2;
    if (mipHeight > 1) mipHeight /= 2;
```

在循环的最后，我们将当前的Mip尺寸除以2。我们在除法之前检查每个尺寸，以确保尺寸永远不会为0。这可以处理图像不是正方形的情况，因为一个Mip尺寸会比另一个尺寸先变为1。发生这种情况时，所有剩余级别的该维度应保持为1。.

```c++
  barrier.subresourceRange.baseMipLevel = mipLevels - 1;
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    vkCmdPipelineBarrier(commandBuffer,
        VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
        0, nullptr,
        0, nullptr,
        1, &barrier);

    endSingleTimeCommands(commandBuffer);
}
```

在结束命令缓冲区之前，我们再插入一个管线屏障。此屏障会将最后一个Mip级别从VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL转换为VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL。这不是由循环处理的，因为最后一个mip级别永远不会被blit。

最后，在createTextureImage中添加对generateMipmaps的调用：

```c++
transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mipLevels);
    copyBufferToImage(stagingBuffer, textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
//transitioned to VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL while generating mipmaps
...
generateMipmaps(textureImage, texWidth, texHeight, mipLevels);
```

## Linear filtering support

## 线性过滤支持

使用vkCmdBlitImage之类的内置函数来生成所有Mip级别非常方便，但是遗憾的是，不能保证所有平台都支持该功能。它需要我们用于支持线性过滤的纹理图像格式，可以使用vkGetPhysicalDeviceFormatProperties函数进行检查。我们将为此添加一个检查到generateMipmaps函数。

首先添加一个附加参数来指定图像格式：

```c++
void createTextureImage() {
    ...

    generateMipmaps(textureImage, VK_FORMAT_R8G8B8A8_SRGB, texWidth, texHeight, mipLevels);
}

void generateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels) {

    ...
}
```

在generateMipmaps函数中，使用vkGetPhysicalDeviceFormatProperties请求纹理图像格式的属性：

```c++
void generateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels) {

    // Check if image format supports linear blitting
    VkFormatProperties formatProperties;
    vkGetPhysicalDeviceFormatProperties(physicalDevice, imageFormat, &formatProperties);

    ...
```

VkFormatProperties结构包含三个名为linearTilingFeatures，optimumTilingFeatures和bufferFeatures的字段，每个字段描述了如何使用格式，具体取决于使用方式。我们创建具有最佳平铺格式的纹理图像，因此我们需要检查optimalTilingFeatures。可以使用VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT来检查是否支持线性过滤功能：

```c++
if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
    throw std::runtime_error("texture image format does not support linear blitting!");
}
```

在这种情况下，有两种选择。您可以实现一个函数，该函数可以搜索常见的纹理图像格式以获取支持线性blit的图像，也可以用stb_image_resize之类库实现软件mipmap生成。然后可以按照与加载原始图像相同的方式将每个Mip级别加载到图像中。

应该注意的是，实际上无论如何都不会在运行时生成Mipmap级别。通常，它们是预先生成的，并与基本级别一起存储在纹理文件中，以提高加载速度。留给读者的练习是在软件中实现调整大小并从文件加载多个级别。

## Sampler

## 采样器

VkImage保留mipmap数据时，VkSampler控制渲染时如何读取该数据。 Vulkan允许我们指定minLod，maxLod，mipLodBias和mipmapMode（“ Lod”表示“Level Of Detail”）。对纹理进行采样时，采样器根据以下伪代码选择一个mip级别：

```c++
lod = getLodLevelFromScreenSize(); //smaller when the object is close, may be negative
lod = clamp(lod + mipLodBias, minLod, maxLod);

level = clamp(floor(lod), 0, texture.mipLevels - 1);  //clamped to the number of mip levels in the texture

if (mipmapMode == VK_SAMPLER_MIPMAP_MODE_NEAREST) {
    color = sample(level);
} else {
    color = blend(sample(level), sample(le
```

如果samplerInfo.mipmapMode为VK_SAMPLER_MIPMAP_MODE_NEAREST，则lod选择要从中采样的Mip级别。如果mipmap模式为VK_SAMPLER_MIPMAP_MODE_LINEAR，则将使用lod选择要采样的两个mip级别。对这些级别进行采样，然后将结果线性混合。

采样操作也受lod影响：

```c++
if (lod <= 0) {
    color = readTexture(uv, magFilter);
} else {
    color = readTexture(uv, minFilter);
}
```

如果物体靠近相机，则将magFilter用作滤镜。如果对象距离相机较远，则使用minFilter。通常，lod为非负数，关闭相机时仅为0。 mipLodBias让我们强迫Vulkan使用比平时更低的lod和水平。

要查看本章的结果，我们需要为textureSampler选择值。我们已经将minFilter和magFilter设置为使用VK_FILTER_LINEAR。我们只需要为minLod，maxLod，mipLodBias和mipmapMode选择值。

```c++
void createTextureSampler() {
    ...
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.minLod = 0; // Optional
    samplerInfo.maxLod = static_cast<float>(mipLevels);
    samplerInfo.mipLodBias = 0; // Optional
    ...
}
```

为了允许使用所有范围的Mip级别，我们将minLod设置为0，将maxLod设置为mip级别数。我们没有理由更改lod值，因此我们将mipLodBias设置为0。

现在运行程序，您应该看到以下内容：

![img](https://vulkan-tutorial.com/images/mipmaps.png)

因为我们的场景是如此简单，所以这并没有太大的区别。如果仔细观察，会有细微的差别。

![img](https://vulkan-tutorial.com/images/mipmaps_comparison.png)

最明显的区别是标志上的文字。使用mipmaps可以使文字变得流畅。如果没有mipmap，则文字因为摩尔纹产生粗糙的边缘和缝隙。

您可以试玩采样器设置，以了解它们如何影响mipmapping。例如，通过更改minLod，可以强制采样器不使用最低的Mip级别：

```c++
samplerInfo.minLod = static_cast<float>(mipLevels / 2);
```

这些设置将产生此图像：

![img](https://vulkan-tutorial.com/images/highmipmaps.png)

当物体距离摄像机较远时，将使用更高的mip级别。