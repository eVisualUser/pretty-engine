#pragma once

#include "PrettyEngine/utils.hpp"
#include <PrettyEngine/world.hpp>
#include <PrettyEngine/debug.hpp>
#include <PrettyEngine/transform.hpp>

#include <custom.hpp>

#include <toml++/toml.h>

namespace PrettyEngine {
	static void ApplyLoadedTransform(toml::table* transformTable, std::shared_ptr<Transform> transform) {
		bool isArray = (*transformTable)["position"].is_array();
		if (isArray) {
			if ((*transformTable)["position"].is_array()) {
				auto position = (*transformTable)["position"].as_array();
				auto arraySize = position->size();
				/*
				if (arraySize >= 1)
					transform->position.x = position[0].value_or(0.0);
				if (arraySize >= 2)
					transform->position.y = position[1].value_or(0.0);
				if (arraySize >= 3)
					transform->position.z = position[2].value_or(0.0);
				*/
			}
		}
	}

	static bool LoadWorld(std::string fileName, std::shared_ptr<World> target) {
		auto world = toml::parse_file(fileName);
		
		if (!world.empty()) {
			target->worldName = world["meta"]["name"].value_or("World");

			if (world["entities"].is_table()) {
				for(auto & entity: *world["entities"].as_table()) {
					std::string type = (*entity.second.as_table())["type"].value_or("undefined");
					std::string newEntity = (*entity.second.as_table())["object"].value_or("undefined");

					if (type == "Entity") {
						AddCustomEntity(newEntity, target);
						auto lastEntity = target->GetLastEntityRegistred();

						lastEntity->unique = entity.first;
						lastEntity->object = type;
						
						auto transform = (*entity.second.as_table())["transform"];
						if (transform.is_table() && target->GetLastEntityRegistred() != nullptr) {

							ApplyLoadedTransform(transform.as_table(), lastEntity);
						}
					}
				}
			}
			
			return true;
		}
		return false;
	}
}
