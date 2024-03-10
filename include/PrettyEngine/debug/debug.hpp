#ifndef H_DEBUG
#define H_DEBUG

#include <boxer/boxer.h>

#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <ctime>

namespace PrettyEngine {
	struct Log {
	public:
		std::string type;
		std::string log;
	};

	static std::vector<Log> logs;
	static bool printDebugMessage = true;
}

static std::string GetFileName(const char* name) {
    std::string filePath(name); 
    size_t lastSlash = filePath.find_last_of("/\\"); 
    std::string fileName = (lastSlash != std::string::npos) ? filePath.substr(lastSlash + 1) : filePath; 
    return fileName;
}

const std::string LOG_ERROR = "ERROR"; 
const std::string LOG_DEBUG = "DEBUG"; 
const std::string LOG_INFO = "INFO"; 
const std::string LOG_WARNING = "WARNING";

static void ShowMessageBox(std::string type, std::string message) {
	auto style = boxer::Style::Info;
	if (type == LOG_ERROR) {
		style = boxer::Style::Error;
	} else if (type == LOG_WARNING) {
		style = boxer::Style::Warning;
	}

	boxer::show(message.c_str(), type.c_str(), style, boxer::Buttons::OK);
}

static bool RepeatedLog(std::string newLog) {
	if (!PrettyEngine::logs.empty()) {
		return newLog == PrettyEngine::logs.back().log;
	}
	return false;
}

static void AddLog(std::string newLog, std::string type) {
	PrettyEngine::Log result;
	result.type = type;
	result.log = newLog;

	PrettyEngine::logs.push_back(result);
}

static std::string GetTimeAsString() {
	std::string result;
	std::time_t now = std::time(0);

	struct tm timeInfo;
 	#if __LINUX__
 		localtime_r(&now, &timeInfo);
 	#else
		localtime_s(&timeInfo, &now);
	#endif

	char buffer[20];

	std::strftime(buffer, 20, "%Y-%m-%d %H:%M:%S", &timeInfo);

	result += buffer;

	return result;
}

#define WaitCout() std::cout.flush();

static void PrintLog(std::string type, std::string time, std::string fileName, const char* function, std::string message, int line) {
#if !ENGINE_EDITOR
	if (type != LOG_DEBUG)
#endif
 {
  std::cout << '[' << type << " Time: " << time << ' ' << fileName << " Line: " << line << " Function: " << function << ']' << ' ' << message << std::endl;
  // Flush the Cout to avoid wrong debug messages
  WaitCout();
 }
}

#define DebugLog(type, msg, msgBox) \
	if (true) { \
	std::stringstream text; \
	text << "Time: " << GetTimeAsString() << std::endl << GetFileName(__FILE__) << std::endl << "Line: " << __LINE__ << std::endl << "Function: " << __FUNCTION__ << std::endl; \
	text << msg << std::endl; \
	if (!RepeatedLog(text.str())) { \
	AddLog(text.str(), type); \
    if (msgBox) { \
    	ShowMessageBox(type, text.str()); \
    }  \
	if (true) { \
	std::stringstream message;  \
	message << msg; \
	PrintLog(type, GetTimeAsString(), GetFileName(__FILE__), __FUNCTION__, message.str(), __LINE__); \
	}}}

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

#endif
