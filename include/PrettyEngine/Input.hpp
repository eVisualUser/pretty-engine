#pragma once

#include <PrettyEngine/KeyCode.hpp>

#include <GLFW/glfw3.h>
#include <glm/vec2.hpp>

#include <unordered_map>

namespace PrettyEngine {
	enum class CursorState {
		Hidden = GLFW_CURSOR_HIDDEN,
		Locked = GLFW_CURSOR_DISABLED,
		Normal = GLFW_CURSOR_NORMAL,
	};

	static glm::vec2 wheelScroll;

	static void MouseWheelScrollCallBack(GLFWwindow* window, double x, double y) {
        wheelScroll.x = static_cast<float>(x);
        wheelScroll.y = static_cast<float>(y);
    }

	class Input {
	public:
		Input(GLFWwindow* window) {
			this->SetWindow(window);
		}

		void SetWindow(GLFWwindow* window) {
			this->_window = window;

			glfwSetScrollCallback(window, MouseWheelScrollCallBack);
		}

		bool GetKeyPress(KeyCode key) {
			return GLFW_PRESS == glfwGetKey(this->_window, (int)key);
		}

		bool GetKeyDown(KeyCode key) {
		    static std::unordered_map<int, bool> keyStates;
		    int state = glfwGetKey(this->_window, (int)key);
		    bool isKeyDown = (state == GLFW_PRESS) && !keyStates[(int)key];
		    keyStates[(int)key] = (state == GLFW_PRESS);

		    return isKeyDown;
		}

		bool GetKeyUp(KeyCode key) {
			return GLFW_RELEASE == glfwGetKey(this->_window, (int)key);
		}

		bool GetMouseButton(int button = 0) {
			int state = glfwGetMouseButton(this->_window, button);
			if (state == GLFW_PRESS) {
				return true;
			}
			return false;
		}

		bool GetMouseButtonClick(int button = 0) {
			static std::unordered_map<int, bool> keyStates;
		    int state = glfwGetMouseButton(this->_window, button);
		    bool isKeyDown = (state == GLFW_PRESS) && !keyStates[button];
		    keyStates[button] = (state == GLFW_PRESS);

		    return isKeyDown;
		}

		glm::vec2 GetCursorPosition() {
			double x, y;
			glfwGetCursorPos(this->_window, &x, &y);

			auto out = glm::vec2(x, y);
			return out;
		}

		glm::vec2 GetCursorDelta() {
			return this->_cursorLastPosition - this->GetCursorPosition();
		}

	    float GetMouseWheelDelta() {
	        return wheelScroll.y;
	    }

	    void Update() {
	    	wheelScroll = glm::vec2();
        
        	this->_cursorLastPosition = this->GetCursorPosition();
	    }

	    void SetCursorState(CursorState state) {
			glfwSetInputMode(this->_window, GLFW_CURSOR, (int)state);
		}

	private:
		glm::vec2 _cursorLastPosition = glm::vec2();

		GLFWwindow* _window;
	};
}
