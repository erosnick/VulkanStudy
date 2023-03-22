#pragma once

#include <string>
#include <vector>
#include <algorithm>

#include <cstdint>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include "Vertex.h"

// A simple material with a diffuse color and an (optional) diffuse texture.
//
// If the material does not define a diffuse texture, the `diffuseTexturePath`
// string is empty (std::string::empty()).
//
// If the material defines a diffuse texture, you will have to load it with
// e.g., the stbi_load() function from <stb_image.h>.
struct SimpleMaterialInfo
{
	std::string materialName;  // This is purely informational and for debugging

	glm::vec3 diffuseColor;
	std::string diffuseTexturePath;
	std::string alphaTexturePath;

	uint32_t diffuseTextureIndex = 0;
	uint32_t alphaTextureIndex = 0;

	float metallic = 0.5f;
	float roughness = 0.5f;
};

// A simple mesh
//
// A mesh links a set of vertices with a specific material. 
//
// The material of the mesh is identified by the `materialIndex` member. It is
// an index into the `SimpleModel::materials` vector.
//
// The vertices belonging to the mesh are identified by the `vertexStartIndex`
// and `vertexCount` members. The mesh contains `vertexCount` members, starting
// at index `vertexStartIndex`. For textured meshes (`textured` set to `true`),
// the vertices are found in the `SimpleModel::dataTextured::positions` and
// `SimpleModel::dataTextured::texcoords` arrays. For untextured meshes
// (`textured` set to `false`), the vertices are instead found in the
// `SimpleModel::dataUntextured::positions` array (and do not have any texture
// coordinates).
struct SimpleMeshInfo
{
	std::string meshName;  // This is purely informational and for debugging

	std::size_t materialIndex;

	bool textured : 1;
	bool alphaTextured : 1;

	std::size_t vertexStartIndex = 0;
	std::size_t vertexCount = 0;
	std::size_t indexStartIndex = 0;
	std::size_t indexCount = 0;

	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

	glm::mat4 transform = glm::mat4(1.0f);
};

// Simple model.
//
// Note: you probably want to use this for loading only. Once you have copied
// the mesh data into Vulkan buffers, you are unlikely to need it any longer.
struct SimpleModel
{
	std::string modelSourcePath;

	std::vector<SimpleMaterialInfo> materials;
	std::vector<SimpleMeshInfo> meshes;

	void setTransform(const glm::mat4& transform)
	{
		std::transform(meshes.begin(), meshes.end(), meshes.begin(), [=](SimpleMeshInfo mesh) { mesh.transform = transform; return mesh; });
	}

	struct Data_
	{
		std::vector<glm::vec3> positions;
		std::vector<glm::vec3> normals;
		std::vector<glm::vec2> texcoords;
	} dataTextured;

	struct Data2_
	{
		std::vector<glm::vec3> positions;
		std::vector<glm::vec3> normals;
	} dataUntextured;

	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

	std::size_t indexCount = 0;
};

