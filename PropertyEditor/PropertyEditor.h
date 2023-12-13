#pragma once

#include <PrettyEngine/editor/PropertyEditor.hpp>
#include <vector>
#include <memory>

// Generated files
#include<EditorPropertyBool.hpp>

#include<EditorPropertyString.hpp>

/// Create an instance of all PropertyEditor
static std::vector<std::shared_ptr<PrettyEngine::PropertyEditor>> GeneratePropertyEditorList() {
	std::vector<std::shared_ptr<PrettyEngine::PropertyEditor>> result;

	result.push_back(std::make_shared<Custom::EditorPropertyBool>());
	result.push_back(std::make_shared<Custom::EditorPropertyString>());

	return result;
}
