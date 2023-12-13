#ifndef HPP_ASSET_MANAGER
#define HPP_ASSET_MANAGER

#include <PrettyEngine/data.hpp>
#include <PrettyEngine/tags.hpp>
#include <PrettyEngine/serial.hpp>
#include <PrettyEngine/utils.hpp>
#include <PrettyEngine/localization.hpp>

#include <vector>
#include <string>
#include <unordered_map>
#include <fstream>

namespace PrettyEngine { 
	class Asset: public virtual Tagged, public virtual SerialObject {
	public:
		Asset(std::string newPath) { 
			this->remote = false;
			this->path = newPath;

			this->CreateMeta();

			// SerializedField serialTags("str[]", "tags", MakeStringFromStringVector(this->longTermTags));
			// this->AddSerializedField(serialTags);

			this->Deserialize(ReadFileToString(this->GetMetaPath()), SerializationFormat::Toml);
		}
				
		~Asset() {
			if (!WriteFileString(this->GetMetaPath(), this->Serialize(SerializationFormat::Toml))) {
				DebugLog(LOG_ERROR, "Failed to write meta file: " << this->GetMetaPath(), true);
			}
		}

		std::string GetMetaPath() { return GetEnginePublicPath(this->path + ".meta", true); }

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