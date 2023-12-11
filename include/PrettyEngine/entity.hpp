#ifndef H_ENTITY
#define H_ENTITY

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

		Transform* GetTransform() {
			return dynamic_cast<Transform*>(this->owner);
		}

	public:
   		/// The entity that own the component.
		DynamicObject* owner;

  		/// True if start was never called.
		bool worldFirst = true;
	};

 	/// An object that is part from a world and can contain components.
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

  		/// True if start was never called.
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
			DebugLog(LOG_ERROR, this->entityName << " Component not found: " << unique, true);
			return nullptr;
		}

		std::vector<std::shared_ptr<Component>> components;
	private:
		std::string _entityGUID = xg::newGuid();
	
	public:
		std::unordered_map<std::string, std::shared_ptr<Entity>>* entities;
	};
};

#endif