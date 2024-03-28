#include <PrettyEngine/debug/debug.hpp>
#include <PrettyEngine/simplex.hpp>

int main() { 

	PrettyEngine::Simplex simplex;

	simplex.PushFront(glm::vec3(0, 1, 0));
	simplex.PushFront(glm::vec3(0, 2, 0));
	simplex.PushFront(glm::vec3(0, 3, 0));
	simplex.PushFront(glm::vec3(0, 4, 0));

	try {
		if(simplex[0] != glm::vec3(0, 4, 0)) { DebugLog(LOG_ERROR, "Failed Simplex PushFront", false); } else { DebugLog(LOG_INFO, "Success Simplex PshFront 0", false); }
		if(simplex[1] != glm::vec3(0, 3, 0)) { DebugLog(LOG_ERROR, "Failed Simplex PushFront", false); } else { DebugLog(LOG_INFO, "Success Simplex PshFront 1", false); }
		if(simplex[2] != glm::vec3(0, 2, 0)) { DebugLog(LOG_ERROR, "Failed Simplex PushFront", false); } else { DebugLog(LOG_INFO, "Success Simplex PshFront 2", false); }
		if(simplex[3] != glm::vec3(0, 1, 0)) { DebugLog(LOG_ERROR, "Failed Simplex PushFront", false); } else { DebugLog(LOG_INFO, "Success Simplex PshFront 3", false); }
	} catch(std::string err) {
		DebugLog(LOG_ERROR, "Out: " << err, false);
		return 1;
	}

	return 0;
}
