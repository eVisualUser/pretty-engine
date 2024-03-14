#ifndef HPP_COMPONENT_RENDER
#define HPP_COMPONENT_RENDER

#include "PrettyEngine/debug/debug.hpp"
#include "PrettyEngine/dynamicObject.hpp"
#include <Guid.hpp>
#include <PrettyEngine/render/texture.hpp>
#include <PrettyEngine/entity.hpp>
#include <PrettyEngine/render/mesh.hpp>
#include <PrettyEngine/shaders.hpp>
#include <PrettyEngine/render/visualObject.hpp>
#include <PrettyEngine/assetManager.hpp>

#include <memory>

using namespace PrettyEngine;

namespace Custom {
class Render : public PrettyEngine::Component {
public:
	void OnSetup() override {
		this->AddSerializedField(SERIAL_TOKEN(bool), "UseTexure", SERIAL_TOKEN(false));

	    this->AddSerializedField(SERIAL_TOKEN(bool), "UseTextureBase", SERIAL_TOKEN(false));
		this->AddSerializedField(SERIAL_TOKEN(std::string), "TextureBase", "");

	    this->AddSerializedField(SERIAL_TOKEN(bool), "UseTextureTransparency", SERIAL_TOKEN(false));
		this->AddSerializedField(SERIAL_TOKEN(std::string), "TextureTransparency", "");

	    this->AddSerializedField(SERIAL_TOKEN(bool), "UseTextureNormal", SERIAL_TOKEN(false));
		this->AddSerializedField(SERIAL_TOKEN(std::string), "TextureNormal", "");

	    this->AddSerializedField(SERIAL_TOKEN(PrettyEngine::Mesh), "Mesh", "");
		this->AddSerializedField(SERIAL_TOKEN(std::string), "MeshGUID", xg::newGuid());
		this->AddSerializedField(SERIAL_TOKEN(bool), "UseLight", SERIAL_TOKEN(false));
		this->AddSerializedField(SERIAL_TOKEN(bool), "SunLight", SERIAL_TOKEN(false));
		this->AddSerializedField(SERIAL_TOKEN(bool), "ScreenObject", SERIAL_TOKEN(false));

		this->AddSerializedField(SERIAL_TOKEN(glm::vec4), "Color", "1;1;1;1");

		this->AddSerializedField(SERIAL_TOKEN(bool), "WireFrame", SERIAL_TOKEN(false));
	}

	void OnEditorStart() override {
        this->OnStart();

        this->publicFuncions.insert_or_assign("Refresh Base Texture", [this]() { this->RefreshTextureBase(); });
        this->publicFuncions.insert_or_assign("Refresh Transparency Texture", [this]() { this->RefreshTextureTransparency(); });
        this->publicFuncions.insert_or_assign("Refresh Normal Texture", [this]() { this->RefreshTextureNormal(); });
	}

	void OnEditorUpdate() override { this->OnUpdate(); }

	void RefreshTextureBase() {
		if (this->GetSerializedFieldValue("UseTextureBase") == "true") {
			const auto texturePath = GetEnginePublicPath(this->GetSerializedFieldValue("TextureBase"), true);
			this->baseTexture = Asset(texturePath);

			if (this->baseTexture.Exist()) {
				if (this->visualObject.HaveTexture(TextureType::Base)) {
					this->visualObject.RemoveTexture(TextureType::Base);
				}
				this->texture = this->engineContent->renderer.AddTexture(texturePath, &this->baseTexture, TextureType::Base, TextureWrap::ClampToBorder, TextureFilter::Linear, TextureChannels::RGBA);
				this->visualObject.AddTexture(this->texture);
			}
		}
	}

	void RefreshTextureTransparency() {
		if (this->GetSerializedFieldValue("UseTextureTransparency") == "true") {
			const auto texturePath = GetEnginePublicPath(this->GetSerializedFieldValue("TextureTransparency"), true);
			this->transparencyTexture = Asset(texturePath);

			if (this->transparencyTexture.Exist()) {
				if (this->visualObject.HaveTexture(TextureType::Transparency)) {
					this->visualObject.RemoveTexture(TextureType::Transparency);
				}
				this->textureTransparency = this->engineContent->renderer.AddTexture(texturePath, &this->transparencyTexture, TextureType::Transparency, TextureWrap::ClampToBorder, TextureFilter::Linear, TextureChannels::RGBA);
				this->visualObject.AddTexture(this->textureTransparency);
			}
		}
	}

	void RefreshTextureNormal() {
		if (this->GetSerializedFieldValue("UseTextureNormal") == "true") {
			const auto texturePath = GetEnginePublicPath(this->GetSerializedFieldValue("TextureTransparency"), true);
			this->normalTexture = Asset(texturePath);

			if (this->normalTexture.Exist()) {
				if (this->visualObject.HaveTexture(TextureType::Normal)) {
					this->visualObject.RemoveTexture(TextureType::Normal);
				}
				this->textureNormal = this->engineContent->renderer.AddTexture(texturePath, &this->normalTexture, TextureType::Normal, TextureWrap::ClampToBorder, TextureFilter::Linear, TextureChannels::RGBA);
				this->visualObject.AddTexture(this->textureNormal);
			}
		}
	}

	/// Initialize the rendering (textures, shaders, mesh)
	void Init() {
		const auto defaultVertexShaderName = this->engineContent->renderer.AddShader("DefaultVertexShader", ShaderType::Vertex, Shaders::SHADER_VERTEX_VERTEX);
		const auto defaultFragmentShaderName = this->engineContent->renderer.AddShader("defaultFragmentShader", ShaderType::Fragment, Shaders::SHADER_FRAGMENT_FRAGMENT);

		const auto shader = this->engineContent->renderer.AddShaderProgram("Default", defaultVertexShaderName, defaultFragmentShaderName);

		this->renderModel.SetShaderProgram(shader);

		this->visualObject.screenObject = (this->GetSerializedFieldValue("ScreenObject") == "true");

		this->visualObject.wireFrame = (this->GetSerializedFieldValue("WireFrame") == "true");

		const auto color = ParseCSVLine(this->GetSerializedFieldValue("Color"));

		this->visualObject.baseColor.r = std::stof(color[0]);
		this->visualObject.baseColor.g = std::stof(color[1]);
		this->visualObject.baseColor.b = std::stof(color[2]);
		this->visualObject.opacity = std::stof(color[3]);

		// The mesh GUID is for now locked at this value, because mesh loading is not supported 
	    this->meshGuid = "rect"; // this->GetSerializedFieldValue("MeshGUID");

	    this->visualObject.useLight = (this->GetSerializedFieldValue("UseLight") == "true");
	    this->visualObject.sunLight = (this->GetSerializedFieldValue("SunLight") == "true");
	    
		this->RefreshTextureBase();
		this->RefreshTextureNormal();
		this->RefreshTextureTransparency();
		
	    this->visualObject.AddRenderModel(&this->renderModel);

	    this->renderModel.useTexture = (this->GetSerializedFieldValue("UseTexture") == "true");

	    // Check if the mesh is already loaded
		if (this->mesh == nullptr) {
			const auto meshPath = GetEnginePublicPath(this->GetPublicVarValue("Mesh"), true);

			this->renderModel.RemoveMesh();
			const auto newMesh = CreateRectMesh();
			this->mesh = this->engineContent->renderer.AddMesh(meshGuid, newMesh);
			this->renderModel.SetMesh(this->mesh);

			this->engineContent->renderer.RegisterVisualObject(visualObjectGuid, &this->visualObject);
		}
  	}

  	void OnStart() override {
	    this->Init();
  	}

  	void OnDestroy() override {
    	if (!this->engineContent->renderer.UnRegisterVisualObject(visualObjectGuid)) {
    		DebugLog(LOG_ERROR, "Failed to unregister visual object.", false);
    	}
  	}

  	void OnUpdate() override {
  		this->visualObject.position = dynamic_cast<Entity *>(this->owner)->position;
	    this->visualObject.rotation = dynamic_cast<Entity *>(this->owner)->rotation;
	    this->visualObject.scale = dynamic_cast<Entity *>(this->owner)->scale;
  	}

  	VisualObject* GetVisualObject() { return &this->visualObject; }

private:
  	Mesh *mesh = nullptr;
  	std::string meshGuid = xg::newGuid();
  	Texture *texture = nullptr;
  	Texture *textureTransparency = nullptr;
  	Texture *textureNormal = nullptr;

  	RenderModel renderModel = RenderModel();
  	VisualObject visualObject;
  	std::string visualObjectGuid = xg::newGuid();

	Asset baseTexture;
	Asset transparencyTexture;
	Asset normalTexture;
};
} // namespace Custom

#endif
