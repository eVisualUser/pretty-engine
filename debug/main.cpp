#include <PrettyEngine/debug.hpp>
#include <PrettyEngine/localization.hpp>

int main() {
	
	#ifdef _DEBUG
		DebugLog(LOG_DEBUG, "Def: " << PRETTY_ENGINE_PROJECT, true);
	#endif

	return 0;
}
