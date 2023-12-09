#ifndef H_SETTINGS
#define H_SETTINGS

#include <PrettyEngine/utils.hpp>
#include <PrettyEngine/utils.hpp>

#include <toml++/toml.h>

#include <string>
#include <fstream>

namespace PrettyEngine {
	/// Easy way to manage settings.
	class Settings {
	public:
		Settings(std::string filePath) {
			this->path = filePath;
			if (!FileExist(this->path)) {
				CreateFile(this->path);
			} else {
				this->tomlTable = toml::parse_file(path);
			}
		}

		~Settings() {
			std::ofstream out;
			out.open(this->path);
			if (out.is_open()) {
				out << this->tomlTable;
				out.flush();
				out.close();
			} else {
				DebugLog(LOG_ERROR, "Failed to save game settings", true);
			}
		}

		std::string GetString(std::string categorie, std::string name) {
			return this->tomlTable[categorie][name].value_or("Null");
		}

		int GetInt(std::string categorie, std::string name) {
			return this->tomlTable[categorie][name].value_or(0);
		}

		float GetFloat(std::string categorie, std::string name) {
			return this->tomlTable[categorie][name].value_or(0.0f);
		}

		template<typename T> 
		T GetEnum(std::string categorie, std::string name) {
			return (T)this->tomlTable[categorie][name].value_or(0);
		}

		template<typename T>
		void Set(std::string categorie, std::string name, T value) {
			if (this->tomlTable.get(categorie) != nullptr) {
				this->tomlTable.get(categorie)->as_table()->insert_or_assign(name, value);
			} else {
				this->tomlTable.insert_or_assign(categorie, toml::table());
				return this->Set(categorie, name, value);
			}
		}

	private:
		std::string path;
		toml::table tomlTable;
	};
}

#endif