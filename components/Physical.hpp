#pragma once

#include <PrettyEngine/Collision.hpp>
#include <PrettyEngine/collider.hpp>
#include <PrettyEngine/entity.hpp>
#include <PrettyEngine/debug.hpp>

#include <Guid.hpp>

namespace Custom {
	class Physical: public PrettyEngine::Component {
	public:
		void OnUpdatePublicVariables() override {
			this->CreatePublicVar("rigidbody", "false");
			this->CreatePublicVar("model", "AABB");
			this->CreatePublicVar("layer", "Default");
			this->CreatePublicVar("name", xg::newGuid());
			this->CreatePublicVar("mass", "1");

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

			this->physicalSpace->AddCollider(this->layer, &this->_colliderA);
		}

		void OnEndUpdate() override {
			this->_ownerEntity->position = this->_colliderA.position;
			if (this->_colliderA.colliderModel == PrettyEngine::ColliderModel::Sphere) {
				this->_colliderA.radius = this->_ownerEntity->halfScale.x;
			}
			this->_colliderA.SetScale(this->_ownerEntity->scale);
			this->_colliderA.rotation =  this->_ownerEntity->scale;
		}

		void OnDestroy() override {
			this->physicalSpace->RemoveCollider(this->layer, &this->_colliderA);
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
