cmake_minimum_required(VERSION 4.2)
project(libqs)

set(CMAKE_CXX_STANDARD 20)

add_library(libqs SHARED
        include/qsni.h
        src/qsni.cpp)

target_include_directories(libqs PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(libqs PRIVATE qsengine)