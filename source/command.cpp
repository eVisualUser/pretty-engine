#include <PrettyEngine/command.hpp>

namespace PrettyEngine {
	std::vector<Command> commands;

	void CommandSystem::Execute(std::string command, bool systemCommand) {
		if (systemCommand) {
			system(command.c_str());
		} else {
			auto parsedCommand = ParseCSVLine(command, ' ');
			for (auto &command : commands) {
				if (command.Target(parsedCommand)) {
					command.Execute(parsedCommand);
				}
			}
		}
	}

	void CommandSystem::AddCommand(Command& command) { commands.push_back(command); }

	void CommandSystem::RemoveCommand(Command& commandToRemove) { 
		int commandIndex = 0;
		for (auto &command : commands) {
			if (command.commandName == commandToRemove.commandName) {
				commands.erase(commands.begin() + commandIndex);
				return;
			}
			commandIndex++;
		}
	}
}
