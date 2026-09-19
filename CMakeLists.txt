cmake_minimum_required(VERSION 4.2)
project(qslang)

set(CMAKE_STATIC_LIBRARY_PREFIX "")
set(CMAKE_SHARED_LIBRARY_PREFIX "")

add_subdirectory(libqs)
add_subdirectory(engine)
add_subdirectory(cli)

add_subdirectory(libraries/stdlib)