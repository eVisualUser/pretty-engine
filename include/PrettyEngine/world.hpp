#pragma once


#include <PrettyEngine/localization.hpp>
#include <PrettyEngine/transform.hpp>
#include <PrettyEngine/visualObject.hpp>
#include <PrettyEngine/audio.hpp>
#include <PrettyEngine/render.hpp>
#include <PrettyEngine/entity.hpp>
#include <PrettyEngine/physics.hpp>
#include <PrettyEngine/reflect.hpp>
#include <PrettyEngine/data.hpp>

#include <Guid.hpp>

#include <sstream>
#include <fstream>
#include <unordered_map>
#include <utility>
#include <memory>

namespace PrettyEngine {
	class World;

	typedef void (*ProcessFunction)(World*);

	static void MTUpdate(bool* alive, bool* update, std::unordered_map<std::string, std::shared_ptr<Entity>>* entities) {
		while(*alive) {
			if (*update) {
				for (auto & entity: *entities) {
					if (entity.second != nullptr) {
						entity.second->OnMTUpdate(); 
					}
				}
				*update = false;
			}
		}
	}

	class World {
	public:
		World() {
			this->updateMTThreadAlive = true;
			this->updateMT = new std::thread(MTUpdate, &this->updateMTThreadAlive, &this->update, &this->entities);

			this->simulationCollider.colliderModel = ColliderModel::AABB;
			this->simulationCollider.scale = glm::vec3(100, 100, 100);
		}

		~World() {
			this->update = false;
			this->updateMTThreadAlive = false;
			this->StopMT();
			this->Clear();
		}

		void StopMT() {
			this->updateMTThreadAlive = false;
			this->updateMT->join();
			delete this->updateMT;
		}

		void Start() {
			for (auto & entity: this->entities) {
				if (entity.second != nullptr && entity.second->worldFirst) {
					this->UpdateLinks();
					entity.second->OnStart(); 
					entity.second->worldFirst = false;
				}
			}
		}

		void Update() {
			this->Start();
			for (auto & entity: this->entities) {
				if (entity.second != nullptr && this->simulationCollider.PointIn(entity.second->position)) {
					entity.second->OnUpdate();
				}
			}
		}

		void StartUpdateMT() {
			this->update = true;
		}

		void WaitUpdateMT() {
			while(update) {
				// Wait to the thread to call the fucntions
			}

			for (auto & entity: this->entities) {
				if (entity.second != nullptr && this->simulationCollider.PointIn(entity.second->position)) {
					entity.second->OnEndUpdate(); 
				}
			}
		}

		void EndUpdate() {
			for (auto & entity: this->entities) {
				if (entity.second != nullptr && this->simulationCollider.PointIn(entity.second->position)) {
					entity.second->OnEndUpdate(); 
				}
			}
		}

		void AlwaysEndUpdate() {
			for (auto & entity: this->entities) {
				if (entity.second != nullptr) {
					entity.second->OnAlwaysUpdate(); 
				}
			}
		}

		void AlwayUpdate() {
			for (auto & entity: this->entities) {
				if (entity.second != nullptr) {
					entity.second->OnAlwaysUpdate(); 
				}
			}
		}

		void RegisterEntity(std::shared_ptr<Entity> entity) {
			this->UpdateLinks();
			this->entities.insert(std::make_pair(entity->GetGUID(), entity));
		}

		void UnRegisterEntity(std::shared_ptr<Entity> entity) {
			this->entities[entity->GetGUID()]->OnDestroy();
			this->entities.erase(entity->GetGUID());
		}

		void SetSimulationDistance(float distance) {
			this->simulationCollider.scale = glm::vec3(distance, distance, distance);
		}

		void UpdateLinks() {
			for (auto & entity: this->entities) {
				entity.second->physicalEngine = this->physicalEngine;
				entity.second->audioEngine = this->audioEngine;
				entity.second->renderer = this->renderer;
				entity.second->engine = this->engine;

				for(auto & component: entity.second->components) {
					component.audioEngine = this->audioEngine;
					component.physicalEngine = this->physicalEngine;
					component.renderer = this->renderer;
					component.engine = this->engine;
				}
			}
		}

		std::vector<std::shared_ptr<Entity>> GetEntitiesByTag(std::string tag) {
			std::vector<std::shared_ptr<Entity>> out;
			for (auto & entity: this->entities) {
				if (entity.second->HaveTag(tag)) {
					out.push_back(entity.second);
				}
			}
			return out;
		}

		std::vector<std::shared_ptr<Entity>> GetEntitiesByTags(std::vector<std::string> tags) {
			std::vector<std::shared_ptr<Entity>> out;
			for (auto & entity: this->entities) {
				for (auto & tag: tags) {
					if (entity.second->HaveTag(tag)) {
						out.push_back(entity.second);
					}
				}
			}
			return out;
		}

		std::shared_ptr<Entity> GetEntityByTag(std::string tag) {
			for (auto & entity: this->entities) {
				if (entity.second->HaveTag(tag)) {
					return entity.second;
				}
			}
			return nullptr;
		}

		std::shared_ptr<Entity> GetEntityByTags(std::vector<std::string> tags) {
			for (auto & entity: this->entities) {
				for (auto & tag: tags) {
					if (entity.second->HaveTag(tag)) {
						return entity.second;
					}
				}
			}
			return nullptr;
		}

		void AddSharedData(std::string id, void* data) {
			this->sharedData.insert(std::make_pair(id, data));
		}

		void* GetSharedData(std::string id) {
			return this->sharedData[id];
		}

		void RemoveSharedData(std::string id) {
			this->sharedData.erase(id);
		}
		
		void AddProcessFunction(std::string id, ProcessFunction function) {
			this->processList.insert(std::make_pair(id, function));
		}
		
		void RemoveProcessFunction(std::string id) {
			this->processList.erase(id);
		}
		
		void CallFunctionProcesses() {
			for (auto & func: this->processList) {
				func.second(this);
			}
		}

		void CallRenderFunctions() {
			for (auto & entity: this->entities) {
				if (entity.second != nullptr) {
					entity.second->OnRender(); 
				}
			}
		}

		void Clear() {
			for(auto & entity: this->entities) {
				entity.second->OnDestroy();
			}

			this->entities.clear();

			return this->ThirdPartyClear();
		}

		void ThirdPartyClear() {
			this->renderer->Clear();
			this->physicalEngine->Clear();
		}
		
	public:
		std::thread* updateMT = nullptr;
		bool update = false;
		bool updateMTThreadAlive = true;

		std::unordered_map<std::string, std::shared_ptr<Entity>> entities;

		PhysicalEngine* physicalEngine = nullptr;
		AudioEngine* audioEngine = nullptr;
		Renderer* renderer = nullptr;

		std::unordered_map<std::string, void*> sharedData;

		std::unordered_map<std::string, ProcessFunction> processList;

		void* engine = nullptr;

		std::string worldName = "DefaultWorldName";

	public:
		Collider simulationCollider = Collider();

	private:
		DataBase* data = nullptr;
	};
}
