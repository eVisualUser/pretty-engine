#include <PrettyEngine/debug/debug.hpp>
#include <PrettyEngine/version.hpp>	

int main() {
	
	PrettyEngine::Version version;
	version.FromString("0.1.0");

	DebugLog(LOG_DEBUG, "Version: " << version.ToString(), true);

	return 0;
}
