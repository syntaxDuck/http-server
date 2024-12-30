cmake_minimum_required(VERSION 3.5)

project(http-server VERSION 0.1)

set(CMAKE_C_COMPILER clang)

# Generate a compile comands file for clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Define source files
file(GLOB SOURCES "${CMAKE_SOURCE_DIR}/src/*.c")
list(APPEND SOURCES main.c)

include_directories(include)

add_executable(http-server ${SOURCES})

add_subdirectory(tests)
