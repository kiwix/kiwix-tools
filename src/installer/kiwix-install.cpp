/*
 * Copyright 2011-2014 Emmanuel Engelhart <kelson@kiwix.org>
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
#include <kiwix/common/pathTools.h>
#include <kiwix/reader.h>
#include <kiwix/manager.h>

enum supportedAction { NONE, ADDCONTENT };

void usage() {
    cout << "Usage: kiwix-install [--verbose] addcontent ZIM_PATH KIWIX_PATH" << endl;
    exit(1);
}

int main(int argc, char **argv) {

  /* Init the variables */
  const char *contentPath = NULL;
  const char *kiwixPath = NULL;
  supportedAction action = NONE;
  bool verboseFlag = false;
  int option_index = 0;
  int c = 0;

  /* Argument parsing */
  while (42) {

    static struct option long_options[] = {
      {"verbose", no_argument, 0, 'v'},
      {0, 0, 0, 0}
    };

    if (c != -1) {
      c = getopt_long(argc, argv, "vi", long_options, &option_index);

      switch (c) {
	case 'v':
	  verboseFlag = true;
	  break;
      }
    } else {
      if (optind < argc) {
	if (action == NONE) {
	  string actionString = argv[optind++];
	  if (actionString == "addcontent" || actionString == "ADDCONTENT") {
	    action = ADDCONTENT;
	  }
	} else if (contentPath == NULL) {
	  contentPath = argv[optind++];
	} else if (kiwixPath == NULL) {
	  kiwixPath = argv[optind++];
	} else {
	  usage();
	}
      } else {
	break;
      }
    }
  }

  /* Check if we have enough arguments */
  if (contentPath == NULL || kiwixPath == NULL) {
    usage();
  }

  /* Make the action */
  if (action == ADDCONTENT) {

    /* Check if the content path exists and is readable */
    if (verboseFlag) { std::cout << "Check if the ZIM file exists..." << std::endl; }
    if (!fileExists(contentPath)) {
      cerr << "The content path '" << contentPath << "' does not exist or is not readable." << endl;
      exit(1);
    }

    /* Check if this is a ZIM file */
    try {
      if (verboseFlag) { std::cout << "Check if the ZIM file is valid..." << std::endl; }
      kiwix::Reader *reader = new kiwix::Reader(contentPath);
      delete reader;
    } catch (exception &e) {
      cerr << "The content available at '" << contentPath << "' is not a ZIM file." << endl;
      exit(1);
    }
    string contentFilename = getLastPathElement(contentPath);

    /* Check if kiwixPath/kiwix/kiwix.exe exists */
    if (verboseFlag) { std::cout << "Check if the Kiwix path is valid..." << std::endl; }
    string kiwixBinaryPath = computeAbsolutePath(kiwixPath, "kiwix/kiwix.exe");
    if (!fileExists(kiwixBinaryPath)) {
      kiwixBinaryPath = computeAbsolutePath(kiwixPath, "kiwix/kiwix");
      if (!fileExists(kiwixBinaryPath)) {
	cerr << "Unable to find the Kiwix binary at '" << kiwixBinaryPath << "[.exe]'." << endl;
	exit(1);
      }
    }

    /* Check if the directory "data" structure exists */
    if (verboseFlag) { std::cout << "Check the target data directory structure..." << std::endl; }
    string dataPath = computeAbsolutePath(kiwixPath, "data/");
    if (!fileExists(dataPath)) {
      makeDirectory(dataPath);
    }

    /* Check if the directory "data/content" structure exists */
    string dataContentPath = computeAbsolutePath(kiwixPath, "data/content/");
    if (!fileExists(dataContentPath)) {
      makeDirectory(dataContentPath);
    }

    /* Check if the directory "data/library" structure exists */
    string dataLibraryPath = computeAbsolutePath(kiwixPath, "data/library/");
    if (!fileExists(dataLibraryPath)) {
      makeDirectory(dataLibraryPath);
    }

    /* Copy the file to the data/content directory */
    if (verboseFlag) { std::cout << "Copy ZIM file to the target directory..." << std::endl; }
    string newContentPath = computeAbsolutePath(dataContentPath, contentFilename);
    if (!fileExists(newContentPath) || getFileSize(contentPath) != getFileSize(newContentPath)) {
      copyFile(contentPath, newContentPath);
    }

    /* Add the file to the library.xml */
    if (verboseFlag) { std::cout << "Create the library XML file..." << std::endl; }
    kiwix::Manager libraryManager;
    string libraryPath = computeAbsolutePath(dataLibraryPath, contentFilename + ".xml");
    string bookId = libraryManager.addBookFromPathAndGetId(newContentPath, "../content/" + contentFilename, "", false);
    if (!bookId.empty()) {
      libraryManager.setCurrentBookId(bookId);
      libraryManager.writeFile(libraryPath);
    } else {
      cerr << "Unable to build or save library file '" << libraryPath << "'" << endl;
    }
  }

  exit(0);
}
