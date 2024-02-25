// This scripts install all the required dependencies from the engine

package main

import {
	"log"
	"os/exec"
}

func main() {
	CheckDependencies()
}

func CheckDependencies() {
	var links []string

	cmd := exec.Command("cmake", "--version")

    // Execute the command and capture its output
    output, err := cmd.Output()
    if err != nil {
     	links = append(links, "https://cmake.org/")   
    }

	if (len(links) != 0) {
		log.Print("List of dependencies that are missing:\n")

		for _, link := range links {
			print("- ", link, "\n")
		}
	}
}
