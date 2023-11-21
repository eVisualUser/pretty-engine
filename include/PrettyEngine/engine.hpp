#ifndef H_ENGINE
#define H_ENGINE

#include <PrettyEngine/KeyCode.hpp>
#include <PrettyEngine/worldLoad.hpp>
#include <PrettyEngine/utils.hpp>
#include <PrettyEngine/audio.hpp>
#include <PrettyEngine/debug.hpp>
#include <PrettyEngine/render.hpp>
#include <PrettyEngine/data.hpp>
#include <PrettyEngine/texture.hpp>
#include <PrettyEngine/Input.hpp>
#include <PrettyEngine/PhysicalSpace.hpp>
#include <PrettyEngine/editor.hpp>
#include <PrettyEngine/EngineContent.hpp>
#include <PrettyEngine/event.hpp>

#include <toml++/toml.h>
#include <implot.h>

#include <memory>
#include <string>

#define CONSOLE_COMMAND_BUFFER_SIZE 100

namespace PrettyEngine {
	class Engine {
	public:
		Engine(std::string config) {
			this->customConfig = toml::parse(config);
			
			auto antiAliasing = this->customConfig["engine"]["render"]["antiAliasing"].value_or(8);
			this->engineContent.renderer.GetAntiAliasing();

			this->engineContent.renderer.CreateWindow();
			this->engineContent.renderer.Setup();

			std::string engineDataBaseFile = this->customConfig["engine"]["database"].as_string()->get();
			this->engineDatabase = std::make_shared<DataBase>(engineDataBaseFile);

			auto windowTitle = this->customConfig["engine"]["render"]["window_title"].value_or("Pretty Engine - Game");
			this->engineContent.renderer.SetWindowTitle(windowTitle);

			double backgroundColor[4];
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
		}
		
		~Engine() {
			ImPlot::DestroyContext(this->_imPlotContext);
			this->_worldManager.Clear();
			this->engineContent.renderer.Clear();
		}
		
		void Exit() {
			this->exit = true;
		}
		
		bool* GetExit() {
			return &this->exit;
		}
		
		void Run() {
			while(this->engineContent.renderer.Valid() && !*this->GetExit()) {
				this->Update();
			}
		}

		void UpdateDebugUI() {
			if (this->engineContent.input.GetKeyDown(KeyCode::F3)) {
				this->showDebugUI = !showDebugUI;
			}
			
			if (this->showDebugUI) {
				this->editor.Update(&this->_worldManager, &this->engineContent.input, &this->engineContent.renderer, &this->engineContent.physicalSpace, &this->frameRateLogs, &this->frameRateTimeLogs);
			}
		}

		void Update() {
			auto worlds = this->_worldManager.GetWorlds();
			this->engineContent.input.Update();

			// Builtin fullscreen support
			if (this->engineContent.input.GetKeyDown(KeyCode::F11)) {
				this->engineContent.renderer.SetFullscreen(!this->engineContent.renderer.GetFullscreen());
			}

			this->engineContent.renderer.UpdateIO();
			if (this->engineContent.renderer.WindowActive()) {
				for (auto & currentWorld: worlds) {
					if (currentWorld != nullptr) {
						currentWorld->CallFunctionProcesses();
						currentWorld->Update();
						
						currentWorld->simulationCollider.position = this->engineContent.renderer.GetCurrentCamera()->position;
						currentWorld->EditorUpdate();
					}
				}

				this->engineContent.renderer.StartUIRendering();
				#if ENGINE_EDITOR
					this->UpdateDebugUI();
				#endif
				for (auto & currentWorld: worlds) {
					if (currentWorld != nullptr) {
						currentWorld->CallRenderFunctions();
						currentWorld->PrePhysics();
					}
				}

				double currentTime = this->engineContent.renderer.GetTime();
				if (lastRenderClearTime + renderClearCoolDown < currentTime) {
					this->engineContent.renderer.Clear();
				}

				if (this->lastFrameRateLog + this->frameRateCoolDown < currentTime && !this->showFrameRateGraph) {
					this->frameRateLogs.push_back(this->engineContent.renderer.GetFPS());
					this->frameRateTimeLogs.push_back(currentTime);
					this->lastFrameRateLog = currentTime;
				}
				
				this->engineContent.physicalSpace.Update(this->engineContent.renderer.GetDeltaTime());
				
				this->engineContent.renderer.Draw();
				this->engineContent.renderer.Show();
			}
			for (auto & currentWorld: worlds) {
				if (currentWorld != nullptr) {
					currentWorld->AlwayUpdate();
					currentWorld->EndUpdate();
				}
			}

			auto requests = this->GetWorldManager()->GetAllDynamicObjectsRequests();
			for (auto & request: requests) {
				if (request == Request::SAVE) {
					this->GetWorldManager()->SaveWorlds();
				} else if (request == Request::EXIT) {
					this->Exit();
				}
			}
		}

		void SetupWorlds() {
			auto worlds = this->_worldManager.GetWorlds();
			for (auto & currentWorld: worlds) {
				currentWorld->engineContent = &this->engineContent;

				currentWorld->UpdateLinks();
			}
		}

		/// Proper way to remove the current worlds
		void ClearWorlds() {
			this->_worldManager.Clear();
		}
		
		Texture* GetTexture(std::string name) {	
			auto dbImages = this->engineDatabase->QuerySQLBlob("SELECT * FROM Textures;");
			auto dbImageName = this->engineDatabase->QuerySQLText("SELECT * FROM Textures;");

			PrettyEngine::Texture* texture = nullptr;

			int index = 0;
			for (auto & img: dbImages) {
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
					texture = this->engineContent.renderer.AddTextureFromData(
						dbImageName[index],
						rawData.data(),
						imgWidth,
						imgHeight,
						PrettyEngine::TextureType::Base,
						PrettyEngine::TextureWrap::ClampToBorder,
						PrettyEngine::TextureFilter::Linear
					);
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

		void SetPhysics(bool value) {
			this->_physicsEnabled = value;
		}

		void TogglePhysics() {
			this->_physicsEnabled = !this->_physicsEnabled;
		}

		bool PhysicsEnabled() {
			return this->_physicsEnabled;
		}

		void SetWindowIcon(std::string textureName) {
			auto rawTextures = this->engineDatabase->QuerySQLBlob("SELECT * FROM Textures;");
			auto names = this->engineDatabase->QuerySQLText("SELECT * FROM Textures;");
			
			int iter = 0;
			for (auto & name: names) {
				if (name == textureName) {
					auto img = rawTextures[iter];
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

		PrettyEngine::WorldManager* GetWorldManager() {
			return &this->_worldManager;
		}
		
	private:
		EventManager eventManager;

		std::shared_ptr<DataBase> engineDatabase;

		PrettyEngine::WorldManager _worldManager;

		bool exit = false;

		bool showDebugUI = false;

		bool _physicsEnabled = true;

		EngineContent engineContent;

		toml::parse_result customConfig;

		double lastRenderClearTime = 0.0f;
		double renderClearCoolDown = 10.0f;

		bool showFrameRateGraph = false;
		double frameRateCoolDown = 0.5f;
		double lastFrameRateLog = 0.0f;
		std::vector<int> frameRateLogs;
		std::vector<int> frameRateTimeLogs;

		ImPlotContext* _imPlotContext;

		Editor editor;
	};
};

#endif
