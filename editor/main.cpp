#include <PrettyEngine/engine.hpp>
#include <PrettyEngine/assets/builtin.hpp>
#include <PrettyEngine/entity.hpp>
#include <PrettyEngine/world.hpp>
#include <PrettyEngine/render.hpp>

#include <PrettyEngine/debug.hpp>

#include <imgui.h>

using namespace PrettyEngine;

class Editor: public PrettyEngine::Entity {
public:
	void OnUpdate() override {
		if (this->renderer->GetKeyPress(KeyCode::LeftControl) && this->renderer->GetKeyDown(KeyCode::S)) {
			DebugLog(LOG_DEBUG, "Save to file: " << this->file, false);
		}
	}

	void OnRender() override {
		if (ImGui::Begin("Project Info")) {
			ImGui::Text("Welcome to the PrettyEditor !");
			ImGui::Separator();
			ImGui::Text("Some utility keys: ");
			ImGui::BulletText("Press F3 to show debug informations.");
			ImGui::BulletText("Press F11 to toggle fullscreen.");
		}
		ImGui::End();
	}

private:
	std::string file = "public/game.toml";
};

int main() {
	auto engine = PrettyEngine::Engine(ASSET_BUILTIN_EDITOR_CONFIG);

	auto mainWorld = PrettyEngine::World();

	auto editor = Editor();

	mainWorld.RegisterEntity(&editor);

	engine.SetCurrentWorld(&mainWorld);

	engine.Run();

	return 0;	
}
