#ifndef H_ENGINE_CONTENT
#define H_ENGINE_CONTENT

#include <PrettyEngine/audio.hpp>
#include <PrettyEngine/render.hpp>
#include <PrettyEngine/PhysicalSpace.hpp>
#include <PrettyEngine/Input.hpp>
#include <PrettyEngine/event.hpp>

namespace PrettyEngine {
	/// Contain all the sub-engines and systems shared by the Engine.
	class EngineContent {
	public:
		Renderer renderer = Renderer();
		AudioEngine audioEngine = AudioEngine();
		Input input = Input();
		PhysicalSpace physicalSpace = PhysicalSpace();
		EventManager eventManager = EventManager();
	};
}

#endif