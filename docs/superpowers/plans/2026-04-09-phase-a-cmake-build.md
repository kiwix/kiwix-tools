# Phase A: Windows CMake Build — Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Build kiwix-tools natively on Windows with CMake, producing a static `kiwix.exe` CLI with `manage` and `search` subcommands.

**Architecture:** vcpkg manifest mode supplies libzim[xapian] and all transitive deps (icu, xapian, liblzma, zstd, pugixml, curl, zlib, docopt). libkiwix is vendored into `src/libkiwix/` with a custom CMakeLists.txt. A new unified CLI in `src/cli/` replaces kiwix-manage and kiwix-search. kiwix-serve (HTTP server) is excluded entirely.

**Tech Stack:** C++17, CMake 3.21+, MSVC 2022 (v143 toolset), vcpkg, Ninja

---

## File Structure

```
kiwix-tools/
├── CMakeLists.txt                        # Top-level project
├── vcpkg.json                            # vcpkg dependency manifest
├── .gitignore                            # Add build/, vcpkg/installed/
├── src/
│   ├── libkiwix/                         # Vendored libkiwix source (copied from D:/Home/libkiwix)
│   │   ├── CMakeLists.txt                # CMake build for libkiwix (NEW)
│   │   ├── include/                      # Vendored headers
│   │   ├── src/                          # Vendored source files
│   │   ├── static/                       # Static resources (templates, i18n)
│   │   ├── scripts/                      # Build-time Python scripts (resource compilers)
│   │   └── mustache/                     # Vendored header-only mustache.hpp
│   │       └── mustache.hpp
│   ├── cli/                              # Unified CLI (NEW)
│   │   ├── CMakeLists.txt
│   │   ├── main.cpp                      # Entry point + subcommand dispatch
│   │   ├── manage_command.cpp            # kiwix manage subcommand
│   │   ├── manage_command.h
│   │   ├── search_command.cpp            # kiwix search subcommand
│   │   ├── search_command.h
│   │   └── version.h                     # Version string
│   ├── manager/                          # Original (kept for reference)
│   ├── searcher/                         # Original (kept for reference)
│   └── server/                           # Original (kept for reference)
└── docs/
```

---

## Task 1: Bootstrap vcpkg

**Files:**
- Create: `vcpkg/` (cloned repo)

- [ ] **Step 1: Clone and bootstrap vcpkg**

```bash
cd D:/Home/kiwix-tools
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg && bootstrap-vcpkg.bat && cd ..
```

- [ ] **Step 2: Verify vcpkg works**

```bash
./vcpkg/vcpkg --version
```

Expected: version string printed

- [ ] **Step 3: Commit (just the .gitignore for vcpkg)**

```bash
echo "vcpkg/installed/" >> .gitignore
echo "vcpkg/packages/" >> .gitignore
echo "vcpkg/downloads/" >> .gitignore
git add .gitignore
git commit -m "chore: add vcpkg to .gitignore"
```

---

## Task 2: Create vcpkg.json manifest

**Files:**
- Create: `vcpkg.json`

- [ ] **Step 1: Create vcpkg.json**

```json
{
  "name": "kiwix-tools",
  "version-string": "3.8.2",
  "dependencies": [
    { "name": "libzim", "features": ["xapian"] },
    "docopt",
    "pugixml",
    "curl",
    "zlib"
  ]
}
```

- [ ] **Step 2: Verify vcpkg can resolve dependencies**

```powershell
cmake -B build -G "Visual Studio 17 2022" -A x64 -DCMAKE_TOOLCHAIN_FILE=vcpkg/scripts/buildsystems/vcpkg.cmake 2>&1 | head -50
```

This will trigger vcpkg to download and build all dependencies. First run will take 30-60 minutes. Expected: vcpkg installs packages into `vcpkg/installed/`. CMake configure may fail after (no CMakeLists.txt yet) — that's OK, we just need vcpkg deps resolved.

- [ ] **Step 3: Commit**

```bash
git add vcpkg.json
git commit -m "chore: add vcpkg dependency manifest"
```

---

## Task 3: Vendor libkiwix source

**Files:**
- Create: `src/libkiwix/` (vendored from D:/Home/libkiwix)

- [ ] **Step 1: Copy libkiwix source into project**

```bash
cd D:/Home/kiwix-tools
mkdir -p src/libkiwix

# Copy headers
cp -r D:/Home/libkiwix/include src/libkiwix/

# Copy source files
cp -r D:/Home/libkiwix/src src/libkiwix/

# Copy static resources
cp -r D:/Home/libkiwix/static src/libkiwix/

# Copy scripts (resource compilers)
cp -r D:/Home/libkiwix/scripts src/libkiwix/
```

- [ ] **Step 2: Vendor mustache.hpp (header-only library)**

```bash
mkdir -p src/libkiwix/mustache
curl -sL "https://raw.githubusercontent.com/kainjow/mustache/master/mustache.hpp" -o src/libkiwix/mustache/mustache.hpp
```

- [ ] **Step 3: Clean up vendored files — remove Meson build files**

```bash
rm -f src/libkiwix/src/meson.build
rm -f src/libkiwix/include/meson.build
rm -f src/libkiwix/static/meson.build
rm -f src/libkiwix/scripts/meson.build
rm -f src/libkiwix/scripts/kiwix-compile-resources.1
rm -f src/libkiwix/scripts/kiwix-compile-i18n.1
rm -f src/libkiwix/scripts/format_code.sh
```

- [ ] **Step 4: Commit**

```bash
git add src/libkiwix/
git commit -m "vendor: add libkiwix source (v14.2.0)"
```

---

## Task 4: Create libkiwix CMakeLists.txt

**Files:**
- Create: `src/libkiwix/CMakeLists.txt`

This is the core of the build. It must:
1. Compile libkiwix source files (excluding server/downloader/aria2)
2. Run the Python resource compiler as a custom command
3. Link against vcpkg-provided deps
4. Produce a static `kiwix` library target

- [ ] **Step 1: Create CMakeLists.txt**

```cmake
cmake_minimum_required(VERSION 3.21)

# === Configuration ===
set(LIBKIWIX_VERSION "14.2.0")

# Generate config.h
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/src/config.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/kiwix_config.h
)
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/src/config.h.in
  ${CMAKE_CURRENT_SOURCE_DIR}/src/config.h
)

# === Resource Compilation ===
# Run kiwix-compile-resources to generate libkiwix-resources.cpp/.h
set(RESOURCE_LIST ${CMAKE_CURRENT_SOURCE_DIR}/static/resources_list.txt)
set(I18N_RESOURCE_LIST ${CMAKE_CURRENT_SOURCE_DIR}/static/i18n_resources_list.txt)
set(RESOURCE_COMPILER ${CMAKE_CURRENT_SOURCE_DIR}/scripts/kiwix-compile-resources)
set(RESOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/resources)

file(MAKE_DIRECTORY ${RESOURCE_DIR})

# Preprocess resources (copy to build dir)
execute_process(
  COMMAND ${CMAKE_COMMAND} -E env python3
    ${CMAKE_CURRENT_SOURCE_DIR}/scripts/kiwix-resources
    --preprocess
    --outdir ${RESOURCE_DIR}
    ${RESOURCE_LIST}
  RESULT_VARIABLE PREPROCESS_RESULT
)
if(NOT PREPROCESS_RESULT EQUAL 0)
  message(WARNING "Resource preprocessing failed, trying with python...")
  execute_process(
    COMMAND python
      ${CMAKE_CURRENT_SOURCE_DIR}/scripts/kiwix-resources
      --preprocess
      --outdir ${RESOURCE_DIR}
      ${RESOURCE_LIST}
  )
endif()

# Compile resources into C++
set(LIB_RESOURCES_CPP ${CMAKE_CURRENT_BINARY_DIR}/libkiwix-resources.cpp)
set(LIB_RESOURCES_H ${CMAKE_CURRENT_BINARY_DIR}/libkiwix-resources.h)

add_custom_command(
  OUTPUT ${LIB_RESOURCES_CPP} ${LIB_RESOURCES_H}
  COMMAND python3 ${RESOURCE_COMPILER}
    --cxxfile ${LIB_RESOURCES_CPP}
    --hfile ${LIB_RESOURCES_H}
    --source_dir ${RESOURCE_DIR}
    ${RESOURCE_LIST}
  DEPENDS ${RESOURCE_LIST} ${RESOURCE_COMPILER}
  COMMENT "Compiling libkiwix resources"
)

# Compile i18n resources
set(I18N_RESOURCES_CPP ${CMAKE_CURRENT_BINARY_DIR}/libkiwix-i18n-resources.cpp)

# Read i18n resource file list
file(STRINGS ${I18N_RESOURCE_LIST} I18N_FILES)

add_custom_command(
  OUTPUT ${I18N_RESOURCES_CPP}
  COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/scripts/kiwix-compile-i18n
    --cxxfile ${I18N_RESOURCES_CPP}
    ${I18N_FILES}
  DEPENDS ${I18N_RESOURCE_LIST} ${I18N_FILES}
  COMMENT "Compiling libkiwix i18n resources"
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
  src/tools/base64.cpp
  src/tools/pathTools.cpp
  src/tools/regexTools.cpp
  src/tools/stringTools.cpp
  src/tools/networkTools.cpp
  src/tools/opdsParsingTools.cpp
  src/tools/languageTools.cpp
  src/tools/otherTools.cpp
  src/tools/archiveTools.cpp
  # Platform-specific
  src/subprocess_windows.cpp
  # Generated resources
  ${LIB_RESOURCES_CPP}
  ${I18N_RESOURCES_CPP}
)

# EXCLUDED (not needed for CLI):
# src/downloader.cpp, src/aria2.cpp (download subsystem)
# src/server.cpp, src/kiwixserve.cpp (HTTP server)
# src/server/*.cpp (server internals)

# === Target ===
add_library(kiwix_lib STATIC ${KIWIX_SOURCES})

target_include_directories(kiwix_lib
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_BINARY_DIR}          # for generated config.h, resources.h
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src       # for internal headers
    ${CMAKE_CURRENT_SOURCE_DIR}/mustache  # for mustache.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/static    # for resource references
)

# Compile definitions
target_compile_definitions(kiwix_lib
  PUBLIC
    LIBKIWIX_VERSION="${LIBKIWIX_VERSION}"
    NOMINMAX
  PRIVATE
    CURL_STATICLIB
)

# === Dependencies (from vcpkg) ===
find_package(libzim CONFIG REQUIRED)
find_package(pugixml CONFIG REQUIRED)
find_package(CURL CONFIG REQUIRED)
find_package(ZLIB REQUIRED)
find_package(ICU REQUIRED)
find_package(unofficial-xapian CONFIG REQUIRED)

target_link_libraries(kiwix_lib
  PUBLIC
    libzim::libzim
    pugixml::pugixml
    CURL::libcurl
    ZLIB::ZLIB
    ICU::i18n ICU::uc
    unofficial::xapian::xapian
  PRIVATE
    iphlpapi
    shlwapi
    winmm
)
```

- [ ] **Step 2: Verify CMake can configure libkiwix alone**

```powershell
cmake -B build -G "Visual Studio 17 2022" -A x64 -DCMAKE_TOOLCHAIN_FILE=vcpkg/scripts/buildsystems/vcpkg.cmake 2>&1 | tail -30
```

Expected: CMake configuration succeeds (may still have issues, we'll fix in next steps)

- [ ] **Step 3: Attempt to build libkiwix**

```powershell
cmake --build build --config Release --target kiwix_lib 2>&1 | tail -50
```

Expected: Compilation errors are likely — fix them iteratively.

- [ ] **Step 4: Fix any compilation errors**

Common fixes needed:
- `mustache.hpp` include path: may need `-I` for `mustache/` directory
- Resource compiler may need `python` instead of `python3` on Windows
- Missing includes for Windows-specific headers
- `server/i18n_utils.h` included by `otherTools.cpp` — we may need to provide a minimal stub or include the file

Iterate until `kiwix_lib` compiles cleanly.

- [ ] **Step 5: Commit**

```bash
git add src/libkiwix/CMakeLists.txt
git commit -m "build: add CMakeLists.txt for vendored libkiwix"
```

---

## Task 5: Create top-level CMakeLists.txt

**Files:**
- Create: `CMakeLists.txt` (root)

- [ ] **Step 1: Create the top-level CMakeLists.txt**

```cmake
cmake_minimum_required(VERSION 3.21)
project(kiwix-tools VERSION 3.8.2 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 11)

# Static linkage
set(CMAKE_FIND_LIBRARY_SUFFIXES .lib .a)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

# Find packages provided by vcpkg
find_package(libzim CONFIG REQUIRED)
find_package(docopt CONFIG REQUIRED)

add_subdirectory(src/libkiwix)
add_subdirectory(src/cli)
```

- [ ] **Step 2: Verify configuration**

```powershell
cmake -B build -G "Visual Studio 17 2022" -A x64 -DCMAKE_TOOLCHAIN_FILE=vcpkg/scripts/buildsystems/vcpkg.cmake 2>&1 | tail -20
```

Expected: Fails on missing `src/cli/CMakeLists.txt` — that's OK, we'll create it next.

- [ ] **Step 3: Commit**

```bash
git add CMakeLists.txt
git commit -m "build: add top-level CMakeLists.txt"
```

---

## Task 6: Create unified CLI — src/cli/

**Files:**
- Create: `src/cli/CMakeLists.txt`
- Create: `src/cli/main.cpp`
- Create: `src/cli/manage_command.h`
- Create: `src/cli/manage_command.cpp`
- Create: `src/cli/search_command.h`
- Create: `src/cli/search_command.cpp`
- Create: `src/cli/version.h`

- [ ] **Step 1: Create src/cli/version.h**

```cpp
#ifndef KIWIX_CLI_VERSION_H_
#define KIWIX_CLI_VERSION_H_

#ifndef KIWIX_TOOLS_VERSION
  #define KIWIX_TOOLS_VERSION "3.8.2"
#endif

#include <kiwix/version.h>
#include <zim/version.h>
#include <iostream>

inline void print_version()
{
  std::cout << "kiwix-tools " << KIWIX_TOOLS_VERSION << std::endl << std::endl;
  kiwix::printVersions();
  std::cout << std::endl;
  zim::printVersions();
}

#endif
```

- [ ] **Step 2: Create src/cli/manage_command.h**

```cpp
#ifndef KIWIX_CLI_MANAGE_COMMAND_H_
#define KIWIX_CLI_MANAGE_COMMAND_H_

int run_manage_command(int argc, char** argv);

#endif
```

- [ ] **Step 3: Create src/cli/manage_command.cpp**

Ported from `src/manager/kiwix-manage.cpp`. Key changes:
- Extracted into a function `run_manage_command()`
- Uses docopt for arg parsing (same as original)
- Same logic for add/remove/show

```cpp
#include <docopt/docopt.h>
#include <kiwix/manager.h>
#include <kiwix/tools.h>
#include <cstdlib>
#include <iostream>

#include "version.h"

using Options = std::map<std::string, docopt::value>;

static const char USAGE[] =
R"(Manage Kiwix library XML files

Usage:
 kiwix manage LIBRARYPATH add [--zimPathToSave=<path>] [--url=<url>] ZIMPATH ...
 kiwix manage LIBRARYPATH (delete|remove) ZIMID ...
 kiwix manage LIBRARYPATH show [ZIMID ...]
 kiwix manage -h | --help

Arguments:
  LIBRARYPATH    The XML library file path.
  ZIMID          ZIM file unique ID.
  ZIMPATH        A path to a ZIM to add.

Options:
  --zimPathToSave=<path>  Replace the ZIM file path stored in the library
  --url=<url>             Set the URL attribute for the online version
  -h --help               Print this help
)";

static void show(const kiwix::Library& library, const std::string& bookId)
{
  try {
    auto& book = library.getBookById(bookId);
    std::cout << "id:\t\t" << book.getId() << std::endl
              << "path:\t\t" << book.getPath() << std::endl
              << "url:\t\t" << book.getUrl() << std::endl
              << "title:\t\t" << book.getTitle() << std::endl
              << "name:\t\t" << book.getName() << std::endl
              << "tags:\t\t" << book.getTags() << std::endl
              << "description:\t" << book.getDescription() << std::endl
              << "creator:\t" << book.getCreator() << std::endl
              << "date:\t\t" << book.getDate() << std::endl
              << "articleCount:\t" << book.getArticleCount() << std::endl
              << "mediaCount:\t" << book.getMediaCount() << std::endl
              << "size:\t\t" << book.getSize() << " KB" << std::endl;
  } catch (std::out_of_range&) {
    std::cout << "No book " << bookId << " in the library" << std::endl;
  }
  std::cout << std::endl;
}

static int handle_show(const kiwix::Library& library, const Options& options)
{
  if (options.at("ZIMID").asStringList().empty()) {
    for (auto& bookId : library.getBooksIds()) {
      show(library, bookId);
    }
  } else {
    for (auto& bookId : options.at("ZIMID").asStringList()) {
      show(library, bookId);
    }
  }
  return 0;
}

static int handle_add(kiwix::LibraryPtr library, const std::string& libraryPath,
                       const Options& options)
{
  kiwix::Manager manager(library);
  auto zimPaths = options.at("ZIMPATH").asStringList();
  for (auto& zimPath : zimPaths) {
    std::string zimPathToSave = zimPath;
    std::string url;
    if (options.at("--zimPathToSave").isString()) {
      zimPathToSave = options.at("--zimPathToSave").asString();
    }
    if (options.at("--url").isString()) {
      url = options.at("--url").asString();
    }
    if (manager.addBookFromPathAndGetId(zimPath, zimPathToSave, url, false).empty()) {
      std::cerr << "Cannot add ZIM " << zimPath << " to the library." << std::endl;
      return 1;
    }
  }
  return 0;
}

static int handle_remove(kiwix::Library& library, const Options& options)
{
  if (library.getBookCount(true, true) == 0) {
    std::cerr << "Library is empty, no book to delete." << std::endl;
    return 1;
  }
  int exitCode = 0;
  for (auto& bookId : options.at("ZIMID").asStringList()) {
    if (!library.removeBookById(bookId)) {
      std::cerr << "Invalid book id '" << bookId << "'." << std::endl;
      exitCode = 1;
    }
  }
  return exitCode;
}

int run_manage_command(int argc, char** argv)
{
  auto library = kiwix::Library::create();
  Options args;

  try {
    args = docopt::docopt_parse(USAGE, {argv + 1, argv + argc}, false, false);
  } catch (docopt::DocoptArgumentError const& error) {
    std::cerr << error.what() << std::endl;
    std::cerr << USAGE << std::endl;
    return -1;
  }

  if (args.at("--help").asBool()) {
    std::cout << USAGE << std::endl;
    return 0;
  }

  std::string libraryPath = args.at("LIBRARYPATH").asString();

  enum Action { NONE, ADD, SHOW, REMOVE } action = NONE;
  if (args.at("add").asBool()) action = ADD;
  else if (args.at("show").asBool()) action = SHOW;
  else if (args.at("remove").asBool() || args.at("delete").asBool()) action = REMOVE;

  libraryPath = kiwix::isRelativePath(libraryPath)
                    ? kiwix::computeAbsolutePath(kiwix::getCurrentDirectory(), libraryPath)
                    : libraryPath;

  kiwix::Manager manager(library);
  if (!manager.readFile(libraryPath, false)) {
    if (kiwix::fileExists(libraryPath) || action != ADD) {
      std::cerr << "Cannot read the library " << libraryPath << std::endl;
      return 1;
    }
  }

  int exitCode = 0;
  switch (action) {
    case SHOW:   exitCode = handle_show(*library, args); break;
    case ADD:    exitCode = handle_add(library, libraryPath, args); break;
    case REMOVE: exitCode = handle_remove(*library, args); break;
    case NONE:   break;
  }

  if (exitCode) return exitCode;

  if (action == REMOVE || action == ADD) {
    if (!library->writeToFile(libraryPath)) {
      std::cerr << "Cannot write the library " << libraryPath << std::endl;
      return 1;
    }
  }

  return 0;
}
```

- [ ] **Step 4: Create src/cli/search_command.h**

```cpp
#ifndef KIWIX_CLI_SEARCH_COMMAND_H_
#define KIWIX_CLI_SEARCH_COMMAND_H_

int run_search_command(int argc, char** argv);

#endif
```

- [ ] **Step 5: Create src/cli/search_command.cpp**

Ported from `src/searcher/kiwix-search.cpp`. Key changes:
- Fixed Windows path for cache directory (uses `%LOCALAPPDATA%` instead of `$HOME/.cache`)
- Extracted into function `run_search_command()`

```cpp
#include <docopt/docopt.h>
#include <zim/search.h>
#include <zim/suggestion.h>
#include <kiwix/spelling_correction.h>
#include <xapian.h>
#include <iostream>
#include <filesystem>

#ifdef _WIN32
#include <windows.h>
#include <shlobj.h>
#endif

#include "version.h"

using Options = std::map<std::string, docopt::value>;

static const char USAGE[] =
R"(Search articles in a ZIM file

Usage:
  kiwix search [options] ZIM PATTERN
  kiwix search -h | --help

Arguments:
  ZIM       The full path of the ZIM file
  PATTERN   Word(s) to search for

Options:
  -s --suggestion    Suggest article titles based on PATTERN prefix
  --spelling         Suggest corrected spelling for PATTERN
  -v --verbose       Give details about the search process
  -h --help          Print this help
)";

static std::filesystem::path getCacheDir()
{
#ifdef _WIN32
  char path[MAX_PATH];
  if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, path))) {
    return std::filesystem::path(path) / "kiwix" / "cache";
  }
  // Fallback
  const char* localAppData = std::getenv("LOCALAPPDATA");
  if (localAppData) {
    return std::filesystem::path(localAppData) / "kiwix" / "cache";
  }
#else
  const char* home = std::getenv("HOME");
  if (home) {
    return std::filesystem::path(home) / ".cache" / "kiwix";
  }
#endif
  return std::filesystem::current_path() / ".kiwix-cache";
}

int run_search_command(int argc, char** argv)
{
  Options args;
  try {
    args = docopt::docopt_parse(USAGE, {argv + 1, argv + argc}, false, false);
  } catch (docopt::DocoptArgumentError const& error) {
    std::cerr << error.what() << std::endl;
    std::cerr << USAGE << std::endl;
    return -1;
  }

  if (args.at("--help").asBool()) {
    std::cout << USAGE << std::endl;
    return 0;
  }

  auto zimPath = args.at("ZIM").asString();
  auto pattern = args.at("PATTERN").asString();
  auto verbose = args.at("--verbose").asBool();

  try {
    zim::Archive archive(zimPath);

    if (args.at("--suggestion").asBool()) {
      zim::SuggestionSearcher searcher(archive);
      searcher.setVerbose(verbose);
      for (const auto& r : searcher.suggest(pattern).getResults(0, 10)) {
        std::cout << r.getTitle() << std::endl;
      }
    } else if (args.at("--spelling").asBool()) {
      auto cacheDir = getCacheDir();
      std::filesystem::create_directories(cacheDir);
      kiwix::SpellingsDB spellingsDB(archive, cacheDir);
      for (const auto& r : spellingsDB.getSpellingCorrections(pattern, 1)) {
        std::cout << r << std::endl;
      }
    } else {
      zim::Searcher searcher(archive);
      searcher.setVerbose(verbose);
      const zim::Query query(pattern);
      for (const auto& r : searcher.search(query).getResults(0, 10)) {
        std::cout << r.getTitle() << std::endl;
      }
    }
  } catch (const std::runtime_error& err) {
    std::cerr << err.what() << std::endl;
    return 1;
  } catch (const Xapian::Error& err) {
    std::cerr << err.get_msg() << std::endl;
    return 1;
  }

  return 0;
}
```

- [ ] **Step 6: Create src/cli/main.cpp**

```cpp
#include <iostream>
#include <string>
#include <cstring>

#include "manage_command.h"
#include "search_command.h"
#include "version.h"

static const char USAGE[] =
R"(Kiwix offline content tools

Usage:
  kiwix <command> [options] [arguments...]

Commands:
  manage    Manage Kiwix library XML files (add, remove, show ZIM files)
  search    Full-text search or title suggestion in a ZIM file

Options:
  -h --help       Print this help
  -V --version    Print version information

Run 'kiwix <command> --help' for more information on a command.
)";

int main(int argc, char** argv)
{
  if (argc < 2) {
    std::cout << USAGE << std::endl;
    return 0;
  }

  std::string command = argv[1];

  if (command == "--help" || command == "-h") {
    std::cout << USAGE << std::endl;
    return 0;
  }

  if (command == "--version" || command == "-V") {
    print_version();
    return 0;
  }

  // Shift argv to pass subcommand args (skip "kiwix" but keep subcommand name)
  int sub_argc = argc - 1;
  char** sub_argv = argv + 1;

  if (command == "manage") {
    return run_manage_command(sub_argc, sub_argv);
  } else if (command == "search") {
    return run_search_command(sub_argc, sub_argv);
  } else {
    std::cerr << "Unknown command: " << command << std::endl;
    std::cerr << USAGE << std::endl;
    return 1;
  }
}
```

- [ ] **Step 7: Create src/cli/CMakeLists.txt**

```cmake
set(CLI_SOURCES
  main.cpp
  manage_command.cpp
  search_command.cpp
)

add_executable(kiwix ${CLI_SOURCES})

target_compile_definitions(kiwix
  PRIVATE
    KIWIX_TOOLS_VERSION="${PROJECT_VERSION}"
    NOMINMAX
)

target_include_directories(kiwix
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(kiwix
  PRIVATE
    kiwix_lib
    libzim::libzim
    docopt::docopt
)

# Copy kiwix.exe to project root for convenience
add_custom_command(TARGET kiwix POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
    $<TARGET_FILE:kiwix>
    ${CMAKE_SOURCE_DIR}/kiwix.exe
  COMMENT "Copying kiwix.exe to project root"
)
```

- [ ] **Step 8: Commit**

```bash
git add src/cli/
git commit -m "feat: add unified kiwix CLI with manage and search subcommands"
```

---

## Task 7: Full build and fix

**Files:**
- Modify: `src/libkiwix/CMakeLists.txt` (fix compilation issues)

This is the iterative fix task. The build will likely fail on first attempt due to:
1. Missing headers / wrong include paths
2. Resource compilation issues (Python scripts on Windows)
3. xapian/ICU link issues on Windows
4. docopt linkage (static vs shared)

- [ ] **Step 1: Run full configure**

```powershell
cmake -B build -G "Visual Studio 17 2022" -A x64 -DCMAKE_TOOLCHAIN_FILE=vcpkg/scripts/buildsystems/vcpkg.cmake
```

- [ ] **Step 2: Run full build**

```powershell
cmake --build build --config Release 2>&1 | tee build.log
```

- [ ] **Step 3: Fix each compilation error iteratively**

Read `build.log`, identify errors, fix in CMakeLists.txt or source, rebuild:

```powershell
cmake --build build --config Release
```

Repeat until clean build.

Common expected fixes:
- **`mustache.hpp` not found**: Ensure `mustache/` dir is in include path
- **`server/i18n_utils.h` not found**: Either vendor `src/server/i18n.h` + `src/server/i18n_utils.h` or stub out the include in `otherTools.cpp`
- **Resource compiler Python path**: Change `python3` to `python` on Windows
- **xapian find_package name**: vcpkg ships xapian as `unofficial-xapian`
- **ICU linkage**: May need both `ICU::i18n` and `ICU::uc`
- **docopt target name**: May be `docopt::docopt` or `docopt`

- [ ] **Step 4: Verify clean build**

```powershell
cmake --build build --config Release
echo $?
```

Expected: Build succeeds with no errors

- [ ] **Step 5: Verify kiwix.exe exists**

```powershell
ls -lh build/src/cli/Release/kiwix.exe
./build/src/cli/Release/kiwix.exe --version
./build/src/cli/Release/kiwix.exe --help
```

Expected: Version info printed, help text displayed

- [ ] **Step 6: Commit all fixes**

```bash
git add -A
git commit -m "fix: resolve compilation issues for Windows CMake build"
```

---

## Task 8: Smoke test kiwix.exe

**Files:**
- None (testing only)

- [ ] **Step 1: Test help output**

```powershell
./build/src/cli/Release/kiwix.exe --help
./build/src/cli/Release/kiwix.exe manage --help
./build/src/cli/Release/kiwix.exe search --help
```

Expected: Help text for each command printed correctly

- [ ] **Step 2: Test version**

```powershell
./build/src/cli/Release/kiwix.exe --version
```

Expected: `kiwix-tools 3.8.2` plus libkiwix/libzim version info

- [ ] **Step 3: Test manage with a small ZIM file**

Download a small test ZIM file (or skip if none available):

```powershell
# Create a test library
./build/src/cli/Release/kiwix.exe manage test_library.xml show
echo "Exit code: $?"
```

Expected: "Cannot read the library" (file doesn't exist) — this is correct behavior.

- [ ] **Step 4: Test unknown command**

```powershell
./build/src/cli/Release/kiwix.exe foobar
echo "Exit code: $?"
```

Expected: "Unknown command: foobar" + help text, exit code 1

---

## Notes

### Resource compilation on Windows

The Python scripts `kiwix-compile-resources` and `kiwix-compile-i18n` run at build time to embed static files (HTML templates, CSS, JS, i18n JSON) into C++ source. On Windows, `python3` may not be on PATH — use `python` instead. The CMakeLists.txt should detect this.

### vcpkg package names

| Package | vcpkg name | CMake find_package target |
|---|---|---|
| libzim | `libzim[xapian]` | `libzim::libzim` |
| docopt | `docopt` | `docopt::docopt` |
| pugixml | `pugixml` | `pugixml::pugixml` |
| libcurl | `curl` | `CURL::libcurl` |
| zlib | `zlib` | `ZLIB::ZLIB` |
| xapian | (via libzim) | `unofficial::xapian::xapian` |
| ICU | (via libzim) | `ICU::i18n`, `ICU::uc` |

### MSVC 2022 toolset

- Installed at: `C:\Program Files\Microsoft Visual Studio\2022\Community`
- MSVC version: 19.44 (v14.44 toolset)
- CMake generator: `Visual Studio 17 2022`
- Platform: `x64`
