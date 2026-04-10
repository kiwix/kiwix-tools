cmake_minimum_required(VERSION 3.21)

set(LIBKIWIX_VERSION "14.2.0")

# === Find Python (needed for resource compilation) ===
find_package(Python3 COMPONENTS Interpreter REQUIRED)

# === Generate config.h ===
set(KIWIX_CONFIG_H "${CMAKE_CURRENT_BINARY_DIR}/kiwix_config.h")
file(WRITE ${KIWIX_CONFIG_H}
  "#ifndef KIWIX_CONFIG_H\n"
  "#define KIWIX_CONFIG_H\n"
  "#define LIBKIWIX_VERSION \"${LIBKIWIX_VERSION}\"\n"
  "#endif\n"
)

# === Resource Compilation ===
set(STATIC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/static")
set(SCRIPTS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/scripts")
set(RESOURCE_LIST "${STATIC_DIR}/resources_list.txt")
set(I18N_RESOURCE_LIST "${STATIC_DIR}/i18n_resources_list.txt")
set(RESOURCE_DIR "${CMAKE_CURRENT_BINARY_DIR}/resources")

file(MAKE_DIRECTORY ${RESOURCE_DIR})

# Step 1: Preprocess resources
add_custom_command(
  OUTPUT "${RESOURCE_DIR}/.preprocess_stamp"
  COMMAND ${Python3_EXECUTABLE} "${SCRIPTS_DIR}/kiwix-resources"
          --preprocess --outdir "${RESOURCE_DIR}" "${RESOURCE_LIST}"
  COMMAND ${CMAKE_COMMAND} -E touch "${RESOURCE_DIR}/.preprocess_stamp"
  DEPENDS "${RESOURCE_LIST}" "${SCRIPTS_DIR}/kiwix-resources"
  COMMENT "Preprocessing libkiwix resources"
)

# Step 2: Compile resources into C++ source
set(LIB_RESOURCES_CPP "${CMAKE_CURRENT_BINARY_DIR}/libkiwix-resources.cpp")
set(LIB_RESOURCES_H "${CMAKE_CURRENT_BINARY_DIR}/libkiwix-resources.h")

add_custom_command(
  OUTPUT "${LIB_RESOURCES_CPP}" "${LIB_RESOURCES_H}"
  COMMAND ${Python3_EXECUTABLE} "${SCRIPTS_DIR}/kiwix-compile-resources"
          --cxxfile "${LIB_RESOURCES_CPP}"
          --hfile "${LIB_RESOURCES_H}"
          --source_dir "${RESOURCE_DIR}"
          "${RESOURCE_LIST}"
  DEPENDS "${RESOURCE_LIST}" "${SCRIPTS_DIR}/kiwix-compile-resources"
          "${RESOURCE_DIR}/.preprocess_stamp"
  COMMENT "Compiling libkiwix resources to C++"
)

# Step 3: Compile i18n resources
file(STRINGS "${I18N_RESOURCE_LIST}" I18N_RELATIVE_FILES)
set(I18N_ABSOLUTE_FILES "")
foreach(f ${I18N_RELATIVE_FILES})
  list(APPEND I18N_ABSOLUTE_FILES "${STATIC_DIR}/${f}")
endforeach()

set(I18N_RESOURCES_CPP "${CMAKE_CURRENT_BINARY_DIR}/libkiwix-i18n-resources.cpp")

add_custom_command(
  OUTPUT "${I18N_RESOURCES_CPP}"
  COMMAND ${Python3_EXECUTABLE} "${SCRIPTS_DIR}/kiwix-compile-i18n"
          --cxxfile "${I18N_RESOURCES_CPP}"
          ${I18N_ABSOLUTE_FILES}
  DEPENDS "${I18N_RESOURCE_LIST}" ${I18N_ABSOLUTE_FILES}
          "${SCRIPTS_DIR}/kiwix-compile-i18n"
  COMMENT "Compiling libkiwix i18n resources to C++"
)

# === Source Files ===
set(KIWIX_SOURCES
  src/book.cpp
  src/bookmark.cpp
  src/library.cpp
  src/manager.cpp
  src/libxml_dumper.cpp
  src/opds_dumper.cpp
  src/html_dumper.cpp
  src/library_dumper.cpp
  src/search_renderer.cpp
  src/name_mapper.cpp
  src/opds_catalog.cpp
  src/spelling_correction.cpp
  src/version.cpp
  src/server/i18n.cpp
  src/tools/base64.cpp
  src/tools/pathTools.cpp
  src/tools/regexTools.cpp
  src/tools/stringTools.cpp
  src/tools/networkTools.cpp
  src/tools/opdsParsingTools.cpp
  src/tools/languageTools.cpp
  src/tools/otherTools.cpp
  src/tools/archiveTools.cpp
  src/subprocess_windows.cpp
  ${LIB_RESOURCES_CPP}
  ${I18N_RESOURCES_CPP}
)

# === Target ===
add_library(kiwix_lib STATIC ${KIWIX_SOURCES})

set(MUSTACHE_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/mustache")

target_include_directories(kiwix_lib
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_BINARY_DIR}
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${MUSTACHE_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/static
)

target_compile_definitions(kiwix_lib
  PUBLIC
    NOMINMAX
  PRIVATE
    LIBKIWIX_VERSION="${LIBKIWIX_VERSION}"
    CURL_STATICLIB
)

# === Dependencies ===
# We use vcpkg's installed directory directly for packages without CMake configs

# libzim: no CMake config - create imported target
add_library(libzim::libzim STATIC IMPORTED)
set_target_properties(libzim::libzim PROPERTIES
  IMPORTED_LOCATION "${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/lib/zim.lib"
  INTERFACE_INCLUDE_DIRECTORIES "${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/include"
)
# libzim transitive deps come from its pkg-config: liblzma, zstd, xapian, icu

# xapian: has custom cmake config but we create imported target for simplicity
add_library(xapian::xapian STATIC IMPORTED)
set_target_properties(xapian::xapian PROPERTIES
  IMPORTED_LOCATION "${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/lib/xapian.lib"
  INTERFACE_INCLUDE_DIRECTORIES "${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/include"
)

# These have proper CMake configs
find_package(docopt CONFIG REQUIRED)
find_package(pugixml CONFIG REQUIRED)
find_package(CURL CONFIG REQUIRED)
find_package(ZLIB REQUIRED)
find_package(ICU COMPONENTS i18n uc REQUIRED)

target_link_libraries(kiwix_lib
  PUBLIC
    libzim::libzim
    xapian::xapian
    pugixml::pugixml
    CURL::libcurl
    ZLIB::ZLIB
    ICU::i18n
    ICU::uc
  PRIVATE
    docopt
    iphlpapi
    shlwapi
    winmm
    ws2_32
)
