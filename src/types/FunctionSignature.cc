#include "FunctionSignature.h"

FunctionSignature::FunctionSignature(
  ScriptType* returnType,
  bool varargs,
  uint32 pCount, ScriptType** pTypes
)
  : ScriptType(TK_FUNC, POINTER_SIZE),
    m_returnType(returnType),
    m_varargs(varargs),
    m_paramTypes(pTypes), m_paramCount(pCount)
{
  composeName();
}

void FunctionSignature::composeName() {
  m_name.append("(");
  if (m_paramCount > 0) {
    for (uint32 i = 0; i < m_paramCount; i++) {
      ScriptType* paramType = m_paramTypes[i];
      if (i != 0) {
        m_name.append(",");
      }
      m_name.append(paramType->getTypeName());
    }
  }

  m_name.append(")");

  if (m_returnType && m_returnType->kind() != TK_VOID) {
    m_name.append("=>");
    m_name.append(m_returnType->getTypeName());
  }
}

FunctionSignature* FunctionSignature::create(
  ScriptType* retType,
  const bool varargs,
  const uint32 pCount,
  ScriptType** pTypes
) {
  const uint64 arrMemSize = pCount * sizeof(ScriptType*);
  constexpr uint64 signatureMemSize = sizeof(FunctionSignature);
  const uint64 totalSpace = arrMemSize + signatureMemSize;

  FunctionSignature* sign = static_cast<FunctionSignature*>(malloc(totalSpace));
  ScriptType** arrStart = reinterpret_cast<ScriptType**>(sign + 1);

  memcpy(arrStart, pTypes, arrMemSize);

  return new (sign) FunctionSignature(retType, varargs, pCount, arrStart);
}

conststring FunctionSignature::getTypeName() const {
  return m_name.c_str();
}

bool FunctionSignature::isVariadic() const {
  return m_varargs;
}

ScriptType* FunctionSignature::getReturnType() const {
  return m_returnType;
}

ScriptType* FunctionSignature::getArgumentType(const uint32 idx) const {
  if (idx >= m_paramCount) {
    return nullptr;
  }
  return m_paramTypes[idx];
}

uint32 FunctionSignature::getArgumentsLength() const {
  return m_paramCount;
}
