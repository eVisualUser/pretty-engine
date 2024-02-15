#ifndef H_COMPONENT_RENDER
#define H_COMPONENT_RENDER

#include <Guid.hpp>
#include <PrettyEngine/debug/debug.hpp>
#include <PrettyEngine/shaders.hpp>
#include <PrettyEngine/render/texture.hpp>
#include <PrettyEngine/utils.hpp>
#include <PrettyEngine/entity.hpp>
#include <PrettyEngine/render/mesh.hpp>
#include <PrettyEngine/render/visualObject.hpp>
#include <PrettyEngine/assetManager.hpp>

#include <memory>

using namespace PrettyEngine;

namespace Custom {
class Render : public PrettyEngine::Component {
public:
	void OnUpdatePublicVariables() override {
		
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
		if (this->GetSerializedFieldValue("UseTextureBase") == "true") {
			const auto baseTexturePath = GetEnginePublicPath(this->GetSerializedFieldValue("TextureBase"), true);
			this->baseTexture = Asset(baseTexturePath);
			
			if (this->baseTexture.Exist()) {
				this->visualObject->RemoveTexture(TextureType::Base);
				this->engineContent->renderer.Clear();
				this->texture = this->engineContent->renderer.AddTexture(baseTexturePath, &this->baseTexture, TextureType::Base, TextureWrap::ClampToBorder, TextureFilter::Linear, TextureChannels::RGBA);
				this->visualObject->AddTexture(this->texture);
			}
		}
	}

	void Init() {
		this->visualObject->screenObject = (this->GetSerializedFieldValue("ScreenObject") == "true");

		this->visualObject->wireFrame = (this->GetSerializedFieldValue("WireFrame") == "true");

		const auto color = ParseCSVLine(this->GetSerializedFieldValue("Color"));

		this->visualObject->baseColor.r = std::stof(color[0]);
		this->visualObject->baseColor.g = std::stof(color[1]);
		this->visualObject->baseColor.b = std::stof(color[2]);
		this->visualObject->opacity = std::stof(color[3]);

		// The mesh GUID is for now locked at this value, because mesh loading is not supported 
	    this->meshGuid = "rect"; // this->GetSerializedFieldValue("MeshGUID");

	    this->visualObject->useLight = (this->GetSerializedFieldValue("UseLight") == "true");
	    this->visualObject->sunLight = (this->GetSerializedFieldValue("SunLight") == "true");

		std::string defaultVertexShaderName = "DefaultVertex";
		std::string defaultFragmentShaderName = "DefaultFragment";

    	this->engineContent->renderer.AddShader(defaultVertexShaderName, ShaderType::Vertex, Shaders::SHADER_VERTEX_VERTEX);
	    this->engineContent->renderer.AddShader(defaultFragmentShaderName, ShaderType::Fragment, Shaders::SHADER_FRAGMENT_FRAGMENT);

	    const auto shader = this->engineContent->renderer.AddShaderProgram("Default", defaultVertexShaderName, defaultFragmentShaderName);

	    this->renderModel.SetShaderProgram(shader);

		this->RefreshBaseTexture();

	    this->visualObject->AddRenderModel(&this->renderModel);

	    this->renderModel.useTexture = (this->GetSerializedFieldValue("UseTexture") == "true");

	    if (this->GetSerializedFieldValue("UseTextureTransparency") == "true") {
			
			const auto baseTexturePath = GetEnginePublicPath(this->GetSerializedFieldValue("TextureTransparency"));
			this->transparencyTexture = Asset(baseTexturePath);
	      	
	      	if (this->transparencyTexture.Exist()) {
		        this->engineContent->renderer.RemoveTexture(this->textureTransparency);

		        this->visualObject->RemoveTexture(this->textureTransparency);
		        this->textureTransparency = this->engineContent->renderer.AddTexture(
		            this->transparencyTexture.GetFilePath(), &this->transparencyTexture, TextureType::Transparency,
		            TextureWrap::ClampToBorder, TextureFilter::Linear,
		            TextureChannels::RGBA);

		        this->visualObject->AddTexture(this->textureTransparency);
	      	}
	    }

	    if (this->GetSerializedFieldValue("UseTextureNormal") == "true") {
		  const auto baseTexturePath = GetEnginePublicPath(this->GetSerializedFieldValue("TextureNormal"), true);
		  this->normalTexture = Asset(baseTexturePath);
	      if (this->normalTexture.Exist()) {
	        this->engineContent->renderer.RemoveTexture(this->normalTexture.GetFilePath());
	        this->visualObject->RemoveTexture(this->textureNormal);
	        this->textureNormal = this->engineContent->renderer.AddTexture(
	            this->normalTexture.GetFilePath(), &this->normalTexture, TextureType::Normal,
	            TextureWrap::ClampToBorder, TextureFilter::Linear,
	            TextureChannels::RGBA);
	        this->visualObject->AddTexture(this->textureNormal);
	      }
	    }

	    bool loadMesh = false;

		const auto meshPath = GetEnginePublicPath(this->GetPublicVarValue("Mesh"), true);
		// Check if the mesh is already loaded
		if (this->mesh == nullptr) {
			this->renderModel.RemoveMesh();
			const auto newMesh = CreateRectMesh();
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

  	VisualObject *GetVisualObject() const { return this->visualObject.get(); }

private:
  	Mesh *mesh = nullptr;
  	std::string meshGuid = xg::newGuid();
  	Texture *texture = nullptr;
  	Texture *textureTransparency = nullptr;
  	Texture *textureNormal = nullptr;

  	RenderModel renderModel = RenderModel();
  	std::shared_ptr<VisualObject> visualObject = std::make_shared<VisualObject>();
  	std::string visualObjectGuid = xg::newGuid();

	Asset baseTexture;
	Asset transparencyTexture;
	Asset normalTexture;
};
} // namespace Custom

#endif
