#include "LexicalAnalyzer.h"

#include <set>

#define NOT_MAIN(name) if (getScope()->stype == SCOPE_MAIN) { m_errors->error(v->location, "%s not allowed here", name); }

Symbol* AnalyzerScope::pushSymbol(stringid name, symboltype type)  {
  Symbol symbol;
  symbol.name = name;
  symbol.type = type;
  symbols.push_back(symbol);
  return &symbols.at(symbols.size() - 1);
}

Symbol* AnalyzerScope::getSymbol(stringid name, symboltype symType) {
  for (auto & symbol : symbols) {
    Symbol* sym = &symbol;
    if (sym->name != name) {
      continue;
    }
    if (symType != SYM_NIL && sym->type != symType) {
      continue;
    }
    return sym;
  }
  return nullptr;
}

AnalyzerScope* LexicalAnalyzer::pushScope(scopetype type) {
  AnalyzerScope scope;
  scope.stype = type;
  m_scopes.push_back(scope);
  return getScope(0);
}

void LexicalAnalyzer::popScope() {
  AnalyzerScope* scope = getScope();

  uint32 symbols = scope->symbols.size();
  char buf[1024];

  for (uint32 i = 0; i < symbols; i++) {
    Symbol* sym = &scope->symbols.at(i);
    if (sym->uses > 0) {
      continue;
    }
    m_strings->getchars(sym->name, buf, 1024);
    m_errors->warn("Symbol %s is unused", buf);
  }

  m_scopes.pop_back();
}

AnalyzerScope* LexicalAnalyzer::getScope(const uint32 back) {
  return m_scopes.data() + (m_scopes.size() - (back + 1));
}

Symbol* LexicalAnalyzer::getSymbol(stringid name, symboltype stype) {
  uint32 len = m_scopes.size();

  for (int32 i = len - 1; i >= 0; i--) {
    AnalyzerScope* scope = &m_scopes[i];
    Symbol* sym = scope->getSymbol(name, stype);

    if (!sym) {
      continue;
    }

    return sym;
  }

  return nullptr;
}

LexicalAnalyzer::LexicalAnalyzer(
  StringTable* strings,
  CompilerErrors* errors,
  Bindings* bindings
) {
  m_strings = strings;
  m_errors = errors;
  m_bindings = bindings;
}

void LexicalAnalyzer::acceptTypeNameExpr(TypeNameExpr* v) {
  Symbol* sym = getSymbol(v->typeName, SYM_STRUCT);
  if (!sym) {
    return;
  }
  sym->uses++;
}

void LexicalAnalyzer::acceptArrayTypeExpr(ArrayTypeExpr* v) {
  v->componentType->acceptVisit(this);
}

void LexicalAnalyzer::acceptPrimitiveTypeExpr(PrimitiveTypeExpr* v) {}

void LexicalAnalyzer::acceptIdentifier(Identifier* v) {
  symboltype st = SYM_NIL;
  if (v->resultType->kind() == TK_FUNC) {
    st = SYM_FUNC;
  }

  Symbol* sym = getSymbol(v->value, st);
  if (!sym) {
    return;
  }

  sym->uses++;
}

void LexicalAnalyzer::acceptCallExpr(CallExpr* v) {
  v->target->acceptVisit(this);
  for (Expr* arg : v->arguments) {
    arg->acceptVisit(this);
  }
}

void LexicalAnalyzer::acceptPropertyAccessExpr(PropertyAccessExpr* v) {
  v->target->acceptVisit(this);

  ScriptType* resType = v->target->getResultingType();
  if (resType->kind() == TK_VOID) {
    return;
  }

  std::string symName;
  symName.append(resType->typeName());
  symName.append(".");

  std::string_view sv = m_strings->getview(v->property->value);
  symName.append(sv);

  stringid combinedId = m_strings->findId(symName);
  if (combinedId == EMPTY_STRING) {
    return;
  }

  Symbol* sym = getSymbol(combinedId, SYM_PROP);
  if (sym) {
    sym->uses++;
  }
}

void LexicalAnalyzer::acceptIndexAccessExpr(IndexAccessExpr* v) {
  v->target->acceptVisit(this);
  v->index->acceptVisit(this);
}

void LexicalAnalyzer::acceptBooleanLiteral(BooleanLiteral* v) {}
void LexicalAnalyzer::acceptCharLiteral(CharLiteral* v) {}
void LexicalAnalyzer::acceptStringLiteral(StringLiteral* v) {}
void LexicalAnalyzer::acceptIntLiteral(IntLiteral* v) {}
void LexicalAnalyzer::acceptFloatLiteral(FloatLiteral* v) {}

void LexicalAnalyzer::acceptObjectLiteral(ObjectLiteral* v) {
  std::set<stringid> propNames;

  for (ObjectLiteralProperty* prop : v->properties) {
    prop->acceptVisit(this);
  }
}

void LexicalAnalyzer::acceptObjectLiteralProperty(ObjectLiteralProperty* v) {
  v->value->acceptVisit(this);
}

void LexicalAnalyzer::acceptBinaryExpr(BinaryExpr* v) {
  v->lhs->acceptVisit(this);
  v->rhs->acceptVisit(this);
}

void LexicalAnalyzer::acceptUnaryExpr(UnaryExpr* v) {
  v->target->acceptVisit(this);
}

void LexicalAnalyzer::acceptTernaryExpr(TernaryExpr* v) {
  v->condition->acceptVisit(this);
  v->left->acceptVisit(this);
  v->right->acceptVisit(this);
}

void LexicalAnalyzer::acceptBlock(Block* v) {
  NOT_MAIN("Block")

  pushScope(getScope()->stype);
  for (Statement* stat : v->statements) {
    stat->acceptVisit(this);
  }
  popScope();
}

void LexicalAnalyzer::acceptIfStatement(IfStatement* v) {
  NOT_MAIN("If Statement")

  v->condition->acceptVisit(this);
  v->body->acceptVisit(this);

  if (v->elseBody) {
    v->elseBody->acceptVisit(this);
  }
}

void LexicalAnalyzer::acceptForStatement(ForStatement* v) {
  NOT_MAIN("For loop")

  pushScope(SCOPE_FORLOOP);

  v->first->acceptVisit(this);
  v->second->acceptVisit(this);
  v->third->acceptVisit(this);

  popScope();
}

void LexicalAnalyzer::acceptLexicalDeclaration(LexicalDeclaration* v) {
  v->typeExpr->acceptVisit(this);

  symboltype symType = SYM_NIL;

  if (v->isConstDeclaration) {
    symType = SYM_CONST;
  } else {
    symType = SYM_VAR;
  }

  AnalyzerScope* scope = getScope();
  stringid varname = v->variableName->value;

  Symbol* existingVar = scope->getSymbol(varname, SYM_VAR);
  Symbol* existingConst = scope->getSymbol(varname, SYM_CONST);

  if (existingConst || existingVar) {
    std::string_view sv = m_strings->getview(varname);

    m_errors->error(v->location, "Duplicate variable defined '%.*s'",
      static_cast<int>(sv.length()), sv.data()
    );
  } else {
    Symbol* sym = getScope()->pushSymbol(v->variableName->value, symType);
    sym->scriptType = v->typeExpr->getReferencedType();
  }

  if (v->value) {
    v->value->acceptVisit(this);
  } else if (v->isConstDeclaration) {
    std::string_view view = m_strings->getview(v->variableName->value);

    m_errors->error(v->location, "Const variable '%.*s' has no value",
      static_cast<int>(view.length()),
      view.data()
    );
  }
}

void LexicalAnalyzer::acceptDoWhileStatement(DoWhileStatement* v) {
  NOT_MAIN("Do While loop")
}

void LexicalAnalyzer::acceptWhileStatement(WhileStatement* v) {
  NOT_MAIN("While loop")
}

void LexicalAnalyzer::acceptControlFlowStatement(ControlFlowStatement* v) {

}

void LexicalAnalyzer::acceptReturnStatement(ReturnStatement* v) {
  NOT_MAIN("Return statement")
}

void LexicalAnalyzer::acceptScriptFileStatement(ScriptFileStatement* v) {
  AnalyzerScope* mainscope = pushScope(SCOPE_MAIN);

  std::vector<NativeBinding>* bindings = m_bindings->getBindings();
  const uint32 bindingsSize = bindings->size();

  for (uint32 i = 0; i < bindingsSize; i++) {
    NativeBinding* bind = &bindings->at(i);
    typekind bindingKind = bind->type->kind();

    stringid nameid = m_strings->allocate(bind->name);
    symboltype st = SYM_NIL;

    switch (bindingKind) {
      case TK_FUNC:
        st = SYM_FUNC;
        break;
      case TK_STRUCT:
        st = SYM_STRUCT;
        break;
      default:
        st = SYM_CONST;
        break;
    }

    Symbol* sym = mainscope->pushSymbol(nameid, st);
    sym->scriptType = bind->type;
    sym->uses = 1;
  }

  for (Statement* s : v->statements) {
    s->acceptVisit(this);
  }

  stringid mainFuncNameId = m_strings->findId("main");
  if (mainFuncNameId != EMPTY_STRING) {
    Symbol* sym = getSymbol(mainFuncNameId, SYM_FUNC);

    if (!sym) {
      m_errors->warn("No int32 main(string[] args) function defined. Script has no entry point");
    } else {
      if (sym->uses != 0) {
        m_errors->warn("Script main function called already?");
      } else {
        sym->uses = 1;
      }
    }
  } else {
    m_errors->warn("No int32 main(string[] args) function defined. Script has no entry point");
  }

  popScope();
}

void LexicalAnalyzer::acceptFunctionParam(FunctionParam* v) {}

void LexicalAnalyzer::acceptFunctionDeclStatement(FunctionDeclStatement* v) {
  AnalyzerScope* scope = getScope();

  v->returnType->acceptVisit(this);

  Symbol* sym = scope->getSymbol(v->name->value, SYM_FUNC);
  if (sym) {
    std::string_view nameview = m_strings->getview(v->name->value);
    m_errors->error(v->location, "Duplicate definition for function '%.*s'",
      static_cast<int>(nameview.length()),
      nameview.data()
    );
  }

  Symbol* funcSym = scope->pushSymbol(v->name->value, SYM_FUNC);
  funcSym->scriptType = v->signature;

  pushScope(SCOPE_FUNCTION);
  std::set<stringid> pnames;

  AnalyzerScope* fscope = getScope();

  for (FunctionParam* param : v->arguments) {
    bool inserted = pnames.insert(param->name->value).second;
    if (!inserted) {
      std::string_view nameview = m_strings->getview(param->name->value);

      m_errors->error(param->location, "Parameter with name %.*s is already defined",
        static_cast<int>(nameview.length()),
        nameview.data()
      );
      continue;
    }

    param->paramType->acceptVisit(this);

    Symbol* psym = fscope->pushSymbol(param->name->value, SYM_VAR);
    psym->scriptType = param->paramType->getReferencedType();
  }

  for (Statement* s : v->functionBody->statements) {
    s->acceptVisit(this);
  }

  popScope();
}

void LexicalAnalyzer::acceptExprStatement(ExprStatement* v) {
  NOT_MAIN("Expression statement")
  v->expression->acceptVisit(this);
}

void LexicalAnalyzer::acceptStructPropertyDecl(StructPropertyDecl* v) {}

void LexicalAnalyzer::acceptStructDecl(StructDecl* v) {
  AnalyzerScope* scope = getScope();
  if (scope->stype != SCOPE_MAIN) {
    m_errors->error(v->location, "Struct declarations are only allowed in global scope");
  }

  Symbol* structSym = scope->pushSymbol(v->name->value, SYM_STRUCT);
  structSym->scriptType = v->type;

  std::string_view structName = m_strings->getview(v->name->value);

  for (const StructPropertyDecl* prop : v->properties) {
    std::string_view propName = m_strings->getview(prop->name->value);

    std::string propSym;
    propSym.append(structName);
    propSym.append(".");
    propSym.append(propName);

    stringid combinedId = m_strings->allocate(propSym);

    Symbol* spropsym = scope->pushSymbol(combinedId, SYM_PROP);
    spropsym->scriptType = prop->propertyType->getReferencedType();

    prop->propertyType->acceptVisit(this);
  }
}

void LexicalAnalyzer::acceptAssertStatement(AssertStatement* v) {
  NOT_MAIN("Assert statement")
}
