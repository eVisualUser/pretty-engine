#ifndef HPP_TAGS
#define HPP_TAGS

#include <string>
#include <vector>

namespace PrettyEngine {
	/// Allow an object to have multiple tags.
	class Tagged {
	public:
		Tagged() {}

		bool HaveTag(std::string otherTag) { 
			for (auto &tag : this->_tags) {
				if (tag == otherTag) {
					return true;
				}
			}
			
			return false;
		}

		void AddTag(std::string tag) {
			this->_tags.push_back(tag);	
		}

		void RemoveTag(std::string tag) {
			for (int i = 0; i < this->_tags.size(); i++) {
				if (this->_tags[i] == tag) {
					this->_tags.erase(this->_tags.begin() + i);
				}
			}
		}
		
	private:
		std::vector<std::string> _tags;
	};
	}

#endif