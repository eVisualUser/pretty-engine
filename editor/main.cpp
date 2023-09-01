#include <PrettyEngine/engine.hpp>
#include <PrettyEngine/assets/builtin.hpp>
#include <PrettyEngine/entity.hpp>
#include <PrettyEngine/world.hpp>
#include <PrettyEngine/render.hpp>
#include <PrettyEngine/debug.hpp>

#include <imgui.h>

#include <memory>

using namespace PrettyEngine;

class Editor: public virtual PrettyEngine::Entity {
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
			ImGui::BulletText("Press Ctr + S to save.");
		}
		ImGui::End();
	}

	void OnDestroy() override {
		
	}

private:
	std::string file = "public/game.toml";
};

void CreateEntity(PrettyEngine::World* world) {
	world->RegisterEntity(std::make_shared<Editor>());
}

int main() {
	auto engine = PrettyEngine::Engine(ASSET_BUILTIN_EDITOR_CONFIG);

	auto mainWorld = PrettyEngine::World();

	std::shared_ptr<Editor> editor = std::make_shared<Editor>();

	CreateEntity(&mainWorld);

	engine.SetCurrentWorld(&mainWorld);

	engine.Run();

	return 0;	
}
