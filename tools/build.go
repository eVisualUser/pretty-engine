package main

import (
	"log"
	"os"
	"os/exec"
	"path/filepath"
)

const (
	InfoColor    = "\033[1;34m"
	NoticeColor  = "\033[1;36m"
	WarningColor = "\033[1;33m"
	ErrorColor   = "\033[1;31m"
	DebugColor   = "\033[0;36m"
	ResetColor   = "\033[0m"
)

func main() {
	if RequiredToolsInstalled() {
		Build()
	}
}

func RequiredToolsInstalled() bool {
	// todo
	return true
}

func Build() {
	log.Print("Clear temporary files")
	clearErr := os.RemoveAll("../Build/public")

	if clearErr != nil {
		log.Fatal(clearErr)
	}

	log.Print("Run CMake")
	cmakeCommand := exec.Command("cmake", "-S", "../", "-G", "Ninja", "-B", "../Build", "-DCMAKE_BUILD_TYPE=Debug")
	cmakeCommand.Stdout = os.Stdout
	cmakeCommand.Stderr = os.Stderr
	cmakeErr := cmakeCommand.Run()

	if cmakeErr != nil {
		log.Fatal(cmakeErr)
	} else {
		log.Print(NoticeColor, "Cmake succeed", ResetColor)
	}

	log.Print("Run Ninja")
	ninjaCommand := exec.Command("ninja", "-C", "../Build")
	ninjaCommand.Stdout = os.Stdout
	ninjaCommand.Stderr = os.Stderr
	ninjaErr := ninjaCommand.Run()

	if ninjaErr != nil {
		log.Fatal(ninjaErr)
	} else {
		log.Print(NoticeColor, "Ninja succeed", ResetColor)
	}

	targetDirectory := "../build"

	buildFiles, err := os.ReadDir(targetDirectory)

	if err != nil {
		log.Fatal(err)
	} else {
		log.Print("Executable files: ")
		for _, buildFile := range buildFiles {

			ext := filepath.Ext(buildFile.Name())

			if ext == ".exe" || ext == ".bin" || ext == ".run" || ext == ".out" {
				fileStat, err := os.Stat(filepath.Join(targetDirectory, buildFile.Name()))

				if err == nil {

					log.Print(NoticeColor, "- ", buildFile.Name(), " modified the ", fileStat.ModTime(), ResetColor)
				} else {
					log.Print("- ", buildFile.Name())
					log.Fatal(err)
				}
			}
		}
	}
}
