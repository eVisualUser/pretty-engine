#ifndef HPP_ID
#define HPP_ID

#include <PrettyEngine/debug/debug.hpp>

#include <string>

#include <Guid.hpp>

namespace PrettyEngine {
	template<typename T>
	class ID {
	public:
		ID() {
			if (typeid(T) == typeid(std::string)) {
				DebugLog(LOG_DEBUG, "ThE SaMe", false);
				this->_id = xg::newGuid().str();
			} else {
				DebugLog(LOG_ERROR, "The type used by ID have no default value.", true);
				throw "The type used by ID have no default value.";
			}
		}

		ID(T defaultValue) {
			this->_id = defaultValue;
		}

		const std::string* GetID() const {
			return &this->_id;
		}

		bool Equal(ID* other) const {
			return this->_id == *other->GetID();
		}

	private:
		T _id;
	};
}

#endif