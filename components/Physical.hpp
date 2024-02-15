#pragma once

#include <PrettyEngine/Collision.hpp>
#include <PrettyEngine/collider.hpp>
#include <PrettyEngine/entity.hpp>
#include <PrettyEngine/debug/debug.hpp>
#include <PrettyEngine/localization.hpp>
#include <PrettyEngine/render/mesh.hpp>

#include <Guid.hpp>

namespace Custom {
	class Physical: public PrettyEngine::Component {
	public:
		void OnUpdatePublicVariables() override {
			this->publicFuncions.insert(std::make_pair("Update gravity", ([this]() {
				auto gravity = PrettyEngine::ParseCSVLine(this->GetSerializedFieldValue("Gravity"), ';');
				for (int i = 0; i < gravity.size(); i++) {
					float axisValue = std::stof(gravity[i]);
					if (i < this->GetCollider()->gravity.length()) {
						this->GetCollider()->gravity[i] = axisValue;
					}
				}
			})));

			this->AddSerializedField(SERIAL_TOKEN(bool), "rigidbody", SERIAL_TOKEN(false));
			this->AddSerializedField(SERIAL_TOKEN(std::string), "layer", "Default");
			this->AddSerializedField(SERIAL_TOKEN(std::string), "name", xg::newGuid());
			this->AddSerializedField(SERIAL_TOKEN(float), "mass", "1");
			this->AddSerializedField(SERIAL_TOKEN(bool), "fixed", "false");
			this->AddSerializedField(SERIAL_TOKEN(glm::vec3), "Gravity", "0;9.81;0");

			if (this->GetSerializedFieldValue("fixed") == "false") {
				this->_colliderA.fixed = false;
			} else {
				this->_colliderA.fixed = true;
			}

			if (this->GetSerializedFieldValue("rigidbody") == "true") {
				this->_colliderA.SetRigidbody(true);
			} else {
				this->_colliderA.SetRigidbody(false);
			}

			this->_colliderA.name = this->GetSerializedFieldValue("name");
			this->layer = this->GetSerializedFieldValue("layer");

			this->_mesh = PrettyEngine::CreateRectMesh();
			this->_colliderA.SetMesh(&this->_mesh);
		}

		void OnStart() override {
			this->_ownerEntity = dynamic_cast<PrettyEngine::Entity*>(this->owner);
			this->engineContent->physicalSpace.AddCollider(this->layer, &this->_colliderA);

			this->_colliderA.position = this->GetTransform()->position;
		}

		void OnEndUpdate() override {
			this->_ownerEntity->position = this->_colliderA.position;
			this->_colliderA.SetScale(this->_ownerEntity->scale);
			this->_ownerEntity->rotation = this->_colliderA.rotation;
		}

		void OnRender() override { 
			auto collisions = this->engineContent->physicalSpace.GetCollisions(this->GetCollider());
			if (collisions != nullptr && !collisions->empty()) {
				if (ImGui::Begin("Collisions ?")) {
					ImGui::Text("COLLISION");
				}
				ImGui::End();
			}
		}

		void OnDestroy() override {
			this->engineContent->physicalSpace.RemoveCollider(this->layer, &this->_colliderA);
		}

		void Move(glm::vec3 direction) {
			this->_colliderA.Move(direction);
		}

		PrettyEngine::Collider* GetCollider() {
			return &this->_colliderA;
		}

	private:
		PrettyEngine::Collider _colliderA;

		PrettyEngine::Mesh _mesh;

		PrettyEngine::Entity* _ownerEntity;

		std::string layer;
	};
}
