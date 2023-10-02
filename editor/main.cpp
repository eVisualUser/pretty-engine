#include <PrettyEngine/utils.hpp>
#include <PrettyEngine/world.hpp>
#include <PrettyEngine/worldLoad.hpp>
#include <PrettyEngine/engine.hpp>
#include <PrettyEngine/assets/builtin.hpp>
#include <PrettyEngine/entity.hpp>
#include <PrettyEngine/render.hpp>
#include <PrettyEngine/debug.hpp>

using namespace PrettyEngine;

bool worldFilter(std::string name) {
	DebugLog(LOG_DEBUG, "World loaded: " << name, false);
	return true;
}

int main() {
	auto engine = PrettyEngine::Engine(ASSET_BUILTIN_EDITOR_CONFIG);
	
	engine.GetWorldManager()
		->AddWorldFile(GetEnginePublicPath("worlds/editor.toml", true));
	
	engine.GetWorldManager()
		->FilterWorldList(worldFilter)
		->ParseWorldsFiles()
		->CreateWorldsInstances()
		->LoadWorlds();

	engine.SetupWorlds();

	engine.Run();

	engine.GetWorldManager()->SaveWorlds();

	return 0;	
}
