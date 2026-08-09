#include "SemanticContext.h"


std::vector<Symbol*>& DependencyGraph::getDependencies(Symbol* symb) {
  return m_graph[symb];
}

bool DependencyGraph::hasDependencies(Symbol* sym) const {
  return m_graph.contains(sym);
}

bool DependencyGraph::addDependency(Symbol* sym, Symbol* dependency) {
  if (!m_graph.contains(sym)) {
    std::vector<Symbol*> dependsList;
    dependsList.push_back(dependency);
    m_graph[sym] = dependsList;
    return true;
  }

  std::vector<Symbol*>& dependsList = m_graph[sym];
  for (const Symbol* existingDepend : dependsList) {
    if (existingDepend != dependency) {
      continue;
    }
    return false;
  }

  dependsList.push_back(dependency);
  return true;
}


SemanticContext::SemanticContext(
  TypeTable& types,
  StringTable& strings,
  CompilerErrors& errors,
  Bindings& bindings,
  NoFreeAllocator& allocator
)
  : m_types(types),
    m_strings(strings),
    m_errors(errors),
    m_bindings(bindings),
    m_allocator(allocator)
{

}

SemanticFile* SemanticContext::makeSemanticFile(ScriptFileStatement* sfs) {
  SemanticFile* file = m_allocator.make<SemanticFile>(m_globalScope, sfs);
  for (LocalFunction* lf : m_localFunctions) {
    file->getLocalFunctions().push_back(lf);
  }
  return file;
}

void SemanticContext::pushLocalFunction(LocalFunction* func) {
  m_localFunctions.push_back(func);
}

std::vector<LocalFunction*>& SemanticContext::getLocalFunctions() {
  return m_localFunctions;
}

std::vector<LocalFunction*>& SemanticContext::getMainFuncCandidates() {
  return m_mainCandidates;
}

void SemanticContext::pushWrongScopeTypeReported(bool reported) {
  m_wrongScopeReports.push_back(reported);
}

void SemanticContext::popWrongScopeReported() {
  if (m_wrongScopeReports.empty()) {
    return;
  }
  m_wrongScopeReports.pop_back();
}

bool SemanticContext::wasWrongScopeReported() const {
  if (m_wrongScopeReports.empty()) {
    return false;
  }
  return m_wrongScopeReports.back();
}

void SemanticContext::pushExpectedType(ScriptType* type) {
  m_expectedTypes.push_back(type);
}

ScriptType* SemanticContext::getExpectedType() const {
  if (m_expectedTypes.empty()) {
    return nullptr;
  }
  return m_expectedTypes.back();
}

void SemanticContext::popExpectedType() {
  m_expectedTypes.pop_back();
}

void SemanticContext::pushStatement(Statement* stat) {
  m_statementStack.push_back(stat);
}

Statement* SemanticContext::getCurrentStatement() const {
  if (m_statementStack.empty()) {
    return nullptr;
  }
  return m_statementStack.back();
}

void SemanticContext::popStatement() {
  m_statementStack.pop_back();
}

void SemanticContext::popScope() {
  if (!m_currentScope) {
    return;
  }
  m_currentScope = m_currentScope->getParent();
}

Scope* SemanticContext::pushScope(scopetype stype, stringid label) {
  Scope* scope = m_allocator.make<Scope>(stype, m_currentScope);

  if (label != EMPTY_STRING) {
    scope->setLoopLabel(label);
  }

  m_currentScope = scope;
  return scope;
}

Scope* SemanticContext::getScope(const uint32 off) const {
  if (!off) {
    return m_currentScope;
  }

  uint32 i = off;
  Scope* scope = m_currentScope;

  while (i != 0) {
    scope = scope->getParent();
  }

  return scope;
}

Scope* SemanticContext::getGlobalScope() const {
  return m_globalScope;
}

void SemanticContext::setGlobalScope(Scope* scope) {
  m_globalScope = scope;
}

TypeTable& SemanticContext::getTypes() const {
  return m_types;
}

StringTable& SemanticContext::getStrings() const {
  return m_strings;
}

CompilerErrors& SemanticContext::getErrors() const {
  return m_errors;
}

Bindings& SemanticContext::getBindings() const {
  return m_bindings;
}

NoFreeAllocator& SemanticContext::getAllocator() {
  return m_allocator;
}

std::unordered_map<Node*, Symbol*>& SemanticContext::getSymbolCache() {
  return m_symbolCache;
}

DependencyGraph& SemanticContext::getDependencyGraph() {
  return m_dependencyGraph;
}
