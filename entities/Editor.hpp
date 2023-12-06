#ifndef H_EDITOR
#define H_EDITOR

#include <Physical.hpp>
#include <PrettyEngine/KeyCode.hpp>
#include <PrettyEngine/PrettyGL.hpp>
#include <PrettyEngine/camera.hpp>
#include <PrettyEngine/debug.hpp>
#include <PrettyEngine/dynamicObject.hpp>
#include <PrettyEngine/entity.hpp>
#include <PrettyEngine/localization.hpp>
#include <PrettyEngine/mesh.hpp>
#include <PrettyEngine/render.hpp>
#include <PrettyEngine/shaders.hpp>
#include <PrettyEngine/texture.hpp>
#include <PrettyEngine/utils.hpp>
#include <PrettyEngine/visualObject.hpp>

#include <Render.hpp>

// Components
#include <LocalizationEditor.hpp>

#include <Guid.hpp>
#include <cstring>
#include <imgui.h>
#include <toml++/toml.h>

#include <memory>
#include <string>
#include <unordered_map>
#include <utility>

using namespace PrettyEngine;

namespace Custom {
class Editor : public virtual Entity {
  public:
    void OnEditorStart() override {
        this->localization = std::make_shared<Localization>();
        this->localization->LoadFile(GetEnginePublicPath("editor.csv", true));

        this->localizationEditorPtr =
            this->GetComponentAs<LocalizationEditor>("LocalizationEditor");
        this->localizationEditorPtr->localization = this->localization;

        // Default camera position (to be able to see the render)
        this->engineContent->renderer.GetCurrentCamera()->position.z = -1.0f;

        keyUp.name = "EditorKeyUp";
        keyUp.key = KeyCode::UpArrow;
        keyUp.mode = KeyWatcherMode::Press;
        this->engineContent->input.AddKeyWatcher(&keyUp);

        keyDown.name = "EditorKeyDown";
        keyDown.key = KeyCode::DownArrow;
        keyDown.mode = KeyWatcherMode::Press;
        this->engineContent->input.AddKeyWatcher(&keyDown);

        keyLeft.name = "EditorKeyLeft";
        keyLeft.key = KeyCode::LeftArrow;
        keyLeft.mode = KeyWatcherMode::Press;
        this->engineContent->input.AddKeyWatcher(&keyLeft);

        keyRight.name = "EditorKeyRight";
        keyRight.key = KeyCode::RightArrow;
        keyRight.mode = KeyWatcherMode::Press;
        this->engineContent->input.AddKeyWatcher(&keyRight);

        // Render to texture
        // this->secondCamera = this->engineContent->renderer.AddCamera();
        // this->secondCamera->active = true;
        // this->secondCamera->SetRenderToTexture(false);

        // this->visualObject->textures.clear();
        // this->visualObject->AddTexture(this->secondCamera->GetTexture());
    }

    void OnDestroy() override {
        this->engineContent->input.RemoveKeyWatcher(&keyUp);
        this->engineContent->input.RemoveKeyWatcher(&keyDown);
        this->engineContent->input.RemoveKeyWatcher(&keyLeft);
        this->engineContent->input.RemoveKeyWatcher(&keyRight);
    }

    void OnAlwaysUpdate() override {
        if (!this->engineContent->renderer.GetWindowFocus()) {
            this->engineContent->renderer.SetWindowOpacity(0.5f);
        } else {
            this->engineContent->renderer.SetWindowOpacity(1.0f);
        }
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

    void OnRender() override {
        this->MenuBar();

        this->cameraZ += this->engineContent->input.GetMouseWheelDelta() * this->engineContent->renderer.GetDeltaTime() * 200.0f;

        this->engineContent->renderer.GetCurrentCamera()->position.z = -this->cameraZ;
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
                        DebugLog(LOG_ERROR,
                                 "Failed to open: " << this->toDoFile, true);
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
            DebugLog(LOG_DEBUG, "Save to file: " << this->file, false);
            this->requests.push_back(Request::SAVE);
        }

        if (this->keyUp.state) {
            this->position.y +=
                this->_speed * this->engineContent->renderer.GetDeltaTime();
        } else if (this->keyDown.state) {
            this->position.y -=
                this->_speed * this->engineContent->renderer.GetDeltaTime();
        }

        if (this->keyLeft.state) {
            this->position.x -= this->_speed * this->engineContent->renderer.GetDeltaTime();
        } else if (this->keyRight.state) {
            this->position.x += this->_speed * this->engineContent->renderer.GetDeltaTime();
        }

        if (this->editorGuide) {
            if (ImGui::Begin(this->localization->Get("Editor Guide").c_str())) {
                ImGui::Text("%s", this->localization->Get("Welcome to the PrettyEditor !").c_str());
                ImGui::Separator();
                ImGui::Text("%s", this->localization->Get("Some shortcut: ").c_str());
                ImGui::BulletText("%s", this->localization->Get("Press F3 to show debug informations.").c_str());
                ImGui::BulletText("%s", this->localization->Get("Press F11 to toggle fullscreen.").c_str());
                ImGui::BulletText("%s", this->localization->Get("Press Ctr + S to save.").c_str());

                ImGui::Separator();
                ImGui::Text("Tools");
                if (ImGui::Button("Localization Editor")) {
                    this->localizationEditorPtr->Toggle();
                }
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
            if (ImGui::Begin("actionBox", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove)) {
                if (ImGui::Button(this->localization->Get("Save Editor").c_str())) {
                    this->requests.push_back(Request::SAVE);
                }

                if (ImGui::Button(this->localization->Get("Save Editor localization").c_str())) {
                    this->localization->Save();
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
                ImGui::InputText("", this->consoleCommandBuffer, 100);
                if (ImGui::Button("Execute")) {
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

                        auto keyName = key->name;

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
    bool actionBox = false;
    ImVec2 actionBoxStartPos;

    LocalizationEditor *localizationEditorPtr;

    std::string file = "game.toml";

    std::shared_ptr<Localization> localization;

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

    float cameraZ = 0.0f;

    bool todoEditor = false;
 
    char toDoFile[100];
    char toDoAdd[100];
    std::unordered_map<std::string, bool> todoList;
};
} // namespace Custom

#endif