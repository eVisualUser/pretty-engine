/*
 * The tests use manual multithreading to avoid running multiple program.
*/

#include <assert.h>

#include <PrettyEngine/engine.hpp>
#include <PrettyEngine/assets/builtin.hpp>

#include <thread>

int main() { 
	auto engineContent = std::thread ([]{
		auto engine = PrettyEngine::Engine(ASSET_BUILTIN_EDITOR_CONFIG);

		assert(engine.GetEngineContent()->audioEngine.GetAllAudioDevicesNames().size() > 0 && "No audio device found");
		assert(engine.GetEngineContent()->renderer.Valid() && "Window don't valid by default");
	});

	engineContent.join();

	return 0;
}
