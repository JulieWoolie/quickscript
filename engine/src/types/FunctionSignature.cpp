#include "qs/types/FunctionSignature.hpp"

#include <__stdarg_va_arg.h>

#include "qs/parse/keyw_lookup.hpp"
#include "qs/parse/token.hpp"
#include "qs/strings/stringreader.hpp"
#include "qs/strings/unicode_binary_props.hpp"
#include "qs/types/ConstTypes.hpp"
#include "qs/types/ScriptArrayType.hpp"
#include "qs/types/types.hpp"

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

FunctionSignature* FunctionSignature::copy(const FunctionSignature* sign) {
  return create(sign->m_returnType, sign->m_varargs, sign->m_paramCount, sign->m_paramTypes);
}

FunctionSignature* FunctionSignature::make(ScriptType* retType, const uint32 pCount, ...) {
  va_list list;
  va_start(list, pCount);

  ScriptType* paramArray[pCount];
  ScriptType* returnType = retType;

  if (!returnType) {
    returnType = ConstTypes::VOID();
  }

  for (uint32 i = 0; i < pCount; i++) {
    ScriptType* argType = va_arg(list, ScriptType*);
    if (!argType) {
      argType = ConstTypes::VOID();
    }
    paramArray[i] = argType;
  }

  va_end(list);

  return create(returnType, false, pCount, paramArray);
}

FunctionSignature* FunctionSignature::make(ScriptType* retType, bool variadic, uint32 pCount, ...) {
  va_list list;
  va_start(list, pCount);

  ScriptType* paramArray[pCount];
  ScriptType* returnType = retType;

  if (!returnType) {
    returnType = ConstTypes::VOID();
  }

  for (uint32 i = 0; i < pCount; i++) {
    ScriptType* argType = va_arg(list, ScriptType*);
    if (!argType) {
      argType = ConstTypes::VOID();
    }
    paramArray[i] = argType;
  }

  va_end(list);

  if (paramArray[pCount - 1]->kind() != TK_ARRAY) {
    throw std::runtime_error("Last argument must be an array type");
  }

  return create(returnType, variadic, pCount, paramArray);
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

  if (callParamCount < (funcParamCount - funcVariadic)) {
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

#define SIGT_EOF -1
#define SIGT_UNKNOWN 0
#define SIGT_RETURN_TYPE_PREFIX 1
#define SIGT_OPEN_PARENTHESIS 2
#define SIGT_CLOSE_PARENTHESIS 3
#define SIGT_COMMA 4
#define SIGT_ARRAY_BRACKETS 5
#define SIGT_VARIADIC 6
#define SIGT_TYPENAME 7

typedef int8 sigtoken;

static sigtoken nextToken(const StringReader& reader) {
  if (!reader.hasNext()) {
    return SIGT_EOF;
  }

  const utf32char ch = reader.peek();

  switch (ch) {
    case '(':
      return SIGT_OPEN_PARENTHESIS;
    case ')':
      return SIGT_CLOSE_PARENTHESIS;
    case ',':
      return SIGT_COMMA;
    case '-':
    case '=':
      if (reader.peek(1) == '>') {
        return SIGT_RETURN_TYPE_PREFIX;
      }
      return SIGT_UNKNOWN;
    case '[':
      if (reader.peek(1) == ']') {
        return SIGT_ARRAY_BRACKETS;
      }
      return SIGT_UNKNOWN;
    case '.':
      if (reader.peek(1) == '.' && reader.peek(2) == '.') {
        return SIGT_VARIADIC;
      }
      return SIGT_UNKNOWN;
    default:
      if (ucIsXidStart(ch)) {
        return SIGT_TYPENAME;
      }
      return SIGT_UNKNOWN;
  }
}

static void skipToken(const sigtoken tt, StringReader& reader) {
  switch (tt) {
    case SIGT_COMMA:
    case SIGT_OPEN_PARENTHESIS:
    case SIGT_CLOSE_PARENTHESIS:
    case SIGT_UNKNOWN:
      reader.next();
      break;

    case SIGT_ARRAY_BRACKETS:
    case SIGT_RETURN_TYPE_PREFIX:
      reader.next();
      reader.next();
      break;

    case SIGT_VARIADIC:
      reader.next();
      reader.next();
      reader.next();
      break;

    case SIGT_TYPENAME:
      while (ucIsXidContinue(reader.peek())) {
        reader.next();
      }
      break;

    default:
      break;
  }
}

static bool isValidTypeIdentifier(const std::string_view& view) {
  const conststring str = view.data();
  const tokentype keywType = tokenTypeFromString(str, view.length());

  switch (keywType) {
    case TT_KEYW_BOOL:
    case TT_KEYW_UINT8:
    case TT_KEYW_INT8:
    case TT_KEYW_UINT16:
    case TT_KEYW_INT16:
    case TT_KEYW_UINT32:
    case TT_KEYW_INT32:
    case TT_KEYW_UINT64:
    case TT_KEYW_INT64:
    case TT_KEYW_FLOAT32:
    case TT_KEYW_FLOAT64:
    case TT_KEYW_STRING:
    case TT_KEYW_VOID:
      return true;
    default:
      return false;
  }
}

static bool isValidTypeName(StringReader& reader, bool& variadic) {
  reader.skipWhitespace();

  const uint32 start = reader.cursor();
  const sigtoken tt = nextToken(reader);

  if (tt != SIGT_TYPENAME) {
    return false;
  }

  skipToken(tt, reader);

  const uint32 end = reader.cursor();
  const std::string_view sv = reader.substring(start, end);

  if (!isValidTypeIdentifier(sv)) {
    return false;
  }

  reader.skipWhitespace();

  while (nextToken(reader) == SIGT_ARRAY_BRACKETS) {
    skipToken(SIGT_ARRAY_BRACKETS, reader);
    reader.skipWhitespace();
  }

  if (nextToken(reader) == SIGT_VARIADIC) {
    if (variadic) {
      return false;
    }

    skipToken(SIGT_VARIADIC, reader);
    variadic = true;

    return true;
  }

  return true;
}

#define INVALID_SIGNATURE (-1)

static int32 isValidSignature(StringReader& reader, bool& variadic) {
  // Type signature syntax rules:
  //
  // type-signature:
  //      '(' type-name-list ')' '->' type-name
  //      '(' type-name-list ')' '=>' type-name
  //      '(' type-name-list ')'
  //      type-name-list '->' type-name
  //      type-name-list '=>' type-name
  //      type-name-list
  //
  // type-name-list:
  //      type-name
  //      type-name-list type-name
  //      type-name-list ',' type-name
  //
  // type-name:
  //      array-name
  //      array-name '...'
  //
  // array-name:
  //      base-name
  //      array-name '[]'
  //
  // base-name:
  //      REGEXP [a-zA-Z_$0-9]+
  //

  reader.skipWhitespace();
  const sigtoken first = nextToken(reader);
  bool parentheses = false;

  if (first == SIGT_OPEN_PARENTHESIS) {
    parentheses = true;
    skipToken(first, reader);
  }

  int32 argCount = 0;

  while (true) {
    reader.skipWhitespace();
    const sigtoken tt = nextToken(reader);

    if (tt == TT_EOF) {
      return argCount;
    }

    if (tt == SIGT_CLOSE_PARENTHESIS) {
      if (!parentheses) {
        return INVALID_SIGNATURE;
      }

      skipToken(tt, reader);
      break;
    }

    if (tt == SIGT_RETURN_TYPE_PREFIX) {
      if (parentheses) {
        return INVALID_SIGNATURE;
      }
      break;
    }

    if (tt == SIGT_COMMA) {
      skipToken(tt, reader);
      continue;
    }

    if (!isValidTypeName(reader, variadic)) {
      return INVALID_SIGNATURE;
    }

    argCount++;
  }

  reader.skipWhitespace();
  const sigtoken ending = nextToken(reader);

  if (ending == SIGT_RETURN_TYPE_PREFIX) {
    skipToken(SIGT_RETURN_TYPE_PREFIX, reader);

    bool retVariadic = false;
    const bool validReturn = isValidTypeName(reader, retVariadic);

    if (!validReturn || retVariadic) {
      return INVALID_SIGNATURE;
    }

    return argCount;
  }

  if (ending != SIGT_EOF) {
    return INVALID_SIGNATURE;
  }

  return argCount;
}

static ScriptType* parseType(StringReader& reader) {
  const uint32 start = reader.cursor();
  while (ucIsXidContinue(reader.peek())) {
    reader.next();
  }
  const uint32 end = reader.cursor();

  const std::string_view sv = reader.substring(start, end);
  const tokentype ttype = tokenTypeFromString(sv.data(), sv.length());

  ScriptType* type = nullptr;

  switch (ttype) {
    case TT_KEYW_BOOL:
      type = ConstTypes::BOOL();
      break;
    case TT_KEYW_UINT8:
      type = ConstTypes::UINT8();
      break;
    case TT_KEYW_INT8:
      type = ConstTypes::INT8();
      break;
    case TT_KEYW_UINT16:
      type = ConstTypes::UINT16();
      break;
    case TT_KEYW_INT16:
      type = ConstTypes::INT16();
      break;
    case TT_KEYW_UINT32:
      type = ConstTypes::UINT32();
      break;
    case TT_KEYW_INT32:
      type = ConstTypes::INT32();
      break;
    case TT_KEYW_UINT64:
      type = ConstTypes::UINT64();
      break;
    case TT_KEYW_INT64:
      type = ConstTypes::INT64();
      break;
    case TT_KEYW_FLOAT32:
      type = ConstTypes::FLOAT32();
      break;
    case TT_KEYW_FLOAT64:
      type = ConstTypes::FLOAT64();
      break;
    case TT_KEYW_STRING:
      type = ConstTypes::STRING();
      break;
    case TT_KEYW_VOID:
      type = ConstTypes::VOID();
      break;
    default:
      return nullptr;
      break;
  }

  reader.skipWhitespace();

  sigtoken tt = SIGT_UNKNOWN;
  while ((tt = nextToken(reader)) == SIGT_ARRAY_BRACKETS || tt == SIGT_VARIADIC) {
    skipToken(SIGT_ARRAY_BRACKETS, reader);
    reader.skipWhitespace();
    type = new ScriptArrayType(type);
  }

  return type;
}

FunctionSignature* FunctionSignature::parse(const conststring str) {
  StringReader reader = StringReader(str);

  bool variadic = false;
  const int32 argCount = isValidSignature(reader, variadic);

  if (argCount == INVALID_SIGNATURE) {
    return nullptr;
  }

  reader.cursor() = 0;

  ScriptType* argTypes[argCount];
  ScriptType* returnType = nullptr;

  uint32 argI = 0;

  while (true) {
    reader.skipWhitespace();
    const sigtoken tt = nextToken(reader);

    if (tt == SIGT_EOF || tt == SIGT_RETURN_TYPE_PREFIX) {
      break;
    }

    if (tt == SIGT_COMMA || tt == SIGT_CLOSE_PARENTHESIS || tt == SIGT_OPEN_PARENTHESIS) {
      skipToken(tt, reader);
      continue;
    }

    ScriptType* argType = parseType(reader);
    argTypes[argI++] = argType;
  }

  if (nextToken(reader) == SIGT_RETURN_TYPE_PREFIX) {
    skipToken(SIGT_RETURN_TYPE_PREFIX, reader);
    reader.skipWhitespace();
    returnType = parseType(reader);
  } else {
    returnType = ConstTypes::VOID();
  }

  return create(returnType, variadic, argCount, argTypes);
}
