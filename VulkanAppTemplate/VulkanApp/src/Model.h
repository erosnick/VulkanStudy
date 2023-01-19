#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string>
#include <vector>
#include <array>

#include "glm.h"

struct Vertex
{
	Vertex() = default;
	Vertex(float x, float y, float z, float nx, float ny, float nz, float tx, float ty, float r, float g, float b)
		: position{ x, y, z }, normal(nx, ny, nz), texcoord(tx, ty), color(r, g, b)
	{}

	Vertex(const glm::vec3& inPosition, const glm::vec3& inNormal, const glm::vec2& inTexcoord, const glm::vec3& inColor)
	: position(inPosition), normal(inNormal), texcoord(inTexcoord), color(inColor)
	{}

	bool operator==(const Vertex& other) const {
		return (other.position == position) &&
			(other.normal == normal) &&
			(other.texcoord == texcoord);
	}

	static VkVertexInputBindingDescription getBindingDescription()
	{
		// All of our per - vertex data is packed together in one array, so we're only going to have one binding. 
		// The binding parameter specifies the index of the/binding in the array of bindings. The stride parameter
		// specifies the number of bytes from one entry to the next, and the inputRate parameter can have one of the following values:
		// VK_VERTEX_INPUT_RATE_VERTEX: Move to the next data entry after each vertex
		// VK_VERTEX_INPUT_RATE_INSTANCE : Move to the next data entry after each instance
		VkVertexInputBindingDescription vertexInputBindingDescription{};
		vertexInputBindingDescription.binding = 0;
		vertexInputBindingDescription.stride = sizeof(Vertex);
		vertexInputBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return vertexInputBindingDescription;
	}

	static std::array<VkVertexInputAttributeDescription, 4> getAttributeDescriptions()
	{
		// The binding parameter tells Vulkan from which binding the per - vertex data comes.The location parameter references 
		// the location directive of the input in the vertex shader.The input in the vertex shader with location 0 is the position,
		// which has two 32 - bit float components.

		// The format parameter describes the type of data for the attribute. A bit confusingly, the formats are specified using the 
		// same enumeration as color formats.The following shader types and formats are commonly used together :

		// float : VK_FORMAT_R32_SFLOAT
		// vec2 : VK_FORMAT_R32G32_SFLOAT
		// vec3 : VK_FORMAT_R32G32B32_SFLOAT
		// vec4 : VK_FORMAT_R32G32B32A32_SFLOAT
		std::array<VkVertexInputAttributeDescription, 4> attributeDescriptions{};

		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(Vertex, position);

		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Vertex, normal);

		attributeDescriptions[2].binding = 0;
		attributeDescriptions[2].location = 2;
		attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[2].offset = offsetof(Vertex, texcoord);

		attributeDescriptions[3].binding = 0;
		attributeDescriptions[3].location = 3;
		attributeDescriptions[3].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[3].offset = offsetof(Vertex, color);

		return attributeDescriptions;
	}

	glm::vec3 position{ 0.0f, 0.0f, 0.0f };
	glm::vec3 normal{ 0.0f, 0.0f, 0.0f };
	glm::vec2 texcoord{ 0.0f, 0.0f };
	glm::vec3 color{ 1.0f, 1.0f, 1.0f };
};

namespace std {
	template<> struct hash<Vertex> {
		size_t operator()(Vertex const& vertex) const {
			return ((hash<glm::vec3>()(vertex.position)
				  ^ (hash<glm::vec3>()(vertex.normal) << 1)) >> 1)
				  ^ (hash<glm::vec2>()(vertex.texcoord) << 1);
		}
	};
}

struct Mesh
{
	void addVertex(const Vertex& vertex) { vertices.emplace_back(vertex); }
	void addIndex(uint32_t index) { indices.emplace_back(index); }

	const std::vector<Vertex>& getVertices() const { return vertices; }
	const std::vector<uint32_t>& getIndices() const { return indices; }

	void computeNormals();

	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
};

class Model
{
public:
	bool load(const std::string& path);

	void draw() const;

	Mesh mesh;

	glm::vec3 position{ 0.0f };
	glm::vec3 scale{ 1.0f };
	glm::vec3 rotation{ 0.0f };
	glm::vec4 color{ 1.0f };
};