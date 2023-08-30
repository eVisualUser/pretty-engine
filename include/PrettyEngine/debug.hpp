#pragma once

#include <boxer/boxer.h>

#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <chrono>

namespace PrettyEngine {
	static std::vector<std::string> logs;
	static bool printDebugMessage = true;
} 

static std::string GetFileName(const char* name) {
    std::string filePath(name); 
    size_t lastSlash = filePath.find_last_of("/\\"); 
    std::string fileName = (lastSlash != std::string::npos) ? filePath.substr(lastSlash + 1) : filePath; 
    return fileName;
}

#define LOG_ERROR "ERROR" 
#define LOG_DEBUG "DEBUG" 
#define LOG_WARNING "WARNING" 

static void ShowMessageBox(std::string type, std::string message) {
	auto style = boxer::Style::Info;
	if (type == LOG_ERROR) {
		style = boxer::Style::Error;
	} else if (type == LOG_WARNING) {
		style = boxer::Style::Warning;
	}

	boxer::show(message.c_str(), type.c_str(), style, boxer::Buttons::OK);
}

static void AddLog(std::string newLog) {
	PrettyEngine::logs.push_back(newLog);
}

static std::string GetTimeAsString() {
	std::string out;

	auto const time = std::chrono::current_zone()->to_local(std::chrono::system_clock::now());
    out = std::format("{:%Y-%m-%d %X}", time);

	return out;
}

#define WaitCout() std::cout.flush();

#define DebugLog(type, msg, msgBox) \
	if (true) { \
	std::stringstream text; \
	text << "Time: " << GetTimeAsString() << std::endl << GetFileName(__FILE__) << std::endl << "Line: " << __LINE__ << std::endl << "Function: " << __FUNCTION__ << std::endl; \
	text << msg << std::endl; \
	AddLog(text.str()); \
    if (msgBox) { \
    	ShowMessageBox(type, text.str()); \
    }  \
	} \
	std::cout << '[' << type << " Time: " << GetTimeAsString() << ' ' << GetFileName(__FILE__) << " Line: " << __LINE__ << " Function: " << __FUNCTION__ << ']' << ' ' << msg << std::endl;

namespace PrettyEngine {
	namespace Debug {
		static size_t number = 0;

		static void PrintDebugNumber() {
			number++;
			std::cout << "[DEBUG] Number: " << number << std::endl;
		}

		static void ResetDebugNumber() {
			number = 0;
		}
		
		static void CheckNumber(size_t otherNumber) {
			if (number == otherNumber)
				std::cout << "[DEBUG] Number: Success" << std::endl;
			else
				std::cout << "[DEBUG] Number: Failed" << std::endl;
		}
	}
}
