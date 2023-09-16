#pragma once

#include "PrettyEngine/debug.hpp"
#include <PrettyEngine/utils.hpp>

#include <cstddef>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <iostream>

namespace PrettyEngine {
	static std::vector<std::string> ParseCSVLine(std::string line, char separator = ';') {
		std::vector<std::string> out;

		std::string buffer;
		char lastChar = ' ';
		for (auto & c: line) {
			if (c == separator && lastChar == '/') {
				buffer.pop_back();
				buffer.push_back(separator);
			} else if (c != separator) {
				buffer.push_back(c);
			} else {
				out.push_back(buffer);
				buffer.clear();
			}
			lastChar = c;
		}
		if (!buffer.empty()) {
			out.push_back(buffer);
		}

		for(auto & line: out) {
			StringReplace(&line, "/;", ";");
		}

		return out;
	}

	class Localization {
	public:
		void LoadFile(std::string path) {
			this->filePath = path;
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

		void AddLang(std::string lang) {
			StringReplace(&lang, ";", "/;");
			this->content[0].push_back(lang);
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
			StringReplace(&first, "/;", ";");

			if (langIndex < 0) {
				langIndex = this->lastLangIndex;
			}			

			bool foundLocalization = false;
			for (auto & line: this->content) {
				if (line.front() == first) {
					if (langIndex >= line.size()) {
						return line.front();
					} else {
						return line[langIndex];
					}
					foundLocalization = true;
				}
			}

			if (!foundLocalization) {
				this->content.push_back(std::vector<std::string>({first}));
			}

			DebugLog(LOG_ERROR, "Missing localization for: \"" << first << "\"", false);
			return first;
		}

		unsigned int GetLangIndex(std::string lang) {
			StringReplace(&lang, "/;", ";");

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

		void Save() {
			std::stringstream out;
			for(auto & line: this->content) {
				for(auto & column: line) {
					StringReplace(&column, ";", "/;");
					out << column << ';';
				}
				out << '\n';
			}

			std::ofstream file(this->filePath);
			if (file.is_open()) {
				file << out.str();
				file.close();
				return;
			}
			DebugLog(LOG_ERROR, "Could not save localization file: " << this->filePath, true);
		}

	private:
		int lastLangIndex = 0;
		std::vector<std::vector<std::string>> content;
		std::string filePath;
	};
};
