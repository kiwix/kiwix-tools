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
