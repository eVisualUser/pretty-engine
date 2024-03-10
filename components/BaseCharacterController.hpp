#pragma once

#include "PrettyEngine/utils.hpp"
#include <PrettyEngine/KeyCode.hpp>
#include <Physical.hpp>
#include <PrettyEngine/serial.hpp>
#include <PrettyEngine/entity.hpp>

#include <string>

using namespace PrettyEngine;

namespace Custom {
	class BaseCharacterController: public virtual Component {
	public:
		void OnSetup() override {
			this->AddSerializedField(SERIAL_TOKEN(std::string), "colliderName", "Collider");
			this->AddSerializedField(SERIAL_TOKEN(float), "speed", "100");
		}

		void OnPrePhysics() override {
			auto rigidbody = dynamic_cast<Entity*>(this->owner)->GetComponentAs<Physical>(this->GetSerializedFieldValue("colliderName")).ShowError();
			
			auto speed = std::stof(this->GetSerializedFieldValue("speed"));

			if (rigidbody != nullptr) {
				auto movement = glm::vec3(0.0f, 0.0f, 0.0f);
				
				if (this->engineContent->input.GetKeyPress(KeyCode::LeftArrow)) {
					movement.x -= speed * this->engineContent->renderer.GetDeltaTime();
				} else if (this->engineContent->input.GetKeyPress(KeyCode::RightArrow)) {
					movement.x += speed * this->engineContent->renderer.GetDeltaTime();
				}

				if (this->engineContent->input.GetKeyPress(KeyCode::UpArrow)) {
					movement.y += speed * this->engineContent->renderer.GetDeltaTime();
				} else if (this->engineContent->input.GetKeyPress(KeyCode::DownArrow)) {
					movement.y -= speed * this->engineContent->renderer.GetDeltaTime();
				}

				rigidbody->GetCollider()->Move(movement);
			}
		}
	};
}