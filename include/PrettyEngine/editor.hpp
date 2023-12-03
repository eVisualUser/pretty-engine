#ifndef H_INTERN_EDITOR
#define H_INTERN_EDITOR

#include "PrettyEngine/localization.hpp"
#include "components.hpp"
#include <PrettyEngine/debug.hpp>
#include <PrettyEngine/entity.hpp>
#include <PrettyEngine/utils.hpp>
#include <PrettyEngine/worldLoad.hpp>
#include <PrettyEngine/world.hpp>
#include <PrettyEngine/Input.hpp>
#include <PrettyEngine/PhysicalSpace.hpp>
#include <PrettyEngine/render.hpp>

#include <imgui.h>
#include <implot.h>
#include <string>

namespace PrettyEngine {
	/// Builtin editor
	class Editor {
	public:
		Editor() {
			this->debugLocalization.LoadFile(GetEnginePublicPath("debug_localization.csv", true));

			// Load the list of all components
			auto componentListFile = GetEnginePublicPath("../../components/list.csv");
			if (FileExist(componentListFile)) {
				auto fileContent = ReadFileToString(componentListFile);
				this->existingComponents = ParseCSVLine(fileContent);
			}
		}

		~Editor() {
			this->debugLocalization.Save();
		}

		void Update(WorldManager* worldManager, Input* input, Renderer* renderer, PhysicalSpace* physicalSpace, std::vector<int>* frameRateLogs, std::vector<int>* frameRateTimeLogs) {			
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

					if (frameRateLogs->size() > 1000) {
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
										this->selectedEntities.push_back(entity.second.get());
									}
								}
							}
							ImGui::EndTable();
							for(auto & selectedEntity: this->selectedEntities) {
								int index = 0;
								if (ImGui::Begin(selectedEntity->entityName.c_str(), NULL, ImGuiWindowFlags_MenuBar)) {
									if (ImGui::BeginMenuBar()) {
										if (ImGui::Button("Close")) {
											this->selectedEntities.erase(this->selectedEntities.begin() + index);
											ImGui::EndMenuBar();
											ImGui::End();
											break;
										}
									}
									ImGui::EndMenuBar();

									ImGui::Text("Object: %s", selectedEntity->object.c_str());
									ImGui::Separator();
									
									ImGui::InputFloat("Pos X", &selectedEntity->position.x);
									ImGui::InputFloat("Pos Y", &selectedEntity->position.y);
									ImGui::InputFloat("Pos Z", &selectedEntity->position.z);

									auto rotationEuler = selectedEntity->GetEulerRotation();
									ImGui::InputFloat("Rot X", &rotationEuler.x);
									ImGui::InputFloat("Rot Y", &rotationEuler.y);
									ImGui::InputFloat("Rot Z", &rotationEuler.z);
									selectedEntity->SetRotationUsingEuler(rotationEuler);

									glm::vec3 scale = selectedEntity->scale;
									ImGui::InputFloat("Scale X", &scale.x);
									ImGui::InputFloat("Scale Y", &scale.y);
									ImGui::InputFloat("Scale Z", &scale.z);
									selectedEntity->SetScale(scale);

									int componentIndex = 0;
									for(auto & component: selectedEntity->components) {
										ImGui::LabelText("", "%s", component->unique.c_str());

										if (ImGui::Button("Remove")) {
											selectedEntity->components.erase(selectedEntity->components.begin() + componentIndex);
											break;
										}

										if (ImGui::Button("Call OnStart")) {
											component->OnStart();
										} else if (ImGui::Button("Call OnUpdate")) {
											component->OnUpdate();
										} else if (ImGui::Button("Call OnRender")) {
											component->OnRender();
										}

										for(auto & publicElement: component->publicMap) {
											publicElement.second.resize(100);
											ImGui::InputText(publicElement.first.c_str(), publicElement.second.data(), 100);
											while(publicElement.second.back() == '\u0000') {
												publicElement.second.pop_back();
											}
										}
										ImGui::Separator();
										componentIndex++;
									}

									if (ImGui::Button("Add Component")) {
										this->createComponent = !this->createComponent;
									}

									if (this->createComponent) {
										for(auto & componentName: this->existingComponents) {
											std::string buttonText = "Add " + componentName;
											if (ImGui::Button(buttonText.c_str())) {
												int componentNameCount = 0;

												std::string newComponentName = componentName;

												for(auto & component: selectedEntity->components) {
													if (component->unique.starts_with(componentName)) {
														componentNameCount++;
													}
												}

												if (componentNameCount > 0) {
													newComponentName += "(" + std::to_string(componentNameCount) + ")";
												}

												auto customComponent = GetCustomComponent(componentName);
												customComponent->unique = newComponentName;
												customComponent->OnUpdatePublicVariables();

												selectedEntity->components.push_back(customComponent);
											}
										}
									}
								}
								ImGui::End();
								index++;
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

		std::vector<Entity*> selectedEntities;

		std::vector<std::string> existingComponents;

		bool createComponent = false;
	};
}

#endif