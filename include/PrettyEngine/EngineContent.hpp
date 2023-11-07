#pragma once

#include <PrettyEngine/audio.hpp>
#include <PrettyEngine/render.hpp>
#include <PrettyEngine/PhysicalSpace.hpp>
#include <PrettyEngine/Input.hpp>
#include <PrettyEngine/event.hpp>

namespace PrettyEngine {
	class EngineContent {
	public:
		Renderer renderer = Renderer();
		AudioEngine audioEngine = AudioEngine();
		Input input = Input();
		PhysicalSpace physicalSpace = PhysicalSpace();
		EventManager eventManager = EventManager();
	};
}
