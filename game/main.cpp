#include <PrettyEngine/assets/builtin.hpp>
#include <PrettyEngine/debug/debug.hpp>
#include <PrettyEngine/engine.hpp>
#include <PrettyEngine/worldLoad.hpp>

using namespace PrettyEngine;

bool worldFilter(std::string name) {
	if (name.starts_with("editor") || name.starts_with("Editor") || name.starts_with("EDITOR")) {
		return false;
	}

	return true;
}

int main() {
	auto engine = PrettyEngine::Engine(ASSET_BUILTIN_EDITOR_CONFIG);
	engine.GetWorldManager()->AddWorldFile(GetEnginePublicPath("worlds/game.toml", true));
	engine.GetWorldManager()->FilterWorldList(worldFilter);
	engine.GetWorldManager()->ParseWorldsFiles();
	engine.GetWorldManager()->CreateWorldsInstances();

	for (auto &error : engine.GetWorldManager()->FindErrors()) {
		DebugLog(LOG_ERROR, error, true);
	}

	engine.GetWorldManager()->LoadWorlds();

	engine.SetupWorlds();

	engine.Run();

	return 0;
}
