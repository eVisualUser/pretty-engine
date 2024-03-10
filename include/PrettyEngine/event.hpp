#ifndef HPP_EVENT
#define HPP_EVENT

#include <PrettyEngine/tags.hpp>

#include <string>
#include <vector>

namespace PrettyEngine {
	/// An event is used to share data between objects without direct access.
	class Event: public Tagged {
	public:
		std::string name;
		void* content;
	};

 	/// An object that can listen to events.
	class EventListener {
	public:
		virtual void OnEvent(Event* event) {}
	};

 	/// Manage and distribute the events to the listeners.
	class EventManager {
	public:
		void SendEvent(Event *event) {
			for (auto &eventListener : this->_eventListeners) {
				eventListener->OnEvent(event);
			}
		}

		void RegisterListener(EventListener* listener) {
			this->_eventListeners.push_back(listener);
		}

		void UnRegisterListener(EventListener* listener) {
			for(size_t i = 0; i < this->_eventListeners.size(); i++) {
				if (this->_eventListeners[i] == listener) {
					this->_eventListeners.erase(this->_eventListeners.begin() + i);
				}
			}
		}

	private:
		std::vector<Event*> _events;
		std::vector<EventListener*> _eventListeners;
	};
}

#endif