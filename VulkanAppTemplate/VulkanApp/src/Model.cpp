#include "Model.h"

#include <iostream>
#include <unordered_map>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

bool Model::load(const std::string& path)
{
	std::string inputfile = path;
	tinyobj::ObjReaderConfig reader_config;

	tinyobj::ObjReader reader;

	if (!reader.ParseFromFile(inputfile, reader_config)) 
	{
		if (!reader.Error().empty()) 
		{
			std::cerr << "TinyObjReader: " << reader.Error();
		}
		return false;
	}

	if (!reader.Warning().empty()) 
	{
		std::cout << "TinyObjReader: " << reader.Warning();
	}

	auto& attrib = reader.GetAttrib();
	auto& shapes = reader.GetShapes();
	auto& objMaterials = reader.GetMaterials();

	size_t materialIndex = 0;

	std::unordered_map<Vertex, uint32_t> uniqueVertices;

	bool hasNormal = false;

	for (const auto& shape : shapes) 
	{
		Mesh mesh;
		for (const auto& index : shape.mesh.indices) 
		{
			Vertex vertex = {};

			vertex.position = 
			{
				attrib.vertices[3 * index.vertex_index + 0],
				attrib.vertices[3 * index.vertex_index + 1],
				attrib.vertices[3 * index.vertex_index + 2]
			};

			// Check if 'normal_index' is zero of positive. negative = no normal data
			if (index.normal_index >= 0) 
			{
				tinyobj::real_t nx = attrib.normals[3 * size_t(index.normal_index) + 0];
				tinyobj::real_t ny = attrib.normals[3 * size_t(index.normal_index) + 1];
				tinyobj::real_t nz = attrib.normals[3 * size_t(index.normal_index) + 2];
				vertex.normal = { nx, ny, nz };
				hasNormal = true;
			}

			if (index.texcoord_index >= 0)
			{
				vertex.texcoord = 
				{
					attrib.texcoords[2 * index.texcoord_index + 0],
					attrib.texcoords[2 * index.texcoord_index + 1]
				};
			}

			if (uniqueVertices.count(vertex) == 0) 
			{
				uniqueVertices[vertex] = static_cast<uint32_t>(mesh.getVertices().size());
				mesh.addVertex(vertex);
			}

			mesh.addIndex(uniqueVertices[vertex]);
		}

		if (!hasNormal)
		{
			mesh.computeNormals();
		}

		const auto& material = objMaterials[shape.mesh.material_ids[0]];

		auto meshMaterial = std::make_shared<Material>();

		meshMaterial->Ka = { material.ambient[0], material.ambient[1], material.ambient[2] };
		meshMaterial->Kd = { material.diffuse[0], material.diffuse[1], material.diffuse[2] };
		meshMaterial->Ke = { material.emission[0], material.emission[2], material.emission[2] };
		meshMaterial->Ks = { material.specular[0], material.specular[1], material.specular[2] };

		meshMaterial->shininess = material.shininess;
		meshMaterial->ior = material.ior;
		meshMaterial->eta = 1.0f / meshMaterial->ior;

		if (!material.diffuse_texname.empty())
		{
			meshMaterial->diffuseTexturePath = material.diffuse_texname;
		}

		mesh.setMaterial(meshMaterial);

		materialIndex++;

		meshes.emplace_back(mesh);
	}

	return true;
}

void Model::draw() const
{
}

void Mesh::computeNormals()
{
	for (auto i = 0; i < indices.size(); i += 3)
	{
		auto& v0 = vertices[indices[i]];
		auto& v1 = vertices[indices[i + 1]];
		auto& v2 = vertices[indices[i + 2]];

		auto e01 = v1.position - v0.position;
		auto e02 = v2.position - v0.position;

		auto normal = glm::normalize(glm::cross(e01, e02));

		v0.normal = normal;
		v1.normal = normal;
		v2.normal = normal;
	}
}
