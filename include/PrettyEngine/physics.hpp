#pragma once

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

// Bullet
#include <LinearMath/btScalar.h>
#include <BulletCollision/CollisionDispatch/btGhostObject.h>
#include <BulletCollision/CollisionDispatch/btCollisionWorld.h>
#include <BulletCollision/BroadphaseCollision/btBroadphaseProxy.h>
#include <btBulletCollisionCommon.h>
#include <BulletDynamics/ConstraintSolver/btSequentialImpulseConstraintSolver.h>
#include <BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h>
#include <BulletCollision/CollisionDispatch/btCollisionObject.h>
#include <BulletCollision/CollisionShapes/btBoxShape.h>
#include <BulletCollision/CollisionShapes/btCollisionShape.h>
#include <BulletDynamics/Dynamics/btRigidBody.h>
#include <BulletDynamics/ConstraintSolver/btGeneric6DofConstraint.h>
#include <LinearMath/btMotionState.h>
#include <LinearMath/btTransform.h>
#include <LinearMath/btVector3.h>

#include <iostream>
#include <unordered_map>
#include <vector>

namespace PrettyEngine {
	static bool Vec3Greater(glm::vec3 a, glm::vec3 b) {
		return (a.x >= b.x && a.y >= b.y && a.z >= b.z);
	}

	static bool Vec3Lower(glm::vec3 a, glm::vec3 b) {
		return (a.x <= b.x && a.y <= b.y && a.z <= b.z);
	}

	enum class ColliderModel {
		AABB,
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
					auto min = this->GetMin();
					auto max = this->GetMax();

					return (Vec3Greater(other->GetMin(), min) && Vec3Lower(other->GetMax(), max));
				} else if (other->colliderModel == ColliderModel::Sphere) {
					glm::vec3 buffer = this->position - other->position;
					buffer = glm::clamp(buffer, this->GetMax(), this->GetMin());
					return glm::distance(buffer, other->position) < other->radius;
				}
			} else if (this->colliderModel == ColliderModel::Sphere) {
				if (other->colliderModel == ColliderModel::Sphere) {
					return other->radius + this->radius >= glm::distance(this->position, other->position);
				} else if (other->colliderModel == ColliderModel::AABB) {
					glm::vec3 buffer = other->position - this->position;
					buffer = glm::clamp(buffer, other->GetMax(), other->GetMin());
					return glm::distance(buffer, this->position) < this->radius;
				}
			}

			return this->BadSetup();
		}

		glm::vec3 GetMin() {
			return (this->position) - this->halfScale;
		}

		glm::vec3 GetMax() {
			return (this->position) + this->halfScale;
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
	};

	static void BulletLinkTest() {
		// Initialize Bullet3
	    btDefaultCollisionConfiguration collisionConfig;
	    btCollisionDispatcher dispatcher(&collisionConfig);
	    btDbvtBroadphase broadphase;
	    btSequentialImpulseConstraintSolver solver;
	    btDiscreteDynamicsWorld dynamicsWorld(&dispatcher, &broadphase, &solver, &collisionConfig);

	    // Create a rigid body (sphere) and add it to the dynamics world
	    btTransform transform;
	    transform.setIdentity();
	    btVector3 position(0, 0, 0);
	    transform.setOrigin(position);
	    btScalar radius = 1.0;
	    btCollisionShape* sphereShape = new btSphereShape(radius);
	    btDefaultMotionState* motionState = new btDefaultMotionState(transform);
	    btRigidBody::btRigidBodyConstructionInfo rbInfo(1.0, motionState, sphereShape);
	    btRigidBody* rigidBody = new btRigidBody(rbInfo);
	    dynamicsWorld.addRigidBody(rigidBody);

	    // Simulate a few time steps
	    for (int i = 0; i < 10; ++i) {
	        dynamicsWorld.stepSimulation(1.0 / 60.0);
	        btVector3 spherePosition = rigidBody->getWorldTransform().getOrigin();
	        std::cout << "Time Step " << i << ": Sphere position: (" << spherePosition.x() << ", "
	                  << spherePosition.y() << ", " << spherePosition.z() << ")" << std::endl;
	    }
	}

	enum class PhysicalType {
		Unknown = 0,
		RigidBody,
		TriggerSensor,
		Collider,
	};

	static int physicalObjectIDMax = 0;

	class PhysicalObject: virtual public Transform, virtual public Tagged {
	public:
		PhysicalObject() {
			this->bulletTransform.setIdentity();
		}

		~PhysicalObject() {
			delete this->rigidBody;
			delete this->motionState;
			delete this->ghostObject;
		}

		virtual void OnPhysicPreUpdate(void* physicEngine) {}
		virtual void OnPhysicPostUpdate(void* physicEngine) {}

	public:
		void CreateRigidbody(btCollisionShape* collisionShape) {
			collisionShape->calculateLocalInertia(1.0f, this->localInertia);

			this->bulletTransform.setOrigin(btVector3(this->position.x, this->position.y, this->position.z));
			this->bulletTransform.setRotation(btQuaternion(this->rotation.x, this->rotation.y, this->rotation.z, this->rotation.w));

			this->motionState = new btDefaultMotionState(this->bulletTransform);

			btRigidBody::btRigidBodyConstructionInfo rbInfo(1.0f, this->motionState, collisionShape);
			this->rigidBody = new btRigidBody(rbInfo);
			this->rigidBody->setMassProps(1.0f, this->localInertia);
			this->physicalType = PhysicalType::RigidBody;
			this->bulletTransform = this->rigidBody->getWorldTransform();
			this->rigidBody->setUserIndex(physicID);
		}

		void CreateCollider(btCollisionShape* collisionShape, bool isTrigger = true) {
			this->bulletTransform.setOrigin(btVector3(this->position.x, this->position.y, this->position.z));
			this->bulletTransform.setRotation(btQuaternion(this->rotation.x, this->rotation.y, this->rotation.z, this->rotation.w));

			this->ghostObject = new btGhostObject();

			this->ghostObject->setCollisionShape(collisionShape);
			this->ghostObject->setWorldTransform(this->bulletTransform);
			this->ghostObject->setUserIndex(this->physicID);

			if (isTrigger) {
				this->ghostObject->setCollisionFlags(this->ghostObject->getCollisionFlags() | btCollisionObject::CF_NO_CONTACT_RESPONSE);
				this->physicalType = PhysicalType::TriggerSensor;
			} else {
				this->physicalType = PhysicalType::Collider;
			}

		}
		
		void SetMass(float mass, glm::vec3 inertia = glm::zero<glm::vec3>()) {
			if (this->physicalType == PhysicalType::RigidBody) {
				this->rigidBody->setMassProps(mass, btVector3(inertia.x, inertia.y, inertia.z));
			}
		}

		void LinkBulletDynamicWorld(btDiscreteDynamicsWorld* dynamicWorld) {
			if (this->physicalType == PhysicalType::RigidBody) {
				dynamicWorld->addRigidBody(this->rigidBody);
			} else if (this->physicalType == PhysicalType::TriggerSensor || this->physicalType == PhysicalType::Collider) {
				dynamicWorld->addCollisionObject(this->ghostObject, btBroadphaseProxy::SensorTrigger, btBroadphaseProxy::AllFilter & ~btBroadphaseProxy::SensorTrigger);
			}
		}

		void UnLinkBulletDynamicWorld(btDiscreteDynamicsWorld* dynamicWorld) {
			if (this->physicalType == PhysicalType::RigidBody) {
				dynamicWorld->removeRigidBody(this->rigidBody);
			} else if (this->physicalType == PhysicalType::TriggerSensor ||this->physicalType == PhysicalType::Collider) {
				dynamicWorld->removeCollisionObject(this->ghostObject);
			}
		}

		void UpdatePhysicTransform(void* physicalEngine) {
			this->OnPhysicPreUpdate(physicalEngine);

			this->bulletTransform.setOrigin(btVector3(this->position.x, this->position.y, this->position.z));
			this->bulletTransform.setRotation(btQuaternion(this->rotation.x, this->rotation.y, this->rotation.z, this->rotation.w));

			if (this->physicalType == PhysicalType::RigidBody) {
				this->rigidBody->setWorldTransform(this->bulletTransform);
			} else if (this->physicalType == PhysicalType::TriggerSensor || this->physicalType == PhysicalType::Collider) {
				this->ghostObject->setWorldTransform(this->bulletTransform);
			}
		}

		void UpdateObjectTransform(void* physicalEngine) {
			btTransform newTransform;

			if (this->physicalType == PhysicalType::RigidBody) {
				newTransform = this->rigidBody->getWorldTransform();
			} else if (this->physicalType == PhysicalType::TriggerSensor || this->physicalType == PhysicalType::Collider) {
				newTransform = this->ghostObject->getWorldTransform();
			} else {
				return;
			}

			this->bulletTransform = newTransform;

			auto newPosition = newTransform.getOrigin();
			this->position.x = newPosition.getX();
			this->position.y = newPosition.getY();
			this->position.z = newPosition.getZ();
		
			auto newRotation = newTransform.getRotation();
			this->rotation.x = newRotation.getX();
			this->rotation.y = newRotation.getY();
			this->rotation.z = newRotation.getZ();
			this->rotation.w = newRotation.getW();

			this->OnPhysicPostUpdate(physicalEngine);
		}

		void UnLink() {
			this->unlink = true;
		}

		void ResetLinearVelocity() {
			if (this->physicalType == PhysicalType::RigidBody) {
				this->rigidBody->setLinearVelocity(btVector3(0.0f, 0.0f, 0.0f));
			} else if (this->physicalType == PhysicalType::TriggerSensor || this->physicalType == PhysicalType::Collider) {
				return;
			} else {
				return;
			}
		}

		void SetGravity(glm::vec3 gravity) {
			if (this->physicalType == PhysicalType::RigidBody) {
				this->rigidBody->setGravity(btVector3(gravity.x, gravity.y, gravity.z));
			} else if (this->physicalType == PhysicalType::TriggerSensor || this->physicalType == PhysicalType::Collider) {
				return;
			} else {
				return;
			}

		}

		void SetLinearVelocity(glm::vec3 newVelocity) {
			if (this->physicalType == PhysicalType::RigidBody) {
				this->rigidBody->setLinearVelocity(btVector3(newVelocity.x, newVelocity.y, newVelocity.z));
			}
		}

		glm::vec3 GetLinearVelocity() {
			glm::vec3 out;
			out.x = this->rigidBody->getLinearVelocity().getX();
			out.y = this->rigidBody->getLinearVelocity().getY();
			out.z = this->rigidBody->getLinearVelocity().getZ();
			return out;
		}

		void SetLinearVelocity(glm::vec3* newVelocity) {
			if (this->physicalType == PhysicalType::RigidBody) {
				this->rigidBody->setLinearVelocity(btVector3(newVelocity->x, newVelocity->y, newVelocity->z));
			}
		}

		btRigidBody* GetRigidBody() {
			return this->rigidBody;
		}

		int GetID() {
			if (this->physicalType == PhysicalType::RigidBody) {
				return this->physicID;
			}
			return this->physicID;
		}

		virtual void OnCollide(PhysicalObject* other) {}

	public:
		PhysicalType physicalType;

		btRigidBody* rigidBody = nullptr;
		btGhostObject* ghostObject = nullptr;

		btDefaultMotionState* motionState;
		btTransform bulletTransform;

		btVector3 localInertia = btVector3(1.0f, 1.0f, 1.0f);

		bool unlink = false;
		bool collisionFeedBack = false;

		const int physicID = ++physicalObjectIDMax;

		std::string id;
	};

	class PhysicalEngine {
	public:
		PhysicalEngine(): _dispatcher(&_collisionConfig), _world(&_dispatcher, &_broadphase, &_solver, &_collisionConfig) {
			this->_world.setGravity(btVector3(0.0f, 0.0f, 0.0f));

			this->simulationSpace.colliderModel = ColliderModel::AABB;
			this->SetSimulationSpace(100.0f);
		}
		
		~PhysicalEngine() {}

		void SetSimulationSpace(float distance) {
			this->simulationSpace.SetScale(glm::vec3(distance, distance, distance));
			this->simulationSpace.UpdateHalfScale();		
		}

		void Simulate() {
			if (this->simulationSpacePosition != nullptr) {
				this->simulationSpace.position = *simulationSpacePosition;
			}

			for (auto & pair: this->_objects) {
				pair.second->UpdatePhysicTransform(this);

				if (pair.second->rigidBody != nullptr) {
					if (this->simulationSpace.PointIn(pair.second->position)) {
						pair.second->rigidBody->activate();
					} else {
						pair.second->rigidBody->setActivationState(DISABLE_SIMULATION);
					}
				}
				
				if (pair.second->ghostObject != nullptr) {
					if (this->simulationSpace.PointIn(pair.second->position)) {
						pair.second->ghostObject->activate();
					} else {
						pair.second->ghostObject->setActivationState(DISABLE_SIMULATION);
					}
				}

				if (pair.second->unlink) {
					this->UnLinkObject(pair.second->id);
				}
			}

			this->_world.updateVehicles(this->_stepTime);
			this->_world.updateAabbs();
			this->_world.stepSimulation(this->_stepTime);

			for(int i = 0; i < this->_dispatcher.getNumManifolds(); i++) {
				auto manifold = this->_dispatcher.getManifoldByIndexInternal(i);

				auto col0 = manifold->getBody0();
				auto col1 = manifold->getBody1();

				for (auto & object: this->_objects) {
					if (object.second->GetID() == col0->getUserIndex()) {
						for (auto & object2: this->_objects) {
							if (object2.second->GetID() == col1->getUserIndex()) {
								object.second->OnCollide(object2.second);
								object2.second->OnCollide(object.second);
							}
						}
					}
				}
			}

			for (auto & pair: this->_objects) {
				pair.second->UpdateObjectTransform(this);
			}
		}

		void SetStepTime(float newStepTime) {
			this->_stepTime = newStepTime;
		}

		void LinkObject(std::string id, PhysicalObject* object) {
			object->LinkBulletDynamicWorld(&this->_world);
			object->id = id;
			this->_objects.insert(std::make_pair(id, object));
		}

		void UnLinkObject(std::string id) {
			this->_objects[id]->UnLinkBulletDynamicWorld(&this->_world);
			_objects.erase(id);
		}

		glm::vec3 GetGlobalGravity() {
			btVector3 gravity = this->_world.getGravity();
			return glm::vec3(gravity.getX(), gravity.getY(), gravity.getZ());
		}

		void SetGlobalGravity(glm::vec3 newGravity) {
			this->_world.setGravity(btVector3(newGravity.x, newGravity.y, newGravity.z));
		}

		void AddConstraint(std::string id, btGeneric6DofConstraint* constraints) {
			this->_constraints.insert(std::make_pair(id, constraints));
			this->_world.addConstraint(constraints);
		}

		void RemoveConstraint(std::string id) {
			this->_world.removeConstraint(this->_constraints[id]);
			this->_constraints.erase(id);
		}

		void SetSimulationSpacePosition(glm::vec3* position) {
			this->simulationSpacePosition = position;
		}

		void Clear() {
			for(auto & constraint: this->_constraints) {
				this->RemoveConstraint(constraint.first);
			}

			for(auto & physicalObject: this->_objects) {
				this->UnLinkObject(physicalObject.first);
			}
		}
		
	private:
		std::unordered_map<std::string, PhysicalObject*> _objects;

		btDefaultCollisionConfiguration _collisionConfig;
	    btCollisionDispatcher _dispatcher;
	    btDbvtBroadphase _broadphase;
	    btSequentialImpulseConstraintSolver _solver;

		btDiscreteDynamicsWorld _world;
		float _stepTime = 0.0f;

		std::unordered_map<std::string, btGeneric6DofConstraint*> _constraints;

		Collider simulationSpace;
		glm::vec3* simulationSpacePosition = nullptr;
	};

	#define GET_PHYSICAL_ENGINE(ptr) (PrettyEngine::PhysicalEngine*)ptr
}
