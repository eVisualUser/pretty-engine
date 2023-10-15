#pragma once

#include <PrettyEngine/collider.hpp>

namespace PrettyEngine {
	struct Collision {
	public:
		Collider* colliderA;
		Collider* colliderB;
	};
}
