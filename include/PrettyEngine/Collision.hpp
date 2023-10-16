#pragma once

#include <PrettyEngine/collider.hpp>

namespace PrettyEngine {
	struct Collision {
	public:
		Collider* colliderSource;
		Collider* colliderOther;

		bool colliderCenterInOther = false;
	};
}
