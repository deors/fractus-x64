cmake_minimum_required(VERSION 3.21)

project(fractus-x64 VERSION 0.1.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

option(FRACTUS_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" OFF)

add_subdirectory(src)
