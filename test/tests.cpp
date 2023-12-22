#include <assert.h>

#include <PrettyEngine/engine.hpp>
#include <PrettyEngine/assets/builtin.hpp>
#include <PrettyEngine/KeyCode.hpp>

int main() { 

	auto engine = PrettyEngine::Engine(ASSET_BUILTIN_EDITOR_CONFIG);

	assert(engine.GetEngineContent()->audioEngine.GetAllAudioDevicesNames().size() > 0 && "No audio device found");
	assert(engine.GetEngineContent()->renderer.Valid() && "Window don't valid by default");

	return 0;
}
