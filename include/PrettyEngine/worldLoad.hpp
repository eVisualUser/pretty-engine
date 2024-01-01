#ifndef H_WORLD_LOAD
#define H_WORLD_LOAD

#include <PrettyEngine/dynamicObject.hpp>
#include <PrettyEngine/collider.hpp>
#include <PrettyEngine/world.hpp>
#include <PrettyEngine/utils.hpp>
#include <PrettyEngine/debug/debug.hpp>
#include <PrettyEngine/transform.hpp>
#include <PrettyEngine/serial.hpp>

#include <custom.hpp>
#include <components.hpp>

#include <fstream>
#include <glm/vec3.hpp>

#include <memory>
#include <vector>

namespace PrettyEngine {
	/// Manage the loading and saving of Worlds.
	class WorldManager {
	public:
   		/// Remove a world instance.
		void RemoveWorld(std::string worldName) {
			int index = 0;
			for(auto & world: this->_worldsInstances) {
				if (world->worldName == worldName) {
					this->_worldsInstances.erase(this->_worldsInstances.begin() + index);
					return;
				}
				index++;
			}
		}

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

  		void ActiveAllWorldsFiles() {
   			for(auto & worldFile: this->_worldsFiles) {
    			this->_worldsFilesToLoad.push_back(&worldFile);
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

		std::vector<std::string> FindErrors() {
			std::vector<std::string> result;

			for(auto & path: this->_worldsFiles) {
				if (!FileExist(path)) {
					result.push_back("World file not found: " + path);
				}
			}

			if (this->_worldsInstances.size() < this->_worldsFiles.size()) {
				result.push_back("Missing world instances");
			}

			return result;
		}

		void SaveWorlds() {
			int index = 0;
			for(auto & world: this->_worldsInstances) {
				auto & filePath = this->_worldsFiles[index];

				std::ofstream out;
				out.open(filePath);

				if (out.is_open()) {
					auto base = toml::parse("");
					base.insert_or_assign("meta", toml::table{});
					base["meta"].as_table()->insert_or_assign("name", world->worldName);
					
					// Skip the world if not in editor
					if (base["meta"]["editor_only"].value_or(false) == true) {
						#if ENGINE_EDITOR
						
						#else
						continue;
						#endif
					}

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

						entityTable->insert_or_assign("serial", toml::table(toml::parse(entity.second->Serialize(SerializationFormat::Toml))));

						auto componentTable = toml::table();
						for(auto & component: entity.second->components) {
							componentTable.insert_or_assign(component->unique, toml::table(toml::parse(component->Serialize(SerializationFormat::Toml))));
						}
						entityTable->insert_or_assign("components", componentTable);
					}
					out << base;
					out.flush();
					out.close();
				}

				index++;
			}
		}

  		void ClearWorldInstances() {
   			this->_worldsInstances.clear();
  		}

		void LoadWorlds(bool forceLoad = false) {
			int index = 0;
			for(auto & world: this->_worldsFilesParsed) {
				if (!world.empty()) {
					auto & target = this->_worldsInstances[index];
					if (!target->loaded || forceLoad) {
						target->Clear();
						target->worldName = world["meta"]["name"].value_or("World");

						if (world["entities"].is_table()) {
							for (auto &entity : *world["entities"].as_table()) {
								std::string newEntity = (*entity.second.as_table())["object"].value_or("undefined");
								std::string newEntityName = (*entity.second.as_table())["name"].value_or("undefined");

								CreateCustomEntity(newEntity, target);

								auto lastEntity = target->GetLastEntityRegistred();
								lastEntity->entityName = newEntityName;
	
								// Entity serial
								if (entity.second.is_table()) {
									if (entity.second.as_table()->contains("serial")) {
										auto entityTable = entity.second.as_table();

										auto serial = (*entityTable)["serial"]["fields"];
										if (serial.is_table()) {
											auto serialTable = serial.as_table();
											for (auto & element : *serialTable) {
												if (element.second.is_array()) {
													auto array = element.second.as_array();
													SerializedField serialField;
													serialField.name = element.first;
													
													if (array->size() > 0) {
														serialField.type = array->at(0).value_or("null");
													}
													
													if (array->size() > 1) {
														serialField.value = array->at(1).value_or("null");
													}
													lastEntity->AddSerializedField(serialField);
												}
											}
										}
									}
								}

								// Components serial
								if (entity.second.is_table()) {
									auto entityTable = entity.second.as_table();
									if (entityTable->contains("components")) {
										auto serial = (*entityTable)["components"];
										if (serial.is_table()) {
											auto serialTable = serial.as_table();
											for (auto &element : *serialTable) {
												auto elementTable = element.second.as_table();
												
												auto objectName = (*elementTable)["ObjectName"].value_or("null");

												auto component = GetCustomComponent(objectName);

												std::stringstream ss;
												ss << *element.second.as_table();

												component->Deserialize(ss.str());
												component->object = component->GetObjectSerializedName();
												component->unique = component->GetObjectSerializedUnique();
												component->owner = lastEntity.get();

												component->OnUpdatePublicVariables();

												lastEntity->components.push_back(component);
											}
										}
									}
								}

								lastEntity->unique = entity.first;
								lastEntity->object = newEntity;
								lastEntity->OnUpdatePublicVariables();

								auto transform = (*entity.second.as_table())["transform"];
								if (transform.is_table() && target->GetLastEntityRegistred() != nullptr) {
									lastEntity->FromToml(transform.as_table());
								}
							}
						}
						target->loaded = true;
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

  		void Reload() {
   			this->_worldsFilesParsed.clear();
   			this->_worldsInstances.clear();

   			this->ParseWorldsFiles();
   			this->CreateWorldsInstances();
   			this->LoadWorlds();
  		}

	private:
		std::vector<std::string> _worldsFiles;

		std::vector<std::string*> _worldsFilesToLoad;
		std::vector<toml::parse_result> _worldsFilesParsed;
		std::vector<std::shared_ptr<World>> _worldsInstances;
	};
}

#endif
