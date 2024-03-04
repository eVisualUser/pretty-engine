#include <PrettyEngine/worldLoad.hpp>
#include <PrettyEngine/engine.hpp>
#include <PrettyEngine/assets/builtin.hpp>
#include <PrettyEngine/debug/debug.hpp>

using namespace PrettyEngine;

bool worldFilter(std::string name) {

	// Add your filter logic here

	return true;
}

int main() {
	auto engine = PrettyEngine::Engine(ASSET_BUILTIN_EDITOR_CONFIG);
	engine.GetWorldManager()->AddWorldFile(GetEnginePublicPath("worlds/editor.toml", true));
	engine.GetWorldManager()->FilterWorldList(worldFilter);
	engine.GetWorldManager()->ParseWorldsFiles();
	engine.GetWorldManager()->CreateWorldsInstances();

	for(auto & error: engine.GetWorldManager()->FindErrors()) {
		DebugLog(LOG_ERROR, error, true);
	}

	engine.GetWorldManager()->LoadWorlds();

	engine.SetupWorlds();
	
	engine.Run();

	return 0;	
}
