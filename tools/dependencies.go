// This scripts install all the required dependencies from the engine

package main

func CheckDependencies() {
	var links []string

	links = append(links, "https://cmake.org/")
	links = append(links, "https://llvm.org/")

	print("List of dependencies that are not safe to be installed from script:\n")

	for _, link := range links {
		print("- ", link, "\n")
	}
}
