#pragma once

#include <PrettyEngine/debug.hpp>

#include <vector>
#include <string>
#include <fstream>

namespace PrettyEngine {
	#define PRINT_GLM_VEC3(vec) std::cout << vec.x << ';' << vec.y << ';' << vec.z << std::endl;
	#define PRINT_GLM_VEC3_PTR(vec) std::cout << vec->x << ';' << vec->y << ';' << vec->z << std::endl;

	class OString {
	public:
		virtual std::string ToString() { return "Not overriden"; }
	};

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
