#pragma once

#include <PrettyEngine/collider.hpp>
#include <PrettyEngine/Collision.hpp>

#include <openGJK/openGJK.h>

#include <string>
#include <unordered_map>
#include <utility>
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

		std::vector<Collision> FindCollisions(Collider* collider) {
			std::vector<Collision> out;
			
			auto layer = this->FindColliderLayer(collider);
			
			for(auto & i: *layer) {
				if (i->name != collider->name && i->OtherIn(collider)) {
					Collision collision;
					collision.colliderSource = collider;
					collision.colliderOther = i;

					out.push_back(collision);
				}
			}

			return out;
		}

		void RigidbodyApplyVelocity(std::vector<Collision>* collisions, Collider* collider) {
			collider->velocity += collider->gravity;

			auto startPosition = collider->position;
			collider->position += collider->velocity;

			collider->velocity = glm::vec3(0.0f, 0.0f, 0.0f);
		}

		void Update(float deltaTime) {
			this->_collisions.clear();
			for(auto & layer: this->_colliders) {
				for(auto & collider: layer.second) {
					auto collisions = this->FindCollisions(collider);
					this->UpdateRigidBody(collider, &collisions, deltaTime);
					this->RigidbodyApplyVelocity(&collisions, collider);
					this->_collisions.insert(std::make_pair(collider, collisions));
				}
			}
		}

		void UpdateRigidBody(Collider* collider, std::vector<Collision>* collisions, float deltaTime) {
			if (collider->isRigidBody) {
				for(auto & collision: *collisions) {
					this->RigidbodyApplyVelocity(collisions, collider);
				}
			}
		}

		std::vector<Collision>* GetCollisions(Collider* collider) {
			for(auto & collisions: this->_collisions) {
				if (collisions.first->name == collider->name) {
					return &collisions.second;
				}
			}

			gkSimplex simplex;

			return nullptr;
		}

	private:
		std::unordered_map<std::string, std::vector<Collider*>> _colliders;

		std::unordered_map<Collider*, std::vector<Collision>> _collisions;
	};
}
