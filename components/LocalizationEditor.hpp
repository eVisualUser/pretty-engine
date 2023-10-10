#pragma once

#include <PrettyEngine/debug.hpp>
#include <PrettyEngine/entity.hpp>
#include <PrettyEngine/utils.hpp>
#include <PrettyEngine/localization.hpp>

#include <memory>

namespace Custom {
	using namespace PrettyEngine;

	class LocalizationEditor: public PrettyEngine::Component {
	public:
		void OnRender() override {
			if (this->active) {
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
					if (FileExist(path) && ImGui::Button(this->localization->Get("Open File").c_str())) {
						this->currentLocalization.reset();
						this->currentLocalization = std::make_shared<Localization>();
						this->currentLocalization->LoadFile(path);
					}

					if (!FileExist(path) && ImGui::Button(this->localization->Get("Create File").c_str())) {
						CreateFile(path);
					}
					
					if (this->currentLocalization.get() != nullptr) {
						for(auto & index: this->localizationsToRemove) {
							this->currentLocalization->RemoveLocalizationUsingIndex(index);
						}
						this->localizationsToRemove.clear();

						if (ImGui::Button(this->localization->Get("New").c_str())) {
							this->currentLocalization->CreateLocalization();
						}
						
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
									std::string addButtonName = "+ ";
									addButtonName += line.front();
									if (ImGui::Button(addButtonName.c_str())) {
										line.push_back("");
									}
								}

								std::string removeButtonName = "- ";
								removeButtonName += line.front();
								if (ImGui::Button(removeButtonName.c_str())) {
									this->localizationsToRemove.push_back(columnIndex);
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
		}

		void Toggle() {
			this->active = !this->active;
		}

	public:
		bool active = false;

		std::shared_ptr<Localization> localization;		

		std::shared_ptr<Localization> currentLocalization;

		char localizationFileBuffer[100];

		bool saveLoclizationOnClose = false;

		std::vector<int> localizationsToRemove;
	};
}
