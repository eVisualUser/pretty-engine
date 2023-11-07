#pragma once

#include <string>
#include <vector>

namespace PrettyEngine {
	class Event {
	public:
		std::string name;
		void* content;
	}; 

	class EventListener {
	public:
		void OnEvent(Event* event) {}
	};

	class EventManager {
	public:
		void RegisterEvent(Event* event) {
			this->_events.push_back(event);
		}

		void Update() {
			for(auto & event: this->_events) {
				for(auto & listener: this->_eventListeners) {
					listener->OnEvent(event);
				}
			}
			this->_events.clear();
		}

		void RegisterListener(EventListener* listener) {
			this->_eventListeners.push_back(listener);
		}

		void UnRegisterListener(EventListener* listener) {
			for(size_t i = 0; i < this->_eventListeners.size(); i++) {
				if (this->_eventListeners[i] == listener) {
					this->_eventListeners.erase(this->_eventListeners.begin() + static_cast<int>(i));
				}
			}
		}

	private:
		std::vector<Event*> _events;
		std::vector<EventListener*> _eventListeners;
	};
}
