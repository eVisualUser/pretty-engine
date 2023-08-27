#pragma once

#include <PrettyEngine/debug.hpp>
#include <PrettyEngine/transform.hpp>

#include <glm/vec3.hpp>
#include <limits.h>

#include <unordered_map>
#include <string>
#include <vector>

namespace PrettyEngine {
	class Space: public Transform {
	public:
		void SetOffset(glm::vec3 offset) {
			this->preLoadStartOffset = -offset;
			this->preLoadEndOffset = offset;
		}

		void SetStartPoint(glm::vec3 point) {
			this->startPoint = point;
		}

		void SetEndPoint(glm::vec3 point) {
			this->startPoint = point;
		}

		glm::vec3 GetRealStartPoint() {
			return this->startPoint + this->preLoadStartOffset + this->position;
		}

		glm::vec3 GetRealEndPoint() {
			return this->endPoint + this->preLoadEndOffset + this->position;
		}

		/// Return true if the point is in the space scope
		bool CheckPoint(glm::vec3 point) {
			auto realStartPoint = this->GetRealStartPoint(); 
			auto realEndPoint = this->GetRealEndPoint();

			if (point.x < realStartPoint.x || point.y < realStartPoint.y || point.z < realStartPoint.z)
				return false;
			else if (point.x > realEndPoint.x || point.y > realEndPoint.y || point.z > realEndPoint.z)
				return false;

			return true; 
		}

	public:
		glm::vec3 startPoint = glm::vec3(-1.0f, -1.0f, -1.0f);
		glm::vec3 endPoint = glm::vec3(1.0f, 1.0f, 1.0f);

		/// Added to start point to active the space before the target being in
		glm::vec3 preLoadStartOffset;
		/// Added to end point to active the space before the target being in
		glm::vec3 preLoadEndOffset;
	};

	template<typename T>
	class SimulatedObject: public Transform {
	public:
		bool RegisteredInObjectPack() {
			return (this->objectPack != nullptr);
		}

	public:
		/// True if the object is simulated(active)
		bool simulated = true;
		/// Name of the object
		std::string name;

		std::string objectPack = nullptr;
	};

	template<typename T>
	class ObjectPack {
	public:
		size_t GetObjectLimit() {
			return this->objectLimit;
		}

		bool ReachedLimit() {
			return (this->GetObjectLimit() >= this->objectCount);
		}

		void SetObjectLimit(size_t value) {
			this->objectLimit = value;
		}

		void SetSpace(Space newSpace) {
			this->space = newSpace;
		}

		bool IsValidSpace(glm::vec3 testPoint) {
			return this->space.CheckPoint(testPoint);
		}
	public:
		std::string name = "DefaultPack";

		Space space;

		size_t objectCount = 0;
		size_t objectLimit = INT_MAX;
	};

	template<typename T>
	class SimSpace {
	public:
		bool RegisterSimulatedObject(ObjectPack<T>* pack, SimulatedObject<T>* object) {
			if (!pack->ReachedLimit()) {
				pack->objectCount++;
				object->objectPack = pack->name;
			}
			return false;
		}

		void UnRegisterSimulatedObject(ObjectPack<T>* pack, SimulatedObject<T>* object) {
			pack->objectCount--;
			object->objectPack = nullptr;
		}

		ObjectPack<T>* AddObjectPack(std::string name) {
			auto objectPack = ObjectPack<T>();
			objectPack.name = name;

			this->objectPackList.push_back(objectPack);
			return &this->objectPackList.back();
		}

		std::vector<ObjectPack<T>*> GetValidObjectPack() {
			std::vector<ObjectPack<T>*> out;

			for (auto & objectPack: this->objectPackList) {
				if (objectPack->IsValidSpace(*this->cameraPosition)) {
					out.push_back(objectPack);					
				}
			}
			return out;
		}
	public:
		glm::vec3* cameraPosition;

		std::vector<ObjectPack<T>> objectPackList;
	};
}
