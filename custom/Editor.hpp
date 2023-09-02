#pragma once

#include <PrettyEngine/entity.hpp>
#include <PrettyEngine/render.hpp>

#include <imgui.h>

using namespace PrettyEngine;

namespace Custom {
	class Editor: public virtual Entity {
	public:
		void OnUpdate() override {
			if (this->renderer->GetKeyPress(KeyCode::LeftControl) && this->renderer->GetKeyDown(KeyCode::S)) {
				DebugLog(LOG_DEBUG, "Save to file: " << this->file, false);
			}
		}
		
		void OnRender() override {
			if (ImGui::Begin("Project Info")) {
				ImGui::Text("Welcome to the PrettyEditor !");
				ImGui::Separator();
				ImGui::Text("Some utility keys: ");
				ImGui::BulletText("Press F3 to show debug informations.");
				ImGui::BulletText("Press F11 to toggle fullscreen.");
				ImGui::BulletText("Press Ctr + S to save.");
			}
			ImGui::End();
		}

		void OnDestroy() override {
			
		}

	private:
		std::string file = "public/game.toml";
	};
}