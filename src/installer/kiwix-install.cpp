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
#include <unistd.h>
#include <pathTools.h>
#include <kiwix/xapianIndexer.h>
#include <kiwix/cluceneIndexer.h>

enum supportedBackend { XAPIAN, CLUCENE };
enum supportedAction { NONE, ADDCONTENT };

void usage() {
    cout << "Usage: kiwix-install [--verbose|-v] [--backend|-b=xapian|clucene] [--buildIndex|-i] ADDCONTENT ZIM_PATH KIWIX_PATH" << endl;
    exit(1);
}

int main(int argc, char **argv) {

  /* Init the variables */
  const char *contentPath = NULL;
  const char *kiwixPath = NULL;
  supportedAction action = NONE;
  bool verboseFlag = false;
  bool buildIndexFlag = false;
  int option_index = 0;
  int c = 0;
  supportedBackend backend = XAPIAN;

  /* Argument parsing */
  while (42) {

    static struct option long_options[] = {
      {"verbose", no_argument, 0, 'v'},
      {"buildIndex", no_argument, 0, 'i'},
      {"backend", required_argument, 0, 'b'},
      {0, 0, 0, 0}
    };
    
    if (c != -1) {
      c = getopt_long(argc, argv, "vb:", long_options, &option_index);

      switch (c) {
	case 'v':
	  verboseFlag = true;
	  break;
        case 'i':
	  buildIndexFlag = true;
	  break;
	case 'b':
	  if (!strcmp(optarg, "clucene")) {
	    backend = CLUCENE;
	  } else if (!strcmp(optarg, "xapian")) {
	    backend = XAPIAN;
	  } else {
	    usage();
	  }
	  break;
      }
    } else {
      if (optind < argc) {
	if (action == NONE) {
	  string actionString = argv[optind++];
	  if (actionString == "ADDCONTENT") {
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
    if (!fileExists(contentPath)) {
      cerr << "The content path '" << contentPath << "' does not exist or is not readable." << endl;
      exit(1);
    }
  }

  exit(0);
}
