#include <PrettyEngine/engine.hpp>
#include <PrettyEngine/assets/builtin.hpp>

#include <thread>

static void Engine() {
	PrettyEngine::Engine* engine = new PrettyEngine::Engine(ASSET_BUILTIN_CONFIG);
	
	delete engine;
}

int main() {

	auto engine = std::thread(Engine);

	// If you want do something during the game execution.
	
	engine.join();

	return 0;
}
