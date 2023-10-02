#pragma once

#include <PrettyEngine/debug.hpp>
#include <PrettyEngine/audio.hpp>
#include <PrettyEngine/render.hpp>
#include <PrettyEngine/visualObject.hpp>
#include <PrettyEngine/physics.hpp>
#include <PrettyEngine/physicsEngine.hpp>
#include <PrettyEngine/transform.hpp>
#include <PrettyEngine/tags.hpp>
#include <PrettyEngine/dynamicObject.hpp>

#include <Guid.hpp>
#include <toml++/toml.h>

#include <memory>
#include <unordered_map>

namespace PrettyEngine {
	#define DefaultEntityName "AnyEntity"

	class Component: public DynamicObject {
	public:
		virtual void OnStart() {}
		virtual void OnUpdate() {}
		virtual void OnDestroy() {}

		std::string owner;

		bool worldFirst = true;
	};

	class Entity: public virtual DynamicObject, public virtual Transform {
	public:
		~Entity() {
			this->publicMap.clear();
			this->components.clear();
		}

		std::string GetGUID() {
			return this->_entityGUID;
		}
	public:
		std::shared_ptr<Renderer> renderer;
		std::shared_ptr<AudioEngine> audioEngine;
		std::shared_ptr<PhysicalEngine> physicalEngine;

		void* engine;

		bool worldFirst = true;

		std::string entityName = DefaultEntityName;

	public:
		template<typename T>
		Component* AddComponent() {
			T component;
			this->components.push_back(component);
		}

		void RemoveComponent(Component* component) {
			for(int i = 0; i < this->components.size(); i++) {
				if (this->components[i]->unique == component->unique) {
					this->components.erase(this->components.begin() + i);
					break;
				}
			}
		}

		template<typename T>
		T* GetComponentAs(std::string unique) {
			for(auto & component: this->components) {
				if (component->unique == unique) {
					return dynamic_cast<T*>(component.get());
				}
			}
			return nullptr;
		}

		std::vector<std::shared_ptr<Component>> components;
	private:
		std::string _entityGUID = xg::newGuid();
	};
};
