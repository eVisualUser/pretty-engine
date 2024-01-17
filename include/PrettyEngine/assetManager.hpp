#ifndef HPP_ASSET_MANAGER
#define HPP_ASSET_MANAGER

#include <PrettyEngine/data.hpp>
#include <PrettyEngine/tags.hpp>
#include <PrettyEngine/serial.hpp>
#include <PrettyEngine/gc.hpp>

#include <vector>
#include <string>
#include <fstream>

namespace PrettyEngine { 
	class Asset: public virtual Tagged, public virtual SerialObject, public virtual GCObject {
	public:
		Asset() {}

		Asset(std::string newPath) { 
			this->remote = false;
			this->path = newPath;

			this->SetObjectSerializedName("Asset");
			this->SetSerializedUnique(newPath);

			if (this->Exist()) {
				this->CreateMeta();

				this->AddSerializedField(SERIAL_TOKEN(bool), "exist", this->Exist() ? "true" : "false");
				this->AddSerializedField(SERIAL_TOKEN(std::string), "metaCreationDate", GetTimeAsString());
				this->AddSerializedField(SERIAL_TOKEN(std::string), "local", "any");
				this->AddSerializedField(SERIAL_TOKEN(bool), "used", "false");

				this->Deserialize(ReadFileToString(this->GetMetaPath()), SerializationFormat::Toml);
				
				this->SetObjectSerializedName("Asset");
				this->SetSerializedUnique(newPath);
			}
		}

		std::string GetFilePath() {
			return GetEnginePublicPath(this->GetObjectSerializedUnique(), true);
		}

		void SetUsed(bool state) {
			this->GetSerializedField("used")->value = state ? "true" : "false";
		}

		bool Exist() { return FileExist(this->GetFilePath()); }

		std::vector<unsigned char> Read() {
			this->GetSerializedField("used")->value = "true";

			std::ifstream input(this->GetFilePath(), std::ios::binary);

			std::vector<unsigned char> output;

			if (input.is_open()) {
				this->GetSerializedField("exist")->value = "true";

				char buffer;
				while (input.get(buffer)) {
					output.push_back(buffer);
				}

				input.close();
			} else {
				this->GetSerializedField("exist")->value = "false";
				DebugLog(LOG_ERROR, "Failed to open: " << this->path, true);
			}
			return output;
		}

		std::string ReadToString() { return ReadFileToString(this->GetFilePath()); }

		~Asset() {
			if (this->Exist() && !WriteFileString(this->GetMetaPath(), this->Serialize(SerializationFormat::Toml))) {
				DebugLog(LOG_ERROR, "Failed to write meta file: " << this->GetMetaPath(), true);
			}
		}

		std::string GetMetaPath() { return this->GetFilePath() + ".meta"; }

		bool HaveMeta() { return FileExist(this->GetMetaPath()); }

		void CreateMeta() { 
			if (!this->HaveMeta() && !CreateFile(this->GetMetaPath())) {
				DebugLog(LOG_ERROR, "Failed to create meta file: " << this->GetMetaPath(), true);
			}
		}

	private:
		std::string path;
		bool remote = false;
	};

	class AssetDataBase {
	public:
		static std::vector<SQLBlobData> GetBinary(std::string directory, std::string assetName);
		static std::vector<std::string> GetText(std::string directory, std::string assetName);
		static void SetText(std::string directory, std::string assetName, std::string text);
	};
}

#endif