#ifndef QUICKSCRIPT_NATIVEINTERFACE_H
#define QUICKSCRIPT_NATIVEINTERFACE_H

#include "common.h"
#include "objects.h"
#include "types/ScriptType.h"

#include <vector>

#define TYPEDEF_FUNC(returnType, name, ...) typedef returnType (*name)(__VA_ARGS__)

class NativeCall {
  uint64* const m_args;
  ScriptType** const m_types;
  const uint32 m_argCount;

  uint64 m_returnValue = 0;

  public:
    NativeCall(ScriptType** argType, uint64* args, uint32 argCount);

    void setReturnValue(uint64 value);
    void setF64ReturnValue(float64 value);
    void setF32ReturnValue(float32 value);

    bool getBoolArgument(uint32 idx);

    int8 getI8Argument(uint32 idx);
    uint8 getU8Argument(uint32 idx);
    int16 getI16Argument(uint32 idx);
    uint16 getU16Argument(uint32 idx);
    int32 getI32Argument(uint32 idx);
    uint32 getU32Argument(uint32 idx);
    int64 getI64Argument(uint32 idx);
    uint64 getU64Argument(uint32 idx);
    float32 getF32Argument(uint32 idx);
    float64 getF64Argument(uint32 idx) const;

    QsArray getStringArgument(uint32 idx);

    QsArray getArrayArgument(uint32 idx);

    QsObject getObjectArgument(uint32 idx);

    QsArray getScriptString(uint64 heapAddr);

    QsArray getScriptArray(uint64 heapAddr);

    ScriptType* getArgumentType(uint32 idx);

    uint8* allocHeap(uint64 size);

    void heapFree(uint8* addr);
};

TYPEDEF_FUNC(void, NativeFunction, NativeCall& call);

#define BINDTYPE_INVALID 0
#define BINDTYPE_VARIABLE 1
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
  NativeFunction m_func;

  NativeFunctionBinding(conststring name, NativeFunction func);

  public:
    static NativeFunctionBinding* create(conststring name, NativeFunction func);

    static void destroy(NativeFunctionBinding* bind);

    bindingtype btype() const override;

    NativeFunction getFunction() const;
};

class BindingsObject {
  std::vector<NativeBinding*> m_bindings;

  public:
    BindingsObject();

    static BindingsObject* create();

    static void create(BindingsObject* obj);

    const std::vector<NativeBinding*>& getBindings() const;

    void addFunctionBinding(conststring name, NativeFunction func);
};

#endif //QUICKSCRIPT_NATIVEINTERFACE_H