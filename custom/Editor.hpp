#pragma once

#include <PrettyEngine/texture.hpp>
#include <PrettyEngine/mesh.hpp>
#include <PrettyEngine/shaders.hpp>
#include <PrettyEngine/visualObject.hpp>
#include <PrettyEngine/dynamicObject.hpp>
#include <PrettyEngine/debug.hpp>
#include <PrettyEngine/entity.hpp>
#include <PrettyEngine/localization.hpp>
#include <PrettyEngine/render.hpp>
#include <PrettyEngine/utils.hpp>
#include <PrettyEngine/gl.hpp>

// Components
#include <LocalizationEditor.hpp>

#include <Guid.hpp>
#include <imgui.h>

#include <cstring>
#include <memory>
#include <string>

using namespace PrettyEngine;

namespace Custom {
	class Editor : public virtual Entity {
	public:
	  void OnStart() override {
	    this->localization = std::make_shared<Localization>();
	    this->localization->LoadFile(GetEnginePublicPath("editor.csv", true));

	    this->localizationEditorPtr =
	    this->GetComponentAs<LocalizationEditor>("LocalizationEditor");
	    this->localizationEditorPtr->localization = this->localization;

	    this->renderer->GetCurrentCamera()->position.z = -1;
	  }

	  void OnUpdate() override {
	    if (this->input->GetKeyPress(KeyCode::LeftControl) &&
	        this->input->GetKeyDown(KeyCode::S)) {
	      	DebugLog(LOG_DEBUG, "Save to file: " << this->file, false);	  
	      	this->requests.push_back(Request::SAVE);    	
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
	      ImGui::Text(
	          "%s",
	          this->localization->Get("Welcome to the PrettyEditor !").c_str());
	      ImGui::Separator();
	      ImGui::Text("%s", this->localization->Get("Some shortcut: ").c_str());
	      ImGui::BulletText(
	          "%s", this->localization->Get("Press F3 to show debug informations.")
	                    .c_str());
	      ImGui::BulletText(
	          "%s",
	          this->localization->Get("Press F11 to toggle fullscreen.").c_str());
	      ImGui::BulletText(
	          "%s", this->localization->Get("Press Ctr + S to save.").c_str());

	      ImGui::Separator();
	      ImGui::Text("Tools");
	      if (ImGui::Button("Localization Editor")) {
	        this->localizationEditorPtr->Toggle();
	      }
	    }
	    ImGui::End();

	    if (this->input->GetMouseButtonClick(1)) {
	      actionBox = !actionBox;
	      auto cursorPosition = this->input->GetCursorPosition();
	      actionBoxStartPos = ImVec2(cursorPosition.x, cursorPosition.y);
	    }

	    if (actionBox) {
	      ImGui::SetNextWindowPos(actionBoxStartPos);
	      if (ImGui::Begin("actionBox", NULL,
	                       ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove)) {
	        if (ImGui::Button(this->localization->Get("Save Editor").c_str())) {
	        	this->requests.push_back(Request::SAVE);
	        }
	      }
	      ImGui::End();
	    }
	  }

	  void OnDestroy() override { 
	  	this->localization->Save();
	  }

	private:
	  bool actionBox = false;
	  ImVec2 actionBoxStartPos;

	  LocalizationEditor *localizationEditorPtr;

	  std::string file = "game.toml";

	  std::shared_ptr<Localization> localization;
	};
} // namespace Custom
