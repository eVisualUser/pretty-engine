#pragma once	

#include <PrettyEngine/mesh.hpp>
#include <PrettyEngine/texture.hpp>
#include <PrettyEngine/transform.hpp>
#include <PrettyEngine/gl.hpp>
#include <PrettyEngine/text.hpp>
#include <PrettyEngine/camera.hpp>

#include <string>

#define GET_RENDERER(ptr) (PrettyEngine::Renderer*)ptr

namespace PrettyEngine {
	class RenderModel {
	public:
		void SetMesh(Mesh* newMesh) {
			this->mesh = newMesh;
			this->mesh->userCount++;
		}
		
		void SetShaderProgram(GLShaderProgramRefs* newShaderProgram) {
			this->shaderProgram = newShaderProgram;
		}

	public:
		GLShaderProgramRefs* shaderProgram;
		Mesh* mesh;
		DrawMode drawMode = DrawMode::Triangles;
		Projection* projection = nullptr;
		bool useTexture = false;
		bool overrideProjection = false;
	};
	
	class VisualObject: virtual public Transform {
	public:
		~VisualObject() {
			for (auto & texture: this->textures) {
				texture->userCount--;
			}
		}

		void AddTexture(Texture* texture) {
			texture->userCount++;
			this->textures.push_back(texture);
		}

		void AddRenderModel(RenderModel* newRenderModel) {
			this->renderModel = newRenderModel;
		}

		void RemoveRenderModel() {
			this->renderModel = nullptr;
		}

		void RemoveTexture(std::string name) {
			unsigned int index = 0;
			for (auto & texture: this->textures) {
				if (texture->name == name && texture->useGC) {
					texture->userCount--;
					this->textures.erase(this->textures.begin() + index);
				}
				index++;
			}
		}

		void RemoveTexture(Texture* reference) {
			unsigned int index = 0;
			for (auto & texture: this->textures) {
				if (texture->name == reference->name) {
					texture->userCount--;
					this->textures.erase(this->textures.begin() + index);
				}
				index++;
			}
		}

		virtual void OnDraw(void* renderer) {}
		virtual void OnRendererRegister(void* renderer) {}
		virtual void OnRendererUnRegister(void* renderer) {}

		void Toggle3D() {
			this->d3 = !this->d3;
		}

	public:
		bool render = true;
		bool active = true;
		bool d3 = false;

		glm::vec3 baseColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);

		unsigned int renderLayer = 0;

		RenderModel* renderModel;

		VisualObject* parent;
		bool haveParent = false;

		std::vector<Texture*> textures;
		float opacity = 1.0f;

		bool renderText = false;
		Text* text;
		float lineJump = 0.1f;
		float textSize = 0.5f;

		bool allowRenderCube = true;

		int lightLayer = 0;

		bool sunLight = false;

		bool useLight = true;
	};
}
