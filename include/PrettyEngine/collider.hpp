#ifndef H_COLLIDER
#define H_COLLIDER

// Pretty Engine
#include "PrettyEngine/utils.hpp"
#include <PrettyEngine/render/mesh.hpp>
#include <PrettyEngine/debug/debug.hpp>
#include <PrettyEngine/transform.hpp>
#include <PrettyEngine/tags.hpp>
#include <PrettyEngine/simplex.hpp>

// GLM
#include <glm/gtc/quaternion.hpp>
#include <glm/geometric.hpp>
#include <glm/vec3.hpp>

namespace PrettyEngine {
	/// Repsesent an object in a world that react/affect the physics
	class Collider: public Transform, public virtual Tagged {
	public:
		Collider() = default;

		bool OtherIn(Collider* other) { 
			return this->GJKCheck(*other, 10);
		}

		void SetMesh(Mesh* newMesh) { this->mesh = newMesh;	}

		/// Return the minimum position
		glm::vec3 GetMin() const {
			return (this->position) - this->scale;
		}

		glm::vec3 GetInverseMin(const bool& halfScale = false) {
			auto scale = this->scale;

			if (halfScale) {
				scale = this->halfScale;
			}

			auto result = (this->position) - scale;

			result.x += scale.x * 2;

			return result;
		}

		#pragma region GJK

		glm::vec3 GJKFindFurthestPoint(const glm::vec3& direction) {
			if (this->mesh->vertices.empty()) {
				return glm::vec3(0.0f);
			}

			glm::vec3 furthestPoint = this->TransformPosition(this->mesh->vertices[0].position);
			float maxDistance = glm::dot(furthestPoint, direction);

			for (size_t i = 1; i < this->mesh->vertices.size(); ++i) {
				const auto& vertex = this->mesh->vertices[i];
				const auto transformedVertex = this->TransformPosition(vertex.position);

				float distance = glm::dot(transformedVertex, direction);
				if (distance > maxDistance) {
					maxDistance = distance;
					furthestPoint = transformedVertex;
				}
			}

			return furthestPoint;
		}

		/// Return the vertex on the Minkowski difference
		glm::vec3 GJKSupport(Collider &colliderB, glm::vec3 direction) {
			return this->GJKFindFurthestPoint(direction) - colliderB.GJKFindFurthestPoint(-direction);
		}

		bool GJKCheck(Collider &colliderB, int maxIterations) {
			glm::vec3 relativeDirection = this->position - colliderB.position;

			// Get default support
			auto support = this->GJKSupport(colliderB, relativeDirection);

			// Create the simplex
			auto simplex = Simplex();
			simplex.PushFront(-support);

			// Direction toward the origin
			glm::vec3 direction = -support;

			while (true) {
				support = this->GJKSupport(colliderB, direction);

				if (glm::dot(support, direction) <= 0) {
					return false; // no collision
				}

				simplex.PushFront(support);

				if (GJKNextSimplex(simplex, direction)) {
					return true;
				}
			}
		}

		bool GJKSameDirection(const glm::vec3 &direction, const glm::vec3 &ao) {
			return glm::dot(direction, ao) > 0;
		}

		bool GJKLine(Simplex &simplex, glm::vec3 &direction) {
			glm::vec3 a = simplex[0];
			glm::vec3 b = simplex[1];

			glm::vec3 ab = b - a;
			glm::vec3 ao = -a;

			if (GJKSameDirection(ab, ao)) {
				direction = glm::cross(glm::cross(ab, ao), ab);
			} else {
				simplex = {a};
				direction = ao;
			}

			return false;
		}

		bool GJKTriangle(Simplex &simplex, glm::vec3 &direction) {
			glm::vec3 a = simplex[0];
			glm::vec3 b = simplex[1];
			glm::vec3 c = simplex[2];

			glm::vec3 ab = b - a;
			glm::vec3 ac = c - a;
			glm::vec3 ao = -a;

			glm::vec3 abc = cross(ab, ac);

			if (GJKSameDirection(cross(abc, ac), ao)) {
				if (GJKSameDirection(ac, ao)) {
					simplex = {a, c};
					direction = cross(cross(ac, ao), ac);
				} else {
					return GJKLine(simplex = {a, b}, direction);
				}
			} else {
				if (GJKSameDirection(cross(ab, abc), ao)) {
					return GJKLine(simplex = {a, b}, direction);
				} else {
					if (GJKSameDirection(abc, ao)) {
						direction = abc;
					} else {
						simplex = {a, c, b};
						direction = -abc;
					}
				}
			}

			return false;
		}

		bool GJKTetrahedron(Simplex &simplex, glm::vec3 &direction) {
			glm::vec3 a = simplex[0];
		    glm::vec3 b = simplex[1];
		    glm::vec3 c = simplex[2];
		    glm::vec3 d = simplex[3];

		    glm::vec3 ab = b - a;
		    glm::vec3 ac = c - a;
		    glm::vec3 ad = d - a;

		    glm::vec3 abc = glm::cross(ab, ac);
		    glm::vec3 acd = glm::cross(ac, ad);
		    glm::vec3 adb = glm::cross(ad, ab);

		    glm::vec3 ao = -a;

		    // Check if the origin is on the same side of each face as point 'a'
		    bool isOutsideABC = glm::dot(abc, ao) > 0;
		    bool isOutsideACD = glm::dot(acd, ao) > 0;
		    bool isOutsideADB = glm::dot(adb, ao) > 0;

		    if (isOutsideABC && isOutsideACD && isOutsideADB) {
		        // Origin is outside the tetrahedron, we need to find the new direction
		        if (!GJKSameDirection(abc, ao)) {
		            return GJKTriangle(simplex = {a, b, c}, direction);
		        } else if (!GJKSameDirection(acd, ao)) {
		            return GJKTriangle(simplex = {a, c, d}, direction);
		        } else if (!GJKSameDirection(adb, ao)) {
		            return GJKTriangle(simplex = {a, d, b}, direction);
		        }
		    }

		    // Origin is inside or on the tetrahedron
		    return true;
		}

		bool GJKNextSimplex(Simplex &simplex, glm::vec3 &direction) {
			switch (simplex.size()) {
			case 2:
				return GJKLine(simplex, direction);
			case 3:
				return GJKTriangle(simplex, direction);
			case 4:
				return GJKTetrahedron(simplex, direction);
			default:
				return false;
			}
		}

		#pragma endregion

		void SetRigidbody(bool state = true) {
			this->isRigidBody = state;
		}

		/// Move based on the velocity
		void Move(glm::vec3 direction) {
			this->velocity += direction;
		}
	private:
		void BadSetup() const {
			DebugLog(LOG_ERROR, "Collider: " << this->name << " have no detection model set", true);
		}
		
	public:
		Mesh* mesh = nullptr;

		std::string name = "DefaultColliderName";

		bool isRigidBody = false;

		float mass = 1.0f;

		float bounce = 1.0f;
		
		glm::vec3 gravity = glm::vec3(0.0f, 0.0f, 0.0f);

		glm::vec3 velocity = glm::vec3(0.0f, 0.0f, 0.0f);

		bool reverseDelta = false;

		bool fixed = false;

		/// Collide only with objects that are fixed
		bool fixedCollisionOnly = false;

		std::string layer = "Default";

		/// Reset the velocity each update
		bool resetVelocity = true;
	};
}

#endif