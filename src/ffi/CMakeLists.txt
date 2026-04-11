set(FFI_SOURCES kiwix_ffi.cpp)

# Static library
add_library(kiwix_ffi_static STATIC ${FFI_SOURCES})
target_include_directories(kiwix_ffi_static
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)
target_compile_definitions(kiwix_ffi_static
  PRIVATE
    KIWIX_FFI_BUILDING
)
# PUBLIC link to kiwix_lib so transitive includes/deps propagate
target_link_libraries(kiwix_ffi_static
  PUBLIC
    kiwix_lib
)

# Shared library (DLL)
add_library(kiwix_ffi SHARED ${FFI_SOURCES})
target_include_directories(kiwix_ffi
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)
target_compile_definitions(kiwix_ffi
  PRIVATE
    KIWIX_FFI_BUILDING
)
# PUBLIC link to kiwix_lib so transitive includes/deps propagate
target_link_libraries(kiwix_ffi
  PUBLIC
    kiwix_lib
)
