#pragma once

#include <PrettyEngine/world.hpp>
#include <PrettyEngine/worldLoad.hpp>
#include <PrettyEngine/utils.hpp>
#include <PrettyEngine/audio.hpp>
#include <PrettyEngine/physics.hpp>
#include <PrettyEngine/physicsEngine.hpp>
#include <PrettyEngine/debug.hpp>
#include <PrettyEngine/render.hpp>
#include <PrettyEngine/localization.hpp>
#include <PrettyEngine/data.hpp>
#include <PrettyEngine/texture.hpp>
#include <PrettyEngine/Input.hpp>

#include <toml++/toml.h>
#include <imgui.h>
#include <implot.h>

#include <memory>
#include <string>
#include <vector>

#define CONSOLE_COMMAND_BUFFER_SIZE 100

namespace PrettyEngine {
	class Engine {
	public:
		Engine(std::string config) {
			this->customConfig = toml::parse(config);
			
			auto antiAliasing = this->customConfig["engine"]["render"]["antiAliasing"].value_or(8);
			this->_renderer->GetAntiAliasing();

			this->_renderer->CreateWindow();
			this->_renderer->Setup();

			std::string engineDataBaseFile = this->customConfig["engine"]["database"].as_string()->get();
			this->engineDatabase = std::make_shared<DataBase>(engineDataBaseFile);

			auto windowTitle = this->customConfig["engine"]["render"]["window_title"].value_or("Pretty Engine - Game");
			this->_renderer->SetWindowTitle(windowTitle);

			double backgroundColor[4];
			backgroundColor[0] = this->customConfig["engine"]["render"]["opengl"]["background_color"][0].value_or(0.0f);
			backgroundColor[1] = this->customConfig["engine"]["render"]["opengl"]["background_color"][1].value_or(0.0f);
			backgroundColor[2] = this->customConfig["engine"]["render"]["opengl"]["background_color"][2].value_or(0.0f);
			backgroundColor[3] = this->customConfig["engine"]["render"]["opengl"]["background_color"][3].value_or(0.0f);
			
			bool createDefaultCamera = this->customConfig["engine"]["render"]["camera"]["create_default_camera"].value_or(false);
			bool setDefaultCameraAsMain = this->customConfig["engine"]["render"]["camera"]["set_default_camera_as_main"].value_or(true);
			
			if (createDefaultCamera) {
				auto newCamera = this->_renderer->AddCamera();
				if (setDefaultCameraAsMain) {
					newCamera->mainCamera = true;
				}
			}

			this->_renderer->SetBackgroundColor(backgroundColor[0], backgroundColor[1], backgroundColor[2], backgroundColor[3]);
		
			this->debugLocalization.LoadFile(GetEnginePublicPath("debug_localization.csv", true));

			this->_imPlotContext = ImPlot::CreateContext();

			this->SetWindowIcon("WindowIcon");

			this->_renderer->ShowWindow();

			this->_input = std::make_shared<Input>(this->_renderer->GetWindow());
		}
		
		~Engine() {
			this->debugLocalization.Save();
			ImPlot::DestroyContext(this->_imPlotContext);
			this->_worldManager.Clear();
			this->_physicalEngine->Clear();
			this->_renderer->Clear();
		}
		
		void Exit() {
			this->exit = true;
		}
		
		bool* GetExit() {
			return &this->exit;
		}
		
		void Run() {
			while(this->_renderer->Valid() && !*this->GetExit()) {
				this->Update();
			}
		}

		void UpdateDebugUI() {
			if (ImGui::IsKeyPressed(ImGuiKey_F3)) {
				this->showDebugUI = !showDebugUI;
			}

			if (this->showDebugUI) {
				if(ImGui::Begin("Console")) {
					int index = 0;
					for (auto & line: logs) {
						ImGui::Text("%i - %s", index, line.c_str());
					}
				}
				ImGui::End();
				
				if (ImGui::Begin(this->debugLocalization.Get("Debug Tools").c_str(),
					NULL,
					ImGuiWindowFlags_MenuBar |
					ImGuiWindowFlags_NoSavedSettings
				)) {
						ImGui::BeginMenuBar();
							for (std::string language: *this->debugLocalization.GetAllLanguages()) {
								if (ImGui::Button(language.c_str())) {
									this->debugLocalization.GetLangIndex(language.c_str());
								}
							}
						ImGui::EndMenuBar();
						
						auto visualObjects = this->debugLocalization.Get("Visual Objects: ");
						ImGui::Text("%s%i", visualObjects.c_str(), this->_renderer->GetVisualObjectsCount());
						auto lights = this->debugLocalization.Get("Lights: ");
						ImGui::Text("%s%i", lights.c_str(), this->_renderer->GetLightCount());
						auto frameRate = this->debugLocalization.Get("Frame rate: ");
						ImGui::Text("%s%i", frameRate.c_str(), (int)glm::floor(1.0f / this->_renderer->GetDeltaTime()));

						if (ImGui::Button(this->debugLocalization.Get("Frame Rate Graph").c_str())) {
							this->showFrameRateGraph = !this->showFrameRateGraph;
						}

						if (this->showFrameRateGraph) {
							auto graphName = this->debugLocalization.Get("Frame Rate Graph");
							auto x = this->debugLocalization.Get("Time");
							auto y = this->debugLocalization.Get("Frame Rate");
							auto data = this->debugLocalization.Get("Frame per second");

						    if (ImPlot::BeginPlot(graphName.c_str(), x.c_str(), y.c_str())) {
							    ImPlot::PlotBars(data.c_str(), this->frameRateLogs.data(), this->frameRateLogs.size());
							    ImPlot::EndPlot();
							}
						}
				}
				ImGui::End();
			}
		}

		void UpdateUtilShortcut() {
			if (this->_input->GetKeyDown(KeyCode::F11)) {
            	this->_renderer->SetFullscreen(!this->_renderer->GetFullscreen());
        	}
		}
		
		void Update() {
			auto worlds = this->_worldManager.GetWorlds();
			this->_renderer->UpdateIO();
			if (this->_renderer->WindowActive()) {
				for (auto & currentWorld: worlds) {
					if (currentWorld != nullptr) {
						currentWorld->CallFunctionProcesses();
						
						currentWorld->simulationCollider.position = this->_renderer->GetCurrentCamera()->position;
						currentWorld->Update();
					}
				}
				
				if (this->_physicsEnabled) {
					this->_physicalEngine->SetStepTime(this->_renderer->GetDeltaTime());
					this->_physicalEngine->Simulate();
				}

				this->_renderer->StartUIRendering();
				this->UpdateDebugUI();
				for (auto & currentWorld: worlds) {
					if (currentWorld != nullptr) {
						currentWorld->CallRenderFunctions();
					}
				}

				double currentTime = this->_renderer->GetTime();
				if (lastRenderClearTime + renderClearCoolDown < currentTime) {
					this->_renderer->Clear();
				}

				if (lastFrameRateLog + frameRateCoolDown < currentTime && !this->showFrameRateGraph) {
					this->frameRateLogs.push_back(this->_renderer->GetFPS());
					this->frameRateTimeLogs.push_back(currentTime);
				}

				this->_renderer->Draw();
				this->_renderer->Show();
			}
			for (auto & currentWorld: worlds) {
				if (currentWorld != nullptr) {
					currentWorld->AlwayUpdate();
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
		
		std::shared_ptr<AudioEngine> GetAudioEngine() {
			return this->_audioEngine;
		}

		std::shared_ptr<PhysicalEngine> GetPhysicalEngine() {
			return this->_physicalEngine;
		}

		std::shared_ptr<Renderer> GetRenderer() {
			return this->_renderer;
		}

		void SetupWorlds() {
			auto worlds = this->_worldManager.GetWorlds();
			for (auto & currentWorld: worlds) {
				currentWorld->physicalEngine = this->_physicalEngine;
				currentWorld->audioEngine = this->_audioEngine;
				currentWorld->renderer = this->_renderer; 
				currentWorld->input = _input;

				currentWorld->UpdateLinks();
			}
		}

		/// Proper way to remove the current world
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
					texture = this->GetRenderer()->AddTextureFromData(
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

					this->_renderer->SetWindowIcon(decoded.data(), imgWidth, imgHeight);

					return;
				}
				iter++;
			}
		}

		PrettyEngine::WorldManager* GetWorldManager() {
			return &this->_worldManager;
		}
		
	private:
		std::shared_ptr<DataBase> engineDatabase;

		PrettyEngine::WorldManager _worldManager;

		bool exit = false;

		Localization debugLocalization;
		bool showDebugUI = false;

		bool _physicsEnabled = true;

	private:
		std::shared_ptr<AudioEngine> _audioEngine = std::make_shared<AudioEngine>();
		std::shared_ptr<PhysicalEngine> _physicalEngine = std::make_shared<PhysicalEngine>();
		std::shared_ptr<Renderer> _renderer = std::make_shared<Renderer>();
		std::shared_ptr<Input> _input = nullptr;

		toml::parse_result customConfig;

		double lastRenderClearTime = 0.0f;
		double renderClearCoolDown = 5.0f;

		bool showFrameRateGraph = false;
		double frameRateCoolDown = 5.0f;
		double lastFrameRateLog = 0.0f;
		std::vector<int> frameRateLogs;
		std::vector<int> frameRateTimeLogs;

		ImPlotContext* _imPlotContext;
	};
};
