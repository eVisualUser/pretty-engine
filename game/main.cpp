#include <PrettyEngine/engine.hpp>
#include <PrettyEngine/assets/builtin.hpp>

#include <thread>

static void EngineThread() {
	PrettyEngine::Engine* engine = new PrettyEngine::Engine(ASSET_BUILTIN_CONFIG);
	
	delete engine;
}

int main() {
	auto engine = std::thread(EngineThread);

	// If you want do something during the game execution.
	
	engine.join();

	return 0;
}
