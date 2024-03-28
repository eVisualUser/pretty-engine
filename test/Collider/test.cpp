#include "PrettyEngine/render/mesh.hpp"
#include <PrettyEngine/collider.hpp>
#include <PrettyEngine/render/mesh.hpp>

using namespace PrettyEngine;

int main() {

	auto mesh = CreatePhysicsRectMesh();

	Collider collider;
	collider.SetMesh(&mesh);

	// todo
	
	return 1;
}