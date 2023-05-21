#include "LoadModelObj.h"

#include <unordered_set>

#include <cassert>
#include <cstring>

#include <rapidobj/rapidobj.hpp>

#include "labutils/error.hpp"
#include "SimpleModel.h"

namespace lut = labutils;

SimpleModel loadSimpleWavefrontObj(char const* aPath)
{
	assert(aPath);

	// Ask rapidobj to load the requested file
	auto result = rapidobj::ParseFile(aPath);
	if (result.error)
		throw lut::Error("Unable to load OBJ file '%s': %s", aPath, result.error.code.message().c_str());

	// OBJ files can define faces that are not triangles. However, Vulkan will
	// only render triangles (or lines and points), so we must triangulate any
	// faces that are not already triangles. Fortunately, rapidobj can do this
	// for us.
	rapidobj::Triangulate(result);

	// Find the path to the OBJ file
	char const* pathBeg = aPath;
	char const* pathEnd = std::strrchr(pathBeg, '/');

	std::string const prefix = pathEnd
		? std::string(pathBeg, pathEnd + 1)
		: ""
		;

	// Convert the OBJ data into a SimpleModel structure.
	// First, extract material data.
	SimpleModel ret;

	ret.modelSourcePath = aPath;

	for (auto const& mat : result.materials)
	{
		SimpleMaterialInfo mi;

		mi.materialName = mat.name;
		mi.diffuseColor = glm::vec3(mat.diffuse[0], mat.diffuse[1], mat.diffuse[2]);
		mi.roughness = mat.specular[0];

		if (!mat.diffuse_texname.empty())
			mi.diffuseTexturePath = prefix + mat.diffuse_texname;

		if (!mat.normal_texname.empty())
			mi.normalTexturePath = prefix + mat.normal_texname;

		if (!mat.bump_texname.empty())
			mi.normalTexturePath = prefix + mat.bump_texname;

		if (!mat.roughness_texname.empty())
			mi.roughnessTexturePath = prefix + mat.roughness_texname;

		if (!mat.metallic_texname.empty())
			mi.metallicTexturePath = prefix + mat.metallic_texname;

		if (!mat.alpha_texname.empty())
			mi.alphaTexturePath = prefix + mat.alpha_texname;

		ret.materials.emplace_back(std::move(mi));
	}

	// Next, extract the actual mesh data. There are some complications:
	// - OBJ use separate indices to positions, normals and texture coords. To
	//   deal with this, the mesh is turned into an unindexed triangle soup.
	// - OBJ uses three methods of grouping faces:
	//   - 'o' = object
	//   - 'g' = group
	//   - 'usemtl' = switch materials
	//  The first two create logical objects/groups. The latter switches
	//  materials. We want to primarily group faces by material (and possibly
	//  secondarily by other logical groupings). 
	//
	// Unfortunately, RapidOBJ exposes a per-face material index.

	std::unordered_set<std::size_t> activeMaterials;
	std::unordered_map<Vertex, uint32_t> globalUniqueVertices;

	for (auto const& shape : result.shapes)
	{
		auto const& shapeName = shape.name;

		// Scan shape for materials
		activeMaterials.clear();

		for (std::size_t i = 0; i < shape.mesh.indices.size(); ++i)
		{
			auto const faceId = i / 3; // Always triangles; see Triangulate() above

			assert(faceId < shape.mesh.material_ids.size());
			auto const matId = shape.mesh.material_ids[faceId];

			assert(matId < int(ret.materials.size()));
			activeMaterials.emplace(matId);
		}

		// Process vertices for active material
		// This does multiple passes over the vertex data, which is less than
		// optimal...
		//
		// Note: we still keep different "shapes" separate. For static meshes,
		// one could merge all vertices with the same material for a bit more
		// efficient rendering.
		for (auto const matId : activeMaterials)
		{
			auto* opos = &ret.dataTextured.positions;
			auto* otex = &ret.dataTextured.texcoords;
			auto* onormals = &ret.dataTextured.normals;

			bool const textured = !ret.materials[matId].diffuseTexturePath.empty();
			bool const alphaTextured = !ret.materials[matId].alphaTexturePath.empty();
			bool const normalTextured = !ret.materials[matId].normalTexturePath.empty();
			bool const roughnessTextured = !ret.materials[matId].roughnessTexturePath.empty();
			bool const metallicTextured = !ret.materials[matId].metallicTexturePath.empty();

			if (!textured)
			{
				opos = &ret.dataUntextured.positions;
				onormals = &ret.dataUntextured.normals;
				otex = nullptr;
			}

			// Keep track of mesh names; this can be useful for debugging.
			std::string meshName;
			if (1 == activeMaterials.size())
				meshName = shapeName;
			else
				meshName = shapeName + "::" + ret.materials[matId].materialName;

			// Extract this material's vertices.
			auto const firstVertex = opos->size();
			size_t firstIndex = ret.indices.size();
			assert(!textured || firstVertex == otex->size());

			std::vector<Vertex> vertices;
			std::vector<uint32_t> indices;
			std::unordered_map<Vertex, uint32_t> uniqueVertices;

			for (std::size_t i = 0; i < shape.mesh.indices.size(); ++i)
			{
				auto const faceId = i / 3; // Always triangles; see Triangulate() above
				auto const faceMat = std::size_t(shape.mesh.material_ids[faceId]);

				if (faceMat != matId)
					continue;

				auto const& idx = shape.mesh.indices[i];

				Vertex vertex;

				auto x = result.attributes.positions[idx.position_index * 3 + 0];
				auto y = result.attributes.positions[idx.position_index * 3 + 1];
				auto z = result.attributes.positions[idx.position_index * 3 + 2];

				opos->emplace_back(glm::vec3{ x, y, z });

				vertex.position = { x, y, z };

				auto nx = result.attributes.normals[idx.normal_index * 3 + 0];
				auto ny = result.attributes.normals[idx.normal_index * 3 + 1];
				auto nz = result.attributes.normals[idx.normal_index * 3 + 2];

				onormals->emplace_back(glm::vec3{ nx, ny, nz });

				vertex.normal = { nx, ny, nz };

				if (textured)
				{
					auto tx = result.attributes.texcoords[idx.texcoord_index * 2 + 0];
					auto ty = result.attributes.texcoords[idx.texcoord_index * 2 + 1];

					otex->emplace_back(glm::vec2{ tx, ty });

					vertex.texcoord = { tx, ty };
				}

				if (uniqueVertices.count(vertex) == 0)
				{
					uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
					vertices.emplace_back(vertex);
				}

				if (globalUniqueVertices.count(vertex) == 0)
				{
					globalUniqueVertices[vertex] = static_cast<uint32_t>(ret.vertices.size());
					ret.vertices.emplace_back(vertex);
				}

				ret.indices.emplace_back(globalUniqueVertices[vertex]);
				indices.emplace_back(globalUniqueVertices[vertex]);
			}

			SimpleMeshInfo meshInfo;

			meshInfo.meshName = std::move(meshName);
			meshInfo.materialIndex = matId;
			meshInfo.textured = textured;
			meshInfo.alphaTextured = alphaTextured;
			meshInfo.normalTextured = normalTextured;
			meshInfo.roughnessTextured = roughnessTextured;
			meshInfo.metallicTextured = metallicTextured;

			auto const vertexCount = vertices.size();

			auto const indexCount = ret.indices.size() - firstIndex;

			meshInfo.vertexCount = vertexCount;
			meshInfo.indexStartIndex = firstIndex;
			meshInfo.indexCount = indexCount;
			meshInfo.vertices = vertices;
			meshInfo.indices = indices;

			ret.indexCount += indexCount;

			ret.meshes.emplace_back(meshInfo);
		}
	}

	return ret;
}

