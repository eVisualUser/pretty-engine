#ifndef UNIQUE_H
#define UNIQUE_H

#include <string>

namespace PrettyEngine {
	class UniqueName {
	public:
		static std::string* GetUniqueName(const std::string& base);
		static void ClearNames();
	};
}

#endif