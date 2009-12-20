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
  kiwix::Indexer *indexer = new kiwix::Indexer(zimFilePath, xapianDirectoryPath);

  /* Start the indexing */
  if (indexer != NULL) {
    indexer->startIndexing();
    while (indexer->indexNextPercent()) {};
  } else {
    cout << "Unable to start the indexation process" << endl;
    exit(1);
  }

  exit(0);
}
