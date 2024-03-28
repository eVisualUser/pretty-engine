#ifndef H_EDITOR
#define H_EDITOR

// Components
#include <Render.hpp>
#include <LocalizationEditor.hpp>
#include <Physical.hpp>

#include <Guid.hpp>
#include <cstring>
#include <imgui.h>

#include <string>
#include <unordered_map>
#include <utility>

#include <PrettyEngine/KeyCode.hpp>
#include <PrettyEngine/render/PrettyGL.hpp>
#include <PrettyEngine/render/camera.hpp>
#include <PrettyEngine/debug/debug.hpp>
#include <PrettyEngine/dynamicObject.hpp>
#include <PrettyEngine/entity.hpp>
#include <PrettyEngine/localization.hpp>
#include <PrettyEngine/render/mesh.hpp>
#include <PrettyEngine/render/render.hpp>
#include <PrettyEngine/shaders.hpp>
#include <PrettyEngine/render/texture.hpp>
#include <PrettyEngine/utils.hpp>
#include <PrettyEngine/render/visualObject.hpp>
#include <PrettyEngine/event.hpp>
#include <PrettyEngine/command.hpp>

using namespace PrettyEngine;

namespace Custom {
class Editor : public virtual Entity {
  public:
    void OnSetup() override {
        this->_cameraSpeed.DeserializeValue();
        this->_renderToTexture.DeserializeValue();
    }

    void OnEditorStart() override {
    	this->CreateTextureCamera();

        keyUp.name = "EditorKeyUp";
        keyUp.key = KeyCode::UpArrow;
        keyUp.mode = KeyWatcherMode::Press;
        this->engineContent->input.AddKeyWatcher(&keyUp);
        keyUp.actionOnKey = [this]{
            this->position.y += this->_speed * this->engineContent->renderer.GetDeltaTime();
        };

        keyDown.name = "EditorKeyDown";
        keyDown.key = KeyCode::DownArrow;
        keyDown.mode = KeyWatcherMode::Press;
        this->engineContent->input.AddKeyWatcher(&keyDown);
        keyDown.actionOnKey = [this]{
            this->position.y -= this->_speed * this->engineContent->renderer.GetDeltaTime();
        };

        keyLeft.name = "EditorKeyLeft";
        keyLeft.key = KeyCode::LeftArrow;
        keyLeft.mode = KeyWatcherMode::Press;
        this->engineContent->input.AddKeyWatcher(&keyLeft);
        keyLeft.actionOnKey = [this]{
            this->position.x -= this->_speed * this->engineContent->renderer.GetDeltaTime();
        };

        keyRight.name = "EditorKeyRight";
        keyRight.key = KeyCode::RightArrow;
        keyRight.mode = KeyWatcherMode::Press;
        this->engineContent->input.AddKeyWatcher(&keyRight);
        keyRight.actionOnKey = [this]{
            this->position.x += this->_speed * this->engineContent->renderer.GetDeltaTime();
        };
    }

    void OnDestroy() override {
        this->engineContent->input.RemoveKeyWatcher(&keyUp);
        this->engineContent->input.RemoveKeyWatcher(&keyDown);
        this->engineContent->input.RemoveKeyWatcher(&keyLeft);
        this->engineContent->input.RemoveKeyWatcher(&keyRight);

    	this->engineContent->renderer.RemoveCamera(this->secondCamera);
    }

    void MenuBar() {
        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::Button("Input Debugger")) {
                this->inputDebugger = !this->inputDebugger;
            }

            if (ImGui::Button("Console")) {
                this->showConsole = !this->showConsole;
            }

            if (ImGui::Button("Help")) {
                this->editorGuide = !this->editorGuide;
            }

            if (ImGui::Button("To-Do")) {
                this->todoEditor = !this->todoEditor;
            }
        }
        ImGui::EndMainMenuBar();
    }

    void OnUpdate() override {
    #if ENGINE_EDITOR
        this->_cameraSpeed.DeserializeValue();
    #endif
    }

	void OnStart() override {
#if ENGINE_EDITOR
	    this->engineContent->input.RemoveKeyWatcher(&keyUp);
	    this->engineContent->input.RemoveKeyWatcher(&keyDown);
	    this->engineContent->input.RemoveKeyWatcher(&keyLeft);
	    this->engineContent->input.RemoveKeyWatcher(&keyRight);

        this->_cameraSpeed.DeserializeValue();
        this->_renderToTexture.DeserializeValue();
#endif

    	this->CreateTextureCamera();
	}

	void CreateTextureCamera() {
        if (this->engineContent->renderer.cameraList.size() <= 1) {
        	this->secondCamera = this->engineContent->renderer.AddCamera();
        } else {
            this->secondCamera = &this->engineContent->renderer.cameraList.back();
        }

        if (this->secondCamera != nullptr) {
            this->secondCamera->active = true;
            this->secondCamera->SetTextureResolution(glm::vec2(1920, 1080));
            this->secondCamera->SetRenderToTexture(true);
        }
    }

    void OnRender() override {
    	if ((*this->_renderToTexture.Get()) && this->secondCamera != nullptr) {
    		auto render = *this->GetComponentAs<Render>("Render").Resolve([this](Render** render) {
				this->AddComponent<Render>("Render");
				return true;
			})->GetValue();
    		if (render != nullptr) {
    			render->GetVisualObject()->RemoveTexture(TextureType::Base);
    			this->secondCamera->GetTexture()->textureType = TextureType::Base;
    			render->GetVisualObject()->AddTexture(this->secondCamera->GetTexture());
    		} else {
    			DebugLog(LOG_ERROR, "Failed to get Render component", false);
    		}
    	}

        this->MenuBar();

        this->cameraZ += this->engineContent->renderer.GetDeltaTime() * this->engineContent->input.GetMouseWheelDelta() * (*this->_cameraSpeed.Get());
        
        this->engineContent->renderer.GetCurrentCamera()->position.z = this->cameraZ;
        this->engineContent->renderer.GetCurrentCamera()->position.x = -this->position.x;
        this->engineContent->renderer.GetCurrentCamera()->position.y = -this->position.y;

        if (this->todoEditor) {
            if (ImGui::Begin("To-Do")) {
                ImGui::InputText("File: ", this->toDoFile, 100);
                ImGui::SameLine();
                auto realPath = GetEnginePublicPath(this->toDoFile, true);
                if (FileExist(realPath) && ImGui::Button("Load")) {
                    this->todoList.clear();
                    auto parsed = toml::parse_file(realPath);
                    if (parsed["todo"] != NULL) {
                        for (auto &element : *parsed["todo"].as_table()) {
                            std::string idName = element.first.str().data();
                            bool value = element.second.value_or(false);
                            this->todoList.insert_or_assign(idName, value);
                        }
                    }
                } else if (!FileExist(realPath) && ImGui::Button("Create")) {
                    CreateFile(realPath);
                }
                
                if (FileExist(realPath) && ImGui::Button("Save")) {
                    auto parsed = toml::parse_file(realPath);
                    if (!parsed.contains("todo")) {
                        toml::table newTable;
                        parsed.insert("todo", newTable);
                    }
                    auto table = parsed["todo"].as_table();
                    table->clear();
                    for (auto &element : this->todoList) {
                        table->insert_or_assign(element.first, element.second);
                    }
                    std::ofstream out;
                    out.open(realPath);
                    if (out.is_open()) {
                        out << parsed;
                        out.flush();
                    } else {
                        DebugLog(LOG_ERROR, "Failed to open: " << this->toDoFile, true);
                    }
                    out.close();
                }

                if (!this->todoList.empty()) {
                    if (ImGui::BeginTable("ToDoEditor", 2)) {
                        ImGui::TableSetupColumn("Name");
                        ImGui::TableSetupColumn("State");
                        ImGui::TableHeadersRow();

                        for (auto &element : this->todoList) {
                            ImGui::TableNextRow();
                            ImGui::TableNextColumn();
                            ImGui::LabelText("", "%s", element.first.c_str());
                            ImGui::SameLine();
                            if (ImGui::Button("Remove")) {
                                this->todoList.erase(element.first);
                                break;
                            }
                            ImGui::TableNextColumn();
                            ImGui::Checkbox("", &element.second);
                        }
                    }
                    ImGui::EndTable();
                }
                ImGui::InputText("New Element", this->toDoAdd, 100);
                if (!this->todoList.contains(this->toDoAdd) &&
                    ImGui::Button("Add")) {
                    this->todoList.insert(std::make_pair(this->toDoAdd, false));
                }
            }
            ImGui::End();
        }

        if (this->engineContent->input.GetKeyPress(KeyCode::LeftControl) &&
            this->engineContent->input.GetKeyDown(KeyCode::S)) {
            DebugLog(LOG_DEBUG, "Save World", false);
			Event saveEvent; 
			saveEvent.AddTag("save");
			this->engineContent->eventManager.SendEvent(&saveEvent);
        }

        if (this->editorGuide) {
            if (ImGui::Begin("Editor Guide")) {
                ImGui::Text("%s", "Welcome to the PrettyEditor !");
                ImGui::Separator();
                ImGui::Text("%s", "Some shortcut: ");
                ImGui::BulletText("%s", "Press F3 to show debug informations.");
                ImGui::BulletText("%s", "Press F11 to toggle fullscreen.");
                ImGui::BulletText("%s", "Press Ctr + S to save.");

                ImGui::Separator();
                ImGui::Text("Tools");
            }
            ImGui::End();
        }

        if (this->engineContent->input.GetMouseButtonClick(1)) {
            actionBox = !actionBox;
            auto cursorPosition = this->engineContent->input.GetCursorPosition();
            actionBoxStartPos = ImVec2(cursorPosition.x, cursorPosition.y);
        }

        if (actionBox) {
            ImGui::SetNextWindowPos(actionBoxStartPos);
            if (ImGui::Begin("actionBox", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove)) {
                if (ImGui::Button("Save Editor")) {
                    this->requests.push_back(Request::SAVE);
                }

                if (ImGui::Button("Console")) {
                    this->showConsole = !showConsole;
                }
            }
            ImGui::End();
        }

        if (this->showConsole) {
            if (ImGui::Begin("Console")) {
                int index = 0;
                for (auto &line : logs) {
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

                ImGui::InputText("Command", this->consoleCommandBuffer, 100);
                if (ImGui::Button("Execute")) {
                    std::string cmd = this->consoleCommandBuffer;
					PrettyEngine::CommandSystem::Execute(cmd);
                }
            }
            ImGui::End();
        }

        if (this->inputDebugger) {
            if (ImGui::Begin("Input Debugger")) {
                auto keyWatchers = this->engineContent->input.GetKeyWatchers();
                if (ImGui::BeginTable("Key Stats", 3) &&
                    !keyWatchers->empty()) {
                    ImGui::TableSetupColumn("Name");
                    ImGui::TableSetupColumn("Mode");
                    ImGui::TableSetupColumn("State");

                    ImGui::TableHeadersRow();

                    for (auto &key : *keyWatchers) {
                        ImGui::TableNextRow();
                        ImGui::TableNextColumn();

                        std::string keyName = key->name;

                        ImGui::Text("%s", keyName.c_str());

                        ImGui::TableNextColumn();
                        std::string keyMode = KeyWatcherModeToString(key->mode);

                        if (ImGui::Button(
                                (keyMode + ' ' + key->name).c_str())) {
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
        }
    }

  private:
    PublicProperty<float> _cameraSpeed = PUBLIC_FLOAT("Camera Speed", 10.0f);
    PublicProperty<bool> _renderToTexture = PUBLIC_BOOL("Render To Texture", false);

    bool actionBox = false;
    ImVec2 actionBoxStartPos;

    float _speed = 3.0f;

    KeyWatcher keyUp;
    KeyWatcher keyDown;
    KeyWatcher keyLeft;
    KeyWatcher keyRight;

    Camera *secondCamera = nullptr;

    bool showConsole = false;
    char consoleCommandBuffer[100] = "";

    bool inputDebugger = false;

    bool editorGuide = true;

    float cameraZ = -10.0f;

    bool todoEditor = false;
 
    char toDoFile[100];
    char toDoAdd[100];
    std::unordered_map<std::string, bool> todoList;
};
} // namespace Custom

#endif