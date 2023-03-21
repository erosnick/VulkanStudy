#pragma once

#include <string>
#include <vector>
#include <memory>

#include "Vertex.h"

struct Material
{
	bool hasTexture() { return !diffuseTexturePath.empty(); }
	bool hasAlphaTexture() { return !alphaTexturePath.empty(); }

	glm::vec3 Kd;
	glm::vec3 Ka;
	glm::vec3 Ks;
	glm::vec3 Ke;
	float shininess;
	float reflectionFactor;
	float refractionFactor;
	// relative index of refraction(n1/n2)
	float ior;
	float eta;
	float metallic;
	float roughness;
	bool hasNormalMap = false;
	uint32_t diffuseTextureIndex = 0;
	uint32_t alphaTextureIndex = 0;
	std::string diffuseTexturePath;
	std::string alphaTexturePath;
};

struct Mesh
{
	void addVertex(const Vertex& vertex) { vertices.emplace_back(vertex); }
	void addIndex(uint32_t index) { indices.emplace_back(index); }

	const std::vector<Vertex>& getVertices() const { return vertices; }
	const std::vector<uint32_t>& getIndices() const { return indices; }

	const std::shared_ptr<Material>& getMaterial() const { return material; }

	void setMaterial(const std::shared_ptr<Material>& newMaterial)
	{
		material = newMaterial;
	}

	void computeNormals();

	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

	std::shared_ptr<Material> material;
};

class Model
{
public:
	bool load(const std::string& path);

	void draw() const;

	std::vector<Mesh> meshes;

	glm::vec3 position{ 0.0f };
	glm::vec3 scale{ 1.0f };
	glm::vec3 rotation{ 0.0f };
	glm::vec4 color{ 1.0f };
};