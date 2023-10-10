#pragma once

#include <PrettyEngine/debug.hpp>

#include <glm/glm.hpp>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace PrettyEngine {
	class GraphicAPI {
	public:
		GraphicAPI(GLFWwindow* window) {
			glfwMakeContextCurrent(window);
		
			if (!gladLoadGL()) {
	            DebugLog(LOG_ERROR, "Failed to initialize glad", true);
	            glfwTerminate();
	            std::exit(-1);
	        }

	        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
	            glfwTerminate();
	            std::exit(-1);
	        }

	        glEnable(GL_MULTISAMPLE);
	        glEnable(GL_BLEND);
	        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		}

		void CompileShader() {
			
		}

		void Clear(glm::vec4 color) {
			glClearColor(color.r, color.g, color.b, color.a);

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		}

	private:
		GLFWwindow* window;
	};
}