#ifndef QUICKSCRIPT_SCRIPT_ERROR_H
#define QUICKSCRIPT_SCRIPT_ERROR_H

#include <string>

class ScriptError {
  const std::string m_message;
  const std::string m_callStack;

  public:
    ScriptError(const std::string& message, const std::string& callStack);

    explicit ScriptError(const std::string& message);

    const std::string& getMessage() const;
    const std::string& getCallStack() const;
};

std::string formatScriptError(const ScriptError& error);

#endif //QUICKSCRIPT_SCRIPT_ERROR_H
