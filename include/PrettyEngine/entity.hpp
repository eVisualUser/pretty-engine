#pragma once

#include <PrettyEngine/dynamicObject.hpp>
#include <PrettyEngine/debug.hpp>
#include <PrettyEngine/audio.hpp>
#include <PrettyEngine/render.hpp>
#include <PrettyEngine/visualObject.hpp>
#include <PrettyEngine/collider.hpp>
#include <PrettyEngine/transform.hpp>
#include <PrettyEngine/tags.hpp>

#include <Guid.hpp>
#include <toml++/toml.h>

#include <memory>

namespace PrettyEngine {
	#define DefaultEntityName "AnyEntity"

	class Component: public DynamicObject {
	public:
		virtual void OnStart() {}
		virtual void OnUpdate() {}
		virtual void OnDestroy() {}

		DynamicObject* owner;

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
