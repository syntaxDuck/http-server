cmake_minimum_required(VERSION 3.10)

project(http-server C)

set(CMAKE_C_COMPILER clang)

add_executable(http-server src/main.c)

add_subdirectory(tests)
