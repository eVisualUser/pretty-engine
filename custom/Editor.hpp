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
#include <PrettyEngine/KeyCode.hpp>

#include <Render.hpp>

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

	    this->keyUp.name = "EditorKeyUp";
	    this->keyUp.key = KeyCode::UpArrow;
	    this->keyUp.mode = KeyWatcherMode::Press;
	    this->input->AddKeyWatcher(&this->keyUp);

	    this->keyDown.name = "EditorKeyDown";
	    this->keyDown.key = KeyCode::DownArrow;
	    this->keyDown.mode = KeyWatcherMode::Press;
	    this->input->AddKeyWatcher(&this->keyDown);
	    
	    this->keyLeft.name = "EditorKeyLeft";
	    this->keyLeft.key = KeyCode::LeftArrow;
	    this->keyLeft.mode = KeyWatcherMode::Press;
	    this->input->AddKeyWatcher(&this->keyLeft);
	    
	    this->keyRight.name = "EditorKeyRight";
	    this->keyRight.key = KeyCode::RightArrow;
	    this->keyRight.mode = KeyWatcherMode::Press;
	    this->input->AddKeyWatcher(&this->keyRight);
	  }

	  /// Called only in the editor
	  void OnEditorUpdate() override {
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
	  	if (this->keyUp.state) {
	  		this->position.y += this->_speed * this->renderer->GetDeltaTime();
	  	} else if (this->keyDown.state) {
	  		this->position.y -= this->_speed * this->renderer->GetDeltaTime();
	  	}

	  	if (this->keyLeft.state) {
	  		this->position.x -= this->_speed * this->renderer->GetDeltaTime();
	  	} else if (this->keyRight.state) {
	  		this->position.x += this->_speed * this->renderer->GetDeltaTime();
	  	}

	  	this->Rotate(this->input->GetMouseWheelDelta() * 500.0f * this->renderer->GetDeltaTime());

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

	  	this->input->RemoveKeyWatcher(&this->keyUp);
	  	this->input->RemoveKeyWatcher(&this->keyDown);
	  	this->input->RemoveKeyWatcher(&this->keyLeft);
	  	this->input->RemoveKeyWatcher(&this->keyRight);
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
	};
}
