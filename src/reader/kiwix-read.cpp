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

#include <getopt.h>
#include <kiwix/reader.h>
#include <map>
#include <string>

void usage()
{
  cout << "Usage: kiwix-read --suggest=<PATTERN> ZIM_FILE_PATH" << endl;
  exit(1);
}

int main(int argc, char** argv)
{
  /* Init the variables */
  const char* filePath = NULL;
  const char* pattern = NULL;
  int option_index = 0;
  int c = 0;

  kiwix::Reader* reader = NULL;

  /* Argument parsing */
  while (42) {
    static struct option long_options[]
        = {{"verbose", no_argument, 0, 'v'},
           {"suggest", required_argument, 0, 's'},
           {0, 0, 0, 0}};

    if (c != -1) {
      c = getopt_long(argc, argv, "vs:", long_options, &option_index);

      switch (c) {
        case 'v':
          break;
        case 's':
          pattern = optarg;
          break;
      }
    } else {
      if (optind < argc) {
        if (filePath == NULL) {
          filePath = argv[optind++];
        } else {
          usage();
        }
      } else {
        break;
      }
    }
  }

  /* Check if we have enough arguments */
  if (filePath == NULL) {
    usage();
  }

  /* Instanciate the reader */
  reader = new kiwix::Reader(filePath);

  /* Start to read an article */
  if (reader != NULL) {
    string content;
    string contentType;
    string suggestion;

    if (pattern != NULL) {
      std::cout << "Searching suggestions for: " << pattern << std::endl;

      reader->searchSuggestionsSmart(pattern, 10);
      while (reader->getNextSuggestion(suggestion)) {
        std::cout << suggestion << std::endl;
      }
    }

    delete reader;
  } else {
    cerr << "Unable instanciate the Kiwix reader." << endl;
    exit(1);
  }

  exit(0);
}
