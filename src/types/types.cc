#include "types.h"

bool isNumberType(ScriptType* type) {
  if (type->kind() != TK_PRIMITIVE) {
    return false;
  }

  PrimitiveScriptType* prim = (PrimitiveScriptType*) type;

  return pkIsNumberType(prim->getPrimitiveType());
}

bool pkIsSignedType(primitivekind kind) {
  switch (kind) {
    case PK_INT8:
    case PK_INT16:
    case PK_INT32:
    case PK_INT64:
      return true;
    default:
      return false;
  }
}

bool pkIsFloat(primitivekind kind) {
  return kind == PK_FLOAT32 || kind == PK_FLOAT64;
}

PrimitiveScriptType* widestNumberType(PrimitiveScriptType* l, PrimitiveScriptType* r) {
  if (l->getPrimitiveType() == r->getPrimitiveType()) {
    return l;
  }

  primitivekind lkind = l->getPrimitiveType();
  primitivekind rkind = r->getPrimitiveType();

  uint32 lsize = l->stackSizeBytes();
  uint32 rsize = r->stackSizeBytes();

  if (lsize > rsize) {
    return l;
  } else if (rsize > lsize) {
    return r;
  }

  bool lsigned = pkIsSignedType(lkind);
  bool rsigned = pkIsSignedType(rkind);

  if (lsigned && !rsigned) {
    return l;
  }
  if (rsigned && !lsigned) {
    return r;
  }

  if (lkind == PK_BOOL) {
    return r;
  }

  return l;
}

ScriptType* getCommonType(ScriptType* t1, ScriptType* t2) {
  if (t1 == t2) {
    return t1;
  }
  if (t1->kind() == TK_PRIMITIVE && t2->kind() == TK_PRIMITIVE) {
    PrimitiveScriptType* p1 = (PrimitiveScriptType*) t1;
    PrimitiveScriptType* p2 = (PrimitiveScriptType*) t2;
    return widestNumberType(p1, p2);
  }
  return nullptr;
}

bool isAssignableTo(ScriptType* holder, ScriptType* value) {
  if (holder == value) {
    return true;
  }

  if (holder->kind() == TK_PRIMITIVE && value->kind() == TK_PRIMITIVE) {
    PrimitiveScriptType* ph = (PrimitiveScriptType*) holder;
    PrimitiveScriptType* pv = (PrimitiveScriptType*) value;

    primitivekind hk = ph->getPrimitiveType();
    primitivekind vk = pv->getPrimitiveType();

    if (pkIsFloat(vk) && !pkIsFloat(hk)) {
      return false;
    }

    return holder->stackSizeBytes() >= value->stackSizeBytes();
  }

  return false;
}

bool pkIsNumberType(primitivekind kind) {
  switch (kind) {
    case PK_INT8:
    case PK_UINT8:
    case PK_INT16:
    case PK_UINT16:
    case PK_INT32:
    case PK_UINT32:
    case PK_INT64:
    case PK_UINT64:
    case PK_FLOAT32:
    case PK_FLOAT64:
      return true;
    default:
      return false;
  }
}

bool isIntegerType(ScriptType* type) {
  if (type->kind() != TK_PRIMITIVE) {
    return false;
  }

  PrimitiveScriptType* prim = (PrimitiveScriptType*) type;

  return pkIsIntegerType(prim->getPrimitiveType());
}

bool pkIsIntegerType(const primitivekind kind) {
  switch (kind) {
    case PK_INT8:
    case PK_UINT8:
    case PK_INT16:
    case PK_UINT16:
    case PK_INT32:
    case PK_UINT32:
    case PK_INT64:
    case PK_UINT64:
      return true;
    default:
      return false;
  }
}

bool isBooleanType(ScriptType* type) {
  if (type->kind() != TK_PRIMITIVE) {
    return false;
  }
  PrimitiveScriptType* prim = static_cast<PrimitiveScriptType*>(type);
  return prim->getPrimitiveType() == PK_BOOL;
}