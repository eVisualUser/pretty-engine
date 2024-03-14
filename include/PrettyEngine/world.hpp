#ifndef H_WORLD
#define H_WORLD

#include "PrettyEngine/debug/debug.hpp"
#include "components.hpp"
#include "custom.hpp"
#include <PrettyEngine/EngineContent.hpp>
#include <PrettyEngine/transform.hpp>
#include <PrettyEngine/collider.hpp>
#include <PrettyEngine/data.hpp>
#include <PrettyEngine/entity.hpp>

#include <glm/vec3.hpp>

#include <unordered_map>
#include <memory>
#include <future>

namespace PrettyEngine {
	enum class WorldSavingFormat {
		Toml = 0,
		Json = 1,
		Yaml = 2,
	};

	class World;

 	/// Contain entities and manage their access.
	class World {
	public:
		World(std::string path) {
			this->worldAsset = Asset(path);

			this->simulationCollider.SetScale(glm::vec3(100, 100, 100));
		}
		
		~World() {
			this->Clear();
		}

		/// Save the world to the Asset specified.
		void Save(WorldSavingFormat worldSavingFormat = WorldSavingFormat::Toml, const std::string& customOutputFile = "") {
			auto filePath = this->worldAsset.GetFilePath();

			DebugLog(LOG_DEBUG, "Save world: " << filePath, false);

			std::ofstream out;
			if (customOutputFile.empty()) {
				out.open(filePath);
			} else {
				out.open(customOutputFile);
			}

			if (out.is_open()) {
				auto base = toml::parse("");
				base.insert_or_assign("meta", toml::table{});
				base["meta"].as_table()->insert_or_assign("name", this->worldName);

				base.insert_or_assign("entities", toml::table{});
				for(auto & entity: this->entities) {
					auto entitiesTable = base["entities"].as_table();
					entitiesTable->insert_or_assign(entity.second->serialObjectUnique, toml::table{});
					auto entityTable = (*entitiesTable)[entity.second->serialObjectUnique].as_table();
					entityTable->insert_or_assign("name", entity.second->entityName);
					entityTable->insert_or_assign("object", entity.second->serialObjectName);

					entityTable->insert_or_assign("transform", toml::table{});
					auto transformTable = (*entityTable)["transform"].as_table();
					entity.second->GetTransform()->AddToToml(transformTable);

					entityTable->insert_or_assign("serial", toml::table(toml::parse(entity.second->Serialize(SerializationFormat::Toml))));

					auto componentTable = toml::table();
					for(auto & component: entity.second->components) {
						componentTable.insert_or_assign(component->serialObjectUnique, toml::table(toml::parse(component->Serialize(SerializationFormat::Toml))));
					}
					entityTable->insert_or_assign("components", componentTable);
				}

				// Allow saving to another file format to make it easier to created tools working with world files.
				switch(worldSavingFormat) {
				case WorldSavingFormat::Json:
					out << toml::json_formatter(base);
					break;
				case WorldSavingFormat::Yaml:
					out << toml::yaml_formatter(base);
					break;
				default: case WorldSavingFormat::Toml:
					out << base;
					break;
				}

				out.flush();
				out.close();
			}
		}

		/// Load the World from the Asset.
		void Load() {
			this->Clear();
			if (this->worldAsset.Exist()) {
				toml::parse_result parsedResult;
				try {
					const auto fileContent = worldAsset.ReadToString();
					const auto filePath = worldAsset.GetFilePath();

					parsedResult = toml::parse(fileContent, filePath);
				} catch (const std::bad_alloc& err) {
					DebugLog(LOG_ERROR, err.what(), true);
				}

				if (parsedResult.empty()) {
					DebugLog(LOG_WARNING, "Aborted world loading, because of empty parsed data, from file: " << this->worldAsset.GetFilePath(), false);
					return;
				}
			
				this->worldName = parsedResult["meta"]["name"].value_or("World");

				if (parsedResult["entities"].is_table()) {
					for (auto &entity : *parsedResult["entities"].as_table()) {
						std::string newEntityObject = (*entity.second.as_table())["object"].value_or("undefined");
						std::string newEntityName = (*entity.second.as_table())["name"].value_or("undefined");

						if (auto newEntity = CreateCustomEntity(newEntityObject)) {
							this->RegisterEntity(newEntity);

							if (this->lastEntityRegistred.empty()) {
								DebugLog(LOG_DEBUG, "Failed to load entity: " << newEntity, true);
								continue;
							}

							newEntity->entityName = newEntityName;

							auto entitySerial = std::async([this, entity, newEntity]{
								// Entity serial
								if (entity.second.is_table()) {
									if (entity.second.as_table()->contains("serial")) {
										auto entityTable = entity.second.as_table();

										auto serial = (*entityTable)["serial"]["fields"];
										if (serial.is_table()) {
											auto serialTable = serial.as_table();
											for (auto &element : *serialTable) {
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
													newEntity->AddSerializedField(serialField);
												}
											}
										}
									}
								}
							});

							// Components serial
							auto componentSerial = std::async([this, entity, newEntity] {
								if (entity.second.is_table()) {
									auto entityTable = entity.second.as_table();
									if (entityTable->contains("components")) {
										auto serial = (*entityTable)["components"];
										if (serial.is_table()) {
											auto serialTable = serial.as_table();
											newEntity->components.reserve(serialTable->size());
											for (auto &element : *serialTable) {
												auto elementTable = element.second.as_table();

												auto objectName = (*elementTable)["ObjectName"].value_or("null");

												auto component = GetCustomComponent(objectName);

												std::stringstream ss;
												ss << *element.second.as_table();

												component->Deserialize(ss.str());
												component->serialObjectName = component->GetObjectSerializedName();
												component->serialObjectUnique = component->GetObjectSerializedUnique();
												component->owner = newEntity.get();

												component->OnSetup();

												newEntity->components.push_back(component);
											}
											newEntity->components.shrink_to_fit();
										}
									}
								}
							});

							newEntity->serialObjectName = newEntityObject;
							newEntity->serialObjectUnique = entity.first;
							newEntity->OnSetup();

							auto transform = (*entity.second.as_table())["transform"];
							if (transform.is_table() && this->GetLastEntityRegistred() != nullptr) {
								newEntity->FromToml(transform.as_table());
							}

							componentSerial.get();
							entitySerial.get();

							newEntity->components.shrink_to_fit();
						} else {
							DebugLog(LOG_ERROR, "Failed to load entity: " << newEntityObject << " " << newEntityName, true);
						}
					}
				}
			} else {
				DebugLog(LOG_ERROR, "Failed to load world: " << this->worldAsset.GetFilePath(), true);
				if (Retry("Try to reopen world file ?")) {
					return this->Load();
				}
			}
		}
		
		void Start() {
			for (auto & entity: this->entities) {
				if (entity.second != nullptr) {
					if (entity.second->worldFirst) {
						entity.second->OnStart(); 
						entity.second->worldFirst = false;
					}
					for (auto & component: entity.second->components) {
						if (component->worldFirst) {
							component->owner = entity.second.get();
							component->OnStart();
							component->worldFirst = false;
						}
					}
				}
			}
		}

		void EditorStart() {
#if ENGINE_EDITOR
			for (auto & entity: this->entities) {
				if (entity.second != nullptr) {
					if (entity.second->worldFirst) {
						entity.second->OnEditorStart(); 
						entity.second->worldFirst = false;
					}
					for (auto & component: entity.second->components) {
						if (component->worldFirst) {
							component->OnEditorStart();
							component->worldFirst = false;
						}
					}
				}
			}
#endif
		}

		void Update() {
			this->Start();
			for (auto & entity: this->entities) {
				if (entity.second.get() != nullptr) {
					entity.second->OnUpdate();
					for (auto & component: entity.second->components) {
						component->OnUpdate();
					}
				}
			}
		}

		void EditorUpdate() {
			#if ENGINE_EDITOR
				this->EditorStart();
				for (auto & entity: this->entities) {
					if (entity.second.get() != nullptr) {
						entity.second->OnEditorUpdate();
						for (auto & component: entity.second->components) {
							component->OnEditorUpdate();
						}
					}
				}
			#endif
		}

		void EndUpdate() {
			for (auto & entity: this->entities) {
				if (entity.second != nullptr) {
					entity.second->OnEndUpdate(); 
					for (auto & component: entity.second->components) {
						component->OnEndUpdate();
					}
				}
			}
		}

		void PrePhysics() {
			for (auto & entity: this->entities) {
				if (entity.second != nullptr) {
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

		void RegisterEntity(const std::shared_ptr<Entity>& entity) {
			this->UpdateLinks();
			this->entities.insert(std::make_pair(entity->GetGUID(), entity));
			this->lastEntityRegistred = entity->GetGUID();
		}
		
		std::shared_ptr<Entity> GetLastEntityRegistred() {
			return this->entities[this->lastEntityRegistred];
		}
		
		void UnRegisterEntity(std::shared_ptr<Entity> entity) {
			entity->OnDestroy();
			for (const auto & component: entity->components)
				component->OnDestroy();
			entity.reset();
			this->entities.erase(entity->GetGUID());
		}

		void SetSimulationDistance(float distance) {
			this->simulationCollider.scale = glm::vec3(distance, distance, distance);
		}

		void UpdateLinks() {
			for (const auto & entity: this->entities) {
				entity.second->engineContent = this->engineContent;
				entity.second->world = this;

				for(const auto & component: entity.second->components) {
					component->owner = entity.second.get();
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
		
		void Clear() {
			for(auto & entity: this->entities) {
				entity.second->OnDestroy();
				for (const auto & component: entity.second->components)
					component->OnDestroy();
			}
			this->entities.clear();
		}

		std::unordered_map<std::string, std::shared_ptr<Entity>>* GetEntities() {
			return &this->entities;
		}

		std::unordered_map<std::string, std::shared_ptr<Entity>> entities;

		std::string lastEntityRegistred;

		EngineContent* engineContent;

		void* engine = nullptr;

		std::string worldName = "DefaultWorldName";

		bool loaded = false;

		Asset worldAsset;

	public:
		Collider simulationCollider = Collider();

	private:
		DataBase* data = nullptr;
	};
}

#endif