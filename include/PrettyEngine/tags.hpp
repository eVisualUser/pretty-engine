#pragma once

#include <string>
#include <vector>

namespace PrettyEngine {
	class Tagged {
	public:
		static std::string* CreateSafeTag(std::string tag) {
			Tagged::longTermTags.push_back(tag);
			return &Tagged::longTermTags.back();
		}

		bool HaveTag(std::string* otherTag) {
			for (auto & tag: this->tags) {
				if (tag == otherTag) {
					return true;
				}
			}
			return false;
		}

		bool HaveTag(std::string otherTag) {
			return this->HaveTag(&otherTag);
		}

		void AddTag(std::string* tag) {
			this->tags.push_back(tag);
		}

		void AddTag(std::string tag) {
			auto safeTag = Tagged::CreateSafeTag(tag);
			this->tags.push_back(safeTag);	
		}

		void RemoveTag(std::string* tag) {
			for (int i = 0; i < this->tags.size(); i++) {
				if (*this->tags[i] == *tag) {
					this->tags.erase(this->tags.begin() + i);
				}
			}
		}
		
	public:
		std::vector<std::string*> tags;

	public:
		static std::vector<std::string> longTermTags;
	};
}
