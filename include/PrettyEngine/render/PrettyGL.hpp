#ifndef H_PRETTY_GL
#define H_PRETTY_GL

#include <PrettyEngine/gc.hpp>
#include <PrettyEngine/debug/debug.hpp>
#include <PrettyEngine/localization.hpp>
#include <PrettyEngine/utils.hpp>

#include <glad/glad.h>

#include <string>
#include <unordered_map>

namespace PrettyEngine {
	#define GLENUM (GLenum)

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

    enum class MeshDrawType {
    	/// Can be updated if needed but not recomended for each frame (see MeshDrawType::Stream)
        Dynamic = GL_DYNAMIC_DRAW,
    	/// Cannot be updated each frame
        Static = GL_STATIC_DRAW,
    	/// Can be updated each frame
        Stream = GL_STREAM_DRAW,
    };

	class GLShaderProgramRefs: public GCObject {
	public:
		unsigned int shaderProgram;

		std::unordered_map<std::string, unsigned int> uniforms;

		void CreateUniform(std::string keyName, std::string name) {
			unsigned int id = glGetUniformLocation(this->shaderProgram, name.c_str());
			this->uniforms.insert(std::make_pair(keyName, id));
		}

		void CreateUniformsFromCSV(std::string content) {
			std::string stringBuffer;
			for(auto & c: content) {
				if (c != '\n') {
					stringBuffer.push_back(c);
				} else {
					auto csv = ParseCSVLine(stringBuffer);
					if (csv.size() > 1) {
						this->CreateUniform(csv[0], csv[1]);
					}
					stringBuffer.clear();
				}
			}
		}
	};

	/// Call glGetError() after an OpenGL function call to check for errors
	static bool CheckOpenGLError(const char* file, int line) {
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
	        DebugLog(LOG_ERROR, "OpenGL Error (" << error << "): " << file << ":" << line, true);
	        return true;
	    }
	    return false;
	}

	// Place this macro after each significant OpenGL function call
	#define GL_CHECK_ERROR() PrettyEngine::CheckOpenGLError(__FILE__, __LINE__)
}

#endif