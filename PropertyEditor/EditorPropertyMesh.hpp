#ifndef HPP_EDITOR_PROPERTY_MESH
#define HPP_EDITOR_PROPERTY_MESH

#include <PrettyEngine/editor/PropertyEditor.hpp>

#include <imgui.h>

namespace Custom {
	class EditorPropertyMesh : public PrettyEngine::PropertyEditor {
	public:
		void Edit(PrettyEngine::SerializedField* serializedField) override { 
			if (serializedField->type == "PrettyEngine::Mesh" || serializedField->type == "Mesh") {
				ImGui::Text("Only the \"rect\" mesh is available for now.");
			}
		}
	};
}

#endif