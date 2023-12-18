#ifndef HPP_EDITOR_PROPERTY_STRING
#define HPP_EDITOR_PROPERTY_STRING

#include <PrettyEngine/editor/PropertyEditor.hpp>
#include <imgui.h>
#include <string>

namespace Custom {
	class EditorPropertyString : public PrettyEngine::PropertyEditor {
	public:
		EditorPropertyString() {}

		void Edit(PrettyEngine::SerializedField* serializedField) override {
			if (serializedField->type == SERIAL_TOKEN(std::string)) {
				char buffer[100];
				strcpy_s(buffer, serializedField->value.c_str());
				ImGui::InputText(serializedField->name.c_str(), buffer, 100);
				serializedField->value = buffer;
			}
		}
	};
}

#endif