#pragma once

#include <glad/glad.h>

#include <string>
#include <unordered_map>
#include <iostream>

namespace PrettyEngine {
	enum class DrawMode {
		Triangles = GL_TRIANGLES,
		TriangleStrip = GL_TRIANGLE_STRIP,
		TriangleFan = GL_TRIANGLE_FAN,
		Lines = GL_LINES,
		Points = GL_POINTS,
		Quads = GL_QUADS,
		Polygone = GL_POLYGON,
	};

	enum class ShaderType {
		Vertex = GL_VERTEX_SHADER,
		Fragment = GL_FRAGMENT_SHADER,
		Geometry = GL_GEOMETRY_SHADER,
		TessControl = GL_TESS_CONTROL_SHADER,
		TessEval = GL_TESS_EVALUATION_SHADER,
		Compute = GL_COMPUTE_SHADER,
	};

	class GLShaderProgramRefs {
	public:
		unsigned int shaderProgram;

		std::unordered_map<std::string, unsigned int> uniforms;

	public:
		void CreateUniform(std::string keyName, std::string name) {
			unsigned int id = glGetUniformLocation(this->shaderProgram, name.c_str());
			this->uniforms.insert(std::make_pair(keyName, id));
		}
	};

	// Call glGetError() after an OpenGL function call to check for errors
	static void CheckOpenGLError(const char* file, int line) {
	    GLenum errorCode;
	    while ((errorCode = glGetError()) != GL_NO_ERROR) {
	        std::string error;
	        switch (errorCode) {
	            case GL_INVALID_ENUM:                  error = "GL_INVALID_ENUM"; break;
	            case GL_INVALID_VALUE:                 error = "GL_INVALID_VALUE"; break;
	            case GL_INVALID_OPERATION:             error = "GL_INVALID_OPERATION"; break;
	            case GL_STACK_OVERFLOW:                error = "GL_STACK_OVERFLOW"; break;
	            case GL_STACK_UNDERFLOW:               error = "GL_STACK_UNDERFLOW"; break;
	            case GL_OUT_OF_MEMORY:                 error = "GL_OUT_OF_MEMORY"; break;
	            case GL_INVALID_FRAMEBUFFER_OPERATION: error = "GL_INVALID_FRAMEBUFFER_OPERATION"; break;
	        }
	        std::cout << "OpenGL Error (" << error << "): " << file << ":" << line << std::endl;
	    }
	}

	// Place this macro after each significant OpenGL function call
	#define GL_CHECK_ERROR() PrettyEngine::CheckOpenGLError(__FILE__, __LINE__)
}
