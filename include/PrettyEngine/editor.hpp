#pragma once

#include "PrettyEngine/debug.hpp"
#include <PrettyEngine/entity.hpp>
#include <PrettyEngine/utils.hpp>
#include <PrettyEngine/worldLoad.hpp>
#include <PrettyEngine/world.hpp>
#include <PrettyEngine/Input.hpp>
#include <PrettyEngine/PhysicalSpace.hpp>
#include <PrettyEngine/render.hpp>

#include <imgui.h>
#include <implot.h>

namespace PrettyEngine {
	/// Builtin editor
	class Editor {
	public:
		Editor() {
			this->debugLocalization.LoadFile(GetEnginePublicPath("debug_localization.csv", true));
		}

		~Editor() {
			this->debugLocalization.Save();
		}

		void Update(WorldManager* worldManager, Input* input, Renderer* renderer, PhysicalSpace* physicalSpace, std::vector<int>* frameRateLogs, std::vector<int>* frameRateTimeLogs) {
			if (ImGui::Begin("Input Debugger")) {
				ImGui::Text("Mouse scroll delta: %f", input->GetMouseWheelDelta());

				auto keyWatchers = input->GetKeyWatchers();
				if (ImGui::BeginTable("Key Stats", 3) && !keyWatchers->empty()) {
					ImGui::TableSetupColumn("Name");
					ImGui::TableSetupColumn("Mode");
					ImGui::TableSetupColumn("State");
					
					ImGui::TableHeadersRow();
					
					for (auto & key: *keyWatchers) {
						ImGui::TableNextRow();
						ImGui::TableNextColumn();

						auto keyName = key->name;

						ImGui::Text("%s", keyName.c_str());
						
						ImGui::TableNextColumn();
						std::string keyMode = KeyWatcherModeToString(key->mode);

						if (ImGui::Button(keyMode.c_str())) {
							if (keyMode == "Down") {
								key->mode = KeyWatcherMode::Press;
							} else if (keyMode == "Press") {
								key->mode = KeyWatcherMode::Up;
							} else if (keyMode == "Up") {
								key->mode = KeyWatcherMode::Down;
							}
						}

						ImGui::TableNextColumn();
						ImGui::Checkbox((keyName + " State").c_str(), &key->state);
					}
				}
				ImGui::EndTable();
			}
			ImGui::End();
			
			if(ImGui::Begin("Console")) {
				int index = 0;
				for (auto & line: logs) {
					ImVec4 color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);

					if (line.type == LOG_DEBUG) {
						color = ImVec4(2.29f, 2.04f, 0.16f, 1.0f);
					} else if (line.type == LOG_WARNING) {
						color = ImVec4(2.47f, 1.42f, 4.0f, 1.0f);
					} else if (line.type == LOG_ERROR) {
						color = ImVec4(2.52f, 0.58f, 0.58f, 1.0f);
					} else if (line.type == LOG_INFO) {
						color = ImVec4(0.10f, 0.78f, 2.52f, 1.0f);
					}

					ImGui::TextColored(color, "%i: %s", index, line.log.c_str());
					index++;
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
				ImGui::Text("%s%i", visualObjects.c_str(), renderer->GetVisualObjectsCount());
				auto lights = this->debugLocalization.Get("Lights: ");
				ImGui::Text("%s%i", lights.c_str(), renderer->GetLightCount());
				
				auto currentRenderTime = glfwGetTime();
				if (!frameRateLogs->empty()) {
					auto frameRate = this->debugLocalization.Get("Frame rate: ");
					ImGui::Text("%s%i", frameRate.c_str(), frameRateLogs->back());

					if (ImGui::Button(this->debugLocalization.Get("Frame Rate Graph").c_str())) {
						this->showFrameRateGraph = !this->showFrameRateGraph;
					}

					if (this->showFrameRateGraph) {
						auto graphName = this->debugLocalization.Get("Frame Rate Graph");
						auto x = this->debugLocalization.Get("Time");
						auto y = this->debugLocalization.Get("Frame Rate");
						auto data = this->debugLocalization.Get("Frame per second");

					    if (ImPlot::BeginPlot(graphName.c_str(), x.c_str(), y.c_str())) {
						    ImPlot::PlotBars(data.c_str(), frameRateLogs->data(), frameRateLogs->size(), 0.1f);
						    ImPlot::EndPlot();
						}
					}

					if (frameRateLogs->size() > 10000) {
						frameRateLogs->clear();
						frameRateTimeLogs->clear();
					}
				}
			}
			ImGui::End();

			if (ImGui::Begin("World Editor")) {
				if (ImGui::BeginTabBar("Worlds")) {
					for(auto & world: worldManager->GetWorlds()) {
						if (ImGui::BeginTabItem(world->worldName.c_str())) {
							ImGui::Text("Entities: %lli", world->entities.size());
							if (ImGui::BeginTable("Entities", 4)) {
								ImGui::TableSetupColumn("Name");
								ImGui::TableSetupColumn("Object");
								ImGui::TableSetupColumn("Components");
								ImGui::TableSetupColumn("Select");
								ImGui::TableHeadersRow();

								for(auto & entity: world->entities) {
									ImGui::TableNextRow();
									ImGui::TableNextColumn();
									ImGui::Text("%s", entity.second->entityName.c_str());
									ImGui::TableNextColumn();
									ImGui::Text("%s", entity.second->object.c_str());
									ImGui::TableNextColumn();
									ImGui::Text("%lli", entity.second->components.size());
									ImGui::TableNextColumn();
									std::string buttonName = "Select " + entity.second->entityName;
									if (ImGui::Button(buttonName.c_str())) {
										this->selectedEntity = entity.second.get();
									}
								}
							}
							ImGui::EndTable();
							if (this->selectedEntity != nullptr) {
								if (ImGui::Begin(this->selectedEntity->entityName.c_str())) {
									ImGui::Text("Object: %s", this->selectedEntity->object.c_str());
									ImGui::Separator();
									
									ImGui::InputFloat("Pos X", &this->selectedEntity->position.x);
									ImGui::InputFloat("Pos Y", &this->selectedEntity->position.y);
									ImGui::InputFloat("Pos Z", &this->selectedEntity->position.z);

									auto rotationEuler = this->selectedEntity->GetEulerRotation();
									ImGui::InputFloat("Rot X", &rotationEuler.x);
									ImGui::InputFloat("Rot Y", &rotationEuler.y);
									ImGui::InputFloat("Rot Z", &rotationEuler.z);
									this->selectedEntity->SetRotationUsingEuler(rotationEuler);

									ImGui::InputFloat("Scale X", &this->selectedEntity->scale.x);
									ImGui::InputFloat("Scale Y", &this->selectedEntity->scale.y);
									ImGui::InputFloat("Scale Z", &this->selectedEntity->scale.z);
								}
								ImGui::End();
							}
						}
						ImGui::EndTabItem();
					}
				}
				ImGui::EndTabBar();
			}
			ImGui::End();
		}

	private:
		char textInputworldToLoad[100];

		Localization debugLocalization;

		bool showFrameRateGraph = false;

		Entity* selectedEntity = nullptr;
	};
}
