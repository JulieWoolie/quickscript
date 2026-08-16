#include "FunctionSignature.h"

#include "ScriptArrayType.h"
#include "types.h"

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
  composeName(m_name, m_returnType, m_paramCount, m_paramTypes);
}

void FunctionSignature::composeName(std::string& out, ScriptType* retType, uint32 pCount, ScriptType** pTypes) {
  out.append("(");
  if (pCount > 0) {
    for (uint32 i = 0; i < pCount; i++) {
      ScriptType* paramType = pTypes[i];
      if (i != 0) {
        out.append(",");
      }

      if (paramType) {
        out.append(paramType->getTypeName());
      } else {
        out.append("?");
      }
    }
  }

  out.append(")");

  if (retType) {
    out.append("=>");
    out.append(retType->getTypeName());
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
  ScriptType** arrStart;

  if (pCount != 0) {
    arrStart = reinterpret_cast<ScriptType**>(sign + 1);
    memcpy(arrStart, pTypes, arrMemSize);
  } else {
    arrStart = nullptr;
  }

  return new (sign) FunctionSignature(retType, varargs, pCount, arrStart);
}

void FunctionSignature::free(FunctionSignature* type) {
  std::free((void*) type);
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

int32 FunctionSignature::callSignatureMatches(FunctionSignature* callSign, FunctionSignature* funcSign) {
  const uint32 callParamCount = callSign->m_paramCount;
  const uint32 funcParamCount = funcSign->m_paramCount;
  const bool funcVariadic = funcSign->m_varargs;

  if (callParamCount < funcParamCount) {
    return SIGN_DOES_NOT_MATCH;
  }
  if (!funcVariadic && callParamCount > funcParamCount) {
    return SIGN_DOES_NOT_MATCH;
  }

  int32 score = 0;

  for (uint32 i = 0; i < callParamCount; i++) {
    ScriptType* callingType = callSign->m_paramTypes[i];
    ScriptType* targetType = nullptr;

    if (i >= funcParamCount) {
      targetType = static_cast<ScriptArrayType*>(funcSign->m_paramTypes[funcParamCount-1])->getComponentType();
    } else {
      targetType = funcSign->m_paramTypes[i];
    }

    if (i == (funcParamCount - 1) && funcVariadic && targetType->kind() == TK_ARRAY) {
      if (targetType == callingType) {
        score += 2;
        continue;
      }
      targetType = static_cast<ScriptArrayType*>(targetType)->getComponentType();
    }

    if (callingType == targetType) {
      score += 2;
      continue;
    }

    if (isAssignableTo(targetType, callingType)) {
      score += 1;
      continue;
    }

    return SIGN_DOES_NOT_MATCH;
  }

  return score;
}