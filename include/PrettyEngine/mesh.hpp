#pragma once

#include <PrettyEngine/debug.hpp>
#include <PrettyEngine/gc.hpp>

#include <glm/glm.hpp>
#include <glad/glad.h>

#include <vector>

namespace PrettyEngine {
	enum class MeshDrawType {
		Dynamic = GL_DYNAMIC_DRAW,
		Static = GL_STATIC_DRAW,
		Stream = GL_STREAM_DRAW,
	};

	struct Vertex {
	public:
		glm::vec3 position;
		glm::vec4 color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
		glm::vec2 textureCoord;
	};

	class Mesh: public GCObject {
	public:
		/// Convert to an array buffer
		std::vector<float> CreateBuffer() {
			std::vector<float> data;

			for (auto & vertex: vertices) {
				// Position
				data.push_back(vertex.position.x);
				data.push_back(vertex.position.y);
				data.push_back(vertex.position.z);

				// Color
				data.push_back(vertex.color.r);
				data.push_back(vertex.color.g);
				data.push_back(vertex.color.b);

				// Texture Coord (UV)
				data.push_back(vertex.textureCoord.x);
				data.push_back(vertex.textureCoord.y);
			}

			return data;
		}

		/// Call when you don't need anymore to load/update the mesh
		void ClearBuffer() {
			this->vertices.clear();
		}

		void Cleanup() {
		    glDeleteBuffers(1, &vbo);
		    glDeleteVertexArrays(1, &vao);
		    glDeleteBuffers(1, &ebo);
		}
	public:
		std::vector<Vertex> vertices;
		std::vector<unsigned int> indices;
		
		int vertexCount;
		
		MeshDrawType drawType = MeshDrawType::Static;

		unsigned int vao;
		unsigned int vbo;
		unsigned int ebo;

		std::string name;
	};

	static Mesh CreateRectMesh(float width = 1.0f, float height = 1.0f, glm::vec4 color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)) {
		auto out = Mesh();

		width = 0.5f * width;
		height = 0.5f * height;

		Vertex vA;
		vA.position = glm::vec3(-width, height, 0.0f),
		vA.color = color;
		vA.textureCoord = glm::vec2(0.0f, 0.0f);

		Vertex vB;
		vB.position = glm::vec3(width, height, 0.0f),
		vB.color = color;
		vB.textureCoord = glm::vec2(1.0f, 0.0f);

		Vertex vC;
		vC.position = glm::vec3(width, -height, 0.0f),
		vC.color = color;
		vC.textureCoord = glm::vec2(1.0f, 1.0f);

		Vertex vD;
		vD.position = glm::vec3(-width, -height, 0.0f),
		vD.color = color;
		vD.textureCoord = glm::vec2(0.0f, 1.0f);

		out.vertices.push_back(vA);
		out.vertices.push_back(vB);
		out.vertices.push_back(vC);
		out.vertices.push_back(vD);

		out.indices.push_back(0);
		out.indices.push_back(1);
		out.indices.push_back(2);
		out.indices.push_back(2);
		out.indices.push_back(3);
		out.indices.push_back(0);

		out.vertexCount = 6;

		return out;
	} 
}
