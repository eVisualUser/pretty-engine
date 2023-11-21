#ifndef H_GRAPHICS
#define H_GRAPHICS

#include <PrettyEngine/gl.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace PrettyEngine {
	class Graphics {
	public:
		static void BindVariable(unsigned int id, unsigned int value, int lenght = 1) {
			glUniform1uiv(id, lenght, &value);
		}

		static void BindVariable(unsigned int id, int value, int lenght = 1) {
			glUniform1iv(id, lenght, &value);
		}

		static void BindVariable(unsigned int id, float value, int lenght = 1) {
			glUniform1fv(id, lenght, &value);
		}

		static void BindVariable(unsigned int id, double value, int lenght = 1) {
			glUniform1dv(id, lenght, &value);
		}

		static void BindVariable(unsigned int id, glm::vec2 value, int lenght = 1) {
			glUniform2fv(id, lenght, glm::value_ptr(value));
		}

		static void BindVariable(unsigned int id, glm::vec3 value, int lenght = 1) {
			glUniform3fv(id, lenght, glm::value_ptr(value));
		}

		static void BindVariable(unsigned int id, glm::vec4 value, int lenght = 1) {
			glUniform4fv(id, lenght, glm::value_ptr(value));
		}

		static void BindVariable(unsigned int id, glm::mat4 value, int lenght = 1) {
			glUniformMatrix4fv(id, lenght, GL_FALSE, glm::value_ptr(value));
		}
	};
}

#endif