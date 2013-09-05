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
#include <iostream>
#include <cstdlib>
#include <pathTools.h>
#include <kiwix/manager.h>

using namespace std;

enum supportedAction { NONE, ADD, SHOW, REMOVE };

void show(kiwix::Library library) {
    std::vector<kiwix::Book>::iterator itr;
    unsigned int inc = 1;
    for ( itr = library.books.begin(); itr != library.books.end(); ++itr ) {
      std::cout << "#" << inc++
		<< std::endl << "id:\t\t" << itr->id
		<< std::endl << "path:\t\t" << itr->path
		<< std::endl << "url:\t\t" << itr->url
		<< std::endl << "title:\t\t" << itr->title
		<< std::endl << "description:\t" << itr->description
		<< std::endl << "creator:\t" << itr->creator
		<< std::endl << "date:\t\t" << itr->date
		<< std::endl << "articleCount:\t" << itr->articleCount
		<< std::endl << "mediaCount:\t" << itr->mediaCount
		<< std::endl << "size:\t\t" << itr->size << " KB"
		<< std::endl << std::endl;
    }
}

void usage() {
    cerr << "Usage:" << endl;
    cerr << "\tkiwix-manage LIBRARY_PATH add ZIM_PATH [--zimPathToSave=../content/foobar.zim] [--current] [--indexBackend=xapian|clucene] [--indexPath=FULLTEXT_IDX_PATH] [--url=http://...metalink]" << endl;
    cerr << "\tkiwix-manage LIBRARY_PATH show [CONTENTID1] [CONTENTID2] ... (show everything if no param.)" << endl;
    cerr << "\tkiwix-manage LIBRARY_PATH remove CONTENTID1 [CONTENTID2]" << endl;
}

int main(int argc, char **argv) {

  string libraryPath = "";
  supportedAction action = NONE;
  string zimPath = "";
  kiwix::Manager libraryManager;
  int option_index = 0;
  int c = 0;

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
  }

  /* Print usage)) if necessary */
  if (libraryPath == "" || action == NONE) {
    usage();
    exit(1);
  }

  /* Try to read the file */
  libraryManager.readFile(libraryPath, false);

  /* SHOW */
  if (action == SHOW) {
    show(libraryManager.cloneLibrary());
  } else if (action == ADD) {
    string zimPath;
    string zimPathToSave = ".";
    string indexPath;
    kiwix::supportedIndexType indexBackend = kiwix::XAPIAN;
    string url;
    string origID="";
    bool setCurrent = false;

    if (argc>3) {
      zimPath = argv[3];
    }

    /* Options parsing */
    optind = 2;
    while (42) {

      static struct option long_options[] = {
	{"url", required_argument, 0, 'u'},
	{"origId", required_argument, 0, 'o'},
	{"indexPath", required_argument, 0, 'i'},
	{"indexBackend", required_argument, 0, 'b'},
	{"zimPathToSave", required_argument, 0, 'z'},
	{"current", no_argument, 0, 'c'},
	{0, 0, 0, 0}
      };

      c = getopt_long(argc, argv, "cz:u:o:i:b:", long_options, &option_index);

      if (c != -1) {

	switch (c) {
        case 'u':
	  url = optarg;
	  break;

        case 'o':
	  origID = optarg;
	  break;

        case 'c':
	  setCurrent = true;
	  break;

	case 'i':
	  indexPath = optarg;
	  break;

	case 'b':
	  if (!strcmp(optarg, "clucene")) {
	    indexBackend = kiwix::CLUCENE;
	  } else if (!strcmp(optarg, "xapian")) {
	    indexBackend = kiwix::XAPIAN;
	  } else {
	    usage();
	  }
	  break;

	case 'z':
	  zimPathToSave = optarg;
	  break;

	}
      } else {
	break;
      }
    }

    if (zimPath != "") {
      zimPathToSave = zimPathToSave == "." ? zimPath : zimPathToSave;
      string bookId = libraryManager.addBookFromPathAndGetId(zimPath, zimPathToSave, url,origID, false);

      if (!bookId.empty()) {

	if (setCurrent)
	  libraryManager.setCurrentBookId(bookId);

	/* Save the index infos if necessary */
	if (!indexPath.empty())
	  libraryManager.setBookIndex(bookId, indexPath, indexBackend);

      } else {
	cerr << "Unable to build or save library file '" << libraryPath << "'" << endl;
      }
    } else {
      std::cerr << "Invalid zim file path" << std::endl;
    }

  } else if (action == REMOVE) {
    unsigned int bookIndex = 0;
    const unsigned int totalBookCount = libraryManager.getBookCount(true, true);

    if (argc>3) {
      bookIndex = atoi(argv[3]);
    }

    if (bookIndex > 0 && bookIndex <= totalBookCount) {
      libraryManager.removeBookByIndex(bookIndex - 1);
    } else {
      if (totalBookCount > 0) {
	std::cerr << "Invalid book index number. Please give a number between 1 and " << totalBookCount << std::endl;
      } else {
	std::cerr << "Invalid book index number. Library is empty, no book to delete." << std::endl;
      }
    }
  }

  /* Rewrite the library file */
  if (action == REMOVE || action == ADD)
    libraryManager.writeFile(libraryPath);

  exit(0);
}
