#ifndef H_INTERN_EDITOR
#define H_INTERN_EDITOR

#include <PrettyEngine/EngineContent.hpp>
#include <PrettyEngine/editor/PropertyEditor.hpp>
#include <PrettyEngine/Input.hpp>
#include <PrettyEngine/PhysicalSpace.hpp>
#include <PrettyEngine/debug/debug.hpp>
#include <PrettyEngine/entity.hpp>
#include <PrettyEngine/render/render.hpp>
#include <PrettyEngine/transform.hpp>
#include <PrettyEngine/utils.hpp>
#include <PrettyEngine/world.hpp>
#include <PrettyEngine/worldLoad.hpp>

// Generated files
#include <components.hpp>
#include <custom.hpp>
#include <PropertyEditor.h>

#include <functional>
#include <imgui.h>
#include <memory>
#include <string>
#include <filesystem>

namespace PrettyEngine {
/// Builtin editor
class Editor {
  public:
	Editor() {
		// Load the list of all components
		auto componentListFile = GetEnginePublicPath("../../components/list.csv", true);
		if (FileExist(componentListFile)) {
			auto fileContent = ReadFileToString(componentListFile);
			this->existingComponents = ParseCSVLine(fileContent);
		}

		// Load the list of all entities
		auto entitiesListFile = GetEnginePublicPath("../../entities/list.csv", true);
		if (FileExist(entitiesListFile)) {
			auto fileContent = ReadFileToString(entitiesListFile);
			this->existingEntities = ParseCSVLine(fileContent);
		}

		this->_propertyEditorList = GeneratePropertyEditorList();
	}

	static void ImGuiInputString(const char* label, std::string* input) {
		char buffer[100];
		strcpy(buffer, input->c_str());
		ImGui::InputText(label, buffer, 100);
		*input = buffer;
	}

	void ShowWorldDebugInfo(Renderer *renderer) {
		if (ImGui::Begin("Debug Tools", nullptr, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoSavedSettings)) {
			ImGui::Text("Visual Objects : %i", renderer->GetVisualObjectsCount());
			ImGui::Text("Lights: %i", renderer->GetLightCount());
		}

		ImGui::End();
	}

	void ShowTransform(Transform *transform) {
		std::string headerName = "Transform: " + transform->GetObjectSerializedName();
		if (ImGui::CollapsingHeader(headerName.c_str())) {
			ImGui::InputFloat("Pos X", &transform->position.x);
			ImGui::InputFloat("Pos Y", &transform->position.y);
			ImGui::InputFloat("Pos Z", &transform->position.z);

			auto rotationEuler = transform->GetEulerRotation();
			rotationEuler = glm::degrees(rotationEuler);
			ImGui::InputFloat("Rot X", &rotationEuler.x);
			ImGui::InputFloat("Rot Y", &rotationEuler.y);
			ImGui::InputFloat("Rot Z", &rotationEuler.z);
			rotationEuler = glm::radians(rotationEuler);
			transform->SetRotationUsingEuler(rotationEuler);

			glm::vec3 scale = transform->scale;
			ImGui::InputFloat("Scale X", &scale.x);
			ImGui::InputFloat("Scale Y", &scale.y);
			ImGui::InputFloat("Scale Z", &scale.z);
			transform->SetScale(scale);
		}
	}

	void ShowManualFunctionsCalls(DynamicObject *dynamicObject) {
		for (auto & publicFunction: dynamicObject->publicFuncions) {
			std::string buttonName = "";
 			buttonName += publicFunction.first + " -> ";
			buttonName += dynamicObject->serialObjectUnique;

 			if (ImGui::Button(buttonName.c_str())) {
				publicFunction.second();
 			}
		}
	}

	void ShowCreateNewEntity(std::shared_ptr<World> world) {
		for (auto &entity : this->existingEntities) {
			std::string buttonName = "Add Entity: ";
			buttonName += entity;
			if (ImGui::Button(buttonName.c_str())) {	
					world->RegisterEntity(CreateCustomEntity(entity));
    			int entitiesWithSameName = 0;
    			for(auto & worldEntity: world->entities) {
					if (worldEntity.second->serialObjectUnique == entity) {
						entitiesWithSameName++;
     				}
    			}
					world->GetLastEntityRegistred()->serialObjectUnique = entity;
					world->GetLastEntityRegistred()->serialObjectName = entity;
    			if (entitiesWithSameName > 0) {
					world->GetLastEntityRegistred()->serialObjectUnique += "_" + std::to_string(entitiesWithSameName);
    			}
			}
		}
	}

	void ShowToolScripts() {
		if (ImGui::Begin("Tool Scripts")) {
			for (auto &entry : std::filesystem::recursive_directory_iterator(GetEnginePublicPath("../../tools", true))) {
				if (!entry.is_directory() && entry.path().extension() == ".go") {
					std::string buttonName = "Run: ";
					buttonName += entry.path().filename().string();
					if (ImGui::Button(buttonName.c_str())) {
						std::string command;
						command += "go run ";
						command += '\"' + entry.path().string() + '\"';
						DebugLog(LOG_INFO, "Execute script: " << entry.path().parent_path(), true);
						if (system(command.c_str())) {
							DebugLog(LOG_ERROR, "Script failed", true);
						} else {
							DebugLog(LOG_INFO, "Script succeed", true);
						}
					}
				}
			}
		}
		ImGui::End();
	}

	void ShowRenderDebugger(Renderer *renderer) {
		if (ImGui::Begin("Render Debugger")) {
			if (ImGui::CollapsingHeader("Camera Debugger")) {
				for (auto &camera : renderer->cameraList) {
					ImGui::Text("Camera: %lli", camera.id);

					ImGui::InputFloat3("Position", &camera.position[0]);
					ImGui::Text("Rotation: %f;%f;%f", camera.rotation.x, camera.rotation.y, camera.rotation.z);
					std::string renderToTexture = std::to_string(camera.id);
					renderToTexture += " Render to texture state";

					bool renderToTextureState = camera.renderToTexture;
					ImGui::Checkbox(renderToTexture.c_str(), &renderToTextureState);
					if (renderToTextureState != camera.renderToTexture) {
						camera.SetRenderToTexture(renderToTextureState);
					}

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

			if (ImGui::CollapsingHeader("VisualObjects")) {
				int layerIndex = 0;
				for (auto &layer : renderer->visualObjects) {
					std::string headerTitle = "Layer ";
					headerTitle += std::to_string(layerIndex);
					if (ImGui::CollapsingHeader(headerTitle.c_str())) {
						for (auto &visualObject : layer) {
							ImGui::Text("VisualObject: %s", visualObject.first.c_str());
						}
					}
					layerIndex++;
				}
			}

			if (ImGui::CollapsingHeader("Textures")) {
				for (auto & texture : renderer->glTextures) {
					if (ImGui::BeginTable("TextureTable", 1)) {
						ImGui::TableHeader("Texture");
						ImGui::TableNextRow();
						ImGui::TableNextColumn();
						if (ImGui::CollapsingHeader(texture.first.c_str())) {
							ImTextureID imguiTextureId = (ImTextureID)(intptr_t)texture.second.textureID;
							ImGui::Image(imguiTextureId, ImVec2(100, 100));
							ImGui::SameLine();
							ImGui::Text("User count: %i", texture.second.userCount);
						}
					}
					ImGui::EndTable();
				}
			}
		}
		ImGui::End();
	}

	void ShowSelectedEntities() {
		int index = 0;
		for (auto &selectedEntity : this->selectedEntities) {
			if (ImGui::Begin(selectedEntity->GetGUID().c_str(), nullptr, ImGuiWindowFlags_MenuBar)) {
				if (ImGui::BeginMenuBar()) {
					if (ImGui::Button("Close")) {
						this->selectedEntities.erase(this->selectedEntities.begin() + index);
						ImGui::EndMenuBar();
						ImGui::End();
						return;
					}
				}
				ImGui::EndMenuBar();

				ImGui::Text("Object: %s", selectedEntity->GetObjectSerializedName().c_str());
				this->ShowManualFunctionsCalls(selectedEntity);

				char entityNameBuffer[100];
				strcpy(entityNameBuffer, selectedEntity->serialObjectUnique.c_str());
				ImGui::InputText("Entity Unique", entityNameBuffer, 100);
				selectedEntity->serialObjectUnique = entityNameBuffer;

				for (auto &serializedField : selectedEntity->serialFields) {
					for (auto &propertyEditor : this->_propertyEditorList) {
						propertyEditor->Edit(&serializedField);
					}
				}

				ImGui::Separator();

				this->ShowTransform(selectedEntity);

				int componentIndex = 0;
				for (auto & component : selectedEntity->components) {
					std::string headerName = "Component: ";
					headerName += component->serialObjectName;
					headerName += " " + std::to_string(componentIndex);

					if (ImGui::CollapsingHeader(headerName.c_str())) {
						// Unique attribute edit
						{
							char componentUniqueBuffer[100];
							strcpy(componentUniqueBuffer, component->serialObjectUnique.c_str());

							std::string inputTextName = component->serialObjectName + " Unique " + std::to_string(componentIndex);

							ImGui::InputText(inputTextName.c_str(), componentUniqueBuffer, 100);
							component->serialObjectUnique = componentUniqueBuffer;
						}

						this->ShowManualFunctionsCalls(component.get());

						std::string removeButtonName = "Remove: ";
						removeButtonName += component->serialObjectUnique;
						if (ImGui::Button(removeButtonName.c_str())) {
							selectedEntity->components.erase(selectedEntity->components.begin() + componentIndex);
							break;
						}

						for (auto &publicElement : component->publicMap) {
							char buffer[100];
							strcpy_s(buffer, publicElement.second.c_str());
							ImGui::InputText(publicElement.first.c_str(), buffer, 100);
							component->SetPublicVarValue(publicElement.first, buffer);
						}
						
						for (auto &serializedField : component->serialFields) {
							for (auto &propertyEditor : this->_propertyEditorList) {
								propertyEditor->Edit(&serializedField);
							}
						}
						ImGui::Separator();
					}
					componentIndex++;
				}

				if (ImGui::CollapsingHeader("Add component")) {
					for (auto &componentName : this->existingComponents) {
						std::string buttonText = "Add " + componentName;
						if (ImGui::Button(buttonText.c_str())) {
							int componentNameCount = 0;
							std::string newComponentName = componentName;

							for (auto &component : selectedEntity->components) {
								if (!component->serialObjectUnique.empty() && component->serialObjectUnique.starts_with(componentName)) {
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
								customComponent->serialObjectUnique = newComponentName;
								customComponent->OnSetup();
								customComponent->owner = selectedEntity;
								customComponent->serialObjectName = componentName;
								customComponent->SetObjectSerializedName(componentName);
								customComponent->SetSerializedUnique(newComponentName);

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
			Editor::ImGuiInputString("New world file path:", &this->worldFileBuffer);

			const auto realWorldPath = GetEnginePublicPath(this->worldFileBuffer, true);
			if (!FileExist(realWorldPath)) {
				Editor::ImGuiInputString("New world name:", &this->worldNameBuffer);
				if (ImGui::Button("Create a new world")) {
					if (CreateFile(realWorldPath)) {
						DebugLog(LOG_DEBUG, "Created new world: " << realWorldPath, false);
						worldManager->AddWorld(realWorldPath, this->worldNameBuffer)->Load();

						return ImGui::End();
					} else {
						DebugLog(LOG_DEBUG, "Failed to create file: " << realWorldPath, true);
					}
				}
			} else if (ImGui::Button("Load")) {
				DebugLog(LOG_DEBUG, "Load world: " << realWorldPath, false);
				worldManager->AddWorld(realWorldPath, this->worldNameBuffer)->Load();
				return ImGui::End();
			}
			
			if (ImGui::BeginTabBar("Worlds", ImGuiTabBarFlags_AutoSelectNewTabs)) {
				// Iterate the worlds in the world manager
				for (auto & world : *worldManager->GetWorlds()) {
					// Show enities table
					if (ImGui::BeginTabItem(world->worldName.c_str())) {
						Editor::ImGuiInputString("World Name:", &world->worldName);

						if (ImGui::Button("Unload")) {
							worldManager->RemoveWorld(world->worldName);
							ImGui::EndTabItem();
							break;
						} else if (ImGui::Button("Save & Unload")) {
							world->Save();
							worldManager->RemoveWorld(world->worldName);
							ImGui::EndTabItem();
							break;
						} else if (ImGui::Button("Save")) {
							world->Save();
						}
						
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
								ImGui::Text("%s", entity.second->serialObjectName.c_str());
								ImGui::TableNextColumn();
								ImGui::Text("%lli", entity.second->components.size());
								ImGui::TableNextColumn();
								
								std::string buttonName = "Select " + entity.second->GetGUID();
								
								if (ImGui::Button(buttonName.c_str())) {
									this->selectedEntities.push_back(entity.second.get());
								}

    						std::string buttonRemove = "Remove " + entity.second->entityName;
    						if (ImGui::Button(buttonRemove.c_str())) {
									world->entities.erase(entity.second->GetGUID());
     							this->selectedEntities.clear();
     							break;
      					}
							}
						}
						ImGui::EndTable();
						ImGui::EndTabItem();
					}
				}
			}
			ImGui::EndTabBar();
		}
		ImGui::End();
	}

	void ShowCollisionDebugger(EngineContent* engineContent) const {
		if (ImGui::Begin("Collision Debugger")) {
			for(auto & layer: *engineContent->physicalSpace.GetAllLayers()) {
				if (ImGui::CollapsingHeader(layer.first.c_str())) {
					for(auto & collider: layer.second) {
						if (ImGui::CollapsingHeader(collider->name.c_str())) {
							if (const auto collisions = engineContent->physicalSpace.GetCollisions(collider)) {
								for(auto & collision: *collisions) {
									ImGui::Text("%s", collision.colliderSource->name.c_str());
									ImGui::SameLine();
									ImGui::Text(" > ");
									ImGui::SameLine();
									ImGui::Text("%s", collision.colliderOther->name.c_str());
									if (ImGui::CollapsingHeader((collision.colliderSource->name + "Mesh").c_str())) {
										if (collision.colliderSource->mesh != nullptr) {
											int index = 0;

											for(auto & vertex: collision.colliderSource->mesh->vertices) {
												auto transformedVertex = collision.colliderSource->TransformPosition(vertex.position);
												ImGui::InputFloat3((collision.colliderSource->name + "Vertex[" + std::to_string(index) + "]").c_str(), &transformedVertex[0]);
												index++;
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
		ImGui::End();
	}

	bool Update(EngineContent* engineContent, WorldManager* worldManager, bool *isEditor) {
		auto changedState = false;
		if (worldManager != nullptr) {
			if (isEditor != nullptr) {
				if (ImGui::Begin("Play Mode")) {
					ImGui::Checkbox("Reload on Play", &this->reloadOnPlay);
					if (ImGui::IsItemHovered()) {
						ImGui::SetTooltip("It can cause more loading time to play but provide a more accurate result, avoiding editor artefacts.");
					}

					ImGui::Checkbox("Save a copy on stop", &this->saveCopyWhenStop);
					if (ImGui::IsItemHovered()) {
						ImGui::SetTooltip("Save a copy of the worlds to allow testing later on them.");
					}

					if (*isEditor && ImGui::Button("Play")) {
						DebugLog(LOG_DEBUG, "[EDITOR] -> Play", false);

						worldManager->SaveWorlds();

						if (this->reloadOnPlay) {
							this->selectedEntities.clear();

							worldManager->Reload();
						}

						changedState = true;
					} else	if (!*isEditor && ImGui::Button("Stop")) {
						DebugLog(LOG_DEBUG, "[EDITOR] -> Stop", false);

						if (this->saveCopyWhenStop) {
							int index = 0;
							for(auto & world: *worldManager->GetWorlds()) {
								auto timeAsString = GetTimeAsString();
								StringReplaceChar(&timeAsString, ':', '_');
								auto filePath = GetEnginePublicPath(this->worldCopyDir, true) + "/" + world->worldName + "/" + std::move(timeAsString) + "(" + std::to_string(index) + ")" + ".toml";
								StringReplaceChar(&filePath, ' ', '_');
								if (CreateFile(filePath)) {
									world->Save(WorldSavingFormat::Toml, filePath);
								} else {
									DebugLog(LOG_DEBUG, "Failed to save copy of world:  " << world->worldName << " to " << filePath, true);
								}
								index++;
							}
						}

			    		this->selectedEntities.clear();

						worldManager->Reload();

						changedState = true;
					}
				}					
			}

			ImGui::End();

			this->ShowWorldDebugInfo(&engineContent->renderer);
			this->ShowWorldEditor(worldManager);
			this->ShowSelectedEntities();
			this->ShowRenderDebugger(&engineContent->renderer);
			this->ShowToolScripts();
			this->ShowCollisionDebugger(engineContent);
		} else {
			DebugLog(LOG_ERROR, "Nullptr of WorldManager", true);
			std::exit(-1);
		}
	
		return changedState;
	}

  private:
		char textInputworldToLoad[100];

		std::string worldFileBuffer = "/worlds/new_world.toml";
		std::string worldNameBuffer = "NewWorld";

		std::vector<Entity*> selectedEntities;

		std::vector<std::string> existingComponents;
		std::vector<std::string> existingEntities;

		std::vector<std::shared_ptr<PropertyEditor>> _propertyEditorList;

		bool createComponent = false;
		bool reloadOnPlay = false;
		bool saveCopyWhenStop = false;

		std::string worldCopyDir = "../WorldsCopies";
};
} // namespace PrettyEngine

#endif