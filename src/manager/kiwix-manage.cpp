/*
 * Copyright 2011 Emmanuel Engelhart <kelson@kiwix.org>
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

#ifndef _WIN32
#include <unistd.h>
#endif
#include <getopt.h>
#include <kiwix/tools/pathTools.h>
#include <kiwix/tools/stringTools.h>
#include <kiwix/manager.h>
#include <kiwix/downloader.h>
#include <cstdlib>
#include <iostream>

#include <thread>
#include <chrono>

using namespace std;

enum supportedAction { NONE, ADD, SHOW, REMOVE, DOWNLOAD };

void show(kiwix::Library* library, const std::string& bookId)
{
  try {
    auto& book = library->getBookById(bookId);
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

void usage()
{
  cerr << "Usage:" << endl;
  cerr << "\tkiwix-manage LIBRARY_PATH add ZIM_PATH "
          "[--zimPathToSave=../content/foobar.zim] [--current] "
          "[--url=http://...metalink]"
       << endl;
  cerr << "\tkiwix-manage LIBRARY_PATH show [CONTENTID1] [CONTENTID2] ... "
          "(show everything if no param.)"
       << endl;
  cerr << "\tkiwix-manage LIBRARY_PATH remove CONTENTID1 [CONTENTID2]" << endl;
}


bool handle_show(kiwix::Library* library, const std::string& libraryPath,
                 int argc, char* argv[])
{
  if (argc > 3 ) {
    for(auto i=3; i<argc; i++) {
       std::string bookId = argv[i];
       show(library, bookId);
    }
  } else {
    auto booksIds = library->getBooksIds();
    for(auto& bookId: booksIds) {
      show(library, bookId);
    }
  }
  return(0);
}

bool handle_add(kiwix::Library* library, const std::string& libraryPath,
                int argc, char* argv[])
{
  string zimPath;
  string zimPathToSave = ".";
  string url;
  string origID = "";
  int option_index = 0;
  int c = 0;
  bool resultCode = 0;

  if (argc <= 3) {
    std::cerr << "Path to zim file to add is missing in the command line" << std::endl;
    return (-1);
  }

  /* Options parsing */
  optind = 3;
  static struct option long_options[] = {
    {"url", required_argument, 0, 'u'},
    {"origId", required_argument, 0, 'o'},
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

      case 'o':
        origID = optarg;
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
      zimPathToSave = zimPathToSave == "." ? zimPath : zimPathToSave;
      manager.addBookFromPathAndGetId(zimPath, zimPathToSave, url, false);
    } else {
      std::cerr << "Invalid zim file path" << std::endl;
      resultCode = 1;
    }
  }

  return(resultCode);
}

bool handle_remove(kiwix::Library* library, const std::string& libraryPath,
                   int argc, char* argv[])
{
  std::string bookId;
  const unsigned int totalBookCount = library->getBookCount(true, true);
  bool exitCode = 0;

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

bool handle_download(kiwix::Library* library, const std::string& libraryPath,
                     int argc, char* argv[])
{
  std::string bookId;
  bool exitCode = false;

  if (argc > 3) {
     bookId = argv[3];
  }

  auto& book = library->getBookById(bookId);
  auto did = book.getDownloadId();
  kiwix::Downloader downloader;
  kiwix::Download* download = nullptr;
  if (!did.empty()) {
    std::cout << "try resume " << did << std::endl;
    try {
      download = downloader.getDownload(did);
    } catch(...) {}
  }
  if (nullptr == download || download->getStatus() == kiwix::Download::K_UNKNOWN) {
    download = downloader.startDownload(book.getUrl());
    book.setDownloadId(download->getDid());
  }
  int step = 60*5;
  while (step--) {
    download->updateStatus();
    if (download->getStatus() == kiwix::Download::K_COMPLETE) {
      auto followingId = download->getFollowedBy();
      if (followingId.empty()) {
        book.setPath(download->getPath());
        book.setDownloadId("");
        std::cout << "File downloaded to " << book.getPath() << std::endl;
        break;
      } else {
        download = downloader.getDownload(followingId);
      }
    } else if (download->getStatus() == kiwix::Download::K_ACTIVE) {
      std::cout << download->getDid() << " : "
                << kiwix::beautifyFileSize(download->getCompletedLength()) << "/"
                << kiwix::beautifyFileSize(download->getTotalLength())
                << " (" << kiwix::beautifyFileSize(download->getDownloadSpeed()) << "/s) "
                << " [" << kiwix::beautifyFileSize(download->getVerifiedLength()) << "]"
                << "[" << step << "]    \n" << std::flush;
    } else if (download->getStatus() == kiwix::Download::K_ERROR) {
      std::cout << "File Error" << std::endl;
      exitCode = true;
      break;
    }
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }

  downloader.close();
  return exitCode;
}

int main(int argc, char** argv)
{
  string libraryPath = "";
  supportedAction action = NONE;
  kiwix::Library library;

  /* Argument parsing */
  if (argc > 2) {
    libraryPath = argv[1];
    string actionString = argv[2];

    if (actionString == "add")
      action = ADD;
    else if (actionString == "show")
      action = SHOW;
    else if (actionString == "remove" || actionString == "delete")
      action = REMOVE;
    else if (actionString == "download")
      action = DOWNLOAD;
  }

  /* Print usage)) if necessary */
  if (libraryPath == "" || action == NONE) {
    usage();
    exit(1);
  }

  /* Try to read the file */
  libraryPath = isRelativePath(libraryPath)
                    ? computeAbsolutePath(getCurrentDirectory(), libraryPath)
                    : libraryPath;
  kiwix::Manager manager(&library);
  manager.readFile(libraryPath, false);

  /* SHOW */
  bool exitCode = 0;
  switch (action) {
    case SHOW:
      exitCode = handle_show(&library, libraryPath, argc, argv);
      break;
    case ADD:
      exitCode = handle_add(&library, libraryPath, argc, argv);
      break;
    case REMOVE:
      exitCode = handle_remove(&library, libraryPath, argc, argv);
      break;
    case DOWNLOAD:
      exitCode = handle_download(&library, libraryPath, argc, argv);
      break;
    case NONE:
      break;
  }

  /* Rewrite the library file */
  if (action == REMOVE || action == ADD || action == DOWNLOAD) {
    library.writeToFile(libraryPath);
  }

  exit(exitCode);
}
