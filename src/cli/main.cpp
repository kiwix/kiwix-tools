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
