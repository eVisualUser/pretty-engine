#include "PrettyEngine/debug.hpp"
#include <PrettyEngine/physicsEngine.hpp>

namespace PrettyEngine {
	void PhysicalEngine::SetSimulationSpacePosition(glm::vec3* position) {
		this->simulationSpacePosition = position;
	}

	void PhysicalEngine::RemoveConstraint(std::string id) {
		this->_world.removeConstraint(this->_constraints[id].get());
		this->_constraints.erase(id);
	}

	void PhysicalEngine::AddConstraint(std::string id, btGeneric6DofConstraint* constraints) {
		this->_constraints.insert(std::make_pair(id, constraints));
		this->_world.addConstraint(constraints);
	}

	void PhysicalEngine::SetGlobalGravity(glm::vec3 newGravity) {
		this->_world.setGravity(btVector3(newGravity.x, newGravity.y, newGravity.z));
	}

	glm::vec3 PhysicalEngine::GetGlobalGravity() {
		btVector3 gravity = this->_world.getGravity();
		return glm::vec3(gravity.getX(), gravity.getY(), gravity.getZ());
	}

	void PhysicalEngine::Clear() {
		for(auto & constraint: this->_constraints) {
			this->RemoveConstraint(constraint.first);
		}

		while(!this->_objects.empty()) {
			for(auto & physicalObject: this->_objects) {
				if (physicalObject.second.get() != nullptr) {
					this->UnLinkObject(physicalObject.first);
				}
				break;
			}	
		}
	}

	void PhysicalEngine::LinkObject(std::string id, std::shared_ptr<PhysicalObject> object) {
		object->LinkBulletDynamicWorld(&this->_world);
		object->id = id;
		this->_objects.insert(std::make_pair(id, object));
	}

	void PhysicalEngine::UnLinkObject(std::string id) {
		if (this->_objects[id] != nullptr) {
			this->_objects[id]->UnLinkBulletDynamicWorld(&this->_world);
			_objects.erase(id);
		}
	}
}
