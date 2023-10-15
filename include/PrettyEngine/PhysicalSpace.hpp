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
					collision.colliderA = collider;
					collision.colliderB = i;

					out.push_back(collision);
				}
			}

			return out;
		}

		void UpdateRigidBodies(float deltaTime) {
			for(auto & layer: this->_colliders) {
				for(auto & collider: layer.second) {
					this->UpdateRigidBody(collider, deltaTime);
				}
			}
		}

		void UpdateRigidBody(Collider* collider, float deltaTime) {
			if (collider->isRigidBody) {
				auto collisions = this->GetCollisions(collider);
				for(auto & collision: collisions) {
					if (collision.colliderB->isRigidBody) {
						DebugLog(LOG_DEBUG, "Collision !", false);
						// collision.colliderA->position.x += glm::distance(collision.colliderA->position, collision.colliderB->position);
					}
				}
			}
		}

	private:
		std::unordered_map<std::string, std::vector<Collider*>> _colliders;
	};
}
