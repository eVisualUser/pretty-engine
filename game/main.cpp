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
	
	delete engine;
}

int main() {
	auto engine = std::thread(EngineThread);

	// If you want to do something during the game execution.
	
	engine.join();

	return 0;
}
