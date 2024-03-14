#pragma once

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

		void OnStart() override {
			this->keyWatcherUp.key = KeyCode::UpArrow;
			this->keyWatcherUp.mode = KeyWatcherMode::Press;
			this->keyWatcherUp.name = "BaseCharacter-KeyWatcherUp";
			this->keyWatcherUp.actionOnKey = [this] {
				if (auto collider = this->GetCollider()) {
					const auto speed = std::stof(this->GetSerializedFieldValue("speed"));
					auto direction = glm::vec3(0, 1, 0);
					direction *= (speed * this->engineContent->renderer.GetDeltaTime());
					collider->Move(direction);
				}
			};

			this->keyWatcherDown.key = KeyCode::DownArrow;
			this->keyWatcherDown.mode = KeyWatcherMode::Press;
			this->keyWatcherDown.name = "BaseCharacter-KeyWatcherDown";
			this->keyWatcherDown.actionOnKey = [this] {
				if (auto collider = this->GetCollider()) {
					const auto speed = std::stof(this->GetSerializedFieldValue("speed"));
					auto direction = glm::vec3(0, -1, 0);
					direction *= (speed * this->engineContent->renderer.GetDeltaTime());
					collider->Move(direction);
				}
			};

			this->keyWatcherLeft.key = KeyCode::LeftArrow;
			this->keyWatcherLeft.mode = KeyWatcherMode::Press;
			this->keyWatcherLeft.name = "BaseCharacter-KeyWatcherLeft";
			this->keyWatcherLeft.actionOnKey = [this] {
				if (auto collider = this->GetCollider()) {
					const auto speed = std::stof(this->GetSerializedFieldValue("speed"));
					auto direction = glm::vec3(-1, 0, 0);
					direction *= (speed * this->engineContent->renderer.GetDeltaTime());
					collider->Move(direction);
				}
			};

			this->keyWatcherRight.key = KeyCode::RightArrow;
			this->keyWatcherRight.mode = KeyWatcherMode::Press;
			this->keyWatcherRight.name = "BaseCharacter-KeyWatcherRight";
			this->keyWatcherRight.actionOnKey = [this] {
				if (auto collider = this->GetCollider()) {
					const auto speed = std::stof(this->GetSerializedFieldValue("speed"));
					auto direction = glm::vec3(1, 0, 0);
					direction *= (speed * this->engineContent->renderer.GetDeltaTime());
					collider->Move(direction);
				}
			};

			this->engineContent->input.AddKeyWatcher(&this->keyWatcherUp);
			this->engineContent->input.AddKeyWatcher(&this->keyWatcherDown);
			this->engineContent->input.AddKeyWatcher(&this->keyWatcherLeft);
			this->engineContent->input.AddKeyWatcher(&this->keyWatcherRight);
		}

		void OnDestroy() override {
			this->engineContent->input.RemoveKeyWatcher(&this->keyWatcherUp);
			this->engineContent->input.RemoveKeyWatcher(&this->keyWatcherDown);
			this->engineContent->input.RemoveKeyWatcher(&this->keyWatcherLeft);
			this->engineContent->input.RemoveKeyWatcher(&this->keyWatcherRight);
		}

		Physical* GetCollider() {
			const auto colliderName = this->GetSerializedFieldValue("colliderName");
			return dynamic_cast<Entity*>(this->owner)->GetComponentAs<Physical>(colliderName).Resolve([this, colliderName](Physical** physical) {
				if (const auto owner = dynamic_cast<Entity*>(this->owner)) {
					owner->AddComponent<Physical>(colliderName);
				}
				return true;
			})->ShowError();
		}

	private:
		KeyWatcher keyWatcherUp;
		KeyWatcher keyWatcherDown;
		KeyWatcher keyWatcherLeft;
		KeyWatcher keyWatcherRight;
	};
}