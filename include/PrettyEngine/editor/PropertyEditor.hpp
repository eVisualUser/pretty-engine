#ifndef HPP_PROPERTY_EDITOR
#define HPP_PROPERTY_EDITOR

#include <PrettyEngine/serial.hpp>

namespace PrettyEngine {
	class PropertyEditor {
	public:
		virtual void Edit(SerializedField *serializedField) {}
	};
}

#endif