#ifndef QUICKSCRIPT_NATIVEINTERFACE_H
#define QUICKSCRIPT_NATIVEINTERFACE_H

#include "common.h"
#include "objects.h"
#include "types/ScriptType.h"

class NativeCall {
  uint64* m_args = nullptr;
  ScriptType** m_types = nullptr;
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

    QsArray getStringArgument(uint32 idx);

    QsArray getArrayArgument(uint32 idx);

    QsObject getObjectArgument(uint32 idx);

    QsArray getScriptString(uint64 heapAddr);

    QsArray getScriptArray(uint64 heapAddr);

    ScriptType* getArgumentType(uint32 idx);

    uint8* allocHeap(uint64 size);

    void heapFree(uint8* addr);
};

#endif //QUICKSCRIPT_NATIVEINTERFACE_H