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
#include <kiwix/xapianIndexer.h>
#include <kiwix/cluceneIndexer.h>

enum supportedBackend { XAPIAN, CLUCENE };

void usage() {
    cout << "Usage: kiwix-index [--verbose|-v] [--backend|-b=xapian|clucene] ZIM_PATH INDEX_PATH" << endl;
    exit(1);
}

int main(int argc, char **argv) {

  /* Init the variables */
  char *zimFilePath = NULL;
  char *indexPath = NULL;
  bool verboseFlag = false;
  int option_index = 0;
  int c = 0;
  supportedBackend backend = XAPIAN;

  kiwix::Indexer *indexer = NULL;

  /* Argument parsing */
  while (42) {

    static struct option long_options[] = {
      {"verbose", no_argument, 0, 'v'},
      {"backend", required_argument, 0, 'b'},
      {0, 0, 0, 0}
    };
    
    if (c != -1) {
      c = getopt_long(argc, argv, "vb:", long_options, &option_index);

      switch (c) {
	case 'v':
	  verboseFlag = true;
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
	if (zimFilePath == NULL) {
	  zimFilePath = argv[optind++];
	} else if (indexPath == NULL) {
	  indexPath = argv[optind++];
	} else {
	  usage();
	}
      } else {
	break;
      }
    }
  }

  /* Check if we have enough arguments */
  if (zimFilePath == NULL || indexPath == NULL) {
    usage();
  }

  /* Try to prepare the indexing */
  try {
    if (backend == CLUCENE) {
      indexer = new kiwix::CluceneIndexer(zimFilePath, indexPath);
    } else {
      indexer = new kiwix::XapianIndexer(zimFilePath, indexPath);
    }
  } catch (...) {
    cerr << "Unable to index '" << zimFilePath << "'." << endl;
    exit(1);
  }

  /* Start the indexing */
  if (indexer != NULL) {
    while (indexer->indexNextPercent(verboseFlag)) {};
    delete indexer;
  } else {
    cerr << "Unable instanciate the Kiwix indexer." << endl;
    exit(1);
  }

  exit(0);
}
