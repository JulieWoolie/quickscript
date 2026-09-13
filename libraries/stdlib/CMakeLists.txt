cmake_minimum_required(VERSION 4.2)
project(qs_stdlib)

set(CMAKE_CXX_STANDARD 20)

add_library(qs_stdlib SHARED main.cpp)

target_link_libraries(qs_stdlib PRIVATE libqs)