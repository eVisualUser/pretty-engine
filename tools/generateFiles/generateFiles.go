// This script generate the project files

package main

import (
	"log"
	"os"
	"path/filepath"
	"strings"
	"runtime"
)

func main() {
	GenerateShadersHeader()
	GenerateCustomObjectScript()
}

func MakeCPPFileStringVariable(name string, content string) string {
	var result string

	result += "/// Generated shader variable from file\n"
	result += "static const char* " + name + " = "
	result += "R\"("
	result += content
	result += ")\";"

	return result
}

func GenerateShadersHeader() {
	_, executable, _, _ := runtime.Caller(0)

	directory := filepath.Dir(executable)

	dir := filepath.Join(directory, "../../shaders")

	shaders, err := os.ReadDir(dir)

	shadersMap := make(map[string][]byte)

	if err != nil {
		log.Fatal(err)
	} else {
		for _, shader := range shaders {
			// Verify if the file is a shader
			if filepath.Ext(shader.Name()) == ".vert" || filepath.Ext(shader.Name()) != ".frag" || filepath.Ext(shader.Name()) != ".glsl" {
				content, err := os.ReadFile(dir + "/" + shader.Name())

				if err != nil {
					log.Fatal(err)
				} else {
					var shaderName string
					shaderName = "SHADER_"
					shaderName += strings.ReplaceAll(shader.Name(), ".glsl", "_GLSL")
					shaderName = strings.ReplaceAll(shaderName, ".vert", "_VERTEX")
					shaderName = strings.ReplaceAll(shaderName, ".frag", "_FRAGMENT")
					shadersMap[strings.ToUpper(shaderName)] = content
				}
			}
		}

		var output string

		output += "#pragma once\n\n"

		output += "namespace Shaders {\n\n"

		for shaderName, shaderContent := range shadersMap {
			cleanShaderName := strings.ReplaceAll(shaderName, ".", "_")
			log.Print("Generate: " + cleanShaderName)
			output += MakeCPPFileStringVariable(cleanShaderName, string(shaderContent))
			output += "\n\n"
		}

		output += "\n} // Shaders"

		err = os.WriteFile(filepath.Join(directory, "../../include/PrettyEngine/shaders.hpp"), []byte(output), 'w')

		if err != nil {
			log.Fatal(err)
		} else {
			log.Print("Succeed to generate header: shaders.hpp")
		}
	}
}

/// Generate the required function for object that will be generated in runtime
func GenerateCustomObjectScript() {
	_, executable, _, _ := runtime.Caller(0)

	directory := filepath.Dir(executable)

	dir, err := os.ReadDir(filepath.Join(directory, "../../PropertyEditor"))

	if err != nil {
		log.Fatal(err)
	}

	var fileList []string

	for _, entry := range dir {
		if filepath.Ext(entry.Name()) == ".hpp" {
			fileList = append(fileList, entry.Name())
		}
	}

	out := "#pragma once\n" +
		"\n" +
		"#include <PrettyEngine/editor/PropertyEditor.hpp>\n" +
		"#include <vector>\n" +
		"#include <memory>\n\n"
	out += "// Generated files\n"
	for _, file := range fileList {
		out += "#include<" + file + ">\n\n"
	}

	out += "/// Create an instance of all PropertyEditor\n"
	out += "static std::vector<std::shared_ptr<PrettyEngine::PropertyEditor>> GeneratePropertyEditorList() {\n" +
		"	std::vector<std::shared_ptr<PrettyEngine::PropertyEditor>> result;\n\n"

	for _, file := range fileList {
		fileName := strings.ReplaceAll(file, ".hpp", "")
		out += "	result.push_back(" + "std::dynamic_pointer_cast<PropertyEditor>(std::make_shared<Custom::" + fileName + ">())" + ");\n"
	}

	out += "\n" +
		"	return result;\n" +
		"}\n"

	os.Remove(filepath.Join(directory, "../../PropertyEditor/PropertyEditor.h"))
	err = os.WriteFile(filepath.Join(directory, "../../PropertyEditor/PropertyEditor.h"), []byte(out), 'w')

	if err != nil {
		log.Fatal(err)
	}
}
