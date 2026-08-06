#ifndef QUICKSCRIPT_FUNCTIONSIGNATURE_H
#define QUICKSCRIPT_FUNCTIONSIGNATURE_H

#include <string>

#include "ScriptType.h"

#define SIGN_DOES_NOT_MATCH (-1)

class FunctionSignature: public ScriptType {
  ScriptType* m_returnType;

  const bool m_varargs;
  ScriptType** const m_paramTypes;
  const uint32 m_paramCount;

  std::string m_name;

  void composeName();

  public:
    FunctionSignature(ScriptType* returnType, bool varargs, uint32 pCount, ScriptType** pTypes);

    static FunctionSignature* create(ScriptType* retType, bool varargs, uint32 pCount, ScriptType** pTypes);

    static void free(FunctionSignature* type);

    static int32 callSignatureMatches(FunctionSignature* callSign, FunctionSignature* funcSign);

    conststring getTypeName() const override;

    bool isVariadic() const;

    ScriptType* getReturnType() const;

    void setReturnType(ScriptType* type);

    ScriptType* getArgumentType(uint32 idx) const;

    void setArgumentType(uint32 idx, ScriptType* type);

    uint32 getArgumentsLength() const;
};


#endif //QUICKSCRIPT_FUNCTIONSIGNATURE_H
