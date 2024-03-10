#ifndef H_INPUT
#define H_INPUT

#include <PrettyEngine/utils.hpp>
#include <PrettyEngine/KeyCode.hpp>

#include <GLFW/glfw3.h>
#include <glm/vec2.hpp>

#include <unordered_map>
#include <vector>
#include <string>
#include <functional>

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

    enum class KeyWatcherMode {
    	Press = 0,
    	Down,
    	Up,
    };

    static std::string KeyWatcherModeToString(KeyWatcherMode mode) {
    	if (mode == KeyWatcherMode::Press) {
    		return "Press";
    	} else if (mode == KeyWatcherMode::Down) {
    		return "Down";
    	} else if (mode == KeyWatcherMode::Up) {
    		return "Up";
    	}
    	return "Press";
    }

    static KeyWatcherMode KeyWatcherModeFromString(std::string mode) {
    	if (mode == "Press") {
    		return KeyWatcherMode::Press;
    	} else if (mode == "Down") {
    		return KeyWatcherMode::Down;
    	} else if (mode == "Up") {
    		return KeyWatcherMode::Up;
    	}
    	return KeyWatcherMode::Press;
    }

 	/// Safe way to manage inputs and share them between objects.
    class KeyWatcher {
    public:
    	std::string name = "?";
    	KeyCode key;
    	KeyWatcherMode mode;
    	bool state;

    	std::function<void()> actionOnKey;
    };

 	/// Manage the inputs of the game.
	class Input {
	public:
		Input(GLFWwindow* window) {
			this->SetWindow(window);
		}

		Input() {}

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
	    	wheelScroll = glm::vec2(0.0f, 0.0f);
        
        	this->_cursorLastPosition = this->GetCursorPosition();

        	for(auto & watcher: this->_keyWatchers) {
        		if (watcher->mode == KeyWatcherMode::Press) {
        			watcher->state = this->GetKeyPress(watcher->key);
        		} else if (watcher->mode == KeyWatcherMode::Down) {
        			watcher->state = this->GetKeyDown(watcher->key);
        		} else if (watcher->mode == KeyWatcherMode::Up) {
        			watcher->state = this->GetKeyUp(watcher->key);
        		}

        		if (watcher->state) {
        			(watcher->actionOnKey)();
        		}
        	}
	    }

	    void SetCursorState(CursorState state) {
			glfwSetInputMode(this->_window, GLFW_CURSOR, (int)state);
		}

		void AddKeyWatcher(KeyWatcher* keyWatcher) {
			this->_keyWatchers.push_back(keyWatcher);
			this->_keyWatchersLog.push_back(*keyWatcher);
		}

		void RemoveKeyWatcher(KeyWatcher* keyWatcher) {
			size_t index = 0;
			for(auto & watcher: this->_keyWatchers) {
				if (watcher == keyWatcher) {
					this->_keyWatchers.erase(this->_keyWatchers.begin() + index);
					return;
				}
				index++;
			}
		}

		const std::vector<KeyWatcher*>* GetKeyWatchers() {
			return &this->_keyWatchers;
		}

		bool KeyWatcherExist(KeyWatcher* keyWatcher) {
			if (keyWatcher->name == "?") {
				return false;
			}

			for(auto & key: this->_keyWatchers) {
				if (key->name == keyWatcher->name) {
					return true;
				}
			}

			return false;
		}

  		void Clear() {
   			this->_keyWatchers.clear();
  		}

	private:
		glm::vec2 _cursorLastPosition = glm::vec2();

		std::vector<KeyWatcher*> _keyWatchers;
		std::vector<KeyWatcher> _keyWatchersLog;

		GLFWwindow* _window;

		std::string _savePath = GetEnginePublicPath("input.pe", true);
	};
}

#endif