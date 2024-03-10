#ifndef HPP_EDITOR_PROPERTY_NUMBER
#define HPP_EDITOR_PROPERTY_NUMBER

#include <PrettyEngine/editor/PropertyEditor.hpp>
#include <PrettyEngine/localization.hpp>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <imgui.h>

namespace Custom {
	class EditorPropertyNumber : public PrettyEngine::PropertyEditor {
	public:
		void Edit(PrettyEngine::SerializedField* serializedField) override { 
			if (serializedField->type == "int") {
				int value = std::stoi(serializedField->value);
				ImGui::InputInt(serializedField->name.c_str(), &value);
				serializedField->value = std::to_string(value);
			}
			else if (serializedField->type == "float") {
				float value = std::stof(serializedField->value);
				ImGui::InputFloat(serializedField->name.c_str(), &value);
				serializedField->value = std::to_string(value);
			}
			else if (serializedField->type == "double") {
				double value = std::stod(serializedField->value);
				ImGui::InputDouble(serializedField->name.c_str(), &value);
				serializedField->value = std::to_string(value);
			}
			else if (serializedField->type == SERIAL_TOKEN(glm::vec3)) {
				auto parsed = PrettyEngine::ParseCSVLine(serializedField->value);
				float buffer[3] = {0.0f, 0.0f, 0.0f};
				buffer[0] = std::stof(parsed[0]);
				buffer[1] = std::stof(parsed[1]);
				buffer[2] = std::stof(parsed[2]);
				ImGui::InputFloat3(serializedField->name.c_str(), buffer);

				std::string colorPickerName = "Color picker: ";
				colorPickerName += serializedField->name;
				if (ImGui::CollapsingHeader(colorPickerName.c_str())) {
					ImGui::ColorPicker3(colorPickerName.c_str(), buffer);
				}
				
				std::string newValue;
				for (auto &value : buffer) {
					newValue += std::to_string(value);
					newValue += ';';
				}
				serializedField->value = newValue;
			} else if (serializedField->type == SERIAL_TOKEN(glm::vec4)) {
				auto parsed = PrettyEngine::ParseCSVLine(serializedField->value);
				float buffer[4] = {0.0f, 0.0f, 0.0f, 0.0f};
				buffer[0] = std::stof(parsed[0]);
				buffer[1] = std::stof(parsed[1]);
				buffer[2] = std::stof(parsed[2]);
				buffer[3] = std::stof(parsed[3]);
				ImGui::InputFloat4(serializedField->name.c_str(), buffer);

				std::string colorPickerName = "Color picker: ";
				colorPickerName += serializedField->name;

				if (ImGui::CollapsingHeader(colorPickerName.c_str())) {
					ImGui::ColorPicker4(colorPickerName.c_str(), buffer);
				}
				std::string newValue;
				for (auto &value : buffer) {
					newValue += std::to_string(value);
					newValue += ';';
				}
				serializedField->value = newValue;
			} else if (serializedField->type == SERIAL_TOKEN(glm::vec2)) {
				auto parsed = PrettyEngine::ParseCSVLine(serializedField->value);
				float buffer[2] = {0.0f, 0.0f};
				buffer[0] = std::stof(parsed[0]);
				buffer[1] = std::stof(parsed[1]);
				ImGui::InputFloat2(serializedField->name.c_str(), buffer);
				std::string newValue;
				for (auto &value : buffer) {
					newValue += std::to_string(value);
					newValue += ';';
				}
				serializedField->value = newValue;
			}
		}
	};
}

#endif