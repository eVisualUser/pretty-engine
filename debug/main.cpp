#include <PrettyEngine/debug.hpp>
#include <PrettyEngine/localization.hpp>

int main() {
	
	#ifdef _DEBUG
		DebugLog(LOG_DEBUG, "Def: " << PRETTY_ENGINE_PROJECT, true);
	#endif

	PrettyEngine::Localization localization;
	localization.LoadFile(PrettyEngine::GetEnginePublicPath("./editor.csv", true));
	localization.SetLangIndex(localization.GetLangIndex("English"));
	DebugLog(LOG_DEBUG, localization.Get("Hello"), true);

	return 0;
}
