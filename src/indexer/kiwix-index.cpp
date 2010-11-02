#include <getopt.h>
#include <unistd.h>
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
  supportedBackend backend = XAPIAN;

  kiwix::Indexer *indexer = NULL;

  /* Argument parsing */
  while (42) {

    static struct option long_options[] = {
      {"verbose", no_argument, 0, 'v'},
      {"backend", required_argument, 0, 'b'},
      {0, 0, 0, 0}
    };
    
    int c = getopt_long(argc, argv, "vb:", long_options, &option_index);

    if (c != -1) {

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
      if (argc > 2 && optind + option_index - 1 < argc) {
	if (zimFilePath == NULL) {
	  zimFilePath = argv[optind + option_index - 1];
	} else if (indexPath == NULL) {
	  indexPath = argv[optind + option_index - 1];
	} else {
	  usage();
	}
      } else {
	break;
      }
      option_index++;
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
  } else {
    cerr << "Unable instanciate the Kiwix indexer." << endl;
    exit(1);
  }

  exit(0);
}
