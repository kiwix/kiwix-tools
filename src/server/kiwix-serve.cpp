#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <microhttpd.h>
#include <iostream>
#include <string>
#include <zim/zim.h>
#include <zim/file.h>
#include <zim/article.h>
#include <zim/fileiterator.h>

using namespace std;

static zim::File* zimFileHandler;

static int accessHandlerCallback(void *cls,
				 struct MHD_Connection * connection,
				 const char * url,
				 const char * method,
				 const char * version,
				 const char * upload_data,
				 size_t * upload_data_size,
				 void ** ptr) {

  /* Unexpected method */
  if (0 != strcmp(method, "GET"))
    return MHD_NO;
  
  /* The first time only the headers are valid, do not respond in the first round... */
  static int dummy;
  if (&dummy != *ptr) {
    *ptr = &dummy;
    return MHD_YES;
  }
  
  /* Prepare the variable */
  zim::Article article;
  struct MHD_Response * response;
  string content;
  string mimeType;
  unsigned int contentLength = 0;
 
  /* Prepare the url */
  unsigned int urlLength = strlen(url);
  unsigned int offset = 0;

  /* Ignore the '/' */
  while((offset < urlLength) && (url[offset] == '/')) offset++;

  /* Get namespace */
  char ns[1024];
  unsigned int nsOffset = 0;
  while((offset < urlLength) && (url[offset] != '/')) {
    ns[nsOffset] = url[offset];
    offset++;
    nsOffset++;
  }
  ns[nsOffset] = 0;

  /* Ignore the '/' */
  while((offset < urlLength) && (url[offset] == '/')) offset++;  

  /* Get content title */
  char title[1024];
  unsigned int titleOffset = 0;
  while((offset < urlLength) && (url[offset] != '/')) {
    title[titleOffset] = url[offset];
    offset++;
    titleOffset++;
  }
  title[titleOffset] = 0;

  /* Load the article from the ZIM file */
  cout << "Loading '" << title << "' in namespace '" << ns << "'... " << endl;
  try {
    std::pair<bool, zim::File::const_iterator> resultPair = zimFileHandler->findx(ns[0], title);

    /* Test if the article was found */
    if (resultPair.first == true) {
      cout << ns << "/" << title << " found." << endl;

      /* Get the article */
      zim::Article article = zimFileHandler->getArticle(resultPair.second.getIndex());

      /* If redirect */
      unsigned int loopCounter = 0;
      while (article.isRedirect() && loopCounter++<42) {
	article = article.getRedirectArticle();
      }
  
      /* Get the content */
      contentLength = article.getArticleSize();
      content = string(article.getData().data(), article.getArticleSize());
      cout << "content size: " << contentLength << endl;
       
      /* Get the content mime-type */
      mimeType = article.getMimeType();
      cout << "mimeType: " << mimeType << endl;

      /* Snapshot if text/html */
      if (mimeType == "text/html") {
          cout << "Snapshot: " << content.substr(0, 42) << endl; 
	  mimeType = mimeType + "; charset=utf-8";
      }
    } else {
      /* The found article is not the good one */
      content="";
      contentLength = 0;
      cout << ns << "/" << title << "not  found." << endl;
    }
  } catch (const std::exception& e) {
	std::cerr << e.what() << std::endl;
  }

  /* clear context pointer */
  *ptr = NULL;
  
  /* Create the response */
  response = MHD_create_response_from_data(contentLength,
					   (void *)content.data(),
					   MHD_NO,
					   MHD_NO);

  /* Specify the mime type */
  MHD_add_response_header(response, "Content-Type", mimeType.c_str());

  /* Queue the response */
  int ret = MHD_queue_response(connection,
			   MHD_HTTP_OK,
			   response);
  MHD_destroy_response(response);
  return ret;
}

int main(int argc, char **argv) {
  struct MHD_Daemon *daemon;

  /* Argument check */
  if (argc < 3 || argc > 4) {
    cout << "Usage: ZIM_PATH PORT [INDEX_PATH]" << endl;
    exit(1);
  }
  
  string zimPath = (argv[1]);
  int port = atoi(argv[2]);
  void *page = strdup("42222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222");

  /* Start the HTTP daemon */
  daemon = MHD_start_daemon(MHD_USE_THREAD_PER_CONNECTION,
			    port,
			    NULL,
			    NULL,
			    &accessHandlerCallback,
			    page,
			    MHD_OPTION_END);
  
  if (daemon == NULL) {
    cout << "Unable to instanciate the HTTP daemon."<< endl;
    exit(1);
  }

  /* Instanciate the ZIM file handler */
  try {
    zimFileHandler = new zim::File(zimPath);
  } catch (...) {
    cout << "Unable to open the ZIM file '" << zimPath << "'." << endl; 
    exit(1);
  }

  /* Run endless */
  while (42) sleep(1);
  
  /* Stop the daemon */
  MHD_stop_daemon(daemon);

  exit(0);
}
