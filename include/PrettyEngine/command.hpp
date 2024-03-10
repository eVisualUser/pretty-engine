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
		static void Execute(std::string command, bool systemCommand = false);

		static void AddCommand(Command* command);

		static void RemoveCommand(Command* commandToRemove);
	};
}

#endif