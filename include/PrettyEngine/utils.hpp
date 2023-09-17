#pragma once

#include <vector>
#include <string>
#include <fstream>

#include <PrettyEngine/debug.hpp>

namespace PrettyEngine {
	#define PRINT_GLM_VEC3(vec) std::cout << vec.x << ';' << vec.y << ';' << vec.z << std::endl;
	#define PRINT_GLM_VEC3_PTR(vec) std::cout << vec->x << ';' << vec->y << ';' << vec->z << std::endl;

	class OString {
	public:
		virtual ~OString() {}
		virtual std::string ToString() { return "Not overriden"; }
	};

	static std::string GetEnginePublicPath(std::string base, bool editOriginal = false) {
		#if _DEBUG
			if (editOriginal) {
				return "../assets/ENGINE_PUBLIC/" + base;
			} else {
				return "./public/" + base;
			}
		#else
			return "./public/" + base;
		#endif
	}

	static bool FileExist(std::string path) {
		std::ifstream file(path);
		bool out = file.is_open();
		file.close();

		return out;
	}

	static bool CreateFile(std::string path) {
		if (!FileExist(path)) {
			std::ofstream file(path);
			bool out = file.is_open();
			file.close();

			return out;
		} else {
			DebugLog(LOG_ERROR, "File already exists: " << path, true);
			
			return false;
		}
	}

	static void StringReplace(std::string* str, char base, char replacement) {
		for (auto & c: *str) {
			if (c == base) {
				c = replacement;
			}
		}
	}

	static void StringReplace(std::string* str, std::string base, std::string replacement) {
		bool match = false;
		size_t len = 0;
		size_t matchLen = 0;
		size_t startMatch = 0;
		for (auto & c: *str) {
			if (match && matchLen == base.size()) {
				for(size_t i = 0; i < matchLen; i++) {
					str->erase(str->begin() + startMatch);
				}
				
				std::string reversedString;
				for(auto & c: replacement) {
					reversedString.insert(reversedString.begin() + 0, c);
				}

				for(auto & c: reversedString) {
					str->insert(str->begin() + startMatch, c);
				}
				
				return StringReplace(str, base, replacement);
			}

			if (c == base[matchLen] && matchLen < base.size()) {
				matchLen++;
				
				if (!match) {
					startMatch = len;
				}

				match = true;
			} else {
				matchLen = 0;
				match = false;
			}

			len++;
		}
	}
	template<typename T>
	static bool CheckIfVectorContain(std::vector<T>* list, T* target) {

		for(const auto & value: *list) {
			if (value == *target) {
				return true;
			}
		}

		return false;
	}

	template<typename T>
	static void Clamp(T* base, T min, T max) {
		if (*base > max)
			*base = max;
		else if (*base < min)
			*base = min;
	}

	template<typename T>
	static T AddWithRest(T* base, T value, T max) {
		T buffer = *base;
		buffer += value;
		if (buffer > max) {
			*base = max;
			return buffer - max; 
		}
		*base = buffer;
		return buffer;
	}

	#define GET_VARIABLE_NAME(name) #name

	#if _DEBUG
		#define PUBLIC_ASSET(fileName) "../assets/ENGINE_PUBLIC" + fileName
	#else
		#define PUBLIC_ASSET(fileName) "./public/" + fileName
	#endif

	static std::string FileToString(std::string fileName) {
		std::ifstream ifs(fileName);

		if (!ifs.is_open()) {
			DebugLog(LOG_ERROR, "Failed to read: " << fileName, true);
			return nullptr;
		}

  		std::string content( (std::istreambuf_iterator<char>(ifs) ),
                       	(std::istreambuf_iterator<char>()    ) );

  		ifs.close();

  		return content;
	}
}
