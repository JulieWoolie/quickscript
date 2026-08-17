#include "semantictree.h"

Symbol::Symbol(const stringid name, ScriptType* scriptType)
  : m_name(name), m_type(scriptType), m_flags(0)
{

}

symflags Symbol::getFlags() const {
  return m_flags;
}

void Symbol::setFlags(symflags f) {
  m_flags = f;
}

void Symbol::addFlags(symflags flags) {
  m_flags |= flags;
}

void Symbol::removeFlags(symflags flags) {
  m_flags &= ~flags;
}

stringid Symbol::getName() const {
  return m_name;
}

ScriptType* Symbol::getScriptType() const {
  return m_type;
}

LocalFuncSymbol::LocalFuncSymbol(LocalFunction* func)
  : Symbol(func->getName(), func->getSignature()), m_function(func), m_calls(0)
{

}

LocalFunction* LocalFuncSymbol::getFunction() const {
  return m_function;
}

uint32 LocalFuncSymbol::getCalls() const {
  return m_calls;
}

void LocalFuncSymbol::onCalled() {
  m_calls++;
}

symboltype LocalFuncSymbol::stype() const {
  return SYM_LocalFunc;
}

LocalVarSymbol::LocalVarSymbol(
  stringid name,
  ScriptType* type,
  const uint64 size,
  const uint64 off,
  Statement* decl
)
  : Symbol(name, type),
    m_size(size),
    m_offset(off),
    m_decl(decl)
{

}

Statement* LocalVarSymbol::getDecl() const {
  return m_decl;
}

uint64 LocalVarSymbol::getStackSize() const {
  return m_size;
}

uint64 LocalVarSymbol::getStackOffset() const {
  return m_offset;
}

void LocalVarSymbol::setStackOffset(uint64 off) {
  m_offset = off;
}

std::vector<Location>& LocalVarSymbol::getReads() {
  return m_reads;
}

std::vector<Location>& LocalVarSymbol::getWrites() {
  return m_writes;
}

symboltype LocalVarSymbol::stype() const {
  return SYM_LocalVar;
}

PropertySymbol::PropertySymbol(ScriptType* holderType, stringid name, ScriptType* type)
  : Symbol(name, type), m_holderType(holderType) {
}

ScriptType* PropertySymbol::getHolderType() const {
  return m_holderType;
}

symboltype PropertySymbol::stype() const {
  return SYM_Property;
}

LocalFunction::LocalFunction(stringid name, FunctionDeclStatement* decl)
  : m_name(name), m_decl(decl), m_signature(decl->signature)
{

}

stringid LocalFunction::getName() const {
  return m_name;
}

FunctionDeclStatement* LocalFunction::getDecl() const {
  return m_decl;
}

Scope* LocalFunction::getScope() const {
  return m_bodyScope;
}

void LocalFunction::setScope(Scope* scope) {
  m_bodyScope = scope;
}

FunctionSignature* LocalFunction::getSignature() const {
  return m_signature;
}

bool LocalFunction::isNested() const {
  return m_nested;
}

void LocalFunction::setNested(bool b) {
  m_nested = b;
}

Scope::Scope(scopetype type, Scope* parent)
  : m_type(type), m_parentScope(parent), m_stackSize(0) {
}

std::vector<Symbol*>& Scope::getSymbols() {
  return m_symbols;
}

void Scope::pushSymbol(Symbol* sym) {
  m_symbols.push_back(sym);
}

void Scope::pushSymbol(const Symbol* before, Symbol* sym) {
  if (!before || m_symbols.empty()) {
    m_symbols.push_back(sym);
    return;
  }
  if (before == m_symbols.front()) {
    m_symbols.insert(m_symbols.cbegin(), sym);
    return;
  }

  for (auto it = m_symbols.begin(); it != m_symbols.end(); ++it) {
    if (*it != before) {
      continue;
    }

    m_symbols.insert(it, sym);
    break;
  }
}

void Scope::removeSymbol(Symbol* sym) {
  for (auto it = m_symbols.begin(); it != m_symbols.end(); ) {
    if (*it != sym) {
      ++it;
      continue;
    }
    m_symbols.erase(it);
    break;
  }
}

Symbol* Scope::findVariable(const stringid name) const {
  for (Symbol* sym : m_symbols) {
    if (sym->getName() != name) {
      continue;
    }

    symboltype stype = sym->stype();
    if (stype != SYM_LocalVar) {
      continue;
    }

    return sym;
  }
  return nullptr;
}

Symbol* Scope::findSymbol(const stringid name, const symboltype st) const {
  for (Symbol* sym : m_symbols) {
    if (sym->getName() != name) {
      continue;
    }
    if (st != SYM_NIL && sym->stype() != st) {
      continue;
    }
    return sym;
  }
  return nullptr;
}

ScriptType* Scope::getExpectedReturnType() const {
  return m_expectedReturnType;
}

void Scope::setExpectedReturnType(ScriptType* type) {
  m_expectedReturnType = type;
}

stringid Scope::getLoopLabel() const {
  return m_loopLabel;
}

void Scope::setLoopLabel(stringid loopLabel) {
  m_loopLabel = loopLabel;
}

uint64 Scope::getStackSize() const {
  return m_stackSize;
}

void Scope::setStackSize(uint64 size) {
  m_stackSize = size;
}

scopetype Scope::getType() const {
  return m_type;
}
Scope* Scope::getParent() const {
  return m_parentScope;
}

uint32 PropertySymbol::getReads() const {
  return m_reads;
}
void PropertySymbol::setReads(uint32 reads) {
  m_reads = reads;
}
uint32 PropertySymbol::getWrites() const {
  return m_writes;
}
void PropertySymbol::setWrites(uint32 writes) {
  m_writes = writes;
}

LocalStructSymbol::LocalStructSymbol(stringid name, ScriptStructType* type, StructDecl* decl)
  : Symbol(name,type), m_decl(decl), m_uses(0)
{

}

StructDecl* LocalStructSymbol::getDecl() const {
  return m_decl;
}

symboltype LocalStructSymbol::stype() const {
  return SYM_LocalStruct;
}

uint32 LocalStructSymbol::getUses() const {
  return m_uses;
}

void LocalStructSymbol::setUses(uint32 uses) {
  m_uses = uses;
}

void LocalStructSymbol::used() {
  m_uses++;
}
