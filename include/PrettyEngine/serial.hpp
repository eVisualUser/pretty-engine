#pragma once

#include <PrettyEngine/debug.hpp>

#include <toml++/toml.h>

namespace PrettyEngine {
	class SerialObject {
	public:
		virtual void AddToToml(toml::table* table) { DebugLog(LOG_DEBUG, "To do", false); }
		virtual void FromToml(toml::table* table) { DebugLog(LOG_DEBUG, "To do", false); }
	};
}
