// Generate files to help debug

#ifndef HPP_DEBUG_DUST
#define HPP_DEBUG_DUST

#include <PrettyEngine/debug/debug.hpp>
#include <PrettyEngine/utils.hpp>

namespace PrettyEngine {
  class DebugDust {
  public:
    // Generate a file that contain the logs
    static void GenerateLogFile(std::string path) {
     std::string buffer;
     
     for(auto & log: logs) {
      buffer += log.type + "\n" + log.log + "\n\n";
     }
     
     PrettyEngine::WriteFileString(path, buffer);
    }
  };
}

#endif