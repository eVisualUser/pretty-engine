#include <PrettyEngine/debug/debug.hpp>
#include <PrettyEngine/simplex.hpp>

int main() { 

	PrettyEngine::Simplex simplex;

	simplex.PushFront(glm::vec3(0, 1, 0));
	simplex.PushFront(glm::vec3(0, 2, 0));
	simplex.PushFront(glm::vec3(0, 3, 0));
	simplex.PushFront(glm::vec3(0, 4, 0));

	if (simplex.size() != 4) { DebugLog(LOG_ERROR, "Wrong size, get " << 4 << " but got " << simplex.size(), false); return 1; }
	
	if(simplex[0] != glm::vec3(0, 4, 0)) { DebugLog(LOG_ERROR, "Failed Simplex PushFront", false); return 1; }
	if(simplex[1] != glm::vec3(0, 3, 0)) { DebugLog(LOG_ERROR, "Failed Simplex PushFront", false); return 1; }
	if(simplex[2] != glm::vec3(0, 2, 0)) { DebugLog(LOG_ERROR, "Failed Simplex PushFront", false); return 1; }
	if(simplex[3] != glm::vec3(0, 1, 0)) { DebugLog(LOG_ERROR, "Failed Simplex PushFront", false); return 1; }


	simplex = {glm::vec3(0, 0, 0), glm::vec3(0, 1, 0), glm::vec3(0, 2, 0), glm::vec3(0, 3, 0)};

	if (simplex.size() != 4) { DebugLog(LOG_ERROR, "Wrong size, get " << 4 << " but got " << simplex.size(), false); return 1; }
	
	if(simplex[0] != glm::vec3(0, 0, 0)) { DebugLog(LOG_ERROR, "Failed Simplex Assign", false); return 1; }
	if(simplex[1] != glm::vec3(0, 1, 0)) { DebugLog(LOG_ERROR, "Failed Simplex Assign", false); return 1; }
	if(simplex[2] != glm::vec3(0, 2, 0)) { DebugLog(LOG_ERROR, "Failed Simplex Assign", false); return 1; }
	if(simplex[3] != glm::vec3(0, 3, 0)) { DebugLog(LOG_ERROR, "Failed Simplex Assign", false); return 1; }

	return 0;
}
