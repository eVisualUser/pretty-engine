// Remove all files that are not support by the engine

package main

import (
	"log"
	"os"
	"path/filepath"
)

func main() {
	ClearAssets()
}

func ClearAssets() {
	log.Print("Remove all uncompatible files with the engine...")

	targetDirectory := "../../Assets/ENGINE_PUBLIC"
	entries, err := os.ReadDir(targetDirectory)

	if err != nil {
		log.Fatal(err)
	} else {
		for _, entry := range entries {
			ext := filepath.Ext(entry.Name())
			realPath := filepath.Join(targetDirectory, entry.Name())

			metaPath := filepath.Join(realPath, ".meta")

			if _, err := os.Stat(metaPath); true {
	        	if os.IsNotExist(err) {
	        		if realInfo, _ := os.Stat(realPath); true {
	        			if !realInfo.IsDir() {
	            			log.Print("No meta file for: ", realPath, ", maybe an unused files.")
	        			}
	        		}
	            }
	        }

			requireRemove := false

			switch ext {
			case ".blend":
				requireRemove = true
			case ".blend1":
				requireRemove = true
			case ".fbx":
				requireRemove = true
			case ".mp3":
				requireRemove = true
			case ".mp4":
				requireRemove = true
			default:
				requireRemove = false
			}

			if requireRemove {
				err := os.Remove(realPath)
				if err != nil {
					log.Fatal(err)
				} else {
					log.Print("Removed: ", realPath)
				}
			}

		}
	}
}
