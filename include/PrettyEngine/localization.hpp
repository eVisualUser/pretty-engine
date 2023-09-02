#pragma once

#include "PrettyEngine/debug.hpp"
#include <PrettyEngine/utils.hpp>

#include <vector>
#include <string>
#include <iostream>

namespace PrettyEngine {
	static std::vector<std::string> ParseCSVLine(std::string line, char separator = ';') {
		std::vector<std::string> out;

		std::string buffer;
		for (auto & c: line) {
			if (c != separator) {
				buffer.push_back(c);
			} else {
				out.push_back(buffer);
				buffer.clear();
			}
		}
		if (!buffer.empty()) {
			out.push_back(buffer);
		}

		return out;
	}

	class Localization {
	public:
		void LoadFile(std::string path) {
			auto fileContent = FileToString(path);
			this->LoadString(fileContent);
		}

		void LoadString(std::string content) {
			std::string buffer;
			for (auto & c: content) {
				if (c != '\n') {
					buffer.push_back(c);
				} else {
					this->content.push_back(ParseCSVLine(buffer));
					buffer.clear();
				}
			}
		}

		unsigned int GetLineCount() {
			return this->content.size();
		}

		std::vector<std::string>* GetLine(unsigned int i) {
			if (this->content.size() < i) {
				i = this->GetLineCount();
				return this->GetLine(i);
			} else {
				return &this->content[i];
			}
		}

		std::string Get(std::string first, int langIndex = -1) {
			if (langIndex < 0) {
				langIndex = this->lastLangIndex;
			}			

			for (auto & line: this->content) {
				if (line.front() == first) {
					if (langIndex > line.size()) {
						return line[0];
					} else {
						return line[langIndex];
					}
				}
			}
			DebugLog(LOG_ERROR, "Missing localization for: \"" << first << "\"", false);
			return "[Missing Localization]";
		}

		unsigned int GetLangIndex(std::string lang) {
			auto langLine = this->content[0];
			unsigned int column = 0;
			bool languageFound = false;

			for (auto & strLang: langLine) {
				if (lang == strLang) {
					languageFound = true;
					break;
				}
				column++;
			}

			if (!languageFound) {
				DebugLog(LOG_ERROR, "Language not found: " << lang, false);
				return 0;
			}

			this->lastLangIndex = column;
 			return column;
		}

		std::vector<std::string>* GetAllLanguages() {
			return &this->content[0];
		}

	private:
		int lastLangIndex = 0;
		std::vector<std::vector<std::string>> content;
	};
};
