#ifndef HPP_ASSET_MANAGER
#define HPP_ASSET_MANAGER

#include "boxer/boxer.h"
#include <PrettyEngine/version.hpp>
#include <PrettyEngine/data.hpp>
#include <PrettyEngine/tags.hpp>
#include <PrettyEngine/serial.hpp>
#include <PrettyEngine/gc.hpp>
#include <PrettyEngine/utils.hpp>

#include <vector>
#include <string>
#include <future>

namespace PrettyEngine {
	
	class Asset: public virtual Tagged, public virtual SerialObject, public virtual GCObject {
	public:
		Asset() = default;

		Asset(std::string publicRelativeFilePath) { this->Initialize(publicRelativeFilePath); }

		void Initialize(std::string& publicRelativeFilePath);

		std::string GetFilePath() {
			return this->GetObjectSerializedUnique();
		}

		void SetUsed(bool state) {
			this->GetSerializedField("used")->value = state ? "true" : "false";
		}

		bool Exist() { return FileExist(this->GetFilePath()); }

		std::vector<unsigned char> Read();

		std::future<std::vector<unsigned char>> ReadAsync() {
			return std::async([this]{
				return this->Read();
			});
		}

		std::string ReadToString() { return ReadFileToString(this->GetFilePath()); }

		std::future<std::string> ReadToStringAsync() {
			return std::async([this]{
				return this->ReadToString();
			});
		}

		~Asset() {
			if (this->Exist() && !WriteFileString(this->GetMetaPath(), this->Serialize(SerializationFormat::Toml))) {
				DebugLog(LOG_ERROR, "Failed to write meta file: " << this->GetMetaPath(), true);
				if (boxer::Selection::Yes == boxer::show("Retry writing meta file, retry ?", "Retry ?", boxer::Style::Question, boxer::Buttons::YesNo)) {
					this->~Asset();
				}
			}
		}

		std::string GetMetaPath() { return this->GetFilePath() + ".meta"; }

		bool HaveMeta() { return FileExist(this->GetMetaPath()); }

		void CreateMeta() { 
			if (!this->HaveMeta() && !CreateFile(this->GetMetaPath())) {
				DebugLog(LOG_ERROR, "Failed to create meta file: " << this->GetMetaPath(), true);
				if (boxer::Selection::Yes == boxer::show("Retry create meta file ?", "Retry ?", boxer::Style::Question, boxer::Buttons::YesNo)) {
					return CreateMeta();
				}
			}
		}

	private:
		std::string path;
		Version version;
	};

	class AssetDataBase {
	public:
		static std::vector<SQLBlobData> GetBinary(std::string directory, std::string assetName);
		static std::vector<std::string> GetText(std::string directory, std::string assetName);
		static void SetText(std::string directory, std::string assetName, std::string text);
	};
}

#endif