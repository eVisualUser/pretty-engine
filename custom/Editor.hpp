#pragma once

#include "PrettyEngine/engine.hpp"
#include <Guid.hpp>
#include <PrettyEngine/debug.hpp>
#include <PrettyEngine/localization.hpp>
#include <PrettyEngine/utils.hpp>
#include <PrettyEngine/entity.hpp>
#include <PrettyEngine/render.hpp>
#include <LocalizationEditor.hpp>

#include <cstring>
#include <imgui.h>
#include <string>
#include <memory>

using namespace PrettyEngine;

namespace Custom {
	class Editor: public virtual Entity {
	public:
		void OnStart() override {
			this->localization = std::make_shared<Localization>();
			this->localization->LoadFile(GetEnginePublicPath("editor.csv", true));

			this->localizationEditorPtr = this->GetComponentAs<LocalizationEditor>("LocalizationEditor");
			this->localizationEditorPtr->localization = this->localization;
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
					this->localizationEditorPtr->Toggle();
				}
			}
			ImGui::End();
		}

		void OnDestroy() override {
			this->localization->Save();
		}

	private:
		LocalizationEditor* localizationEditorPtr;

		std::string file = "game.toml";

		std::shared_ptr<Localization> localization;
	};
}
