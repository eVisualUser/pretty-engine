#ifndef H_ENGINE
#define H_ENGINE

#include "PrettyEngine/command.hpp"
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
class Engine final: public EventListener {
  public:
 	/// Initialize the engine based on a toml configuration
	explicit Engine(std::string config) {

		#ifdef ENGINE_EDITOR
		this->editor = new Editor();
		#endif

		auto customConfig = toml::parse(config);

		const int antiAliasing = customConfig["engine"]["render"]["antiAliasing"].value_or(8);
		this->engineContent.renderer.SetAntiAliasing(antiAliasing);

		this->engineContent.renderer.CreateWindow();
		this->engineContent.renderer.Setup();

		this->logLimit = customConfig["engine"]["logs_limit"].value_or(100.0f);

		const auto windowTitle = customConfig["engine"]["render"]["window_title"].value_or("Pretty Engine - Game");
		this->engineContent.renderer.SetWindowTitle(windowTitle);

		double backgroundColor[4] = {0.0, 0.0, 0.0, 0.0};
		backgroundColor[0] = customConfig["engine"]["render"]["opengl"]["background_color"][0].value_or(0.0f);
		backgroundColor[1] = customConfig["engine"]["render"]["opengl"]["background_color"][1].value_or(0.0f);
		backgroundColor[2] = customConfig["engine"]["render"]["opengl"]["background_color"][2].value_or(0.0f);
		backgroundColor[3] = customConfig["engine"]["render"]["opengl"]["background_color"][3].value_or(0.0f);

		const bool createDefaultCamera = customConfig["engine"]["render"]["camera"]["create_default_camera"].value_or(false);
		const bool setDefaultCameraAsMain = customConfig["engine"]["render"]["camera"]["set_default_camera_as_main"].value_or(true);

		if (createDefaultCamera) {
			auto newCamera = this->engineContent.renderer.AddCamera();
			if (setDefaultCameraAsMain) {
				newCamera->active = true;
			}
		}

		this->engineContent.renderer.SetBackgroundColor(backgroundColor[0], backgroundColor[1], backgroundColor[2], backgroundColor[3]);

		this->engineContent.renderer.ShowWindow();

		this->engineContent.input.SetWindow(this->engineContent.renderer.GetWindow());

		this->engineContent.eventManager.RegisterListener(this);

		// Add a command to allow saving the worlds from the console
		this->saveCommand.commandName = "save";
		this->saveCommand.action = [this](std::vector<std::string> args){
      DebugLog(LOG_DEBUG, "Save", false);
			this->_worldManager.SaveWorlds();
		};

		CommandSystem::AddCommand(&this->saveCommand);
	}

	~Engine() {
		CommandSystem::RemoveCommand(&this->saveCommand);

		this->engineContent.eventManager.UnRegisterListener(this);

		this->_worldManager.ClearWorldInstances();
		this->_worldManager.Clear();

		#ifdef ENGINE_EDITOR
		delete this->editor;
		#endif
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
			this->editor->Update(&this->engineContent, &this->_worldManager, &this->isEditor);
		}
	}

 	/// Update the content of the engine.
	void Update() {
		// Engine cleanup
		double currentTime = this->engineContent.renderer.GetTime();
		if (this->lastEngineCleanUp + this->engineCleanup < currentTime) {
			this->engineContent.renderer.Clear();
			this->lastEngineCleanUp = currentTime;
		}

		if (logs.size() > 100) {
			DebugDust::GenerateLogFile("logs.log");
			logs.erase(logs.begin());
		}

#if ENGINE_EDITOR
		// Stop playing the game if an error occurred
		if (logs.size() > 0) {
			auto lastLog = logs[logs.size() - 1];
			if (logs[logs.size() - 1].type == LOG_ERROR && !this->isEditor) {
				this->isEditor = true;
				DebugLog(LOG_WARNING, "An error occurred, play state stopped. Please check the console.", true);
			}
		}
#endif
		auto worlds = this->_worldManager.GetWorlds();
		this->engineContent.input.Update();

			for (auto &currentWorld : *worlds) {
				if (!this->isEditor) {
					currentWorld->PrePhysics();
				}
			}

			this->engineContent.physicalSpace.Update(this->engineContent.renderer.GetDeltaTime());

		// Builtin fullscreen support (F11 is reserved)
		if (this->engineContent.input.GetKeyDown(KeyCode::F11)) {
			this->engineContent.renderer.SetFullscreen(!this->engineContent.renderer.GetFullscreen());
		}

		this->engineContent.renderer.UpdateIO();
		this->engineContent.renderer.StartUIRendering();

#if ENGINE_EDITOR
			this->UpdateDebugUI();
			this->SetupWorlds();
#endif

		if (this->engineContent.renderer.GetCurrentCamera() != nullptr) {
			for (auto &currentWorld : *worlds) {
				currentWorld->simulationCollider.position = this->engineContent.renderer.GetCurrentCamera()->position;
				if (currentWorld != nullptr) {
					currentWorld->CallRenderFunctions();
				}
			}
		}

		for (auto &currentWorld : *worlds) {
			if (currentWorld != nullptr) {
				if (!this->isEditor) {
					currentWorld->Update();
				} else {
					currentWorld->EditorUpdate();
				}
			}
		}

		this->engineContent.renderer.Draw();
		this->engineContent.renderer.Show();

		for (auto &currentWorld : *worlds) {
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
		for (auto &currentWorld : *worlds) {
			currentWorld->engineContent = &this->engineContent;

			currentWorld->UpdateLinks();
		}
	}

	/// Proper way to remove the current worlds
	void ClearWorlds() { this->_worldManager.Clear(); }

 	/// Enable or disable the physics.
	void SetPhysics(bool value) { this->_physicsEnabled = value; }

	void TogglePhysics() { this->_physicsEnabled = !this->_physicsEnabled; }

	bool PhysicsEnabled() const { return this->_physicsEnabled; }

	EngineContent* GetEngineContent() {
		return &this->engineContent;
	}

	PrettyEngine::WorldManager *GetWorldManager() { return &this->_worldManager; }

  private:
  	int logLimit;

		EventManager eventManager;

		PrettyEngine::WorldManager _worldManager;

		bool exit = false;

		bool showDebugUI = true;

		bool _physicsEnabled = true;

		EngineContent engineContent;

		double lastEngineCleanUp = 0.0f;
		double engineCleanup = 5.0f;

		Editor* editor;

		Command saveCommand;

#if ENGINE_EDITOR
	bool isEditor = true;
#else
	bool isEditor = false;
#endif
};
}; // namespace PrettyEngine

#endif
