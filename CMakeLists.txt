cmake_minimum_required(VERSION 3.21)
project(kiwix-tools VERSION 3.8.2 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 11)

# Static linkage
set(CMAKE_FIND_LIBRARY_SUFFIXES .lib .a)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

add_subdirectory(src/libkiwix)
add_subdirectory(src/cli)
