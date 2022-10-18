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

#include <getopt.h>
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

/* Print correct console usage options */
void usage()
{
  std::cout << "Usage:" << std::endl
            << "\tkiwix-manage LIBRARY_PATH add ZIM_PATH [OPTIONS]" << std::endl
            << "\tkiwix-manage LIBRARY_PATH remove ZIM_ID [ZIM_ID]..." << std::endl
            << "\tkiwix-manage LIBRARY_PATH show [ZIM_ID]..." << std::endl
            << std::endl

            << "Purpose:" << std::endl
            << "\tManipulates the Kiwix library XML file"
            << std::endl << std::endl

            << "Arguments:" << std::endl
            << "\tLIBRARY_PATH\tis the XML library file path."
            << std::endl << std::endl
            << "\tACTION\t\tis the pre-defined string to specify the action to run on the XML library file."
            << std::endl << std::endl
            << "\t\t\tMust be one of the following values:" << std::endl
            << "\t\t\t* add: add a ZIM file to the library" << std::endl
            << "\t\t\t* remove: remove a ZIM file from the library" << std::endl
            << "\t\t\t* show: show the content of the library"
            << std::endl << std::endl
            << "\tZIM_ID\t\tZIM file unique ID"
            << std::endl << std::endl
            << "\tOPTIONS\t\tCustom options for \"add\" action:" << std::endl
            << "\t\t\t--zimPathToSave=CUSTOM_ZIM_PATH to replace the current ZIM file path" << std::endl
            << "\t\t\t--url=HTTP_ZIM_URL to create an \"url\" attribute for the online version of the ZIM file" << std::endl
            << std::endl
            << "\t\t\tOther options:" << std::endl
            << "\t\t\t-v, --version to print the software version" << std::endl
            << std::endl

            << "Examples:" << std::endl
            << "\tAdd ZIM files to library: kiwix-manage my_library.xml add first.zim second.zim" << std::endl
            << "\tRemove ZIM files from library: kiwix-manage my_library.xml remove e5c2c003-b49e-2756-5176-5d9c86393dd9" << std::endl
            << "\tShow all library ZIM files: kiwix-manage my_library.xml show" << std::endl
            << std::endl

            << "Documentation:" << std::endl
            << "\tSource code\thttps://github.com/kiwix/kiwix-tools" << std::endl
            << "\tMore info\thttps://wiki.kiwix.org/wiki/Kiwix-manage" << std::endl
            << std::endl;
}

int handle_show(const kiwix::Library& library, const std::string& libraryPath,
                 int argc, char* argv[])
{
  if (argc > 3 ) {
    for(auto i=3; i<argc; i++) {
       std::string bookId = argv[i];
       show(library, bookId);
    }
  } else {
    auto booksIds = library.getBooksIds();
    for(auto& bookId: booksIds) {
      show(library, bookId);
    }
  }
  return(0);
}

int handle_add(std::shared_ptr<kiwix::Library> library, const std::string& libraryPath,
                int argc, char* argv[])
{
  string zimPath;
  string zimPathToSave = ".";
  string url;
  int option_index = 0;
  int c = 0;
  int resultCode = 0;

  if (argc <= 3) {
    std::cerr << "Path to zim file to add is missing in the command line" << std::endl;
    return (-1);
  }

  /* Options parsing */
  optind = 3;
  static struct option long_options[] = {
    {"url", required_argument, 0, 'u'},
    {"zimPathToSave", required_argument, 0, 'z'},
    {0, 0, 0, 0}
  };

  bool has_option = false;
  while (true) {
    c = getopt_long(argc, argv, "cz:u:", long_options, &option_index);
    if (c == -1)
      break;

    has_option = true;
    switch (c) {
      case 'u':
        url = optarg;
        break;
      case 'z':
        zimPathToSave = optarg;
        break;
    }
  }

  if (optind >= argc) {
    std::cerr << "Path to zim file to add is missing in the command line" << std::endl;
    return (-1);
  }

  if (has_option && argc-optind > 1) {
    std::cerr << "You cannot give option and several zim files to add" << std::endl;
    return (-1);
  }

  kiwix::Manager manager(library);

  for(auto i=optind; i<argc; i++) {
    std::string zimPath = argv[i];
    if (!zimPath.empty()) {
      auto _zimPathToSave = zimPathToSave == "." ? zimPath : zimPathToSave;
      if (manager.addBookFromPathAndGetId(zimPath, _zimPathToSave, url, false).empty()) {
        std::cerr << "Cannot add zim " << zimPath << " to the library." << std::endl;
        resultCode = 1;
      }
    } else {
      std::cerr << "Invalid zim file path" << std::endl;
      resultCode = 1;
    }
  }

  return(resultCode);
}

int handle_remove(std::shared_ptr<kiwix::Library> library, const std::string& libraryPath,
                   int argc, char* argv[])
{
  std::string bookId;
  const unsigned int totalBookCount = library->getBookCount(true, true);
  int exitCode = 0;

  if (argc <= 3) {
    std::cerr << "BookId to remove missing in the command line" << std::endl;
    return (-1);
  }

  if (!totalBookCount) {
    std::cerr << "Library is empty, no book to delete."
              << std::endl;
    return 1;
  }

  for (int i = 3; i<argc; i++) {
    bookId = argv[i];

    if (!library->removeBookById(bookId)) {
      std::cerr << "Invalid book id '" << bookId << "'." << std::endl;
      exitCode = 1;
    }
  }

  return(exitCode);
}

int main(int argc, char** argv)
{
  string libraryPath = "";
  supportedAction action = NONE;
  auto library = std::make_shared<kiwix::Library>();

  /* General argument parsing */
  static struct option long_options[] = {
    {"version", no_argument, 0, 'v'},
    {0, 0, 0, 0}
  };

  int option_index = 0;
  int c;
  while (true && argc == 2) {
    c = getopt_long(argc, argv, "v", long_options, &option_index);
    if (c == -1)
      break;

    switch (c) {
      case 'v':
        version();
        return 0;
    }
  }

  /* Action related argument parsing */
  if (argc > 2) {
    libraryPath = argv[1];
    string actionString = argv[2];

    if (actionString == "add")
      action = ADD;
    else if (actionString == "show")
      action = SHOW;
    else if (actionString == "remove" || actionString == "delete")
      action = REMOVE;
  }

  /* Print usage)) if necessary */
  if (libraryPath == "" || action == NONE) {
    usage();
    return -1;
  }

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
      exitCode = handle_show(*library, libraryPath, argc, argv);
      break;
    case ADD:
      exitCode = handle_add(library, libraryPath, argc, argv);
      break;
    case REMOVE:
      exitCode = handle_remove(library, libraryPath, argc, argv);
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
