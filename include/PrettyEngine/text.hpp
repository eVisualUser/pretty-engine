#pragma once

#include <PrettyEngine/utils.hpp>
#include <PrettyEngine/texture.hpp>
#include <PrettyEngine/transform.hpp>

#include <glm/vec3.hpp>

#include <unordered_map>
#include <string>
#include <sstream>

namespace PrettyEngine {
	struct CharacterInfo: public OString {
	public:
		std::string ToString() override {
			std::stringstream ss;

			ss << "Bearing: " << this->bearing << std::endl;
			ss << "Advance: " << this->advance << std::endl;
			ss << "Ascent: " << this->ascent << std::endl;
			ss << "Descent: " << this->descent << std::endl;
			ss << "LineGap: " << this->lineGap << std::endl;
			ss << "GlyphWidth: " << this->glyphWidth << std::endl;
			ss << "GlyphHeight: " << this->glyphHeight << std::endl;
			ss << "BearingY: " << this->bearingY << std::endl;

			return ss.str();
		}

	public:
		float bearing;
		float advance;
		float ascent;
		float descent;
		float lineGap;
		float glyphWidth;
		float glyphHeight;
		float bearingY;
	};

	class Font {
	public:
		~Font() {
			this->Clear();
		}

		void Clear() {
			for (auto & texture: this->textures) {
				if (texture.second != nullptr) {
					texture.second->userCount--;
				}
			}
		}

		void AddTexture(char c, Texture* texture) {
			texture->userCount++;
			this->textures.insert(std::make_pair(c, texture));
		}
		
	public:
		std::unordered_map<char, Texture*> textures;
		std::unordered_map<char, CharacterInfo> charactersInfo;
	};

	class Character: public Transform {
	public:
		char character = 'C';
		Font* font;
		glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f);
		glm::vec3 highlightColor = glm::vec3(1.0f, 1.0f, 1.0f);
		glm::vec3 offset = glm::vec3(0.0f, 0.0f, 0.0f);
		float highlightOpacity = 0.0f;
		float opacity = 1.0f;
		float size = 1.0f;
	};

	class Text {
	public:
		void AddTextUsingTemplate(Character characterTemplate, std::string text) {
			for (auto & c: text) {
				auto character = characterTemplate;
				character.character = c;
				this->content.push_back(character);
			}
		}

	public:
		std::vector<Character> content;

		float textOutLineWidth = 1.0f;
	};
}
