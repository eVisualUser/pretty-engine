#ifndef H_WORLD_LOAD
#define H_WORLD_LOAD

#include <PrettyEngine/dynamicObject.hpp>
#include <PrettyEngine/collider.hpp>
#include <PrettyEngine/world.hpp>
#include <PrettyEngine/utils.hpp>
#include <PrettyEngine/debug/debug.hpp>
#include <PrettyEngine/transform.hpp>
#include <PrettyEngine/serial.hpp>

#include <glm/vec3.hpp>

#include <memory>
#include <utility>
#include <vector>

namespace PrettyEngine {
	/// Manage the loading and saving of Worlds.
	class WorldManager {
	public:
   		/// Remove a world instance.
		void RemoveWorld(std::string worldName) {
			int index = 0;
			for(auto & world: this->_worlds) {
				if (world->worldName == worldName) {
					world.reset();
					this->_worlds.erase(this->_worlds.begin() + index);
					return;
				}
				index++;
			}
		}

		std::shared_ptr<World>* GetWorldByName(std::string worldName) {
			for(auto & world: this->_worlds) {
				if (world->worldName == worldName) {
					return &world;
				}
			}

			return nullptr;
		}

		void SaveWorlds() const {
			for(auto & world: this->_worlds) {
				world->Save();
			}
		}

  		void ClearWorldInstances() {
  			for(auto & world: this->_worlds) {
  				world.reset();
  			}
   			this->_worlds.clear();
  		}

		void LoadWorlds(bool forceLoad = false) {
			int index = 0;
			for(auto & world: this->_worlds) {
				world->Load();
			}
		}
		
		std::vector<std::shared_ptr<World>>* GetWorlds() {
			return &this->_worlds;
		}

		void Clear() {
			this->_worlds.clear();
		}

		bool WorldLoaded(std::string worldName) {
			for(auto & world: this->_worlds) {
				if (world->worldName == worldName) {
					return true;
				}
			}
			return false;
		}

		void RegisterWorld(std::shared_ptr<World> newWorld) {
			this->_worlds.push_back(newWorld);
		}

		World* AddWorld(std::string path, std::string name = "NewWorld") {
			this->_worlds.push_back(std::make_shared<World>(path));

			if (const auto addedWorld = this->_worlds.back()) {
				addedWorld->worldName = std::move(name);
				return addedWorld.get();
			}

			DebugLog(LOG_ERROR, "Failed to get back added world.", true);
			return nullptr;
		}

		std::vector<Request> GetAllDynamicObjectsRequests() {
			std::vector<Request> out;

			for(auto & world: this->_worlds) {
				for(auto & entity: world->entities) {
					for(auto & req: entity.second->requests) {
						out.push_back(req);
					}
					entity.second->requests.clear();
				}
			}

			return out;
		}

		// Reload the worlds
  		void Reload() const {
   			for(const auto & world: this->_worlds) {
   				world->Clear();
   				world->Load();
   			}
  		}

	private:
		std::vector<std::shared_ptr<World>> _worlds;
	};
}

#endif
