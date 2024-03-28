#include <PrettyEngine/UniqueName.hpp>
#include <PrettyEngine/debug/debug.hpp>

#include <vector>

namespace PrettyEngine {
	static std::vector<std::string> _names;

	std::string* UniqueName::GetUniqueName(const std::string& base){
		bool exist = false;
		int index = -1;
		for(const auto & existingName: _names) {
			if (existingName.starts_with(base)) {
				exist = true;
				index++;
			}
		}

		std::string outString;

		if (exist) {
			outString = base + "_" + std::to_string(index);
		} else {
			_names.push_back(base);
			outString = base;
		}

		_names.push_back(outString);
		return &_names.back();
	}

	void UniqueName::ClearNames() {
		_names.clear();
	}
}
