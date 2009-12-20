#include <kiwix/indexer.h>

int main(int argc, char **argv) {
  
  /* Check if we have enough arguments */
  if (argc < 3) {
    cout << "Usage: kiwix-index ZIM_PATH INDEX_PATH" << endl;
    exit(1);
  }

  /* Init the variables */
  char *zimFilePath = argv[1];
  char *xapianDirectoryPath = argv[2];
  kiwix::Indexer *indexer = NULL;

  /* Try to prepare the indexing */
  try {
    indexer = new kiwix::Indexer(zimFilePath, xapianDirectoryPath);
  } catch (...) {
    cout << "Unable to index '" << zimFilePath << "'." << endl;
    exit(1);
  }

  /* Start the indexing */
  if (indexer != NULL) {
    while (indexer->indexNextPercent()) {};
  } else {
    cout << "Unable instanciate the Kiwix indexer." << endl;
    exit(1);
  }

  exit(0);
}
