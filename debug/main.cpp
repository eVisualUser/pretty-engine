#include <BulletCollision/CollisionShapes/btBoxShape.h>
#include <BulletDynamics/ConstraintSolver/btGeneric6DofConstraint.h>
#include <LinearMath/btVector3.h>
#include <PrettyEngine/entity.hpp>
#include <BulletCollision/CollisionShapes/btSphereShape.h>
#include <BulletCollision/CollisionShapes/btStaticPlaneShape.h>
#include <PrettyEngine/camera.hpp>
#include <glm/trigonometric.hpp>
#include <PrettyEngine/visualObject.hpp>
#include <PrettyEngine/gl.hpp>
#include <PrettyEngine/texture.hpp>
#include <PrettyEngine/mesh.hpp>
#include <PrettyEngine/render.hpp>
#include <PrettyEngine/shaders.hpp>
#include <PrettyEngine/utils.hpp>
#include <PrettyEngine/debug.hpp>
#include <PrettyEngine/text.hpp>
#include <PrettyEngine/physics.hpp>
#include <PrettyEngine/light.hpp>
#include <PrettyEngine/audio.hpp>
#include <PrettyEngine/assets/builtin.hpp>
#include <PrettyEngine/world.hpp>
#include <PrettyEngine/localization.hpp>
#include <PrettyEngine/data.hpp>
#include <PrettyEngine/reflect.hpp>
#include <PrettyEngine/engine.hpp>

#include <imgui.h>

#include <Guid.hpp>

#include <glm/glm.hpp>

#include <iostream>
#include <sstream>
#include <thread>
#include <string>

class MyVisualObject: public PrettyEngine::VisualObject, public PrettyEngine::PhysicalObject, public PrettyEngine::InputHandler, virtual public PrettyEngine::Entity {
public:
	void OnStart() override {
		// Initialize audio
		this->audioSource.LoadWAVAudio("public/testcase.wav");
		this->audioSource.SetLooping(true);
		this->audioSource.SetGain(1.0f);
		this->audioSource.SetPitch(1.0f);
		this->audioSource.audioState = &this->audioState;
		this->audioSource.UpdateAudioSource();

		if (this->audioEngine == nullptr) {
			DebugLog(LOG_ERROR, "Audio Engine Pointer broken", true);
		} else {
			this->audioEngine->LinkAudioSource(&this->audioSource);
		}

		if (this->physicalEngine == nullptr) {
			DebugLog(LOG_ERROR, "Physical Engine Pointer broken", true);
		}

		if (this->renderer == nullptr) {
			DebugLog(LOG_ERROR, "Renderer Pointer broken", true);
		}

		if (this->engine == nullptr) {
			DebugLog(LOG_ERROR, "Engine Pointer broken", true);
		}
	}

	void OnDestroy() override {
		this->audioEngine->UnLinkAudioSource(&this->audioSource);
	}

	void OnPhysicPostUpdate(void* physicEngine) override {
	 	if (this->currentCamera != nullptr) {
	 		this->currentCamera->position = -this->position;
	 		this->currentCamera->position.z = -10.0f;
	 	}
	}

	void OnWindowPolls(void *r) override {
		auto renderer = GET_RENDERER(r);

		this->currentCamera = renderer->GetCurrentCamera();
	 	this->ResetLinearVelocity();

	 	double deltaTime = renderer->GetDeltaTime();
	 	
	 	glm::vec3 linearVelocity = glm::vec3(0.0f, 0.0f, 0.0f);

		if (renderer->GetKeyPress(PrettyEngine::KeyCode::Left)) {
	 		linearVelocity.x -= this->speed * deltaTime;
	 	} else if (renderer->GetKeyPress(PrettyEngine::KeyCode::Right)) {
	 		linearVelocity.x += this->speed * deltaTime;
	 	}

	 	if (renderer->GetKeyPress(PrettyEngine::KeyCode::Up)) {
	 		linearVelocity.y += this->speed * deltaTime;
	 	} else if (renderer->GetKeyPress(PrettyEngine::KeyCode::Down)) {
	 		linearVelocity.y -= this->speed * deltaTime;
	 	}

	 	this->SetLinearVelocity(linearVelocity);
	 	
	 	if (renderer->GetKeyPress(PrettyEngine::KeyCode::Escape)) {
	 		((PrettyEngine::Engine*)this->engine)->Exit();
	 	}
	}
	
	void OnDraw(void* r) override {
		auto renderer = GET_RENDERER(r);
		if (!renderer->GetKeyUp(PrettyEngine::KeyCode::Tab)) {
	        ImGui::Begin("Box", nullptr, NULL);

	        if (ImGui::Button("Play/Pause Audio")) {
	        	if (this->audioState == PrettyEngine::AudioState::Play) {
	        		this->audioState = PrettyEngine::AudioState::Pause;
	        	} else {
	        		this->audioState = PrettyEngine::AudioState::Play;
	        	}
	        	
	        	this->audioSource.UpdateAudioSource();
	        }

	        ImGui::End();
    	}

    	auto listener = this->audioEngine->GetListener();

    	listener->position = this->position;

        listener->UpdateAudioListener();
	}

private:
	PrettyEngine::Camera* currentCamera;
	PrettyEngine::AudioSource audioSource;
	PrettyEngine::AudioState audioState = PrettyEngine::AudioState::Stop;

	float speed = 1000.0f;
};

class MyGroundObject: public PrettyEngine::VisualObject, public PrettyEngine::PhysicalObject {};

void Main() {
	PrettyEngine::Engine* engine = new PrettyEngine::Engine(ASSET_BUILTIN_CONFIG);

	auto world = std::make_shared<PrettyEngine::World>();

	engine->SetCurrentWorld(world);

	auto defaultVertexShader = xg::newGuid();
	auto defaultFragmentShader = xg::newGuid();

	// Load and Complile shaders
	engine->GetRenderer()->AddShader(defaultVertexShader, PrettyEngine::ShaderType::Vertex, SHADER_VERTEX);
	engine->GetRenderer()->AddShader(defaultFragmentShader, PrettyEngine::ShaderType::Fragment, SHADER_FRAGMENT);

	auto rect = engine->GetRenderer()->AddMesh(xg::newGuid(), PrettyEngine::CreateRectMesh());
	
	auto teaPot = engine->GetRenderer()->LoadModel("public/demo_object.fbx", xg::newGuid());

	auto other = engine->GetRenderer()->AddMesh(xg::newGuid(), teaPot.second[0]);
	
	// Create a shader program based on two main shaders
	auto defaultShaderProgram = engine->GetRenderer()->AddShaderProgram("Default", defaultVertexShader, defaultFragmentShader);
	
	PrettyEngine::RenderModel modelA;

	modelA.SetShaderProgram(defaultShaderProgram);
	modelA.SetMesh(rect);
	modelA.useTexture = true;

	PrettyEngine::RenderModel modelB;

	modelB.SetShaderProgram(defaultShaderProgram);
	modelB.SetMesh(other);

	PrettyEngine::RenderModel modelC;

	modelC.SetShaderProgram(defaultShaderProgram);
	modelC.SetMesh(rect);
	
	auto myObject = std::make_shared<MyVisualObject>();
	myObject->AddRenderModel(&modelA);
	myObject->renderLayer = 1;
	myObject->useLight = true;

	myObject->AddTexture(engine->GetTexture("SampleA"));

	world->RegisterEntity(myObject);

	auto objectShape = new btBoxShape(btVector3(0.5f, 0.5f, 0.1f));
	myObject->CreateRigidbody(objectShape);

	std::string myObjectGUID = xg::newGuid().str();
	std::string groundObjectGUID = xg::newGuid().str();

	auto groundObject = std::make_shared<MyGroundObject>();
	groundObject->AddRenderModel(&modelB);

	auto groundShape = new btBoxShape(btVector3(0.5f, 0.5f, 0.5f));
	groundObject->CreateCollider(groundShape, true);
	groundObject->position.y -= 2;
	groundObject->position.x += 6;
	groundObject->baseColor = glm::vec3(1.0f, 1.0f, 0.5f);
	groundObject->Toggle3D();

	engine->GetPhysicalEngine()->LinkObject(myObjectGUID, myObject);
	engine->GetPhysicalEngine()->LinkObject(groundObjectGUID, groundObject);
	
	engine->GetPhysicalEngine()->SetGlobalGravity(glm::vec3(0.0f, 0.0f, 0.0f));

	engine->GetRenderer()->RegisterVisualObject(myObjectGUID, myObject);
	engine->GetRenderer()->RegisterVisualObject(groundObjectGUID, groundObject);

	engine->GetRenderer()->RegisterInputHandler(myObjectGUID, myObject);

	PrettyEngine::Light light;
	light.radius = 100.0f;

	engine->GetRenderer()->RegisterLight(xg::newGuid(), &light);

	engine->Run();

	delete engine;
	delete groundShape;
	delete objectShape;
}

int main() {
	std::thread mainThread(Main);
	
	mainThread.join();
	
	return 0;
}
