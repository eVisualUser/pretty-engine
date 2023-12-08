#ifndef H_INTERN_EDITOR
#define H_INTERN_EDITOR

#include "PrettyEngine/audio.hpp"
#include "imgui.h"
#include <PrettyEngine/Input.hpp>
#include <PrettyEngine/PhysicalSpace.hpp>
#include <PrettyEngine/debug.hpp>
#include <PrettyEngine/entity.hpp>
#include <PrettyEngine/render.hpp>
#include <PrettyEngine/transform.hpp>
#include <PrettyEngine/utils.hpp>
#include <PrettyEngine/world.hpp>
#include <PrettyEngine/worldLoad.hpp>
#include <components.hpp>
#include <custom.hpp>

#include <functional>
#include <imgui.h>
#include <implot.h>
#include <memory>
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

		// Load the list of all entities
		auto entitiesListFile = GetEnginePublicPath("../../entities/list.csv");
		if (FileExist(entitiesListFile)) {
			auto fileContent = ReadFileToString(entitiesListFile);
			this->existingEntities = ParseCSVLine(fileContent);
		}
	}

	~Editor() { this->debugLocalization.Save(); }

	void ShowWorldDebugInfo(Renderer *renderer) {
		if (ImGui::Begin(this->debugLocalization.Get("Debug Tools").c_str(), NULL, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoSavedSettings)) {
			ImGui::BeginMenuBar();
			for (std::string language : *this->debugLocalization.GetAllLanguages()) {
				if (ImGui::Button(language.c_str())) {
					this->debugLocalization.GetLangIndex(language.c_str());
				}
			}
			ImGui::EndMenuBar();

			auto visualObjects = this->debugLocalization.Get("Visual Objects: ");
			ImGui::Text("%s%i", visualObjects.c_str(), renderer->GetVisualObjectsCount());
			auto lights = this->debugLocalization.Get("Lights: ");
			ImGui::Text("%s%i", lights.c_str(), renderer->GetLightCount());
		}
		ImGui::End();
	}

	void ShowTransform(Transform *transform) {
		ImGui::InputFloat("Pos X", &transform->position.x);
		ImGui::InputFloat("Pos Y", &transform->position.y);
		ImGui::InputFloat("Pos Z", &transform->position.z);

		auto rotationEuler = transform->GetEulerRotation();
		ImGui::InputFloat("Rot X", &rotationEuler.x);
		ImGui::InputFloat("Rot Y", &rotationEuler.y);
		ImGui::InputFloat("Rot Z", &rotationEuler.z);
		transform->SetRotationUsingEuler(rotationEuler);

		glm::vec3 scale = transform->scale;
		ImGui::InputFloat("Scale X", &scale.x);
		ImGui::InputFloat("Scale Y", &scale.y);
		ImGui::InputFloat("Scale Z", &scale.z);
		transform->SetScale(scale);
	}

	void ShowManualFunctionsCalls(DynamicObject *dynamicObject) {
		std::string onStartButtonName = "Call OnStart: ";
		onStartButtonName += dynamicObject->unique;
		std::string onUpdateButtonName = "Call OnUpdate: ";
		onUpdateButtonName += dynamicObject->unique;
		std::string onRenderButtonName = "Call OnRender: ";
		onRenderButtonName += dynamicObject->unique;

		if (ImGui::Button(onStartButtonName.c_str())) {
			dynamicObject->OnStart();
		} else if (ImGui::Button(onUpdateButtonName.c_str())) {
			dynamicObject->OnUpdate();
		} else if (ImGui::Button(onRenderButtonName.c_str())) {
			dynamicObject->OnRender();
		}
	}

	void ShowCreateNewEntity(std::shared_ptr<World> world) {
		for (auto &entity : this->existingEntities) {
			std::string buttonName = "Add Entity: ";
			buttonName += entity;
			if (ImGui::Button(buttonName.c_str())) {
				CreateCustomEntity(entity, world);
			}
		}
	}

	void ShowRenderDebugger(Renderer *renderer) {
		if (ImGui::Begin("Render Debugger")) {
			if (ImGui::CollapsingHeader("Camera Debugger")) {
				for (auto &camera : renderer->cameraList) {
					ImGui::Text("Camera: %lli", camera.id);
					ImGui::Text("Position: %f;%f;%f", camera.position.x, camera.position.y, camera.position.z);
					ImGui::Text("Rotation: %f;%f;%f", camera.rotation.x, camera.rotation.y, camera.rotation.z);
					std::string renderToTexture = std::to_string(camera.id);
					renderToTexture += " Render to texture state";
					ImGui::Checkbox(renderToTexture.c_str(), &camera.renderToTexture);
					std::string active = std::to_string(camera.id);
					active += " Active State";
					ImGui::Checkbox(active.c_str(), &camera.active);
					ImGui::Separator();
				}
			}

			if (ImGui::CollapsingHeader("Content counter")) {
				ImGui::LabelText("Light count: ", "%lli", renderer->lights.size());
				ImGui::LabelText("VisualObject count: ", "%lli", renderer->visualObjects.size());
				ImGui::LabelText("Light count: ", "%lli", renderer->lights.size());
				ImGui::LabelText("Texture count: ", "%lli", renderer->glTextures.size());
				ImGui::LabelText("Mesh count: ", "%lli", renderer->glMeshList.size());
			}
		}
		ImGui::End();
	}

	void ShowSelectedEntities() {
		for (auto &selectedEntity : this->selectedEntities) {
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
				this->ShowManualFunctionsCalls(selectedEntity);

				ImGui::Separator();

				this->ShowTransform(selectedEntity);

				int componentIndex = 0;
				for (auto &component : selectedEntity->components) {
					std::string headerName = "Component: ";
					headerName += component->unique;

					if (ImGui::CollapsingHeader(headerName.c_str())) {
						ImGui::LabelText("Unique: ", "%s", component->unique.c_str());
						this->ShowManualFunctionsCalls(component.get());

						if (ImGui::Button("Remove")) {
							selectedEntity->components.erase(selectedEntity->components.begin() + componentIndex);
							break;
						}

						for (auto &publicElement : component->publicMap) {
							publicElement.second.resize(100);
							ImGui::InputText(publicElement.first.c_str(), publicElement.second.data(), 100);
							// Remove bad chars generated by string resize
							while (!publicElement.second.empty() && publicElement.second.back() == '\u0000') {
								publicElement.second.pop_back();
							}
						}
						ImGui::Separator();
						componentIndex++;
					}
				}

				if (ImGui::Button("Add Component")) {
					this->createComponent = !this->createComponent;
				}

				if (this->createComponent) {
					for (auto &componentName : this->existingComponents) {
						std::string buttonText = "Add " + componentName;
						if (ImGui::Button(buttonText.c_str())) {
							int componentNameCount = 0;
							std::string newComponentName = componentName;

							for (auto &component : selectedEntity->components) {
								if (!component->unique.empty() && component->unique.starts_with(componentName)) {
									componentNameCount++;
								}
							}

							if (componentNameCount > 0) {
								newComponentName += "(" + std::to_string(componentNameCount) + ")";
							}

							auto customComponent = GetCustomComponent(componentName);
							if (customComponent == nullptr) {
								DebugLog(LOG_ERROR, "Failed to get custom component: " << componentName, true);
							} else {
								customComponent->unique = newComponentName;
								customComponent->OnUpdatePublicVariables();
								customComponent->owner = selectedEntity;
								customComponent->object = componentName;

								selectedEntity->components.push_back(customComponent);
							}
						}
					}
				}
			}
			ImGui::End();
			index++;
		}
	}

	void ShowWorldEditor(WorldManager *worldManager) {
		if (ImGui::Begin("World Editor")) {
			if (ImGui::BeginTabBar("Worlds")) {
				// Iterate the worlds in the world manager
				for (auto &world : worldManager->GetWorlds()) {
					// Show enities table
					if (ImGui::BeginTabItem(world->worldName.c_str())) {
						this->ShowCreateNewEntity(world);
						ImGui::NewLine();
						ImGui::Text("Entities: %lli", world->entities.size());
						if (ImGui::BeginTable("Entities", 4)) {
							ImGui::TableSetupColumn("Name");
							ImGui::TableSetupColumn("Object");
							ImGui::TableSetupColumn("Components");
							ImGui::TableSetupColumn("Select");
							ImGui::TableHeadersRow();

							// Show entities infos
							for (auto &entity : world->entities) {
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
					}
					ImGui::EndTabItem();
				}
			}
			ImGui::EndTabBar();
		}
		ImGui::End();
	}

	void Update(WorldManager *worldManager, Input *input, Renderer *renderer, PhysicalSpace *physicalSpace, bool *isEditor, AudioEngine *audioEngine) {
		auto changedState = false;
		if (ImGui::Begin("Play Mode")) {
			if (*isEditor && ImGui::Button("Play")) {
				worldManager->SaveWorlds();
				*isEditor = false;
				changedState = true;
			}
			ImGui::SameLine();
			if (!*isEditor && ImGui::Button("Stop")) {
				renderer->visualObjects.clear();
				physicalSpace->Clear();
				audioEngine->Clear();
				input->Clear();
				renderer->Clear(true);
    			this->selectedEntities.clear();

				worldManager->Reload();

				*isEditor = true;
				changedState = true;
			}
		}
		ImGui::End();
		if (changedState) {
			return;
		}

		this->ShowWorldDebugInfo(renderer);
		this->ShowWorldEditor(worldManager);
		this->ShowSelectedEntities();
		this->ShowRenderDebugger(renderer);
	}

  private:
	char textInputworldToLoad[100];

	Localization debugLocalization;

	std::vector<Entity *> selectedEntities;

	std::vector<std::string> existingComponents;
	std::vector<std::string> existingEntities;

	bool createComponent = false;
};
} // namespace PrettyEngine

#endif