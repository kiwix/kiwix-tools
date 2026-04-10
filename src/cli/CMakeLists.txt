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
)
