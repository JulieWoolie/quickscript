cmake_minimum_required(VERSION 4.2)
project(quickscript)

set(CMAKE_CXX_STANDARD 20)

add_executable(quickscript src/main.cpp
        src/common.h
        src/parse/lexer.h
        src/parse/lexer.cpp
        src/parse/errors.h
        src/parse/errors.cc
        src/parse/token.h
        src/parse/token.cc
        src/parse/syntaxtree.h
        src/stringtable.h
        src/stringtable.cc
        src/parse/parser.h
        src/parse/parser.cc
        src/allocator.h
        src/types.h
        src/parse/keyw_lookup.h
        src/parse/keyw_lookup.cc
        src/parse/syntaxtree.cc
        src/parse/print-visitor.h
        src/interpreter/opcodes.h
        src/interpreter/opcodes.cc
        src/types.cc
        src/analysis/typeresolver.cc
        src/analysis/typeresolver.h
        src/optimizations.h
        src/optimizations.cc
        src/interpreter/nativeinterface.h
        src/interpreter/nativeinterface.cc
        src/interpreter/interpreter.h)
