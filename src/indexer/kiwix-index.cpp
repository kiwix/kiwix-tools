#include <getopt.h>
#include <unistd.h>
#include <kiwix/indexer.h>

void usage() {
    cout << "Usage: kiwix-index [--verbose|-v] ZIM_PATH INDEX_PATH" << endl;
    exit(1);
}

int main(int argc, char **argv) {

  /* Init the variables */
  char *zimFilePath = NULL;
  char *xapianDirectoryPath = NULL;
  bool verboseFlag = false;
  int option_index = 0;
  kiwix::Indexer *indexer = NULL;

  /* Argument parsing */
  while (42) {

    static struct option long_options[] = {
      {"verbose", no_argument, 0, 'v'},
      {0, 0, 0, 0}
    };
    
    int c = getopt_long(argc, argv, "v", long_options, &option_index);

    if (c != -1) {

      switch (c) {
	case 'v':
	  verboseFlag = true;
	  break;
      }
    } else {
      if (optind + option_index < argc) {
	if (zimFilePath == NULL) {
	  zimFilePath = argv[optind + option_index];
	} else if (xapianDirectoryPath == NULL) {
	  xapianDirectoryPath = argv[optind + option_index];
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
  if (zimFilePath == NULL || xapianDirectoryPath == NULL) {
    usage();
  }

  /* Try to prepare the indexing */
  try {
    indexer = new kiwix::Indexer(zimFilePath, xapianDirectoryPath);
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
