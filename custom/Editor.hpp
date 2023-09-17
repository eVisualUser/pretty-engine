#pragma once

#include "Guid.hpp"
#include "PrettyEngine/debug.hpp"
#include <PrettyEngine/localization.hpp>
#include <PrettyEngine/utils.hpp>
#include <PrettyEngine/entity.hpp>
#include <PrettyEngine/render.hpp>

#include <cstring>
#include <imgui.h>
#include <string>

using namespace PrettyEngine;

namespace Custom {
	class Editor: public virtual Entity {
	public:
		void OnStart() override {
			this->localization = std::make_shared<Localization>();
			this->localization->LoadFile(GetEnginePublicPath("editor.csv", true));
		}

		void OnUpdate() override {
			if (this->renderer->GetKeyPress(KeyCode::LeftControl) && this->renderer->GetKeyDown(KeyCode::S)) {
				DebugLog(LOG_DEBUG, "Save to file: " << this->file, false);
			}
		}

		void OnAlwaysUpdate() override {
			if (!this->renderer->GetWindowFocus()) {
				this->renderer->SetWindowOpacity(0.1f);
			} else {
				this->renderer->SetWindowOpacity(1.0f);
			}
		}
		
		void OnRender() override {
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
					this->localizationEditor = !this->localizationEditor;
				}
			}
			ImGui::End();

			if (this->localizationEditor) {
				if (ImGui::Begin("Localization Editor")) {
					ImGui::Checkbox(this->localization->Get("Save on close").c_str(), &this->saveLoclizationOnClose);
					if (ImGui::Button(this->localization->Get("Save").c_str())) {
						if (this->currentLocalization.get() != nullptr) {
							this->currentLocalization->Save();
							DebugLog(LOG_INFO, "Save localization", false);
						}
					}
					ImGui::Separator();					
					ImGui::InputText(this->localization->Get("Path").c_str(), this->localizationFileBuffer, 100);
					std::string path = GetEnginePublicPath(this->localizationFileBuffer, true);
					if (ImGui::Button(this->localization->Get("Open File").c_str())) {
						this->currentLocalization.reset();
						this->currentLocalization = std::make_shared<Localization>();
						if (FileExist(path)) {
							this->currentLocalization->LoadFile(path);
						} else {
							DebugLog(LOG_WARNING, "File not found: " << path, true);
						}
					}

					if (ImGui::Button(this->localization->Get("Create File").c_str())) {
						CreateFile(path);
					}

					if (this->currentLocalization.get() != nullptr) {
						auto content = this->currentLocalization->GetRawContent();

						auto languages = this->currentLocalization->GetAllLanguages();
						
						if(ImGui::BeginTable(this->localization->Get("Localization Table").c_str(), languages->size())) {
							for (auto & lang: *languages) {
								ImGui::TableSetupColumn(lang.c_str());
							}
							ImGui::TableHeadersRow();

							int lineIndex = 0;
 							for (auto & line: *content) {
								ImGui::TableNextRow();
								int columnIndex = 0;
								for (auto & column: line) {
									ImGui::TableNextColumn();
									char buffer[1000] = "";
									std::strcpy(buffer, column.c_str());
									
									ImGui::InputTextMultiline((std::to_string(columnIndex) + std::to_string(lineIndex)).c_str(), buffer, 1000);
									
									column = buffer;
									
									columnIndex++;
								}
								
								for(int i = columnIndex; i < languages->size(); i++) {
									std::string buttonName = "Add for ";
									buttonName += line.front();
									if (ImGui::Button(buttonName.c_str())) {
										line.push_back("");
									}
								}
								
								lineIndex++;
							}
						}
						ImGui::EndTable();
					}
				}
				ImGui::End();
			}
		}

		void OnDestroy() override {
			if (this->saveLoclizationOnClose) {
				this->currentLocalization->Save();	
			}
			this->currentLocalization.reset();
			this->localization->Save();
		}

	private:
		std::string file = "game.toml";

		bool localizationEditor = false;

		std::shared_ptr<Localization> localization;

		std::shared_ptr<Localization> currentLocalization;

		char localizationFileBuffer[100];

		bool saveLoclizationOnClose = false;
	};
}
