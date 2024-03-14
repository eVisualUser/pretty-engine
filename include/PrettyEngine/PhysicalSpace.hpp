#ifndef H_PHYSICAL_SPACE
#define H_PHYSICAL_SPACE

#include "glm/geometric.hpp"
#include <PrettyEngine/collider.hpp>
#include <PrettyEngine/Collision.hpp>

#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include <glm/glm.hpp>

namespace PrettyEngine {
	/// Physics engine and so manage the PhysicalObjects.
	class PhysicalSpace {
	public:
		void AddCollider(std::string layerName, Collider* collider) {
			auto layer = this->GetOrCreateLayer(std::move(layerName));
			layer->push_back(collider);
		}

		bool RemoveCollider(std::string layerName, Collider* collider) {
			auto layer = this->GetOrCreateLayer(std::move(layerName));
			int index = 0;
			for(auto & i: *layer) {
				if (i->name == collider->name) {
					layer->erase(layer->begin() + index);
					return true;
				}
				index++;
			}
			return false;
		}

		std::unordered_map<std::string, std::vector<Collider*>>* GetAllLayers() { return &this->_colliders;}

		std::vector<Collider*>* GetOrCreateLayer(std::string layerName) {
			for(auto & layer: this->_colliders) {
				if (layer.first == layerName) {
					return &layer.second;
				}
			}
			this->_colliders.insert_or_assign(layerName, std::vector<Collider*>());
			return &this->_colliders[layerName];
		}

		std::vector<Collider*>* FindColliderLayer(Collider* collider) {
			if (collider != nullptr) {
				for(auto & layer: this->_colliders) {
					for(auto i: layer.second) {
						if (i != nullptr && i->name == collider->name) {
							return &layer.second;
						}
					}
				}
			}

			return nullptr;
		}

		std::vector<Collision> FindCollisions(Collider* collider) {
			std::vector<Collision> out;

			const auto layer = this->FindColliderLayer(collider);

			if (collider != nullptr) {
				if (layer != nullptr) {
					for(auto & otherCollider: *layer) {
						if (otherCollider != nullptr) {
							if (otherCollider->name.c_str() != collider->name.c_str() && otherCollider->OtherIn(collider)) {
								Collision collision;
								collision.colliderSource = collider;
								collision.colliderOther = otherCollider;

								out.push_back(collision);
							}
						} else {
							DebugLog(LOG_ERROR, "Null collider registred.", true);
						}
					}
				} else {
					DebugLog(LOG_ERROR, "Failed to find collider layer of: " << collider->name, true);
				}
			} else {
				DebugLog(LOG_ERROR, "Collider is nullptr.", true);
			}

			return out;
		}

		void UpdateRigidbodyPosition(const std::vector<Collision>* collisions, Collider* collider, float deltaTime) const {
			collider->position += collider->velocity;
			collider->position += collider->gravity * collider->mass;

			if (!collisions->empty() && !collider->fixed) {
				for(auto & collision: *collisions) {
					auto delta = collider->position - collision.colliderOther->position;

					glm::vec3 direction = glm::normalize(collider->position - collision.colliderOther->position);

					glm::vec3 supportA = collider->GJKSupport(*collision.colliderOther, direction) / 100.0f;

					float penetrationDepth = glm::length(supportA);

					glm::vec3 separationVector = glm::normalize(supportA) * penetrationDepth;

					collider->position += separationVector;
				}
			}

			if (collider->resetVelocity) {
				collider->velocity = glm::vec3(0.0f, 0.0f, 0.0f);
			}
		}

		void Update(float deltaTime) {
			this->_collisions.clear();
			for(auto & layer: this->_colliders) {
				for(auto & collider: layer.second) {
					auto collisions = this->FindCollisions(collider);
					this->UpdateRigidBody(collider, &collisions, deltaTime);
					this->UpdateRigidbodyPosition(&collisions, collider, deltaTime);
					if (!collisions.empty()) {
						this->_collisions.insert(std::make_pair(collider, collisions));
					}
				}
			}
		}

		void UpdateRigidBody(Collider* collider, std::vector<Collision>* collisions, float deltaTime) const {
			if (collider->isRigidBody) {
				this->UpdateRigidbodyPosition(collisions, collider, deltaTime);
			}
		}

		std::vector<Collision>* GetCollisions(Collider* collider) {
			for(auto & collisions: this->_collisions) {
				if (collisions.first != nullptr && collisions.first->name == collider->name) {
					return &collisions.second;
				}
			}

			return nullptr;
		}

  		void Clear() {
			this->_colliders.clear();
   			this->_collisions.clear();
  		}

	private:
		std::unordered_map<std::string, std::vector<Collider*>> _colliders;

		std::unordered_map<Collider*, std::vector<Collision>> _collisions;
	};
}

#endif