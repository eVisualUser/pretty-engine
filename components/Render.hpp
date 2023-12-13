#ifndef H_COMPONENT_RENDER
#define H_COMPONENT_RENDER

#include <Guid.hpp>
#include <PrettyEngine/debug.hpp>
#include <PrettyEngine/shaders.hpp>
#include <PrettyEngine/texture.hpp>
#include <PrettyEngine/utils.hpp>
#include <PrettyEngine/entity.hpp>
#include <PrettyEngine/mesh.hpp>
#include <PrettyEngine/visualObject.hpp>

#include <memory>

using namespace PrettyEngine;

namespace Custom {
class Render : public PrettyEngine::Component {
public:
	void OnUpdatePublicVariables() override {		
		this->AddActionOnPublicVariableChanged([this](std::string variableName) {
			if (variableName == "TextureBase") {
				this->RefreshBaseTexture();
			}
		});

		this->CreatePublicVar("UseTexture", "false");

	    this->CreatePublicVar("UseTextureBase", "false");
	    this->CreatePublicVar("TextureBase");
	    this->CreatePublicVar("TextureBaseGUID", xg::newGuid());

	    this->CreatePublicVar("UseTextureTransparency", "false");
	    this->CreatePublicVar("TextureTransparency");
	    this->CreatePublicVar("TextureTransparencyGUID", xg::newGuid());

	    this->CreatePublicVar("UseTextureNormal", "false");
	    this->CreatePublicVar("TextureNormal");
	    this->CreatePublicVar("TextureNormalGUID", xg::newGuid());

	    this->CreatePublicVar("Mesh");
	    this->CreatePublicVar("MeshGUID", xg::newGuid());
	    this->CreatePublicVar("UseLight");
	    this->CreatePublicVar("SunLight");
	    this->CreatePublicVar("ScreenObject", "false");

  		this->publicFuncions.insert_or_assign("UpdateRender", [this]() {
			this->Init();
			this->OnUpdate();
		});

		this->publicFuncions.insert_or_assign("Refresh Texture", [this]() { this->RefreshBaseTexture(); });
	}

	void OnEditorStart() override {
		this->OnStart();
		this->RefreshBaseTexture();
	}

	void OnEditorUpdate() override { this->OnUpdate(); }

	void RefreshBaseTexture() {
		if (this->GetPublicVarValue("UseTextureBase") == "true") {
			auto baseTexturePath = this->GetPublicVarValue("TextureBase");
			auto path = GetEnginePublicPath(baseTexturePath, true);
			if (FileExist(path)) {
				this->visualObject->RemoveTexture(TextureType::Base);
				this->engineContent->renderer.Clear();
				this->texture = this->engineContent->renderer.AddTexture(this->textureGuid, path, TextureType::Base, TextureWrap::ClampToBorder, TextureFilter::Linear, TextureChannels::RGBA);
				this->visualObject->AddTexture(this->texture);
			}
		}
	}

	void Init() {
  		this->visualObject->screenObject = (this->GetPublicVarValue("ScreenObject") == "true");

	    this->meshGuid = this->GetPublicVarValue("MeshGUID");
	    this->textureGuid = this->GetPublicVarValue("TextureBaseGUID");
	    this->textureTransparancyGuid =
	        this->GetPublicVarValue("TextureTransparencyGUID");
	    this->textureNormalGuid = this->GetPublicVarValue("TextureNormalGUID");

	    this->visualObject->useLight =
	        (this->GetPublicVarValue("UseLight") == "true");
	    this->visualObject->sunLight =
	        (this->GetPublicVarValue("SunLight") == "true");

	    this->engineContent->renderer.AddShader("DefaultVertex", ShaderType::Vertex,
	                                            Shaders::SHADER_VERTEX_VERTEX);
	    this->engineContent->renderer.AddShader(
	        "DefaultFragment", ShaderType::Fragment, Shaders::SHADER_FRAGMENT_FRAGMENT);
	    auto shader = this->engineContent->renderer.AddShaderProgram(
	        "Default", "DefaultVertex", "DefaultFragment");

	    this->renderModel.SetShaderProgram(shader);
	    this->visualObject->AddRenderModel(&this->renderModel);

	    this->renderModel.useTexture = (this->GetPublicVarValue("UseTexture") == "true");

	    if (this->GetPublicVarValue("UseTextureTransparency") == "true") {
	      auto baseTexturePath = this->GetPublicVarValue("TextureNormal");
	      auto path = GetEnginePublicPath(baseTexturePath, true);
	      if (FileExist(path)) {
	        this->engineContent->renderer.RemoveTexture(
	            this->textureTransparancyGuid);
	        this->visualObject->RemoveTexture(this->textureTransparency);
	        this->textureTransparency = this->engineContent->renderer.AddTexture(
	            this->textureTransparancyGuid, path, TextureType::Transparency,
	            TextureWrap::ClampToBorder, TextureFilter::Linear,
	            TextureChannels::RGBA);
	        this->visualObject->AddTexture(this->textureTransparency);
	      }
	    }

	    if (this->GetPublicVarValue("UseTextureNormal") == "true") {
	      auto baseTexturePath = this->GetPublicVarValue("TextureTransparency");
	      auto path = GetEnginePublicPath(baseTexturePath, true);
	      if (FileExist(path)) {
	        this->engineContent->renderer.RemoveTexture(this->textureGuid);
	        this->visualObject->RemoveTexture(this->textureTransparency);
	        this->textureTransparency = this->engineContent->renderer.AddTexture(
	            this->textureGuid, path, TextureType::Transparency,
	            TextureWrap::ClampToBorder, TextureFilter::Linear,
	            TextureChannels::RGBA);
	        this->visualObject->AddTexture(this->textureTransparency);
	      }
	    }

	    bool loadMesh = false;

		auto meshName = this->GetPublicVarValue("Mesh");
		// Check if the mesh is already loaded
		if (this->mesh == nullptr) {
			this->engineContent->renderer.RemoveMesh(meshGuid);
			auto newMesh = CreateRectMesh();
			this->mesh = this->engineContent->renderer.AddMesh(meshGuid, newMesh);
			this->renderModel.SetMesh(this->mesh);
			loadMesh = true;
		}

	    if (loadMesh) {
	      	this->engineContent->renderer.UnRegisterVisualObject(visualObjectGuid);
	      	this->engineContent->renderer.RegisterVisualObject(visualObjectGuid, this->visualObject);
	    }
  	}

  	void OnStart() override {
	    this->Init();
  	}

	~Render() { this->OnDestroy();	}

  	void OnDestroy() override {
    	this->engineContent->renderer.UnRegisterVisualObject(visualObjectGuid);
  	}

  	void OnUpdate() override {
  		this->visualObject->position = dynamic_cast<Entity *>(this->owner)->position;
	    this->visualObject->rotation = dynamic_cast<Entity *>(this->owner)->rotation;
	    this->visualObject->scale = dynamic_cast<Entity *>(this->owner)->scale;
  	}

  	VisualObject *GetVisualObject() { return this->visualObject.get(); }

private:
  	Mesh *mesh = nullptr;
  	std::string meshGuid = xg::newGuid();
  	Texture *texture = nullptr;
  	Texture *textureTransparency = nullptr;
  	Texture *textureNormal = nullptr;
  	std::string textureGuid = xg::newGuid();
  	std::string textureTransparancyGuid = xg::newGuid();
  	std::string textureNormalGuid = xg::newGuid();

  	RenderModel renderModel;
  	std::shared_ptr<VisualObject> visualObject = std::make_shared<VisualObject>();
  	std::string visualObjectGuid = xg::newGuid();
};
} // namespace Custom

#endif
