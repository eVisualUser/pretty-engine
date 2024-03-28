#include <PrettyEngine/UniqueName.hpp>
#include <PrettyEngine/debug/debug.hpp>

int main() {
	if (const auto name = PrettyEngine::UniqueName::GetUniqueName("Hello")) {
		if ("Hello" != *name) {
			DebugLog(LOG_ERROR, "Excepted Hello but got " << *name, false);
			return 1;
		}
	} else {
		DebugLog(LOG_ERROR, "GetUniqueName return nullptr", false);
	}

	if (const auto name = PrettyEngine::UniqueName::GetUniqueName("Hello")) {
		if ("Hello_1" != *name) {
			DebugLog(LOG_ERROR, "Excepted Hello_1 but got " << *name, false);
			return 1;
		}
	} else {
		DebugLog(LOG_ERROR, "GetUniqueName return nullptr", false);
	}

	if (const auto name = PrettyEngine::UniqueName::GetUniqueName("Hello")) {
		if ("Hello_2" != *name) {
			DebugLog(LOG_ERROR, "Excepted Hello_2 but got " << *name, false);
			return 1;
		}
	} else {
		DebugLog(LOG_ERROR, "GetUniqueName return nullptr", false);
	}

	return 0;
}
