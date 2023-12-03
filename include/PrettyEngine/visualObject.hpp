#ifndef H_VISUAL_OBJECT
#define H_VISUAL_OBJECT

#include <PrettyEngine/tags.hpp>
#include <PrettyEngine/mesh.hpp>
#include <PrettyEngine/texture.hpp>
#include <PrettyEngine/transform.hpp>
#include <PrettyEngine/PrettyGL.hpp>
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

	class VisualDataPack: Tagged {
	public:
		std::string name;
		void* content;
	};
	
	class VisualObject: virtual public Transform {
	public:
		VisualObject() {}
		
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

		bool HaveTexture(TextureType textureType) {
			for(auto & texture: this->textures) {
				if (texture->textureType == textureType) {
					return true;
				}
			}		
			return false;
		}

		Texture* GetTexture(TextureType textureType) {
			for(auto & texture: this->textures) {
				if (texture->textureType == textureType) {
					return texture;
				}
			}		
			return nullptr;
		}

		virtual void OnDraw(void* renderer) {}
		virtual void OnRendererRegister(void* renderer) {}
		virtual void OnRendererUnRegister(void* renderer) {}

		void Toggle3D() {
			this->d3 = !this->d3;
		}

		void Set3DMode(bool value) {
			this->d3 = value;
		}

		void RegisterDataPack(VisualDataPack* dataPack) {
			this->sharedData.push_back(dataPack);
		}

		void UnRegisterDataPack(VisualDataPack* dataPack) {
			for(int i = 0; i < this->sharedData.size(); i++) {
				if (this->sharedData[i]->name == dataPack->name) {
					this->sharedData.erase(this->sharedData.begin() + i);
					return;
				}
			}
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

		bool allowRenderCube = true;

		int lightLayer = 0;

		bool sunLight = false;

		bool useLight = true;

		bool screenObject = false;

		/// A map of any kind of data
		std::vector<VisualDataPack*> sharedData;
	};
}

#endif