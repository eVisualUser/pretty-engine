#pragma once

#include "PrettyEngine/debug.hpp"
#include <PrettyEngine/audio.hpp>
#include <PrettyEngine/render.hpp>
#include <PrettyEngine/visualObject.hpp>
#include <PrettyEngine/physics.hpp>
#include <PrettyEngine/transform.hpp>
#include <PrettyEngine/tags.hpp>

#include <Guid.hpp>
#include <memory>

namespace PrettyEngine {

	#define DefaultEntityName "AnyEntity"

	class DynamicObject: virtual public Tagged {
	public:
		/// Called before the first frame
		virtual void OnStart() {}
		/// Called each frame
		virtual void OnUpdate() {}
		/// Called each frame without world optimization
		virtual void OnAlwaysUpdate() {}
		/// The same as OnUpdate but multi-threaded
		virtual void OnMTUpdate() {}
		/// Called when the update is done to let you to sync what was done in and out the thread, or other stuff
		virtual void OnEndUpdate() {}
		/// Called when the update is done to let you to sync what was done in and out the thread, or other stuff
		virtual void OnAlwaysEndUpdate() {}
		/// Called when object destroyed
		virtual void OnDestroy() {}
		/// Called just before rendering, when the UI is working.
		virtual void OnRender() {}

	public:
		std::shared_ptr<Renderer> renderer;
		std::shared_ptr<AudioEngine> audioEngine;
		std::shared_ptr<PhysicalEngine> physicalEngine;
		void* engine;

	public:
		std::string uniqueGUID;
		std::string objectGUID;
	};

	class Component: virtual public DynamicObject {
	public:
		virtual void OnStart();
		virtual void OnUpdate();
		virtual void OnDestroy();
	};

	class Entity: public virtual DynamicObject, public virtual Transform {
	public:
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
				if (this->components[i].uniqueGUID == component->uniqueGUID) {
					this->components.erase(this->components.begin() + i);
					break;
				}
			}
		}

		std::vector<Component> components;
		
	private:
		std::string _entityGUID = xg::newGuid();
	};
};
