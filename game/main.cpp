#include <PrettyEngine/engine.hpp>
#include <PrettyEngine/assets/builtin.hpp>

#include <thread>

bool worldFilter(std::string name) {
	if (name.starts_with("editor") || name.starts_with("Editor") || name.starts_with("EDITOR")) {
		return false;
	}

	return true;
}

static void EngineThread() {
	PrettyEngine::Engine* engine = new PrettyEngine::Engine(ASSET_BUILTIN_CONFIG);

	engine->GetWorldManager()
		->AddWorldFile(GetEnginePublicPath("worlds/game.toml", true));
	
	engine->GetWorldManager()
		->FilterWorldList(worldFilter)
		->ParseWorldsFiles()
		->CreateWorldsInstances()
		->LoadWorlds();

	engine->SetupWorlds();

	engine->Run();
	
	delete engine;
}

int main() {
	auto engine = std::thread(EngineThread);

	// If you want to add something during the game execution.
	
	engine.join();

	return 0;
}
