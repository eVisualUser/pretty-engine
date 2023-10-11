#pragma once

#include "Guid.hpp"
#include "PrettyEngine/debug.hpp"
#include "PrettyEngine/shaders.hpp"
#include "PrettyEngine/texture.hpp"
#include "PrettyEngine/utils.hpp"
#include "PrettyEngine/visualObject.hpp"
#include <PrettyEngine/entity.hpp>
#include <PrettyEngine/mesh.hpp>
#include <memory>

using namespace PrettyEngine;

namespace Custom {
	class Render: public PrettyEngine::Component {
	public:
		void OnStart() {
			this->CreatePublicVar("Texture Base");
			this->CreatePublicVar("Mesh");
			this->CreatePublicVar("UseLight");
			this->CreatePublicVar("SunLight");

			this->visualObject->useLight = (this->GetPublicVarValue("UseLight") == "true");
			this->visualObject->sunLight = (this->GetPublicVarValue("SunLight") == "true");

			this->renderer->AddShader("DefaultVertex", ShaderType::Vertex, SHADER_VERTEX);
			this->renderer->AddShader("DefaultFragment", ShaderType::Fragment, SHADER_FRAGMENT);
			auto shader = this->renderer->AddShaderProgram("Default", "DefaultVertex", "DefaultFragment");

			this->renderModel.SetShaderProgram(shader);
			this->visualObject->AddRenderModel(&this->renderModel);
		}

		void OnDestroy() {
			this->renderer->UnRegisterVisualObject(visualObjectGuid);
		}

		void OnUpdate() override {
			this->visualObject->position = dynamic_cast<Entity*>(this->owner)->position;
			this->visualObject->rotation = dynamic_cast<Entity*>(this->owner)->rotation;
			this->visualObject->scale = dynamic_cast<Entity*>(this->owner)->scale;

			bool loadMesh = false;

			auto meshName = this->GetPublicVarValue("Mesh");
			if (meshName == "rect") {
				// Check if the mesh is already loaded
				if (this->mesh == nullptr) {
					this->renderer->RemoveMesh(meshGuid);
					auto newMesh = CreateRectMesh();
					this->mesh = this->renderer->AddMesh(meshGuid, newMesh);
					this->renderModel.SetMesh(this->mesh);
					loadMesh = true;
				}
			} else {
				DebugLog(LOG_WARNING, "Missing mesh for: " << this->unique, false);
			}

			auto baseTexturePath = this->GetPublicVarValue("Texture Base");

			auto path = GetEnginePublicPath(baseTexturePath, true);

			if (this->texture == nullptr && FileExist(path)) {
				this->renderer->RemoveTexture(this->textureGuid);
				this->visualObject->RemoveTexture(this->texture);
				this->texture = this->renderer->AddTexture(this->textureGuid, path, TextureType::Base, TextureWrap::ClampToBorder, TextureFilter::Linear, TextureChannels::RGBA);
				this->visualObject->AddTexture(this->texture);
				this->renderModel.useTexture = true;
			}

			if (loadMesh) {
				this->renderer->UnRegisterVisualObject(visualObjectGuid);
				this->renderer->RegisterVisualObject(visualObjectGuid, this->visualObject);
			}
		}

		std::shared_ptr<VisualObject> GetVisualObject() {
			return this->visualObject;
		}

	private:
		Mesh* mesh = nullptr;
		std::string meshGuid = xg::newGuid();
		Texture* texture = nullptr;
		std::string textureGuid = xg::newGuid();

		RenderModel renderModel;
		std::shared_ptr<VisualObject> visualObject = std::make_shared<VisualObject>();
		std::string visualObjectGuid = xg::newGuid();
	};
}
