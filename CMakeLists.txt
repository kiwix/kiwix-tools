cmake_minimum_required(VERSION 3.21)
project(kiwix-tools VERSION 3.8.2 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 11)

# Static linkage
set(CMAKE_FIND_LIBRARY_SUFFIXES .lib .a)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

# Ensure Windows SDK libs are findable when using Ninja generator
if(WIN32)
  # Get Windows SDK lib path from environment or cache
  if(NOT DEFINED WINDOWS_SDK_LIB_DIR)
    file(GLOB _sdk_versions "C:/Program Files (x86)/Windows Kits/10/Lib/*")
    if(_sdk_versions)
      list(SORT _sdk_versions)
      list(GET _sdk_versions -1 _latest_sdk)
      set(WINDOWS_SDK_LIB_DIR "${_latest_sdk}/um/x64" CACHE PATH "Windows SDK lib directory")
      message(STATUS "Using Windows SDK libs from: ${WINDOWS_SDK_LIB_DIR}")
    endif()
  endif()
  if(WINDOWS_SDK_LIB_DIR AND EXISTS "${WINDOWS_SDK_LIB_DIR}/iphlpapi.lib")
    link_directories("${WINDOWS_SDK_LIB_DIR}")
  endif()
endif()

add_subdirectory(src/libkiwix)
add_subdirectory(src/cli)
add_subdirectory(src/ffi)
