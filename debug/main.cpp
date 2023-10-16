#include <PrettyEngine/PhysicalSpace.hpp>
#include <PrettyEngine/collider.hpp>
#include <PrettyEngine/debug.hpp>

#include <iostream>

int main() {

	auto physicalSpace = PrettyEngine::PhysicalSpace();

	PrettyEngine::Collider colliderA;
	colliderA.name = "ColliderA";
	colliderA.colliderModel = PrettyEngine::ColliderModel::AABB;
	colliderA.SetRigidbody(true);

	PrettyEngine::Collider colliderB;
	colliderB.name = "ColliderB";
	colliderB.colliderModel = PrettyEngine::ColliderModel::AABB;
	colliderB.SetScale(glm::vec3(0.5f, 0.5f, 0.5f));
	colliderB.SetRigidbody(true);

	PrettyEngine::Collider colliderC;
	colliderC.name = "ColliderC";
	colliderC.colliderModel = PrettyEngine::ColliderModel::AABB;

	PrettyEngine::Collider colliderD;
	colliderD.name = "ColliderD";
	colliderD.colliderModel = PrettyEngine::ColliderModel::AABB;

	physicalSpace.AddCollider("Default", &colliderA);
	physicalSpace.AddCollider("Default", &colliderB);
	physicalSpace.AddCollider("Default", &colliderC);
	physicalSpace.AddCollider("Default", &colliderD);

	auto collisions = physicalSpace.GetCollisions(&colliderA);

	DebugLog(LOG_DEBUG, "ColliderA Position: " << colliderA.position.x << ';' << colliderA.position.y << ';' << colliderA.position.z, false);
	DebugLog(LOG_DEBUG, "ColliderB Position: " << colliderB.position.x << ';' << colliderB.position.y << ';' << colliderB.position.z, false);

	for(auto & collision: collisions) {
		DebugLog(LOG_DEBUG, "Collision between: " << collision.colliderA->name << '-' << collision.colliderB->name, false);
	}

	DebugLog(LOG_DEBUG, "ColliderA Position: " << colliderA.position.x << ';' << colliderA.position.y << ';' << colliderA.position.z, false);
	DebugLog(LOG_DEBUG, "ColliderB Position: " << colliderB.position.x << ';' << colliderB.position.y << ';' << colliderB.position.z, false);

	int i[5] = {0, 0, 0, 10};

	for(auto & iter: i) {
		iter = 100;
	}

	std::cout << i[0] << std::endl; 

	return 0;
}
