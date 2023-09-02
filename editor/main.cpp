#include <PrettyEngine/engine.hpp>
#include <PrettyEngine/assets/builtin.hpp>
#include <PrettyEngine/entity.hpp>
#include <PrettyEngine/world.hpp>
#include <PrettyEngine/render.hpp>
#include <PrettyEngine/debug.hpp>

#include <custom.hpp>
#include <components.hpp>

using namespace PrettyEngine;

int main() {
	auto engine = PrettyEngine::Engine(ASSET_BUILTIN_EDITOR_CONFIG);

	auto mainWorld = PrettyEngine::World();

	AddCustomEntity("Editor", &mainWorld);
	auto component = GetCustomComponent("MyComponent", &mainWorld);
	component->OnStart();

	engine.SetCurrentWorld(&mainWorld);

	engine.Run();

	return 0;	
}
