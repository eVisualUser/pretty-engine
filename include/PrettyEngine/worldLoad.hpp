#pragma once

#include <PrettyEngine/dynamicObject.hpp>
#include <PrettyEngine/physics.hpp>
#include <PrettyEngine/world.hpp>
#include <PrettyEngine/utils.hpp>
#include <PrettyEngine/debug.hpp>
#include <PrettyEngine/transform.hpp>

#include <custom.hpp>
#include <components.hpp>

#include <fstream>
#include <glm/vec3.hpp>
#include <toml++/toml.h>

#include <memory>
#include <unordered_map>

namespace PrettyEngine {
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

		std::shared_ptr<World> GetWorldByName(std::string worldName) {
			for(auto & world: this->_worldsInstances) {
				if (world->worldName == worldName) {
					return world;
				}
			}

			return nullptr;
		}

		WorldManager* CreateWorldsInstances() {
			for(auto & world: this->_worldsFilesParsed) {
				this->_worldsInstances.push_back(std::make_shared<World>());
			}
			return this;
		}

		void SaveWorlds() {
			int index = 0;
			for(auto & world: this->_worldsInstances) {
				auto filePath = this->_worldsFiles[index];

				std::ofstream out;
				out.open(filePath);

				if (out.is_open()) {
					auto base = toml::parse("");
					base.insert_or_assign("meta", toml::table{});
					base["meta"].as_table()->insert_or_assign("name", world->worldName);

					base.insert_or_assign("entities", toml::table{});
					for(auto & entity: world->entities) {
						auto entitiesTable = base["entities"].as_table();
						entitiesTable->insert_or_assign(entity.second->unique, toml::table{});
						auto entityTable = (*entitiesTable)[entity.second->unique].as_table();
						entityTable->insert_or_assign("name", entity.second->entityName);
						entityTable->insert_or_assign("object", entity.second->object);

						entityTable->insert_or_assign("transform", toml::table{});
						auto transformTable = (*entityTable)["transform"].as_table();
						entity.second->GetTransform()->AddToToml(transformTable);

						auto publicMap = toml::array();
						for(auto & element: entity.second->publicMap) {
							auto pair = toml::array();
							pair.push_back(element.first);
							pair.push_back(element.second);
							publicMap.push_back(pair);
						}

						entityTable->insert_or_assign("public_map", publicMap);

						auto components = toml::array();
						for(auto & component: entity.second->components) {
							auto componentOut = toml::array();

							componentOut.push_back(component->unique);
							componentOut.push_back(component->object);

							auto componentPublicMap = toml::array();
							for(auto & element: component->publicMap) {
								componentOut.push_back(element.first);
								componentOut.push_back(element.second);
							}

							components.push_back(componentOut); 
						}
						entityTable->insert_or_assign("components", components);
					}
					out << base;
					out.flush();
					out.close();
				}

				index++;
			}
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
								lastEntity->FromToml(transform.as_table());
							}
						}
					}
				}
				index++;
			}
		}
		
		std::vector<std::shared_ptr<World>> GetWorlds() {
			std::vector<std::shared_ptr<World>> out;
			
			for(int i = 0; i < this->_worldsInstances.size(); i++) {
				out.push_back(this->_worldsInstances[i]);
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

		bool WorldLoaded(std::string worldName) {
			for(auto & world: this->_worldsInstances) {
				if (world->worldName == worldName) {
					return true;
				}
			}
			return false;
		}

		void RegisterWorld(std::shared_ptr<World> newWorld) {
			this->_worldsInstances.push_back(newWorld);
		}

		std::vector<Request> GetAllDynamicObjectsRequests() {
			std::vector<Request> out;

			for(auto & world: this->_worldsInstances) {
				for(auto & entity: world->entities) {
					for(auto & req: entity.second->requests) {
						out.push_back(req);
					}
					entity.second->requests.clear();
				}
			}

			return out;
		}

	private:
		std::vector<std::string> _worldsFiles;

		std::vector<std::string*> _worldsFilesToLoad;
		std::vector<toml::parse_result> _worldsFilesParsed;
		std::vector<std::shared_ptr<World>> _worldsInstances;
	};
}
