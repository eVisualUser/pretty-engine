#ifndef H_ENGINE
#define H_ENGINE

#include <PrettyEngine/debug/debug.hpp>
#include <PrettyEngine/debug/DebugDust.hpp>
#include <PrettyEngine/EngineContent.hpp>
#include <PrettyEngine/Input.hpp>
#include <PrettyEngine/KeyCode.hpp>
#include <PrettyEngine/PhysicalSpace.hpp>
#include <PrettyEngine/audio.hpp>
#include <PrettyEngine/data.hpp>
#include <PrettyEngine/editor/editor.hpp>
#include <PrettyEngine/event.hpp>
#include <PrettyEngine/render/render.hpp>
#include <PrettyEngine/render/texture.hpp>
#include <PrettyEngine/utils.hpp>
#include <PrettyEngine/worldLoad.hpp>

#include <implot.h>
#include <toml++/toml.h>

#include <memory>
#include <string>

namespace PrettyEngine {
/// Body of the engine, contain everything
class Engine: public EventListener {
  public:
 	/// Initialize the engine based on a toml configuration
	Engine(std::string config) {
		this->customConfig = toml::parse(config);

		auto antiAliasing = this->customConfig["engine"]["render"]["antiAliasing"].value_or(8);
		this->engineContent.renderer.GetAntiAliasing();

		this->engineContent.renderer.CreateWindow();
		this->engineContent.renderer.Setup();

		std::string engineDataBaseFile = this->customConfig["engine"]["database"].as_string()->get();
		this->engineDatabase = std::make_shared<DataBase>(GetEnginePublicPath(engineDataBaseFile, true));

		auto windowTitle = this->customConfig["engine"]["render"]["window_title"].value_or("Pretty Engine - Game");
		this->engineContent.renderer.SetWindowTitle(windowTitle);

		double backgroundColor[4] = {0.0, 0.0, 0.0, 0.0};
		backgroundColor[0] = this->customConfig["engine"]["render"]["opengl"]["background_color"][0].value_or(0.0f);
		backgroundColor[1] = this->customConfig["engine"]["render"]["opengl"]["background_color"][1].value_or(0.0f);
		backgroundColor[2] = this->customConfig["engine"]["render"]["opengl"]["background_color"][2].value_or(0.0f);
		backgroundColor[3] = this->customConfig["engine"]["render"]["opengl"]["background_color"][3].value_or(0.0f);

		bool createDefaultCamera = this->customConfig["engine"]["render"]["camera"]["create_default_camera"].value_or(false);
		bool setDefaultCameraAsMain = this->customConfig["engine"]["render"]["camera"]["set_default_camera_as_main"].value_or(true);

		if (createDefaultCamera) {
			auto newCamera = this->engineContent.renderer.AddCamera();
			if (setDefaultCameraAsMain) {
				newCamera->active = true;
			}
		}

		this->engineContent.renderer.SetBackgroundColor(backgroundColor[0], backgroundColor[1], backgroundColor[2], backgroundColor[3]);

		this->_imPlotContext = ImPlot::CreateContext();

		this->SetWindowIcon("WindowIcon");

		this->engineContent.renderer.ShowWindow();

		this->engineContent.input.SetWindow(this->engineContent.renderer.GetWindow());

		this->engineContent.eventManager.RegisterListener(this);
	}

	~Engine() {
		this->engineContent.eventManager.UnRegisterListener(this);

		ImPlot::DestroyContext(this->_imPlotContext);
		this->_worldManager.Clear();
		this->engineContent.renderer.Clear();
	}

 	/// Ask the engine to exit
	void Exit() { this->exit = true; }

 	/// Return a pointer to the boolean that define if the game must exit.
	bool *GetExit() { return &this->exit; }

 	/// Launch the engine
	void Run() {
		while (this->engineContent.renderer.Valid() && !*this->GetExit()) {
			this->Update();
		}
	}

 	/// Show UI for debug, and the integrated editor.
	void UpdateDebugUI() {
		if (this->engineContent.input.GetKeyDown(KeyCode::F3)) {
			this->showDebugUI = !showDebugUI;
		}

		if (this->showDebugUI) {
			this->editor.Update(&this->_worldManager, &this->engineContent.input, &this->engineContent.renderer, &this->engineContent.physicalSpace, &this->isEditor, &this->engineContent.audioEngine);
		}
	}

 	/// Update the content of the engine.
	void Update() {
		auto worlds = this->_worldManager.GetWorlds();
		this->engineContent.input.Update();

		// Builtin fullscreen support (F11 is reserved)
		if (this->engineContent.input.GetKeyDown(KeyCode::F11)) {
			this->engineContent.renderer.SetFullscreen(!this->engineContent.renderer.GetFullscreen());
		}

		this->engineContent.renderer.UpdateIO();
		if (this->engineContent.renderer.WindowActive()) {
			for (auto &currentWorld : worlds) {
				if (currentWorld != nullptr) {
					if (!this->isEditor) {
						currentWorld->CallFunctionProcesses();
						currentWorld->Update();
					} else {
						currentWorld->EditorUpdate();
					}
				}
			}

			this->engineContent.renderer.StartUIRendering();
#if ENGINE_EDITOR
			this->UpdateDebugUI();
			worlds = this->_worldManager.GetWorlds();
			this->SetupWorlds();
#endif
			if (this->engineContent.renderer.GetCurrentCamera() != nullptr) {
				for (auto &currentWorld : worlds) {
					currentWorld->simulationCollider.position = this->engineContent.renderer.GetCurrentCamera()->position;
					if (currentWorld != nullptr) {
						currentWorld->CallRenderFunctions();
					}
				}
			}

			for (auto &currentWorld : worlds) {
				if (!this->isEditor) {
					currentWorld->PrePhysics();
				}
			}

			// Engine cleanup
			double currentTime = this->engineContent.renderer.GetTime();
			if (this->lastEngineCleanUp + this->engineCleanup < currentTime) {
				this->engineContent.renderer.Clear();
				DebugDust::GenerateLogFile("logs.log");
				this->lastEngineCleanUp = currentTime;
			}

			this->engineContent.physicalSpace.Update(this->engineContent.renderer.GetDeltaTime());

			this->engineContent.renderer.Draw();
			this->engineContent.renderer.Show();
		}
		for (auto &currentWorld : worlds) {
			if (currentWorld != nullptr) {
				currentWorld->AlwayUpdate();
				if (!this->isEditor) {
					currentWorld->EndUpdate();
				}
			}
		}

		auto requests = this->GetWorldManager()->GetAllDynamicObjectsRequests();
		for (auto &request : requests) {
			if (request == Request::SAVE) {
				this->GetWorldManager()->SaveWorlds();
			} else if (request == Request::EXIT) {
				this->Exit();
			}
		}

	}

	void OnEvent(Event *event) override { 
		if (event->HaveTag("save")) {
			this->GetWorldManager()->SaveWorlds();
		} else if (event->HaveTag("exit")) {
			this->Exit();
		}
	}

 	/// Update the worlds pointers for the entities and components (necessary when create a new entity or component).
	void SetupWorlds() {
		auto worlds = this->_worldManager.GetWorlds();
		for (auto &currentWorld : worlds) {
			currentWorld->engineContent = &this->engineContent;

			currentWorld->UpdateLinks();
		}
	}

	/// Proper way to remove the current worlds
	void ClearWorlds() { this->_worldManager.Clear(); }

 	/// Get a texture from the engine database
	Texture *GetTexture(std::string name) {
		auto dbImages = this->engineDatabase->QuerySQLBlob("SELECT * FROM Textures;");
		auto dbImageName = this->engineDatabase->QuerySQLText("SELECT * FROM Textures;");

		PrettyEngine::Texture *texture = nullptr;

		int index = 0;
		for (auto &img : dbImages) {
			int imgBytes;
			std::vector<unsigned char> imgData;
			int imgHeight;
			int imgWidth;
			int imgChannels;

			imgBytes = img.bytes;
			imgData = img.data;

			if (imgBytes <= 0 || imgData.empty()) {
				DebugLog(LOG_ERROR, "Missing texture: " << name, true);
				continue;
			}

			auto rawData = PrettyEngine::DecodeImage(&imgData, imgBytes, &imgHeight, &imgWidth, &imgChannels);

			if (rawData.empty()) {
				DebugLog(LOG_ERROR, "Failed to decode: " << name, true);
			}

			if (dbImageName[index] == name) {
				texture = this->engineContent.renderer.AddTextureFromData(dbImageName[index], rawData.data(), imgWidth, imgHeight, PrettyEngine::TextureType::Base, PrettyEngine::TextureWrap::ClampToBorder, PrettyEngine::TextureFilter::Linear);
				break;
			}

			index++;
		}

		if (texture == nullptr) {
			DebugLog(LOG_ERROR, "Could not find texture: " << name, true);
			return nullptr;
		}

		return texture;
	}

 	/// Enable or disable the physics.
	void SetPhysics(bool value) { this->_physicsEnabled = value; }

	void TogglePhysics() { this->_physicsEnabled = !this->_physicsEnabled; }

	bool PhysicsEnabled() { return this->_physicsEnabled; }

 	/// Set the engine window using the engine database
	void SetWindowIcon(std::string textureName) {
		auto rawTextures = this->engineDatabase->QuerySQLBlob("SELECT * FROM Textures;");
		auto names = this->engineDatabase->QuerySQLText("SELECT * FROM Textures;");

		int iter = 0;
		for (auto &name : names) {
			if (name == textureName) {
				PrettyEngine::SQLBlobData img = rawTextures[iter];
				int imgHeight = 0;
				int imgWidth = 0;
				int imgChannels = 0;

				auto decoded = DecodeImage(&img.data, img.bytes, &imgHeight, &imgWidth, &imgChannels);

				this->engineContent.renderer.SetWindowIcon(decoded.data(), imgWidth, imgHeight);

				return;
			}
			iter++;
		}
	}

	EngineContent* GetEngineContent() {
		return &this->engineContent;
	}

	PrettyEngine::WorldManager *GetWorldManager() { return &this->_worldManager; }

  private:
	EventManager eventManager;

	std::shared_ptr<DataBase> engineDatabase;

	PrettyEngine::WorldManager _worldManager;

	bool exit = false;

	bool showDebugUI = true;

	bool _physicsEnabled = true;

	EngineContent engineContent;

	toml::parse_result customConfig;

	double lastEngineCleanUp = 0.0f;
	double engineCleanup = 5.0f;

	ImPlotContext *_imPlotContext;

	Editor editor;

#if ENGINE_EDITOR
	bool isEditor = true;
#else
	bool isEditor = false;
#endif
};
}; // namespace PrettyEngine

#endif
