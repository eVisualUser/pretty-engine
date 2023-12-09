#include <PrettyEngine/debug.hpp>

int main() {
	
	#ifdef _DEBUG
		DebugLog(LOG_DEBUG, "Def: " << PRETTY_ENGINE_PROJECT, true);
	#endif

	return 0;
}
