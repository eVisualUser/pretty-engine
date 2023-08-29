#pragma once

#include <PrettyEngine/audio.hpp>
#include <PrettyEngine/physics.hpp>
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
			this->engineDatabase = new DataBase(engineDataBaseFile);

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
		
			this->debugLocalization.LoadFile("public/debug_localization.csv");
			this->debugLanguage = this->debugLocalization.GetLangIndex("English");

			this->_imPlotContext = ImPlot::CreateContext();
		}

		~Engine() {
			ImPlot::DestroyContext(this->_imPlotContext);

			delete this->_physicalEngine;
			delete this->_audioEngine;
			delete this->_renderer;
			delete this->engineDatabase;
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

			if (this->_currentWorld != nullptr) {
				this->_currentWorld->Clear();
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
				
				if (ImGui::Begin(this->debugLocalization.Get("Debug Tools", this->debugLanguage).c_str(),
					NULL,
					ImGuiWindowFlags_MenuBar |
					ImGuiWindowFlags_NoSavedSettings
				)) {
						ImGui::BeginMenuBar();
							for (std::string language: *this->debugLocalization.GetAllLanguages()) {
								if (ImGui::Button(language.c_str())) {
									this->debugLanguage = this->debugLocalization.GetLangIndex(language.c_str());
								}
							}
						ImGui::EndMenuBar();
						
						auto visualObjects = this->debugLocalization.Get("Visual Objects: ", this->debugLanguage);
						ImGui::Text("%s%i", visualObjects.c_str(), this->_renderer->GetVisualObjectsCount());
						auto lights = this->debugLocalization.Get("Lights: ", this->debugLanguage);
						ImGui::Text("%s%i", lights.c_str(), this->_renderer->GetLightCount());
						auto frameRate = this->debugLocalization.Get("Frame rate: ", this->debugLanguage);
						ImGui::Text("%s%i", frameRate.c_str(), (int)glm::floor(1.0f / this->_renderer->GetDeltaTime()));

						if (ImGui::Button(this->debugLocalization.Get("Frame Rate Graph", this->debugLanguage).c_str())) {
							this->showFrameRateGraph = !this->showFrameRateGraph;
						}

						if (this->showFrameRateGraph) {
							auto graphName = this->debugLocalization.Get("Frame Rate Graph", this->debugLanguage);
							auto x = this->debugLocalization.Get("Time", this->debugLanguage);
							auto y = this->debugLocalization.Get("Frame Rate", this->debugLanguage);
							auto data = this->debugLocalization.Get("Frame per second", this->debugLanguage);

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
			if (this->_renderer->WindowActive()) {
				if (this->_currentWorld != nullptr) {
					this->_currentWorld->CallFunctionProcesses();
				
					this->_currentWorld->simulationCollider.position = this->_renderer->GetCurrentCamera()->position;
					this->_currentWorld->StartUpdateMT();
					this->_currentWorld->Update();
					this->_currentWorld->AlwayUpdate();
				}

				this->_renderer->UpdateIO();

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


				if (this->_currentWorld != nullptr) {
					this->_currentWorld->WaitUpdateMT();
					this->_currentWorld->EndUpdate();
					this->_currentWorld->AlwayUpdate();
				}
			}
		}
		
		AudioEngine* GetAudioEngine() {
			return this->_audioEngine;
		}

		PhysicalEngine* GetPhysicalEngine() {
			return this->_physicalEngine;
		}

		Renderer* GetRenderer() {
			return this->_renderer;
		}

		void SetCurrentWorld(World* newWorld) {
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

		/// Proper way to remove the current world
		void RemoveCurrentWorld() {
			this->_currentWorld = nullptr;
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
		
	private:
		DataBase* engineDatabase;

		World* _currentWorld = nullptr;

		bool exit = false;

		Localization debugLocalization;
		bool showDebugUI = false;
		int debugLanguage;

		bool _physicsEnabled = true;

	private:
		AudioEngine* _audioEngine = new AudioEngine();
		PhysicalEngine* _physicalEngine = new PhysicalEngine();
		Renderer *_renderer = new Renderer();

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
