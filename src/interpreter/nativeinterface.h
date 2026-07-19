#ifndef QUICKSCRIPT_NATIVEINTERFACE_H
#define QUICKSCRIPT_NATIVEINTERFACE_H

#include "../common.h"
#include "../types.h"

struct QsString {
  uint32 length = 0;
  int8* charData = nullptr;
};

struct QsArray {
  uint32 capacity = 0;
  uint8* data = nullptr;
};

struct QsObject {
  uint8* data = nullptr;

  int8 getI8Property(uint64 offset);
  uint8 getU8Property(uint64 offset);
  int16 getI16Property(uint64 offset);
  uint16 getU16Property(uint64 offset);
  int32 getI32Property(uint64 offset);
  uint32 getU32Property(uint64 offset);
  int64 getI64Property(uint64 offset);
  uint64 getU64Property(uint64 offset);
  float32 getF32Property(uint64 offset);
  float64 getF64Property(uint64 offset);
};

class NativeCall {
  uint64* m_args = nullptr;
  uint32 m_argCount = 0;

  uint64 m_returnValue = 0;

  public:
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

    QsString getStrArgument(uint32 idx);

    QsArray getArrayArgument(uint32 idx);

    QsObject getObjectArgument(uint32 idx);

    QsString getScriptString(uint64 heapAddr);

    uint8* allocHeap(uint64 size);

    void heapFree(uint8* addr);
};

typedef void (*NativeFunction)(NativeCall* call);

struct NativeBinding {
  std::string name;
  void* data = nullptr;
  ScriptType* type = nullptr;
};

class Bindings {
  std::vector<NativeBinding> m_bindings;

  public:
    std::vector<NativeBinding>* getBindings();

    void putConstantF64(conststring name, float64 x);
    void putConstantF32(conststring name, float32 x);

    void putFunction(conststring name, FunctionSignature* signature, NativeFunction func);
};

#endif //QUICKSCRIPT_NATIVEINTERFACE_H