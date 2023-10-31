#pragma once

#include <PrettyEngine/collider.hpp>

namespace PrettyEngine {
	class Collision {
	public:
		Collider* colliderSource;
		Collider* colliderOther;

		bool hit = false;
	};
}