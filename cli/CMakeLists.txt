cmake_minimum_required(VERSION 4.2)
project(quickscript)

set(CMAKE_CXX_STANDARD 20)

add_executable(quickscript
       src/main.cpp
       src/args.hpp
       src/args.cpp
       src/tester.hpp
       src/tester.cpp
)

target_link_libraries(quickscript PRIVATE qsengine)