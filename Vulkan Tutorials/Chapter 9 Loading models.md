# Chapter 9 Loading models

# 加载模型

## Introduction

## 简介

现在，您的程序已准备好渲染带纹理的3D网格，但是顶点和索引数组中的当前几何体还不是很有趣。在本章中，我们将扩展程序以从实际模型文件中加载顶点和索引，以使图形卡实际完成某些工作。

许多图形API教程都要求读者在这样的章节中编写自己的OBJ加载器。这样做的问题是，任何更具吸引力的3D应用程序都将很快需要此文件格式不支持的功能，例如骨骼动画。在本章中，我们将从OBJ模型中加载网格数据，但我们将更多地关注于将网格数据与程序本身集成在一起，而不是从文件中加载网格数据的细节。

## Library

## 库

我们将使用[tinyobjloader](https://github.com/syoyo/tinyobjloader)库从OBJ文件加载顶点和面。它快速且易于集成，因为它是单个文件库，如stb_image。转到上面链接的存储库，然后将tiny_obj_loader.h文件下载到库目录中的文件夹中。确保使用master分支中的文件版本，因为最新的正式版本已过时。

**Visual Studio**

将目录中包含tiny_obj_loader.h的目录添加到“其他包含目录”路径。

![img](https://vulkan-tutorial.com/images/include_dirs_tinyobjloader.png)

**Makefile**

将目录tiny_obj_loader.h添加到GCC的包含目录中：

```makefile
VULKAN_SDK_PATH = /home/user/VulkanSDK/x.x.x.x/x86_64
STB_INCLUDE_PATH = /home/user/libraries/stb
TINYOBJ_INCLUDE_PATH = /home/user/libraries/tinyobjloader

...

CFLAGS = -std=c++17 -I$(VULKAN_SDK_PATH)/include -I$(STB_INCLUDE_PATH) -I$(TINYOBJ_INCLUDE_PATH)
```

## Sample mesh

## 示例网格

在本章中，我们还没有启用光照，因此使用将光照烘焙到纹理中的示例模型会有所帮助。查找此类模型的简单方法是在Sketchfab上查找3D扫描。该站点上的许多模型都以OBJ格式提供，并带有宽容许可。

对于本教程，我决定使用Escadrone的[Chalet Hippolyte Chassande Baroz](https://skfb.ly/HDVU)模型。我调整了模型的大小和方向，以将其用作替换当前几何体的方法：

- [chalet.obj](https://vulkan-tutorial.com/resources/chalet.obj.zip)
- [chalet.jpg](https://vulkan-tutorial.com/resources/chalet.jpg)

它有50万个三角形，因此它是我们应用程序的不错基准。可以随意使用自己的模型，但请确保它仅由一种材质组成，并且其尺寸约为1.5 x 1.5 x 1.5单位。如果大于该值，则必须更改视图矩阵。将模型文件放在着色器和纹理旁边的新models目录中，然后将纹理图像放在textures目录中。

在程序中添加两个新的配置变量以定义模型和纹理路径：

```c++
const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

const std::string MODEL_PATH = "models/chalet.obj";
const std::string TEXTURE_PATH = "textures/chalet.jpg";
```

并更新createTextureImage以使用此路径变量：

```c++
stbi_uc* pixels = stbi_load(TEXTURE_PATH.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
```

## Loading vertices and indices

## 加载顶点和索引

现在，我们将从模型文件中加载顶点和索引，因此您现在应该删除全局顶点和索引数组。将它们替换为类的非const容器：

```c++
std::vector<Vertex> vertices;
std::vector<uint32_t> indices;
VkBuffer vertexBuffer;
VkDeviceMemory vertexBufferMemory;
```

您应该将索引的类型从uint16_t更改为uint32_t，因为将会有比65535多得多的顶点。请记住还要更改vkCmdBindIndexBuffer参数：

```c++
vkCmdBindIndexBuffer(commandBuffers[i], indexBuffer, 0, VK_INDEX_TYPE_UINT32);
```

tinyobjloader库的包含方式与STB库相同。包括tiny_obj_loader.h文件，并确保在一个源文件中定义TINYOBJLOADER_IMPLEMENTATION以包括函数体并避免链接器错误：

```c++
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>
```

现在，我们将编写一个loadModel函数，该函数使用该库用来自网格的顶点数据填充顶点和索引容器。应该在创建顶点和索引缓冲区之前在某处调用它：

```c++c
void initVulkan() {
    ...
    loadModel();
    createVertexBuffer();
    createIndexBuffer();
    ...
}

...

void loadModel() {

}
```

通过调用tinyobj :: LoadObj函数将模型加载到库的数据结构中：

```c++
void loadModel() {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, MODEL_PATH.c_str())) {
        throw std::runtime_error(warn + err);
    }
}
```

OBJ文件由位置，法线，纹理坐标和面组成。面由任意数量的顶点组成，其中每个顶点通过索引指代位置，法线和/或纹理坐标。这使得不仅可以重用整个顶点，而且可以重用单个属性。

attrib容器在其attrib.vertices，attrib.normals和attrib.texcoords向量中保存所有位置，法线和纹理坐标。shape容器包含所有单独的对象及其面。每个面由一个顶点数组组成，每个顶点包含位置，法线和纹理坐标属性的索引。 OBJ模型还可以定义每个面的材质和纹理，但是我们将忽略它们。

err字符串包含错误，warn字符串包含加载文件时发生的警告，例如缺少材料定义。仅当LoadObj函数返回false时，加载才真正失败。如上所述，OBJ文件中的面实际上可以包含任意数量的顶点，而我们的应用程序只能渲染三角形。幸运的是，LoadObj具有一个可选参数，可自动对这些面进行三角剖分，默认情况下已启用。

我们将把文件中的所有面组合成一个模型，因此只需遍历所有形状即可：

```c++
for (const auto& shape : shapes) {

}
```

三角剖分功能已经确保每个面有三个顶点，因此我们现在可以直接在顶点上进行迭代并将它们直接转储到我们的顶点向量中：

```c++
for (const auto& shape : shapes) {
    for (const auto& index : shape.mesh.indices) {
        Vertex vertex = {};

        vertices.push_back(vertex);
        indices.push_back(indices.size());
    }
}
```

为简单起见，我们假设每个顶点目前都是唯一的，因此使用简单的自动增量索引。索引变量的类型为tinyobj :: index_t，其中包含vertex_index，normal_index和texcoord_index成员。我们需要使用这些索引在attrib数组中查找实际的顶点属性：

```c++
vertex.pos = {
    attrib.vertices[3 * index.vertex_index + 0],
    attrib.vertices[3 * index.vertex_index + 1],
    attrib.vertices[3 * index.vertex_index + 2]
};

vertex.texCoord = {
    attrib.texcoords[2 * index.texcoord_index + 0],
    attrib.texcoords[2 * index.texcoord_index + 1]
};

vertex.color = {1.0f, 1.0f, 1.0f};
```

不幸的是，attrib.vertices数组是一个float值数组，而不是glm :: vec3之类的东西，因此您需要将索引乘以3。类似地，每个条目有两个纹理坐标分量。偏移量0、1和2用于访问X，Y和Z分量，或在纹理坐标的情况下访问U和V分量。

现在运行启用优化的程序（例如，Visual Studio中的Release模式，以及GCC`的-O3编译器标志）。这是必要的，否则加载模型会非常缓慢。您应该看到类似以下的内容：

![img](https://vulkan-tutorial.com/images/inverted_texture_coordinates.png)

太好了，几何看上去正确，但是纹理又是怎么回事？ OBJ格式假设一个坐标系，其中垂直坐标0表示图像的底部，但是我们已将图像以从上到下的方向上载到Vulkan，其中0表示图像的顶部。通过翻转纹理坐标的垂直分量来解决此问题：

```c++
vertex.texCoord = {
    attrib.texcoords[2 * index.texcoord_index + 0],
    1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
};
```

当您再次运行程序时，现在应该看到正确的结果：

![img](https://vulkan-tutorial.com/images/drawing_model.png)

## Vertex deduplication

## 消除顶点冗余

不幸的是，我们还没有真正利用索引缓冲区。vertices容器包含许多重复的顶点数据，因为多个三角形中包含许多顶点。我们应该只保留唯一的顶点，并在出现它们时使用索引缓冲区重新使用它们。一种简单的实现方法是使用map或unordered_map跟踪唯一顶点和相应的索引：

```c++
#include <unordered_map>

...

std::unordered_map<Vertex, uint32_t> uniqueVertices = {};

for (const auto& shape : shapes) {
    for (const auto& index : shape.mesh.indices) {
        Vertex vertex = {};

        ...

        if (uniqueVertices.count(vertex) == 0) {
            uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
            vertices.push_back(vertex);
        }

        indices.push_back(uniqueVertices[vertex]);
    }
}
```

每次我们从OBJ文件读取一个顶点时，我们都会检查之前是否已经看到一个位置和纹理坐标完全相同的顶点。如果没有，我们将其添加到vertices并将其索引存储在uniqueVertices容器中。之后，我们将新顶点的索引添加到indices中。如果之前已经看到了完全相同的顶点，则可以在uniqueVertices中查找其索引，并将该索引存储在indices中。

该程序现在将无法编译，因为在哈希表中使用用户定义的类型（例如Vertex结构）作为键要求我们实现两个功能：相等性测试和哈希计算。通过重载Vertex结构中的==运算符，很容易实现前者：

```c++
bool operator==(const Vertex& other) const {
    return pos == other.pos && color == other.color && texCoord == other.texCoord;
}
```

通过为std :: hash <T>指定模板特化来实现Vertex的哈希函数。哈希函数是一个复杂的主题，但是[cppreference.com建议](http://en.cppreference.com/w/cpp/utility/hash)采用以下方法，将结构的字段结合起来以创建体面的质量哈希函数：

```c++
namespace std {
    template<> struct hash<Vertex> {
        size_t operator()(Vertex const& vertex) const {
            return ((hash<glm::vec3>()(vertex.pos) ^
                   (hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^
                   (hash<glm::vec2>()(vertex.texCoord) << 1);
        }
    };
}
```

这段代码应该放在Vertex结构之外。需要使用以下标头包含GLM类型的哈希函数：

```c++
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>
```

哈希函数在gtx文件夹中定义，这意味着从技术上来说，它仍是GLM的实验性扩展。因此，您需要定义GLM_ENABLE_EXPERIMENTAL才能使用它。这意味着该API将来可能会随着新版本的GLM进行更改，但实际上该API非常稳定。

现在，您应该能够成功编译并运行程序。如果检查vertices的大小，那么您会发现它已经从1,500,000缩小到265,645！这意味着每个顶点平均被6个三角形重用。这无疑为我们节省了大量GPU内存。

