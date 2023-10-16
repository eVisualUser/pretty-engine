#pragma once

#include <PrettyEngine/collider.hpp>
#include <PrettyEngine/Collision.hpp>

#include <string>
#include <unordered_map>
#include <vector>

#include <glm/glm.hpp>

namespace PrettyEngine {
	class PhysicalSpace {
	public:
		void AddCollider(std::string layerName, Collider* collider) {
			auto layer = this->GetOrCreateLayer(layerName);
			layer->push_back(collider);
		}

		bool RemoveCollider(std::string layerName, Collider* collider) {
			auto layer = this->GetOrCreateLayer(layerName);
			size_t index = 0;
			for(auto & i: *layer) {
				if (i->name == collider->name) {
					layer->erase(layer->begin() + index);
					return true;
				}
				index++;
			}
			return false;
		}

		std::vector<Collider*>* GetOrCreateLayer(std::string layerName) {
			for(auto & layer: this->_colliders) {
				if (layer.first == layerName) {
					return &layer.second;
				}
			}
			this->_colliders.insert_or_assign(layerName, std::vector<Collider*>());
			return GetOrCreateLayer(layerName);
		}

		std::vector<Collider*>* FindColliderLayer(Collider* collider) {
			for(auto & layer: this->_colliders) {
				for(auto & i: layer.second) {
					if (i->name == collider->name) {
						return &layer.second;
					}
				}
			}

			return nullptr;
		}

		std::vector<Collision> GetCollisions(Collider* collider) {
			std::vector<Collision> out;
			
			auto layer = this->FindColliderLayer(collider);
			
			for(auto & i: *layer) {
				if (i->name != collider->name && i->OtherIn(collider)) {
					Collision collision;
					collision.colliderSource = collider;
					collision.colliderOther = i;

					if (i->PointIn(collider->position)) {
						collision.colliderCenterInOther = true;
					}

					out.push_back(collision);
				}
			}

			return out;
		}

		/// Solution low-cost to avoid big overlap
		void ExpressOverlapExtraction(Collision* collision) {
			if (collision->colliderOther->position != collision->colliderSource->position) {
				collision->colliderSource->Translate(collision->colliderSource->position - collision->colliderOther->position);
			} else {
				collision->colliderSource->position.x += 0.001;
			}
		}

		void RigidbodyApplyVelocity(Collider* collider) {
			auto startPosition = collider->position;
			collider->position += collider->velocity;
			if (!this->GetCollisions(collider).empty()) {
				collider->position = startPosition;
			}
			collider->velocity = glm::vec3(0.0f, 0.0f, 0.0f);
		}

		void UpdateRigidBodies(float deltaTime) {
			for(auto & layer: this->_colliders) {
				for(auto & collider: layer.second) {
					this->UpdateRigidBody(collider, deltaTime);
					this->RigidbodyApplyVelocity(collider);
				}
			}
		}

		void UpdateRigidBody(Collider* collider, float deltaTime) {
			if (collider->isRigidBody) {
				auto collisions = this->GetCollisions(collider);
				for(auto & collision: collisions) {
					if (collision.colliderOther->isRigidBody) {
						if (collision.colliderCenterInOther) {
							this->ExpressOverlapExtraction(&collision);
						} else {
							this->RigidbodyApplyVelocity(collider);
						}
					}
				}
			}
		}

		/// Set the smooth physcis state, when active it will make all physics operation smooth (can impact performance and precision).
		void SetSmoothPhysics(bool state) {
			this->smoothPhysics = state;
		}

	private:
		std::unordered_map<std::string, std::vector<Collider*>> _colliders;

		bool smoothPhysics = true;
	};
}
