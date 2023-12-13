// This script generate the project files

package main

import (
	"log"
	"os"
	"path/filepath"
	"strings"
)

func main() {
	GenerateShadersHeader()
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
	dir := "../shaders"

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

		err := os.WriteFile("../include/PrettyEngine/shaders.hpp", []byte(output), 'w')

		if err != nil {
			log.Fatal(err)
		} else {
			log.Print("Succeed to generate header: shaders.hpp")
		}
	}
}

/// Generate the required function for object that will be generated in runtime
func CustomObjectScript() {

}
