#ifndef H_ASSET_MANAGER
#define H_ASSET_MANAGER

#include <PrettyEngine/data.hpp>

#include <vector>

namespace PrettyEngine {
	class AssetsManager {
	public:
		static std::vector<SQLBlobData> GetBinary(std::string directory, std::string assetName);
		static std::vector<std::string> GetText(std::string directory, std::string assetName);
		static void SetText(std::string directory, std::string assetName, std::string text);
		static void SetBinary(std::string directory, std::string assetName, std::vector<unsigned char> binary);
	};
}

#endif