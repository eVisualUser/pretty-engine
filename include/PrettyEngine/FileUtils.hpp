#ifndef HPP_FILE_UTILS
#define HPP_FILE_UTILS

#include <string>
#include <vector>

namespace PrettyEngine {
	class FileUtils {
	public:
		static std::string MakeStringFromStringVector(std::vector<std::string> input, char separator = ';') {
			std::string out;

			for (auto &i : input) {
				out += i;
				out += ';';
			}

			return out;
		}
	};
}

#endif