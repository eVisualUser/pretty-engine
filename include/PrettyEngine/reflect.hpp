#pragma once

#include <PrettyEngine/debug.hpp>
#include <PrettyEngine/data.hpp>

#include <Guid.hpp>

#include <fstream>
#include <string>
#include <sstream>

namespace PrettyEngine {
	class SharedObject {
	public:
		void SetObjectName(std::string newName) {
			this->name = newName;
		}

	public:
		std::string name = "DefaultName";
	};
}
