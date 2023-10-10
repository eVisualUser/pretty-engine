#pragma once

#include <PrettyEngine/physics.hpp>

namespace PrettyEngine {
	class PhysicalEngine {
	public:
		PhysicalEngine(): _dispatcher(&_collisionConfig), _world(&_dispatcher, &_broadphase, &_solver, &_collisionConfig) {
			this->_world.setGravity(btVector3(0.0f, 0.0f, 0.0f));

			this->simulationSpace.colliderModel = ColliderModel::AABB;
			this->SetSimulationSpace(100.0f);
		}
		
		~PhysicalEngine() {
			this->Clear();
		}
		
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

		void LinkObject(std::string id, std::shared_ptr<PhysicalObject> object);

		void UnLinkObject(std::string id);

		glm::vec3 GetGlobalGravity();

		void SetGlobalGravity(glm::vec3 newGravity);

		void AddConstraint(std::string id, btGeneric6DofConstraint* constraints);

		void RemoveConstraint(std::string id);

		void SetSimulationSpacePosition(glm::vec3* position);

		void Clear();

	private:
		std::unordered_map<std::string, std::shared_ptr<PhysicalObject>> _objects;

		btDefaultCollisionConfiguration _collisionConfig;
	    btCollisionDispatcher _dispatcher;
	    btDbvtBroadphase _broadphase;
	    btSequentialImpulseConstraintSolver _solver;

		btDiscreteDynamicsWorld _world;
		float _stepTime = 0.0f;

		std::unordered_map<std::string, std::shared_ptr<btGeneric6DofConstraint>> _constraints;

		Collider simulationSpace;
		glm::vec3* simulationSpacePosition = nullptr;
	};

	#define GET_PHYSICAL_ENGINE(ptr) (PrettyEngine::PhysicalEngine*)ptr
}