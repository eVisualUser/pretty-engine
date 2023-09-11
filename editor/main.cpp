#include <PrettyEngine/worldLoad.hpp>
#include <PrettyEngine/engine.hpp>
#include <PrettyEngine/assets/builtin.hpp>
#include <PrettyEngine/entity.hpp>
#include <PrettyEngine/world.hpp>
#include <PrettyEngine/worldLoad.hpp>
#include <PrettyEngine/render.hpp>
#include <PrettyEngine/debug.hpp>

using namespace PrettyEngine;

int main() {
	auto engine = PrettyEngine::Engine(ASSET_BUILTIN_EDITOR_CONFIG);

	auto mainWorld = std::make_shared<PrettyEngine::World>();

	LoadWorld("public/worlds/editor.toml", mainWorld);

	engine.SetCurrentWorld(mainWorld);

	engine.Run();

	return 0;	
}
