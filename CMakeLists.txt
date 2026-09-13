cmake_minimum_required(VERSION 4.2)
project(qslang)

add_subdirectory(libqs)
add_subdirectory(engine)
add_subdirectory(cli)

add_subdirectory(libraries/stdlib)