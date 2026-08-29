#ifndef QUICKSCRIPT_NATIVEINTERFACE_H
#define QUICKSCRIPT_NATIVEINTERFACE_H

#include "common.h"
#include "objects.h"
#include "types/ScriptType.h"

#include <vector>

#include "types/FunctionSignature.h"

#define TYPEDEF_FUNC(returnType, name, ...) typedef returnType (*name)(__VA_ARGS__)

class NativeCall {
  uint64* const m_args;
  ScriptType** const m_types;
  const uint32 m_argCount;

  bool m_failedCall = false;
  uint64 m_returnValue = 0;
  std::string m_error;

  public:
    NativeCall(ScriptType** argType, uint64* args, uint32 argCount);

    uint64 getReturnValue() const;

    bool isFailedCall() const;

    const std::string& getErrorMessage() const;

    void setReturn(uint64 value);
    void setF64Return(float64 value);
    void setF32Return(float32 value);

    void setError(const std::string& errorMessage);

    bool getBoolArgument(uint32 idx) const;

    int8 getI8Argument(uint32 idx) const;
    uint8 getU8Argument(uint32 idx) const;
    int16 getI16Argument(uint32 idx) const;
    uint16 getU16Argument(uint32 idx) const;
    int32 getI32Argument(uint32 idx) const;
    uint32 getU32Argument(uint32 idx) const;
    int64 getI64Argument(uint32 idx) const;
    uint64 getU64Argument(uint32 idx) const;
    float32 getF32Argument(uint32 idx) const;
    float64 getF64Argument(uint32 idx) const;

    QsArray getArrayArgument(uint32 idx) const;

    QsObject getObjectArgument(uint32 idx) const;

    QsArray getScriptArray(uint64 heapAddr) const;

    ScriptType* getArgumentType(uint32 idx) const;
};

TYPEDEF_FUNC(void, NativeFunction, NativeCall& call);

#define BINDTYPE_INVALID 0
#define BINDTYPE_CONSTANT 1
#define BINDTYPE_FUNCTION 2
typedef uint8 bindingtype;

class NativeBinding {
  const conststring m_name;

  public:
    explicit NativeBinding(conststring name);

    virtual ~NativeBinding() = default;
    virtual bindingtype btype() const = 0;

    conststring getName() const;
};

class NativeFunctionBinding: public NativeBinding {
  const NativeFunction m_func;
  const FunctionSignature* const m_signature;

  NativeFunctionBinding(conststring name, NativeFunction func, const FunctionSignature* signature);

  public:
    static NativeFunctionBinding* create(conststring name, NativeFunction func, const FunctionSignature* signature);

    static void destroy(NativeFunctionBinding* bind);

    bindingtype btype() const override;

    NativeFunction getFunction() const;

    const FunctionSignature* getSignature() const;
};

class BindingsObject {
  std::vector<NativeBinding*> m_bindings;

  BindingsObject();

  public:
    static BindingsObject* create();

    static void destroy(BindingsObject* obj);

    const std::vector<NativeBinding*>& getBindings() const;

    NativeFunctionBinding* addFunctionBinding(conststring name, const FunctionSignature* signature, NativeFunction func);
};

#endif //QUICKSCRIPT_NATIVEINTERFACE_H