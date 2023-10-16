#pragma once

// Pretty Engine
#include <PrettyEngine/debug.hpp>
#include <PrettyEngine/transform.hpp>
#include <PrettyEngine/utils.hpp>
#include <PrettyEngine/tags.hpp>

// GLM
#include <glm/gtc/constants.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/matrix.hpp>
#include <glm/geometric.hpp>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

namespace PrettyEngine {
	enum class ColliderModel {
		/// Optimized collision detection
		AABB,
		/// Simple Sphere collision detection
		Sphere,
	};

	class Collider: public Transform {
	public:
		bool PointIn(glm::vec3 point) { 
			if (this->colliderModel == ColliderModel::AABB) {
				auto min = (-this->position) - this->halfScale;
				auto max = (-this->position) + this->halfScale;

				return (Vec3Greater(point, min) && Vec3Lower(point, max));
			} else if (this->colliderModel == ColliderModel::Sphere) {
				return glm::distance(this->position, point) < this->radius;
			}

			return this->BadSetup();
		}
		
		bool OtherIn(Collider* other) { 
			if (this->colliderModel == ColliderModel::AABB) {
				if (other->colliderModel == ColliderModel::AABB) {
					auto min = this->GetMinHalf();
					auto max = this->GetMaxHalf();

					return (Vec3Greater(other->GetMaxHalf(), min) && Vec3Lower(other->GetMinHalf(), max));
				} else if (other->colliderModel == ColliderModel::Sphere) {
					glm::vec3 buffer = this->position - other->position;
					buffer = glm::clamp(buffer, this->GetMaxHalf(), this->GetMinHalf());
					return glm::distance(buffer, other->position) < other->radius;
				}
			} else if (this->colliderModel == ColliderModel::Sphere) {
				if (other->colliderModel == ColliderModel::Sphere) {
					return other->radius + this->radius >= glm::distance(this->position, other->position);
				} else if (other->colliderModel == ColliderModel::AABB) {
					glm::vec3 buffer = other->position - this->position;
					buffer = glm::clamp(buffer, other->GetMaxHalf(), other->GetMinHalf());
					return glm::distance(buffer, this->position) < this->radius;
				}
			}

			return this->BadSetup();
		}

		glm::vec3 GetMin() {
			return (this->position) - this->scale;
		}

		glm::vec3 GetMinHalf() {
			return (this->position) - this->halfScale;
		}

		glm::vec3 GetMax() {
			return (this->position) + this->scale;
		}

		glm::vec3 GetMaxHalf() {
			return (this->position) + this->halfScale;
		}

		void SetRigidbody(bool state = true) {
			this->isRigidBody = state;
		}

		void Move(glm::vec3 direction) {
			this->velocity += direction;
		}
		
	private:
		bool BadSetup() {
			DebugLog(LOG_ERROR, "Collider: " << this->name << " have no detection model set", true);
			return false;
		}
		
	public:
		ColliderModel colliderModel;

		float radius = 1.0f;
		
		std::string name = "DefaultColliderName";

		bool isRigidBody;

		/// Mass of the object
		float mass = 1.0f;

		float bounce = 1.0f;
		
		glm::vec3 gravity;

		glm::vec3 velocity;
	};
}
