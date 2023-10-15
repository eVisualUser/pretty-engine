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

		void OnUpdate() override {
			this->_ownerEntity->position = this->_colliderA.position;
		}

		void OnRender() override {
			this->_colliderA.position = this->_ownerEntity->position;
		}

		void OnDestroy() override {
			this->physicalSpace->RemoveCollider("Default", &this->_colliderA);
		}

	private:
		PrettyEngine::Collider _colliderA;

		PrettyEngine::Entity* _ownerEntity;
	};
}
