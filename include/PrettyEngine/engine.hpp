#pragma once

#include "PrettyEngine/utils.hpp"
#include <PrettyEngine/audio.hpp>
#include <PrettyEngine/physics.hpp>
#include <PrettyEngine/physicsEngine.hpp>
#include <PrettyEngine/debug.hpp>
#include <PrettyEngine/render.hpp>
#include <PrettyEngine/world.hpp>
#include <PrettyEngine/localization.hpp>
#include <PrettyEngine/data.hpp>
#include <PrettyEngine/texture.hpp>

#include <memory>
#include <sstream>
#include <toml++/toml.h>

#include <imgui.h>
#include <implot.h>

#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

#define CONSOLE_COMMAND_BUFFER_SIZE 50

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
			bool setDefaultCameraAsMain = this->customConfig["engine"]["render"]["camera"]["set_default_camera_as_main"].value_or(false);
			
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
		}
		
		~Engine() {
			this->debugLocalization.Save();
			ImPlot::DestroyContext(this->_imPlotContext);
			this->RemoveCurrentWorld();
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

		void Update() {	
			this->_renderer->UpdateIO();
			if (this->_renderer->WindowActive()) {
				if (this->_currentWorld != nullptr) {
					this->_currentWorld->CallFunctionProcesses();
				
					this->_currentWorld->simulationCollider.position = this->_renderer->GetCurrentCamera()->position;
					this->_currentWorld->Update();
				}
				
				if (this->_physicsEnabled) {
					this->_physicalEngine->SetStepTime(this->_renderer->GetDeltaTime());
					this->_physicalEngine->Simulate();
				}

				this->_renderer->StartUIRendering();
				this->UpdateDebugUI();
				if (this->_currentWorld != nullptr) {
					this->_currentWorld->CallRenderFunctions();
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
			if (this->_currentWorld != nullptr) {
				this->_currentWorld->AlwayUpdate();
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

		void SetCurrentWorld(std::shared_ptr<World> newWorld) {
			if (this->_currentWorld != nullptr) {
				this->RemoveCurrentWorld();
			}

			newWorld->physicalEngine = this->_physicalEngine;
			newWorld->audioEngine = this->_audioEngine;
			newWorld->renderer = this->_renderer; 
			newWorld->engine = this;

			this->_currentWorld = newWorld;
			this->_currentWorld->UpdateLinks();
		}

		std::shared_ptr<World> GetCurrentWorld() {
			return this->_currentWorld;
		}
		
		/// Proper way to remove the current world
		void RemoveCurrentWorld() {
			if (this->_currentWorld != nullptr) {
				this->_currentWorld->Clear();
				this->_currentWorld = nullptr;
			}
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
		
	private:
		std::shared_ptr<DataBase> engineDatabase;

		std::shared_ptr<World> _currentWorld = nullptr;

		bool exit = false;

		Localization debugLocalization;
		bool showDebugUI = false;

		bool _physicsEnabled = true;

	private:
		std::shared_ptr<AudioEngine> _audioEngine = std::make_shared<AudioEngine>();
		std::shared_ptr<PhysicalEngine> _physicalEngine = std::make_shared<PhysicalEngine>();
		std::shared_ptr<Renderer> _renderer = std::make_shared<Renderer>();

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
	
	static Engine* GetEngine(DynamicObject* object) {
		return static_cast<Engine*>(object->engine);
	}

	static Renderer* GetRenderer(DynamicObject* object) {
		return static_cast<Renderer*>(object->engine);
	}
};
