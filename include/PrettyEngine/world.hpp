#pragma once

#include <PrettyEngine/EngineContent.hpp>
#include <PrettyEngine/transform.hpp>
#include <PrettyEngine/render.hpp>
#include <PrettyEngine/entity.hpp>
#include <PrettyEngine/collider.hpp>
#include <PrettyEngine/data.hpp>

#include <glm/vec3.hpp>

#include <unordered_map>
#include <memory>
#include <thread>

namespace PrettyEngine {
	class World;

	typedef void (*ProcessFunction)(World*);

	static void MTUpdate(bool* alive, bool* update, std::unordered_map<std::string, std::shared_ptr<Entity>>* entities) {
		while(*alive) {
			if (*update) {
				for (auto & entity: *entities) {
					if (entity.second != nullptr) {
						entity.second->OnMTUpdate(); 
						for (auto & component: entity.second->components) {
							component->OnMTUpdate();
						}
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
			this->simulationCollider.SetScale(glm::vec3(100, 100, 100));
		}
		
		~World() {
			this->update = false;
			this->updateMTThreadAlive = false;
			this->StopMT();
			this->Clear();
			delete this->updateMT;
		}

		void StopMT() {
			this->updateMTThreadAlive = false;
			this->updateMT->join();
		}
		
		void Start() {
			for (auto & entity: this->entities) {
				if (entity.second != nullptr) {
					this->UpdateLinks();
					if (entity.second->worldFirst) {
						entity.second->OnStart(); 
						entity.second->worldFirst = false;
					}
					for (auto & component: entity.second->components) {
						if (component->worldFirst) {
							component->OnStart();
							component->worldFirst = false;
						}
					}
				}
			}
		}

		void Update() {
			this->Start();
			for (auto & entity: this->entities) {
				if (entity.second.get() != nullptr && this->simulationCollider.PointIn(entity.second->position)) {
					entity.second->OnUpdate();
					for (auto & component: entity.second->components) {
						component->OnUpdate();
					}
				}
			}
		}

		void EditorUpdate() {
			#if ENGINE_EDITOR
				for (auto & entity: this->entities) {
					if (entity.second.get() != nullptr && this->simulationCollider.PointIn(entity.second->position)) {
						entity.second->OnEditorUpdate();
						for (auto & component: entity.second->components) {
							component->OnEditorUpdate();
						}
					}
				}
			#endif
		}

		void StartUpdateMT() {
			this->update = true;
		}

		void WaitUpdateMT() {
			while(update) {
				// Wait to the thread to call the fucntions
			}

			this->EndUpdate();
		}

		void EndUpdate() {
			for (auto & entity: this->entities) {
				if (entity.second != nullptr && this->simulationCollider.PointIn(entity.second->position)) {
					entity.second->OnEndUpdate(); 
					for (auto & component: entity.second->components) {
						component->OnEndUpdate();
					}
				}
			}
		}

		void PrePhysics() {
			for (auto & entity: this->entities) {
				if (entity.second != nullptr && this->simulationCollider.PointIn(entity.second->position)) {
					entity.second->OnPrePhysics(); 
					for (auto & component: entity.second->components) {
						component->OnPrePhysics();
					}
				}
			}
		}

		void AlwaysEndUpdate() {
			for (auto & entity: this->entities) {
				if (entity.second != nullptr) {
					entity.second->OnAlwaysUpdate(); 
					for (auto & component: entity.second->components) {
						component->OnAlwaysEndUpdate();
					}
				}
			}
		}

		void AlwayUpdate() {
			for (auto & entity: this->entities) {
				if (entity.second != nullptr) {
					entity.second->OnAlwaysUpdate();
					for (auto & component: entity.second->components) {
						component->OnAlwaysUpdate();
					}
				}
			}
		}

		void CallRenderFunctions() {
			for (auto & entity: this->entities) {
				if (entity.second != nullptr) {
					entity.second->OnRender(); 
					for (auto & component: entity.second->components) {
						component->OnRender();
					}
				}
			}
		}

		void CallFunctionProcesses() {
			for (auto & func: this->processList) {
				func.second(this);
			}
		}

		void RegisterEntity(std::shared_ptr<Entity> entity) {
			this->UpdateLinks();
			this->entities.insert(std::make_pair(entity->GetGUID(), entity));
			this->lastEntityRegistred = entity->GetGUID();
		}
		
		std::shared_ptr<Entity> GetLastEntityRegistred() {
			return this->entities[this->lastEntityRegistred];
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
				entity.second->engineContent = this->engineContent;

				for(auto & component: entity.second->components) {
					component->engineContent = this->engineContent;
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

		std::shared_ptr<Entity> GetEntityByName(std::string name) {
			for (auto & entity: this->entities) {
				if (entity.second->entityName == name) {
					return entity.second;
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
		
		void Clear() {
			for(auto & entity: this->entities) {
				entity.second->OnDestroy();
			}
			this->entities.clear();
		}

		std::unordered_map<std::string, std::shared_ptr<Entity>>* GetEntities() {
			return &this->entities;
		}

	public:
		std::thread* updateMT = nullptr;
		bool update = false;
		bool updateMTThreadAlive = true;

		std::unordered_map<std::string, std::shared_ptr<Entity>> entities;

		std::string lastEntityRegistred;

		EngineContent* engineContent;

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
