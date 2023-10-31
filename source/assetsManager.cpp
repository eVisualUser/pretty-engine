#include "PrettyEngine/utils.hpp"
#include <PrettyEngine/assetManager.hpp>

#include <sstream>

namespace PrettyEngine {
	static DataBase assetDataBase = DataBase(GetEnginePublicPath("assets.db", true));
	static std::string tableName = "any";

	std::vector<SQLBlobData> AssetsManager::GetBinary(std::string directory, std::string assetName) {
		std::stringstream command;

		command << "SELECT CASE ";
		command << "WHEN name = '" << assetName;
		command << "' AND path = '" << directory << "' THEN bin ";
    	command << "ELSE NULL END AS result ";
    	command << "FROM " << tableName;

		return assetDataBase.QuerySQLBlob(command.str());
	}

	std::vector<std::string> AssetsManager::GetText(std::string directory, std::string assetName) {
		std::stringstream command;

		command << "SELECT CASE ";
		command << "WHEN name = '" << assetName;
		command << "' AND path = '" << directory << "' THEN text ";
    	command << "ELSE NULL END AS result ";
    	command << "FROM " << tableName;

		return assetDataBase.QuerySQLText(command.str());
	}

	void AssetsManager::SetText(std::string directory, std::string assetName, std::string text) {
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
