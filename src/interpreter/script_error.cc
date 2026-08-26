#include "script_error.h"

ScriptError::ScriptError(const std::string& message, const std::string& callStack)
  : m_message(message),
    m_callStack(callStack)
{

}

ScriptError::ScriptError(const std::string& message) : m_message(message) {

}

const std::string& ScriptError::getMessage() const {
  return m_message;
}

const std::string& ScriptError::getCallStack() const {
  return m_callStack;
}

std::string formatScriptError(const ScriptError& error) {
  std::string res = "";
  res.append(error.getMessage());
  res.append("\n  ");
  res.append(error.getCallStack());
  return res;
}
