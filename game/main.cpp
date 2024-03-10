#include <PrettyEngine/assets/builtin.hpp>
#include <PrettyEngine/debug/debug.hpp>
#include <PrettyEngine/engine.hpp>

int main() {
	auto engine = PrettyEngine::Engine(ASSET_BUILTIN_EDITOR_CONFIG);
	if (auto worldManager = engine.GetWorldManager()) {
		worldManager->AddWorld(PrettyEngine::GetEnginePublicPath("worlds/game.toml", true));
		worldManager->LoadWorlds();
		engine.SetupWorlds();
	} else {
		DebugLog(LOG_ERROR, "Failed to get WorldManager from Engine", true);
	}

	engine.Run();

	return 0;
}
