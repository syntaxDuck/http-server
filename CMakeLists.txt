cmake_minimum_required(VERSION 3.5)

project(http-server VERSION 0.1)

set(CMAKE_C_COMPILER clang)

# Generate a compile comands file for clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Define source files
set(SOURCES main.c src/request.c src/util.c)

include_directories(include)

add_executable(http-server main.c ${SOURCES})

add_subdirectory(tests)
