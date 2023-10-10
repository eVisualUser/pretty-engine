#pragma once

#include <glad/glad.h>

#include <string>

namespace PrettyEngine {
	class Shader {
	public:	
		Shader(std::string name) {
			this->_shaderID = glCreateShader();
		}

		void Compile() {
			glCompileShader(this->_shaderID);
		}

	private:
		unsigned int _shaderID;
	};
}