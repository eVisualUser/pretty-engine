// Command system: add a way to call logic from anywhere

#ifndef HPP_COMMAND
#define HPP_COMMAND

#include <PrettyEngine/localization.hpp>

#include <string>
#include <vector>
#include <functional>

namespace PrettyEngine {
	class Command {
	public:
		bool Target(std::vector<std::string>& command);

		/// Execute the command
		void Execute(std::vector<std::string> &command) { (this->action)(command); }

		std::string commandName = "Any";
		std::function<void(std::vector<std::string> &)> action;
	};

	class CommandSystem {
	public:
		static void Execute(std::string command, bool systemCommand = false) {
			if (systemCommand) {
				system(command.c_str());
			} else {
				auto parsedCommand = ParseCSVLine(command, ' ');
				for (auto &command : CommandSystem::commands) {
					if (command.Target(parsedCommand)) {
						command.Execute(parsedCommand);
					}
				}
			}
		}

		static void AddCommand(Command& command) { CommandSystem::commands.push_back(command); }

		static void RemoveCommand(Command& commandToRemove) { 
			int commandIndex = 0;
			for (auto &command : CommandSystem::commands) {
				if (command.commandName == commandToRemove.commandName) {
					CommandSystem::commands.erase(CommandSystem::commands.begin() + commandIndex);
					return;
				}
				commandIndex++;
			}
		}
	};
}

#endif