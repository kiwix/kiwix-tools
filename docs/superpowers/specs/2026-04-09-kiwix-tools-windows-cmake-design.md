# Kiwix Tools — Windows CMake Build + CLI + FFI + MCP

## Overview

Replatform kiwix-tools from Meson to CMake for native Windows builds, producing:
1. **Static libraries** (libzim, libkiwix) compiled on Windows with MSVC
2. **Unified `kiwix.exe` CLI** replacing kiwix-manage + kiwix-search
3. **FFI/bindgen layer** exposing a C ABI for other languages/apps
4. **MCP server** for LLM agent integration

Built in four sequential phases, each building on the last.

---

## Phase A — Static Library CMake Build

### Goal

A working CMake build on Windows that produces static `.lib` files for libzim and libkiwix, and a `kiwix.exe` CLI binary.

### Dependency Chain

```
kiwix.exe (new unified CLI)
  ├── libkiwix (vendored, CMakeLists.txt)
  │     ├── libzim (from vcpkg)
  │     │     ├── liblzma (vcpkg)
  │     │     ├── zstd (vcpkg)
  │     │     ├── xapian-core (vcpkg)
  │     │     └── icu (vcpkg)
  │     ├── pugixml (vcpkg)
  │     ├── libcurl (vcpkg)
  │     ├── zlib (vcpkg)
  │     ├── xapian-core (shared with libzim)
  │     └── icu (shared with libzim)
  └── docopt (vcpkg)
```

Note: libmicrohttpd is a dependency of libkiwix's server component, which we exclude in Phase A since we drop kiwix-serve.

### Compiler & Toolchain

- **Compiler**: MSVC 2019 (v14.29) — already installed at `C:\Program Files (x86)\Microsoft Visual Studio\2019\Community`
- **C++ standard**: C++17
- **Generator**: Visual Studio 16 2019 / x64 or Ninja
- **Linkage**: Static (no DLLs)

### Dependency Management

**vcpkg manifest mode** handles transitive deps.

- Bootstrap vcpkg into `D:/Home/kiwix-tools/vcpkg/`
- `vcpkg.json` at project root declares direct dependencies
- libzim is an existing vcpkg port with xapian feature
- libkiwix is vendored with a custom `CMakeLists.txt`
- vcpkg's toolchain file handles find_package for everything

### vcpkg.json (manifest)

```json
{
  "name": "kiwix-tools",
  "version": "3.8.2",
  "dependencies": [
    { "name": "libzim", "features": ["xapian"] },
    "docopt",
    "pugixml",
    "curl",
    "zlib"
  ]
}
```

Note: xapian, icu, liblzma, zstd come in transitively via libzim[xapian].

### Directory Structure

```
kiwix-tools/
├── CMakeLists.txt                    # Top-level project
├── vcpkg.json                        # Dependency manifest
├── vcpkg/                            # Bootstrapped vcpkg (gitignored)
├── cmake/
│   └── vcpkg-configuration.json      # Optional: version pinning
├── src/
│   ├── libkiwix/                     # Vendored libkiwix source
│   │   ├── CMakeLists.txt            # NEW CMake build
│   │   ├── include/                  # Vendored headers
│   │   └── src/                      # Vendored source files
│   ├── cli/                          # NEW unified CLI
│   │   ├── CMakeLists.txt
│   │   └── main.cpp
│   ├── manager/                      # Original (reference only)
│   ├── searcher/                     # Original (reference only)
│   └── server/                       # Original (reference only)
├── docs/
└── ...
```

### Top-level CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.21)
project(kiwix-tools VERSION 3.8.2 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Static linkage everywhere
set(CMAKE_FIND_LIBRARY_SUFFIXES .lib .a)
set(BUILD_SHARED_LIBS OFF)

find_package(libzim CONFIG REQUIRED)
find_package(docopt CONFIG REQUIRED)

add_subdirectory(src/libkiwix)
add_subdirectory(src/cli)
```

### libkiwix CMakeLists.txt (src/libkiwix/)

Builds a static library from the vendored libkiwix source. Key aspects:
- Excludes server/HTTP sources (kiwixserve.cpp, server/*, internalServer*) since we don't need kiwix-serve
- Excludes aria2/downloader sources (subprocess, aria2, downloader) — not needed for CLI manage+search
- Finds deps via vcpkg: libzim, pugixml, curl, zlib, xapian, icu
- Platform source: `subprocess_windows.cpp` on Win32
- Defines `CURL_STATICLIB`, `NOMINMAX` on Windows
- Links `iphlpapi`, `shlwapi`, `winmm` on Windows

### Unified CLI (src/cli/)

Replaces kiwix-manage and kiwix-search with a single `kiwix.exe`:

```
kiwix manage <library.xml> add <zim_paths...>
kiwix manage <library.xml> remove <zim_ids...>
kiwix manage <library.xml> show [zim_ids...]
kiwix search <zim_path> <pattern> [--suggestion]
kiwix --version
kiwix --help
```

Implementation:
- Single `main.cpp` that dispatches to subcommands
- `manage` subcommand: ports logic from kiwix-manage.cpp
- `search` subcommand: ports logic from kiwix-search.cpp (fixes `$HOME/.cache` path for Windows)
- Uses docopt for arg parsing
- Fixes Windows path issue in search's `getKiwixCachedDataDirPath()` (use `%LOCALAPPDATA%` or similar)

### Build Steps

```powershell
# One-time setup
cd D:\Home\kiwix-tools
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg && .\bootstrap-vcpkg.bat && cd ..

# Configure (vcpkg manifest auto-installs deps)
cmake -B build -G "Visual Studio 16 2019" -A x64 ^
  -DCMAKE_TOOLCHAIN_FILE=vcpkg/scripts/buildsystems/vcpkg.cmake

# Build
cmake --build build --config Release

# Output: build/src/cli/Release/kiwix.exe
```

### What Phase A Does NOT Include

- kiwix-serve / HTTP server (removed from scope)
- FFI/bindgen (Phase B)
- MCP server (Phase C)
- Windows packaging — winget/scoop (Phase D)
- Tests (deferred to follow-up)

---

## Phase B — FFI/Bindgen Layer

### Goal

Expose a C ABI from the static library so other applications and language bindings can consume libkiwix/libzim without linking C++ directly.

### Approach

**C wrapper API** (`kiwix_ffi.h`) — a flat C89-compatible header that wraps the key libkiwix/libzim types:

```c
// Opaque handles
typedef struct kiwix_library_t kiwix_library_t;
typedef struct kiwix_manager_t kiwix_manager_t;
typedef struct kiwix_archive_t kiwix_archive_t;
typedef struct kiwix_search_result_t kiwix_search_result_t;

// Lifecycle
kiwix_library_t* kiwix_library_new();
void kiwix_library_free(kiwix_library_t* lib);

// Library management
int kiwix_library_add_file(kiwix_library_t* lib, const char* zim_path);
int kiwix_library_save(kiwix_library_t* lib, const char* xml_path);
int kiwix_library_load(kiwix_library_t* lib, const char* xml_path);

// Search
kiwix_archive_t* kiwix_archive_open(const char* zim_path);
void kiwix_archive_free(kiwix_archive_t* archive);
kiwix_search_result_t* kiwix_archive_search(kiwix_archive_t* archive, const char* query, int limit);
kiwix_search_result_t* kiwix_archive_suggest(kiwix_archive_t* archive, const char* prefix, int limit);

// Result iteration
int kiwix_search_result_count(kiwix_search_result_t* results);
const char* kiwix_search_result_title(kiwix_search_result_t* results, int index);
const char* kiwix_search_result_path(kiwix_search_result_t* results, int index);
void kiwix_search_result_free(kiwix_search_result_t* results);

// Article access
char* kiwix_archive_get_article_content(kiwix_archive_t* archive, const char* path);
void kiwix_free_string(char* s);
```

### Implementation

- `src/ffi/kiwix_ffi.h` — public C header
- `src/ffi/kiwix_ffi.cpp` — C++ implementation wrapping libkiwix/libzim
- `src/ffi/CMakeLists.txt` — builds `kiwix_ffi.lib` (static) and optionally `kiwix_ffi.dll` (shared)
- All strings returned use malloc/free semantics (caller frees via `kiwix_free_string`)
- No exceptions cross the C boundary — all errors return error codes or NULL

### Bindgen targets (future)

The C ABI enables:
- **Python**: cffi or ctypes bindings
- **Rust**: bindgen generates safe-ish wrappers
- **Node.js**: node-ffi or napi
- **Go**: cgo

Phase B delivers the C ABI + static lib. Individual language bindings are separate work.

### Build addition

```cmake
# src/ffi/CMakeLists.txt
add_library(kiwix_ffi STATIC kiwix_ffi.cpp)
target_link_libraries(kiwix_ffi PRIVATE kiwix_lib)
# Optionally also build a DLL:
# add_library(kiwix_ffi_shared SHARED kiwix_ffi.cpp)
```

---

## Phase C — MCP Server

### Goal

An MCP (Model Context Protocol) server that lets LLM agents (Claude, etc.) interact with ZIM files — search, read articles, manage libraries.

### Approach

**Python MCP server** using the `mcp` SDK, backed by the FFI layer from Phase B via cffi/ctypes. This avoids requiring a running kiwix-serve HTTP process.

### Tools exposed

| Tool | Description |
|---|---|
| `search_zim` | Full-text search in a ZIM file |
| `suggest_articles` | Title autocomplete/suggestion |
| `read_article` | Get article content by path |
| `list_zims` | List loaded ZIM files |
| `add_zim` | Add a ZIM file to the library |
| `search_library` | Search across multiple ZIM files |

### Implementation

```
src/mcp/
├── pyproject.toml          # Package definition
├── kiwix_mcp/
│   ├── __init__.py
│   ├── server.py           # MCP server entry point
│   ├── ffi.py              # cffi/ctypes bindings to kiwix_ffi
│   └── tools.py            # MCP tool implementations
└── tests/
```

- Transport: stdio (primary) or streamable HTTP
- Uses `mcp` Python SDK (FastMCP)
- Bundles the `kiwix_ffi.dll` or loads it from PATH
- Config: ZIM library path, cache directory

### Why Python MCP over native?

- MCP SDK is mature in Python
- FFI via ctypes is straightforward
- No HTTP server needed — stdio transport for agents
- Easier to iterate and extend than a C++ MCP server

---

## Phase Ordering

```
Phase A (CMake + static lib + CLI)
  ↓
Phase B (FFI/bindgen C ABI)
  ↓
Phase C (Python MCP server via FFI)
  ↓
Phase D (Windows packaging — winget/scoop — future)
```

Each phase is independently testable and usable. Phase A gives you a working CLI. Phase B makes it a library. Phase C makes it agent-ready.

---

## Risks & Mitigations

| Risk | Mitigation |
|---|---|
| libzim vcpkg port may lag upstream version | Pin version in vcpkg-configuration.json or overlay port |
| libkiwix has deep dep chain (icu, xapian) | vcpkg handles this; first build is slow but cached |
| MSVC 2019 may have C++17 gaps vs GCC/Clang | Test early; upgrade to MSVC 2022 if needed |
| xapian on Windows via vcpkg | Untested — may need patches; fallback to building without xapian |
| ICU data file size for static builds | Accept large binary or consider shared ICU |
