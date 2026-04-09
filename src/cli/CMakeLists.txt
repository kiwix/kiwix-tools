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
    docopt::docopt
)

# Copy kiwix.exe to project root for convenience
add_custom_command(TARGET kiwix POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
    $<TARGET_FILE:kiwix>
    ${CMAKE_SOURCE_DIR}/kiwix.exe
  COMMENT "Copying kiwix.exe to project root"
)
