#pragma once

#include <PrettyEngine/physics.hpp>
#include <PrettyEngine/world.hpp>
#include <PrettyEngine/utils.hpp>
#include <PrettyEngine/debug.hpp>
#include <PrettyEngine/transform.hpp>

#include <custom.hpp>
#include <components.hpp>

#include <glm/vec3.hpp>
#include <toml++/toml.h>

#include <memory>
#include <unordered_map>

namespace PrettyEngine {
	static void ApplyLoadedTransform(toml::table* transformTable, std::shared_ptr<Transform> transform) {
		bool isArray = (*transformTable)["position"].is_array();
		if (isArray) {
			if ((*transformTable)["position"].is_array()) {
				auto position = (*transformTable)["position"].as_array();

				transform->position.x = position->get_as<double>(0)->value_or(0.0f);
				transform->position.y = position->get_as<double>(1)->value_or(0.0f);
				transform->position.z = position->get_as<double>(2)->value_or(0.0f);

				auto rotation = (*transformTable)["rotation"].as_array();

				transform->rotation.x = rotation->get_as<double>(0)->value_or(0.0f);
				transform->rotation.y = rotation->get_as<double>(1)->value_or(0.0f);
				transform->rotation.z = rotation->get_as<double>(2)->value_or(0.0f);
				transform->rotation.w = rotation->get_as<double>(3)->value_or(0.0f);

				auto scale = (*transformTable)["scale"].as_array();

				transform->scale.x = scale->get_as<double>(0)->value_or(0.0f);
				transform->scale.y = scale->get_as<double>(1)->value_or(0.0f);
				transform->scale.z = scale->get_as<double>(2)->value_or(0.0f);

				auto arraySize = position->size();
			}
		}
	}

	class WorldManager {
	public:
		void AddWorldFile(std::string fileName) {
			this->_worldsFiles.push_back(fileName);
		}

		void RemoveWorldFile(std::string fileName) {
			int index = 0;
			for(auto & world: this->_worldsFiles) {
				if (world == fileName) {
					this->_worldsFiles.erase(this->_worldsFiles.begin() + index);
					return;
				}
				index++;
			}
		}

		WorldManager* FilterWorldList(bool (*filter)(std::string)) {
			for(auto & worldFile: this->_worldsFiles) {
				if (filter(worldFile)) {
					this->_worldsFilesToLoad.push_back(&worldFile);
				}
			}
			return this;
		}

		WorldManager* ParseWorldsFiles() {
			for(auto & world: this->_worldsFilesToLoad) {
				this->_worldsFilesParsed.push_back(toml::parse_file(*world));
			}
			this->_worldsFilesToLoad.clear();
			return this;
		}

		bool UpdateWorldTrigger(World* world) {
			return this->UpdateWorldTrigger(world->worldName);
		}		

		std::shared_ptr<World> GetWorldByName(std::string worldName) {
			for(auto & world: this->_worldsInstances) {
				if (world->worldName == worldName) {
					return world;
				}
			}

			return nullptr;
		}

		bool UpdateWorldTrigger(std::string worldName) {
			for(auto & world: this->_worldsFilesParsed) {
				auto currentWorldName = world["meta"]["name"].as_string()->value_or("Unknown");
				if (currentWorldName == worldName) {
					if (world["meta"]["trigger"].is_table()) {
						auto rawPosition = world["meta"]["trigger"]["position"].as_array();
						glm::vec3 position;
						if (rawPosition->size() == 1) {
							position.x = rawPosition->get(0)->value_or(0.0f);
						}
						if (rawPosition->size() == 2) {
							position.y = rawPosition->get(1)->value_or(0.0f);
						}
						if (rawPosition->size() == 3) {
							position.z = rawPosition->get(2)->value_or(0.0f);
						}
						this->GetWorldByName(currentWorldName)->simulationCollider.position = position;
						return true;
					} else {
						return false;
					}
				}
			}
			return false;
		}

		WorldManager* CreateWorldsInstances() {
			for(auto & world: this->_worldsFilesParsed) {
				this->_worldsInstances.push_back( std::make_shared<World>());
			}
			return this;
		}
		
		void LoadWorlds() {
			int index = 0;
			for(auto & world: this->_worldsFilesParsed) {
				if (!world.empty()) {
					auto target = this->_worldsInstances[index];
					target->worldName = world["meta"]["name"].value_or("World");
					
					if (world["entities"].is_table()) {
						for(auto & entity: *world["entities"].as_table()) {
							std::string newEntity = (*entity.second.as_table())["object"].value_or("undefined");
							std::string newEntityName = (*entity.second.as_table())["name"].value_or("undefined");
							
							CreateCustomEntity(newEntity, target);
							auto lastEntity = target->GetLastEntityRegistred();
							lastEntity->entityName = newEntityName;

							auto components = (*entity.second.as_table())["components"].as_array();
							for(auto & component: *components) {
								auto array = component.as_array();
								std::string componentName = array->get(0)->value_or("Null");
								std::string componentUnique = array->get(1)->value_or("Null");

								std::unordered_map<std::string, std::string> publicMap;
								int publicMapStartOffset = 2; // Skip the component name and unique
								std::pair<std::string, std::string> pairBuffer;

								for (int i = publicMapStartOffset; i < array->size(); i++) {
									if (i % 2) { // Value
										pairBuffer.second = array->get(i)->value_or("Null");
										publicMap.insert(pairBuffer);
									} else { // Name
										pairBuffer.first = array->get(i)->value_or("Null");
									}
								}

								auto newComponent = GetCustomComponent(componentName);
								newComponent->publicMap = publicMap;
								newComponent->owner = lastEntity->entityName;
								newComponent->object = componentName;
								newComponent->unique = componentUnique;
								lastEntity->components.push_back(newComponent);
							}
							
							auto publicMap = (*entity.second.as_table())["public_map"].as_array();
							for(auto & pair: *publicMap) {
								std::string pairName = pair.as_array()->get_as<std::string>(0)->value_or("Unknown");
								std::string pairValue = pair.as_array()->get_as<std::string>(1)->value_or("");

								lastEntity->publicMap.insert(std::make_pair(pairName, pairValue));
							}

							lastEntity->unique = entity.first;
							lastEntity->object = newEntity;
							
							auto transform = (*entity.second.as_table())["transform"];
							if (transform.is_table() && target->GetLastEntityRegistred() != nullptr) {
								ApplyLoadedTransform(transform.as_table(), lastEntity);
							}
						}
					}
				}
				index++;
			}
		}

		std::vector<std::shared_ptr<World>> GetWorlds(glm::vec3 pointIndice, bool checkPosition = true) {
			std::vector<std::shared_ptr<World>> out;
			
			for(int i = 0; i < this->_worldsInstances.size(); i++) {
				auto world = this->_worldsInstances[i];
				if (this->UpdateWorldTrigger(world->worldName) && checkPosition) {
					if (world->simulationCollider.PointIn(pointIndice)) {
						out.push_back(world);
					}
				} else {
					out.push_back(world);
				}
			}

			return out;
		}

		void Clear() {
			this->_worldsFiles.clear();
			this->_worldsFilesParsed.clear();
			this->_worldsFilesToLoad.clear();
			this->_worldsFilesParsed.clear();
			this->_worldsInstances.clear();
		}

	private:
		std::vector<std::string> _worldsFiles;

		std::vector<std::string*> _worldsFilesToLoad;
		std::vector<toml::parse_result> _worldsFilesParsed;
		std::vector<std::shared_ptr<World>> _worldsInstances;
	};
}
