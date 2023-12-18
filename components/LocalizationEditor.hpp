#pragma once

#include <PrettyEngine/debug/debug.hpp>
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
					ImGui::Checkbox("Save on close", &this->saveLoclizationOnClose);
					if (ImGui::Button("Save")) {
						if (this->currentLocalization.get() != nullptr) {
							this->currentLocalization->Save();
							DebugLog(LOG_INFO, "Save localization", false);
						}
					}
					ImGui::Separator();					
					ImGui::InputText("Path", this->localizationFileBuffer, 100);
					std::string path = GetEnginePublicPath(this->localizationFileBuffer, true);
					if (FileExist(path) && ImGui::Button("Open File")) {
						this->currentLocalization.reset();
						this->currentLocalization = std::make_shared<Localization>();
						this->currentLocalization->LoadFile(path);
					}

					if (!FileExist(path) && ImGui::Button("Create File")) {
						CreateFile(path);
					}
					
					if (this->currentLocalization != nullptr) {
						for(auto & index: this->localizationsToRemove) {
							DebugLog(LOG_DEBUG, "To Remove: " << index, false);
							this->currentLocalization->RemoveLocalizationUsingIndex(index);
						}
						this->localizationsToRemove.clear();

						if (ImGui::Button("New")) {
							this->currentLocalization->CreateLocalization();
						}
						
						auto content = this->currentLocalization->GetRawContent();
						
						auto languages = this->currentLocalization->GetAllLanguages();
						
						if(ImGui::BeginTable("Localization Table", languages->size() + 1)) {
							ImGui::TableSetupColumn("Key");
							for (auto & lang: *languages) {
								ImGui::TableSetupColumn(lang.c_str());
							}
							ImGui::TableHeadersRow();

							int lineIndex = 0;
							for (auto &line : *content) {
								if (lineIndex > 0) {
									ImGui::TableNextRow();
									int columnIndex = 0;
									for (auto &column : line) {
										ImGui::TableNextColumn();
										char buffer[1000] = "";
										strcpy_s(buffer, column.c_str());

										ImGui::InputTextMultiline((std::to_string(columnIndex) + '.' + std::to_string(lineIndex)).c_str(), buffer, 1000);

										column = buffer;

										columnIndex++;
									}

									for (int i = columnIndex; i < languages->size() + 1; i++) {
										std::string addButtonName = "+ ";
										addButtonName += line.front();
										if (ImGui::Button(addButtonName.c_str())) {
											line.push_back("");
										}
									}

									std::string removeButtonName = "Remove ";
									removeButtonName += line.front();
									if (ImGui::Button(removeButtonName.c_str())) {
										this->localizationsToRemove.push_back(lineIndex);
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
		}

		void Toggle() {
			this->active = !this->active;
		}

	public:
		bool active = false;

		std::shared_ptr<Localization> currentLocalization;

		char localizationFileBuffer[100];

		bool saveLoclizationOnClose = false;

		std::vector<int> localizationsToRemove;
	};
}
