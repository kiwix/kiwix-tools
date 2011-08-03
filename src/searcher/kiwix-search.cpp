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
#include <kiwix/xapianSearcher.h>
#include <kiwix/cluceneSearcher.h>

enum supportedBackend { XAPIAN, CLUCENE };

void usage() {
    cout << "Usage: kiwix-search [--verbose|-v] [--backend|-b=xapian|clucene] INDEX_PATH SEARCH" << endl;
    exit(1);
}

int main(int argc, char **argv) {

  /* Init the variables */
  //const char *indexPath = "/home/itamar/.www.kiwix.org/kiwix/43k0i1j4.default/6d2e587b-d586-dc6a-dc6a-e4ef035a1495d15c.index";
  //const char *indexPath = "/home/itamar/testindex";
  const char *indexPath = NULL;
  const char *search = NULL;
  bool verboseFlag = false;
  int option_index = 0;
  int c = 0;
  supportedBackend backend = CLUCENE;

  kiwix::Searcher *searcher = NULL;

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
	if (indexPath == NULL) {
	  indexPath = argv[optind++];
	} else if (search == NULL) {
	  search = argv[optind++];
	} else {
	  cout << indexPath << endl;
	  usage();
	}
      } else {
	break;
      }
    }
  }

  /* Check if we have enough arguments */
  if (indexPath == NULL || search == NULL) {
    usage();
  }

  /* Try to prepare the indexing */
  try {
    if (backend == CLUCENE) {
      searcher = new kiwix::CluceneSearcher(indexPath);
    } else {
      searcher = new kiwix::XapianSearcher(indexPath);
    }
  } catch (...) {
    cerr << "Unable to search through index '" << indexPath << "'." << endl;
    exit(1);
  }

  /* Start the indexing */
  if (searcher != NULL) {
    string searchString(search);
    searcher->search(searchString, 0, 10);
    string url;
    string title;
    unsigned int score;
    while (searcher->getNextResult(url, title, score)) {
      cout << title << endl;
    }
    delete searcher;
    kiwix::CluceneSearcher::terminate();
  } else {
    cerr << "Unable instanciate the Kiwix searcher." << endl;
    exit(1);
  }

  exit(0);
}
