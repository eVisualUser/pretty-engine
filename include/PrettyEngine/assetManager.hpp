#ifndef H_ASSET_MANAGER
#define H_ASSET_MANAGER

#include <PrettyEngine/data.hpp>

#include <vector>
#include <string>

namespace PrettyEngine {
	class AssetDataBase {
	public:
		static std::vector<SQLBlobData> GetBinary(std::string directory, std::string assetName);
		static std::vector<std::string> GetText(std::string directory, std::string assetName);
		static void SetText(std::string directory, std::string assetName, std::string text);
	};
}

#endif