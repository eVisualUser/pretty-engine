#ifndef HPP_PRETTY_VERSION
#define HPP_PRETTY_VERSION

#include <PrettyEngine/utils.hpp>

#include <sstream>

namespace PrettyEngine {
	class Version: OString {
	public:
		Version() = default;

		Version(int newMajor, int newMinor, int newPatch) {
			this->major = newMajor;
			this->minor = newMinor;
			this->patch = newPatch;
		}

		int major = 0;
		int minor = 0;
		int patch = 0;

		std::string ToString() override {
			std::stringstream ss;
			ss << this->major << '.' << this->minor << '.' << this->patch;
			return ss.str();
		}

		void FromString(std::string input) {
			std::string version[3];

			int index = 0;
			std::string buffer;
			for(auto & character: input) {
				if (character == '.') {
					version[index] = buffer;
					buffer.clear();
					index++;
				} else {
					buffer += character;
				}
			}
			version[index] = buffer;
			buffer.clear();

			this->major = std::stoi(version[0]);
			this->minor = std::stoi(version[1]);
			this->patch = std::stoi(version[2]);
		}

		Version* operator+(Version* other) {
			this->major += other->major;
			this->minor += other->major;
			this->patch += other->major;

			return this;
		}

		Version* operator-(Version* other) {
			this->major -= other->major;
			this->minor -= other->major;
			this->patch -= other->major;

			return this;
		}

		Version* operator*(Version* other) {
			this->major *= other->major;
			this->minor *= other->major;
			this->patch *= other->major;

			return this;
		}

		Version* operator/(Version* other) {
			this->major /= other->major;
			this->minor /= other->major;
			this->patch /= other->major;

			return this;
		}

		Version* operator%(Version* other) {
			this->major %= other->major;
			this->minor %= other->major;
			this->patch %= other->major;

			return this;
		}
	};
}

#endif
