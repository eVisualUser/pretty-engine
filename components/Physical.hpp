#pragma once

#include <PrettyEngine/Collision.hpp>
#include <PrettyEngine/collider.hpp>
#include <PrettyEngine/entity.hpp>
#include <PrettyEngine/debug/debug.hpp>
#include <PrettyEngine/localization.hpp>

#include <Guid.hpp>

namespace Custom {
	class Physical: public PrettyEngine::Component {
	public:
		void OnUpdatePublicVariables() override {
			this->AddActionOnPublicVariableChanged([this](std::string var) {
				if (var == "Gravity") {
					PRINT_GLM_VEC3(this->GetCollider()->gravity);
					auto gravity = PrettyEngine::ParseCSVLine(this->GetPublicVarValue("Gravity"), ';');
					for (int i = 0; i < gravity.size(); i++) {
						float axisValue = std::stof(gravity[i]);
						if (i < this->GetCollider()->gravity.length()) {
							this->GetCollider()->gravity[i] = axisValue;
						}
					}
					PRINT_GLM_VEC3(this->GetCollider()->gravity);
				}
			});

			this->CreatePublicVar("rigidbody", "false");
			this->CreatePublicVar("model", "AABB");
			this->CreatePublicVar("layer", "Default");
			this->CreatePublicVar("name", xg::newGuid());
			this->CreatePublicVar("mass", "1");
			this->CreatePublicVar("fixed", "false");
			this->CreatePublicVar("Gravity", "0;9.81;0");

			if (this->GetPublicVarValue("fixed") == "false") {
				this->_colliderA.fixed = false;
			} else {
				this->_colliderA.fixed = true;
			}

			if (this->GetPublicVarValue("model") == "Sphere") {
				this->_colliderA.colliderModel = PrettyEngine::ColliderModel::Sphere;
			} else {
				this->_colliderA.colliderModel = PrettyEngine::ColliderModel::AABB;
			}

			if (this->GetPublicVarValue("rigidbody") == "true") {
				this->_colliderA.SetRigidbody(true);
			} else {
				this->_colliderA.SetRigidbody(false);
			}

			this->_colliderA.name = this->GetPublicVarValue("name");
			this->layer = this->GetPublicVarValue("layer");
		}

		void OnStart() override {
			this->_ownerEntity = dynamic_cast<PrettyEngine::Entity*>(this->owner);
			this->engineContent->physicalSpace.AddCollider(this->layer, &this->_colliderA);

			this->_colliderA.position = this->GetTransform()->position;
		}

		void OnUpdate() override {
			this->_ownerEntity->position = this->_colliderA.position;
			if (this->_colliderA.colliderModel == PrettyEngine::ColliderModel::Sphere) {
				this->_colliderA.radius = this->_ownerEntity->halfScale.x;
			}
			this->_colliderA.SetScale(this->_ownerEntity->scale);
			this->_ownerEntity->rotation = this->_colliderA.rotation;
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

		PrettyEngine::Entity* _ownerEntity;

		std::string layer;
	};
}
