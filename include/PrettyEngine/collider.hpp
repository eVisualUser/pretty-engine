#ifndef H_COLLIDER
#define H_COLLIDER

// Pretty Engine
#include <PrettyEngine/render/mesh.hpp>
#include <PrettyEngine/debug/debug.hpp>
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
		/// Only trigger point in
		AABB,
		/// Simple Sphere collision detection
		Sphere,
	};

	/// Repsesent an object in a world that react/affect the physics
	class Collider: public Transform, public virtual Tagged {
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
				if (other->colliderModel == ColliderModel::AABB && other->layer == this->layer) {
					if (this->fixed || (!this->fixed && !other->fixedCollisionOnly)) {
						auto basePoints = (this->PointIn(other->GetMinHalf()) || this->PointIn(other->GetMaxHalf()));
						auto inversePoints = (this->PointIn(other->GetInverseMin(true)) || this->PointIn(other->GetInverseMax(true)));

						return (basePoints || inversePoints);
					}
				}
				return false;
			} else if (this->colliderModel == ColliderModel::Sphere) {
				if (other->colliderModel == ColliderModel::Sphere) {
					if (glm::distance(other->position, this->position) < other->radius + this->radius) {
						return true;
					} else {
						return false;
					}
				}
			}
			return this->BadSetup();
		}

		/// Return the minimum position
		glm::vec3 GetMin() {
			return (this->position) - this->scale;
		}

		glm::vec3 GetInverseMin(bool halfScale = false) {
			auto scale = this->scale;

			if (halfScale) {
				scale = this->halfScale;
			}

			auto result = (this->position) - scale;

			result.x += scale.x * 2;

			return result;
		}

		/// Return the minimum position based on half of the scale
		glm::vec3 GetMinHalf() {
			return (this->position) - this->halfScale;
		}

		/// Return the maximum position
		glm::vec3 GetMax() {
			return (this->position) + this->scale;
		}

		glm::vec3 GetInverseMax(bool halfScale = false) {
			auto scale = this->scale;

			if (halfScale) {
				scale = this->halfScale;
			}

			auto result = (this->position) + scale;

			result.x -= scale.x * 2;

			return result;
		}

		/// Return the maxime position based on half of the scale
		glm::vec3 GetMaxHalf() {
			return (this->position) + this->halfScale;
		}

		void SetRigidbody(bool state = true) {
			this->isRigidBody = state;
		}

		/// Move based on the velocity
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

		float mass = 1.0f;

		float bounce = 1.0f;
		
		glm::vec3 gravity = glm::vec3(0.0f, 0.0f, 0.0f);

		glm::vec3 velocity;

		bool reverseDelta = false;

		bool fixed = false;

		/// Collide only with objects that are fixed
		bool fixedCollisionOnly = false;

		std::string layer = "Default";
	};
}

#endif