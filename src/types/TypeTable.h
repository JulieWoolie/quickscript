#ifndef QUICKSCRIPT_TYPETABLE_H
#define QUICKSCRIPT_TYPETABLE_H

#include <string>
#include <unordered_map>

#include "FunctionSignature.h"
#include "ScriptType.h"

#define TI_VOID 0
#define TI_BOOL 1
#define TI_INT8 2
#define TI_UINT8 3
#define TI_INT16 4
#define TI_UINT16 5
#define TI_INT32 6
#define TI_UINT32 7
#define TI_INT64 8
#define TI_UINT64 9
#define TI_FLOAT32 10
#define TI_FLOAT64 11
#define TI_STRING 12
#define TI_CLOSURE 13
#define LAST_RESERVED_TYPE_INDEX TI_CLOSURE
typedef uint64 typeindex;

conststring nativeTypeIndexName(typeindex idx);

class TypeTable {
  ScriptType** m_indexLookup = nullptr;
  uint64 m_indexLookupLen = 0;
  uint64 m_indexLookupCap = 0;

  std::unordered_map<std::string, ScriptType*> m_nameLookup;

  void ensureIndexLookupHasSpace(uint32 desiredSize);

  public:
    TypeTable();
    ~TypeTable();

    ScriptType* lookupByIndex(typeindex index) const;

    ScriptType* lookupByName(const std::string& name) const;

    FunctionSignature* getSignature(ScriptType* returnType, bool variadic, uint32 pCount, ScriptType** paramTypes);

    FunctionSignature* copySignatureIntoTable(const FunctionSignature* sign);

    ScriptType* getArrayType(ScriptType* componentType);

    typeindex emplaceType(ScriptType* type);

    int64 findIndex(const ScriptType* type) const;

    uint64 size() const;
};


#endif //QUICKSCRIPT_TYPETABLE_H
