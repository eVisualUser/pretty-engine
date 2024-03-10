#ifndef HPP_EDITOR_PROPERTY_BOOL
#define HPP_EDITOR_PROPERTY_BOOL

#include <PrettyEngine/editor/PropertyEditor.hpp>
#include <imgui.h>

namespace Custom {
	class EditorPropertyBool: public PrettyEngine::PropertyEditor {
	public:
		EditorPropertyBool() {}

		void Edit(PrettyEngine::SerializedField* serializedField) override {
			if (serializedField->type == SERIAL_TOKEN(bool)) {
				bool buffer = serializedField->value == "true" ? true : false;
				ImGui::Checkbox(serializedField->name.c_str(), &buffer);
				serializedField->value = buffer ? "true" : "false";
			}
		}
	};
}

#endif