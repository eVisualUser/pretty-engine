#pragma once

#include <PrettyEngine/utils.hpp>
#include <PrettyEngine/world.hpp>
#include <PrettyEngine/debug.hpp>
#include <PrettyEngine/transform.hpp>

#include <custom.hpp>
#include <components.hpp>

#include <toml++/toml.h>
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

	static bool LoadWorld(std::string fileName, std::shared_ptr<World> target) {
		auto world = toml::parse_file(fileName);
		
		if (!world.empty()) {
			target->worldName = world["meta"]["name"].value_or("World");

			if (world["entities"].is_table()) {
				for(auto & entity: *world["entities"].as_table()) {
					std::string newEntity = (*entity.second.as_table())["object"].value_or("undefined");
					std::string newEntityName = (*entity.second.as_table())["name"].value_or("undefined");

					AddCustomEntity(newEntity, target);
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
			
			return true;
		}
		return false;
	}
}
