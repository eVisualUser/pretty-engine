#include <PrettyEngine/utils.hpp>
#include <PrettyEngine/assetManager.hpp>

#include <fstream>
#include <sstream>

namespace PrettyEngine {
	void Asset::Initialize(std::string& publicRelativeFilePath) {
		this->path = publicRelativeFilePath;

		this->SetObjectSerializedName("Asset");
		this->SetSerializedUnique(publicRelativeFilePath);

		if (this->Exist()) {
			this->CreateMeta();

			this->AddSerializedField(SERIAL_TOKEN(bool), "exist", this->Exist() ? "true" : "false");
			this->AddSerializedField(SERIAL_TOKEN(std::string), "metaCreationDate", GetTimeAsString());
			this->AddSerializedField(SERIAL_TOKEN(std::string), "local", "any");
			this->AddSerializedField(SERIAL_TOKEN(bool), "used", "false");
			this->AddSerializedField(SERIAL_TOKEN(std::string), "version", this->version.ToString());

			if (this->HaveMeta()) {
				this->Deserialize(ReadFileToString(this->GetMetaPath()), SerializationFormat::Toml);
			}

			this->SetObjectSerializedName("Asset");
			this->SetSerializedUnique(publicRelativeFilePath);

			this->version.FromString(this->GetSerializedField("version")->value);
		} else {
			DebugLog(LOG_ERROR, "Failed to open asset: " << publicRelativeFilePath, true);
		}

		this->OptimizeSerialization();
	}

	std::vector<unsigned char> Asset::Read() {
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

	static DataBase assetDataBase = DataBase(GetEnginePublicPath("assets.db", true));
	static std::string tableName = "any";

	std::vector<SQLBlobData> AssetDataBase::GetBinary(std::string directory, std::string assetName) {
		std::stringstream command;

		command << "SELECT CASE ";
		command << "WHEN name = '" << assetName;
		command << "' AND path = '" << directory << "' THEN bin ";
    	command << "ELSE NULL END AS result ";
    	command << "FROM " << tableName;

		return assetDataBase.QuerySQLBlob(command.str());
	}

	std::vector<std::string> AssetDataBase::GetText(std::string directory, std::string assetName) {
		std::stringstream command;

		command << "SELECT CASE ";
		command << "WHEN name = '" << assetName;
		command << "' AND path = '" << directory << "' THEN text ";
    	command << "ELSE NULL END AS result ";
    	command << "FROM " << tableName;

		return assetDataBase.QuerySQLText(command.str());
	}

	void AssetDataBase::SetText(std::string directory, std::string assetName, std::string text) {
		auto exist = false;
		for(auto & name: assetDataBase.QuerySQLText("SELECT name FROM " + tableName)) {
			if (name == assetName) {
				exist = true;
				break;
			}
		}

		std::stringstream command;

		if (exist) {
			command << " UPDATE " << tableName;
			command << " SET text = \"" << text << '\"';
			command << " WHERE path = \"" << directory << '\"';
			command << " AND name = \"" << assetName << '\"';
		}
		else {
			command << "INSERT INTO " << tableName << " VALUES ";
			command << "(\"" + assetName + "\",";
			command << "\"" + directory + "\",";
			command << "\"" + text + "\", \"\")";
		}

		assetDataBase.ExecuteSQL(command.str());
	}
}
