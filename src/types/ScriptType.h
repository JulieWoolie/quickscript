#ifndef QUICKSCRIPT_SCRIPTTYPE_H
#define QUICKSCRIPT_SCRIPTTYPE_H

#include <string_view>
#include "../common.h"

#define POINTER_SIZE 8

#define TK_UNKNOWN    0
#define TK_PRIMITIVE  1
#define TK_STRING     2
#define TK_STRUCT     3
#define TK_ARRAY      4
#define TK_FUNC       5
#define TK_VOID       6
#define TK_CLOSURE    7
typedef uint8 typekind;

// Type flags
#define TFLAG_INDEXABLE 0x1
#define TFLAG_PROPERTY_HOLDER 0x2
#define TFLAGS_NONE 0

class ScriptType {
  const typekind m_kind;
  const uint64 m_stackSizeBytes;

  public:
    ScriptType(typekind kind, uint64 stackSize);
    virtual ~ScriptType() = default;

    virtual conststring getTypeName() const;

    typekind kind() const;

    uint64 stackSizeBytes() const;

    virtual uint32 typeFlags() const;

    virtual ScriptType* getIndexReturnType() const;

    virtual ScriptType* getPropertyType(std::string_view propertyName) const;
};

#endif //QUICKSCRIPT_SCRIPTTYPE_H
