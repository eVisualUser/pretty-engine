#include <PrettyEngine/debug/debug.hpp>
#include <PrettyEngine/localization.hpp>
#include <PrettyEngine/assetManager.hpp>

int main() {
	
	#ifdef _DEBUG
		DebugLog(LOG_DEBUG, "Def: " << PRETTY_ENGINE_PROJECT, true);
	#endif

	auto asset = PrettyEngine::Asset("cpp_logo.png");
	
	DebugLog(LOG_DEBUG, "Out: " << asset.Read().data(), true);

	return 0;
}
