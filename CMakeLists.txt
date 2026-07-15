cmake_minimum_required(VERSION 4.2)
project(quickscript)

set(CMAKE_CXX_STANDARD 20)

add_executable(quickscript main.cpp
        common.h
        lexer.h
        lexer.cpp
        errors.h
        errors.cc
        token.h
        token.cc
        syntaxtree.h
        stringtable.h
        stringtable.cc
        parser.h
        parser.cc
        allocator.h
        types.h
        keyw_lookup.h
        keyw_lookup.cc)
