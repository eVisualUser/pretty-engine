#ifndef HPP_UTILS
#define HPP_UTILS

#include <PrettyEngine/debug/debug.hpp>

#include <string>
#include <vector>
#include <filesystem>
#include <fstream>

#include <glm/vec3.hpp>

namespace PrettyEngine {
	static bool Vec3Greater(glm::vec3 a, glm::vec3 b) {
		return (a.x >= b.x && a.y >= b.y && a.z >= b.z);
	}

	static bool Vec3Lower(glm::vec3 a, glm::vec3 b) {
		return (a.x <= b.x && a.y <= b.y && a.z <= b.z);
	}

	#define PRINT_GLM_VEC3(vec) std::cout << vec.x << ';' << vec.y << ';' << vec.z << std::endl;
	#define PRINT_GLM_VEC3_PTR(vec) std::cout << vec->x << ';' << vec->y << ';' << vec->z << std::endl;

	class OString {
	public:
		virtual ~OString() {}
		virtual std::string ToString() { return "Not overriden"; }
	};

	/// Concat the path of the public directory.
	static std::string GetEnginePublicPath(std::string base, bool editOriginal = false) {
		#if ENGINE_EDITOR
			if (editOriginal) {
				return std::string(PRETTY_ENGINE_PROJECT) + "/assets/ENGINE_PUBLIC/" + base;
			} else {
				return "./public/" + base;
			}
		#else
			return "./public/" + base;
		#endif
	}

	static bool FileExist(const std::string& path) {
		return std::filesystem::exists(path);
	}

	static bool CreateFile(const std::string& path) {
		const auto fsPath = std::filesystem::path(path);
		std::filesystem::create_directories(fsPath.root_directory());

		if (!FileExist(path)) {
			try {
				std::ofstream file(path, std::ios::out | std::ios::trunc);
				bool out = file.is_open();

				file.close();

				return out;
			} catch (const std::exception& e) {
				DebugLog(LOG_ERROR, "Fail: " << e.what(), true);
				return false;
			}
		} else {
			DebugLog(LOG_ERROR, "File already exists: " << path, true);

			return false;
		}
	}

	static std::string ReadFileToString(const std::string& path, size_t readSize = 4096) {
		auto stream = std::ifstream{path.data()};
		stream.exceptions(std::ios_base::badbit);

		auto out = std::string{};
		auto buf = std::string(readSize, '\0');
		while (stream.read(& buf[0], static_cast<std::streamsize>(readSize))) {
			out.append(buf, 0, stream.gcount());
		}
		out.append(buf, 0, stream.gcount());

		return out;
	}

	static bool WriteFileString(std::string path, std::string content) {
		std::ofstream outFile;
		outFile.open(path);
		if (outFile.is_open()) {

			outFile << content;

			outFile.flush();
			outFile.close();

			return true;
		}

		DebugLog(LOG_ERROR, "Failed to write: " << path, true);
		if (boxer::Selection::Yes == boxer::show("Failed to write file, retry ? ", "Retry ?", boxer::Style::Question, boxer::Buttons::YesNo)) {
			return WriteFileString(path, content);
		}

		return false;
	}

	static void StringReplaceChar(std::string* str, char base, char replacement) {
		for (auto & c: *str) {
			if (c == base) {
				c = replacement;
			}
		}
	}

	static std::vector<std::string> StringSplit(std::string* source, char separator) { 
		std::vector<std::string> out;

		std::string buffer;
		for (auto &c : *source) {
			if (c != separator) {
				buffer += c;
			} else {
				out.push_back(buffer);
			}
		}
		if (!buffer.empty()) {
			out.push_back(buffer);
		}

		return out;
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
}

#endif