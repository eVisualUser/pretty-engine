#ifndef H_COLLISION
#define H_COLLISION

#include <PrettyEngine/collider.hpp>

namespace PrettyEngine {
	class Collision {
	public:
		Collider* colliderSource;
		Collider* colliderOther;

		bool hit = false;
	};
}

#endif