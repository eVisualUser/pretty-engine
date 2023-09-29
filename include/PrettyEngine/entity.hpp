#pragma once

#include <PrettyEngine/debug.hpp>
#include <PrettyEngine/audio.hpp>
#include <PrettyEngine/render.hpp>
#include <PrettyEngine/visualObject.hpp>
#include <PrettyEngine/physics.hpp>
#include <PrettyEngine/physicsEngine.hpp>
#include <PrettyEngine/transform.hpp>
#include <PrettyEngine/tags.hpp>

#include <Guid.hpp>
#include <memory>
#include <unordered_map>

namespace PrettyEngine {
	
	#define DefaultEntityName "AnyEntity"

	class DynamicObject: public Tagged {
	public:
		/// Called before the first frame.
		virtual void OnStart() {}
		/// Called each frame.
		virtual void OnUpdate() {}
		/// Called each frame without world optimization.
		virtual void OnAlwaysUpdate() {}
		/// The same as OnUpdate but multi-threaded.
		virtual void OnMTUpdate() {}
		/// Called when the update is done to let you to sync what was done in and out the thread, or other stuff.
		virtual void OnEndUpdate() {}
		/// Called when the update is done to let you to sync what was done in and out the thread, or other stuff.
		virtual void OnAlwaysEndUpdate() {}
		/// Called when object destroyed.
		virtual void OnDestroy() {}
		/// Called just before rendering, when the UI is working.
		virtual void OnRender() {}
		/// Called before the entity is cleared.
		virtual void OnClear() {}

		/// Remove all related elements of the entity in the different parts of the engine
		void Clear() {
			for(auto & visualObject: this->visualObjects) {
				this->renderer->UnRegisterVisualObject(visualObject);
			}

			for(auto & physicalObject: this->visualObjects) {
				this->physicalEngine->UnLinkObject(physicalObject);
			}
		}
		
	public:
		std::shared_ptr<Renderer> renderer;
		std::shared_ptr<AudioEngine> audioEngine;
		std::shared_ptr<PhysicalEngine> physicalEngine;
		void* engine;

		std::vector<std::string> visualObjects;
		std::vector<std::string> physicalObjects;

		std::unordered_map<std::string, std::string> publicMap;

	public:
		std::string unique;
		std::string object;
	};

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
