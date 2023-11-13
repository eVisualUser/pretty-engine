#pragma once

#include <Physical.hpp>
#include <PrettyEngine/camera.hpp>
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

#include <memory>
#include <string>

using namespace PrettyEngine;

namespace Custom {
	class Editor : public virtual Entity {
	public:
	  void OnStart() override {
	    this->localization = std::make_shared<Localization>();
	    this->localization->LoadFile(GetEnginePublicPath("editor.csv", true));

	    this->localizationEditorPtr = this->GetComponentAs<LocalizationEditor>("LocalizationEditor");
	    this->localizationEditorPtr->localization = this->localization;
	    this->rigidbody = this->GetComponentAs<Physical>("Physical");

	    this->engineContent->renderer.GetCurrentCamera()->position.z = -1;

	    this->visualObject = this->GetComponentAs<Render>("Render")->GetVisualObject();

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

	    // this->secondCamera = this->engineContent->renderer.AddCamera();
	    // this->secondCamera->active = true;
	    // this->secondCamera->SetRenderToTexture(false);

	   	//this->visualObject->textures.clear();
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
	      this->engineContent->renderer.SetWindowOpacity(0.1f);
	    } else {
	      this->engineContent->renderer.SetWindowOpacity(1.0f);
	    }
	  }

	  void OnRender() override {
	  	auto newCamPosition = -this->position;
	  	newCamPosition.z -= 5;
	  	
	  	this->engineContent->renderer.GetCurrentCamera()->position = newCamPosition;

	  	if (this->engineContent->input.GetKeyPress(KeyCode::LeftControl) &&
	        this->engineContent->input.GetKeyDown(KeyCode::S)) {
	      	DebugLog(LOG_DEBUG, "Save to file: " << this->file, false);	  
	      	this->requests.push_back(Request::SAVE);    	
	    }

	    // Update the position
	    auto moveDirection = glm::vec3(0.0f, 0.0f, 0.0f);

	    if (this->keyUp.state) {
	  		moveDirection.y += this->_speed * this->engineContent->renderer.GetDeltaTime();
	  	} else if (this->keyDown.state) {
	  		moveDirection.y -= this->_speed * this->engineContent->renderer.GetDeltaTime();
	  	}

	  	if (this->keyLeft.state) {
	  		moveDirection.x -= this->_speed * this->engineContent->renderer.GetDeltaTime();
	  	} else if (this->keyRight.state) {
	  		moveDirection.x += this->_speed * this->engineContent->renderer.GetDeltaTime();
	  	}

	  	auto collisions = this->engineContent->physicalSpace.GetCollisions(this->rigidbody->GetCollider());

	  	if (collisions != nullptr) {
	 	  	if (!collisions->empty()) {
		  		this->visualObject->baseColor = glm::vec3(0.67, 0.22, 0.17);
		  	} else {
		  		this->visualObject->baseColor = glm::vec3(1.0f, 1.0f, 1.0f);
		  	}
	  	}

	  	this->rigidbody->Move(moveDirection);

	  	this->Rotate(this->engineContent->input.GetMouseWheelDelta() * 500.0f * this->engineContent->renderer.GetDeltaTime());

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

	    if (this->engineContent->input.GetMouseButtonClick(1)) {
	      actionBox = !actionBox;
	      auto cursorPosition = this->engineContent->input.GetCursorPosition();
	      actionBoxStartPos = ImVec2(cursorPosition.x, cursorPosition.y);
	    }

	    if (actionBox) {
	      ImGui::SetNextWindowPos(actionBoxStartPos);
	      if (ImGui::Begin("actionBox", NULL,
	                       ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove)) {
	        if (ImGui::Button(this->localization->Get("Save Editor").c_str())) {
	        	this->requests.push_back(Request::SAVE);
	        }

	        if (ImGui::Button(this->localization->Get("Save Editor localization").c_str())) {
	        	this->localization->Save();
	        }
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

	  Physical* rigidbody;
	  VisualObject* visualObject;

	  Camera* secondCamera = nullptr;
	};
}
