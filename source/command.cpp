#include <PrettyEngine/command.hpp>

namespace PrettyEngine {
	std::vector<Command> commands;

	bool Command::Target(std::vector<std::string>& command) {
		return (command[0] == this->commandName);
	}

	void CommandSystem::Execute(std::string commandStr, bool systemCommand) {
		if (systemCommand) {
			system(commandStr.c_str());
		} else {
			auto parsedCommand = ParseCSVLine(commandStr, ' ');
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
