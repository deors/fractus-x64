cmake_minimum_required(VERSION 3.21)

project(fractus-x64 VERSION 0.1.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

option(FRACTUS_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" OFF)
option(FRACTUS_BUILD_TESTS "Build test suite" ON)

if(MINGW)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static -fopenmp")
endif()

add_subdirectory(src)

if(FRACTUS_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
