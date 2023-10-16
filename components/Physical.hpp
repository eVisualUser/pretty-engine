#pragma once

#include <PrettyEngine/collider.hpp>
#include <PrettyEngine/entity.hpp>
#include <PrettyEngine/debug.hpp>

#include <Guid.hpp>

namespace Custom {
	class Physical: public PrettyEngine::Component {
	public:
		void OnStart() override {
			this->_ownerEntity = dynamic_cast<PrettyEngine::Entity*>(this->owner);

			this->_colliderA.name = xg::newGuid();
			this->_colliderA.colliderModel = PrettyEngine::ColliderModel::AABB;
			this->_colliderA.SetRigidbody(true);

			this->physicalSpace->AddCollider("Default", &this->_colliderA);
		}

		void OnEndUpdate() override {
			this->_ownerEntity->position = this->_colliderA.position;
			this->_colliderA.scale = this->_ownerEntity->scale;
			this->_colliderA.rotation =  this->_ownerEntity->scale;
		}

		void OnDestroy() override {
			this->physicalSpace->RemoveCollider("Default", &this->_colliderA);
		}

		void Move(glm::vec3 direction) {
			this->_colliderA.Move(direction);
		}

	private:
		PrettyEngine::Collider _colliderA;

		PrettyEngine::Entity* _ownerEntity;
	};
}
