#pragma once

#include "PrettyEngine/debug.hpp"
#include <PrettyEngine/audio.hpp>
#include <PrettyEngine/render.hpp>
#include <PrettyEngine/visualObject.hpp>
#include <PrettyEngine/physics.hpp>
#include <PrettyEngine/transform.hpp>
#include <PrettyEngine/tags.hpp>

#include <Guid.hpp>

namespace PrettyEngine {

	#define DefaultEntityName "AnyEntity"

	class Entity: virtual public Tagged, virtual public Transform {
	public:
		Entity() {
			this->OnCreate();
		}

		~Entity() {
			this->OnDestroy();
		}

		std::string GetGUID() {
			return this->_entityGUID;
		}
		
	public:
		/// Called in the constructor to avoid custructor override errors
		virtual void OnCreate() {}
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
		
	public:
		Renderer* renderer;
		AudioEngine* audioEngine;
		PhysicalEngine* physicalEngine;

		void* engine;

		bool worldFirst = true;

		std::string entityName = DefaultEntityName;
		
	private:
		std::string _entityGUID = xg::newGuid();
	};
}
