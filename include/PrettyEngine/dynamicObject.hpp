#pragma once

#include <PrettyEngine/audio.hpp>
#include <PrettyEngine/render.hpp>
#include <PrettyEngine/physicsEngine.hpp>
#include <PrettyEngine/serial.hpp>
#include <PrettyEngine/tags.hpp>

#include <Guid.hpp>

#include <memory>

namespace PrettyEngine {
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
}