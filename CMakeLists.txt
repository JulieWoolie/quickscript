cmake_minimum_required(VERSION 4.2)
project(quickscript)

set(CMAKE_CXX_STANDARD 20)

add_executable(quickscript main.cpp
        common.h
        lexer.h
        lexer.cpp
        parser.h
        errors.h
        errors.cc
        token.h
        token.cc
        syntaxtree.h
        syntaxtree.cc
        parser.cc
        stringtable.h
        stringtable.cc)
