/*
 * Copyright 2011-2019 Emmanuel Engelhart <kelson@kiwix.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU  General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

#include <docopt/docopt.h>
#include <kiwix/manager.h>
#include <kiwix/tools.h>
#include <cstdlib>
#include <iostream>

#include "../version.h"

using namespace std;

enum supportedAction { NONE, ADD, SHOW, REMOVE };

void show(const kiwix::Library& library, const std::string& bookId)
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

// Older version of docopt doesn't declare Options. Let's declare it ourself.
using Options = std::map<std::string, docopt::value>;


/* Print correct console usage options */
static const char USAGE[] =
R"(Manipulates the Kiwix library XML file

Usage:
 kiwix-manage LIBRARYPATH add [--zimPathToSave=<custom_zim_path>] [--url=<http_zim_url>] ZIMPATH ...
 kiwix-manage LIBRARYPATH (delete|remove) ZIMID ...
 kiwix-manage LIBRARYPATH show [ZIMID ...]
 kiwix-manage -v | --version
 kiwix-manage -h | --help

Arguments:
  LIBRARYPATH    The XML library file path.
  ZIMID          ZIM file unique ID.
  ZIMPATH        A path to a ZIM to add.

Options:
  Custom options for "add" action:
    --zimPathToSave=<custom_zim_path>  Replace the current ZIM file path
    --url=<http_zim_url>               Create an "url" attribute for the online version of the ZIM file

  Other options:
    -h --help                          Print this help
    -v --version                       Print the software version

Examples:
 Add ZIM files to library:       kiwix-manage my_library.xml add first.zim second.zim
 Remove ZIM files from library:  kiwix-manage my_library.xml remove e5c2c003-b49e-2756-5176-5d9c86393dd9
 Show all library ZIM files:     kiwix-manage my_library.xml show

Documentation:
  Source code  https://github.com/kiwix/kiwix-tools
  More info    https://wiki.kiwix.org/wiki/kiwix-manage
)";

int handle_show(const kiwix::Library& library, const std::string& libraryPath,
                 const Options& options)
{
  if (options.at("ZIMID").asStringList().empty()) {
    auto booksIds = library.getBooksIds();
    for(auto& bookId: booksIds) {
      show(library, bookId);
    }
  } else {
    auto bookIds = options.at("ZIMID").asStringList();
    for(auto& bookId: bookIds) {
       show(library, bookId);
    }
  }

  return(0);
}

int handle_add(kiwix::LibraryPtr library, const std::string& libraryPath,
                const Options& options)
{
  string zimPathToSave;
  string url;

  kiwix::Manager manager(library);

  auto zimPaths = options.at("ZIMPATH").asStringList();
  for (auto& zimPath: zimPaths) {
    if (options.at("--zimPathToSave").isString()) {
      zimPathToSave = options.at("--zimPathToSave").asString();
    } else {
      zimPathToSave = zimPath;
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

int handle_remove(kiwix::Library& library, const std::string& libraryPath,
                   const Options& options)
{
  const unsigned int totalBookCount = library.getBookCount(true, true);
  int exitCode = 0;

  if (!totalBookCount) {
    std::cerr << "Library is empty, no book to delete."
              << std::endl;
    return 1;
  }

  auto bookIds = options.at("ZIMID").asStringList();
  for (auto& bookId: bookIds) {
    if (!library.removeBookById(bookId)) {
      std::cerr << "Invalid book id '" << bookId << "'." << std::endl;
      exitCode = 1;
    }
  }

  return(exitCode);
}

int main(int argc, char** argv)
{
  supportedAction action = NONE;
  auto library = kiwix::Library::create();

  Options args;
  try {
    args = docopt::docopt_parse(USAGE, {argv+1, argv+argc}, false, false);
  } catch (docopt::DocoptArgumentError const & error ) {
    std::cerr << error.what() << std::endl;
    std::cerr << USAGE << std::endl;
    return -1;
  }

  if (args["--help"].asBool()) {
    std::cout << USAGE << std::endl;
    return 0;
  }

  if (args["--version"].asBool()) {
    version();
    return 0;
  }

  std::string libraryPath = args.at("LIBRARYPATH").asString();

  if (args.at("add").asBool())
    action = ADD;
  else if (args.at("show").asBool())
    action = SHOW;
  else if (args.at("remove").asBool() || args.at("delete").asBool())
    action = REMOVE;

  /* Try to read the file */
  libraryPath = kiwix::isRelativePath(libraryPath)
                    ? kiwix::computeAbsolutePath(kiwix::getCurrentDirectory(), libraryPath)
                    : libraryPath;
  kiwix::Manager manager(library);
  if (!manager.readFile(libraryPath, false)) {
    if (kiwix::fileExists(libraryPath) || action!=ADD) {
      std::cerr << "Cannot read the library " << libraryPath << std::endl;
      return 1;
    }
  }

  /* SHOW */
  int exitCode = 0;
  switch (action) {
    case SHOW:
      exitCode = handle_show(*library, libraryPath, args);
      break;
    case ADD:
      exitCode = handle_add(library, libraryPath, args);
      break;
    case REMOVE:
      exitCode = handle_remove(*library, libraryPath, args);
      break;
    case NONE:
      break;
  }

  if (exitCode) {
    return exitCode;
  }

  /* Rewrite the library file */
  if (action == REMOVE || action == ADD) {
    // writeToFile return true (1) if everything is ok => exitCode is 0
    if (!library->writeToFile(libraryPath)) {
      std::cerr << "Cannot write the library " << libraryPath << std::endl;
      return 1;
    }
  }

  return 0;
}
