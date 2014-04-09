/*
 * Copyright 2009-2014 Emmanuel Engelhart <kelson@kiwix.org>
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

#define KIWIX_MIN_CONTENT_SIZE_TO_DEFLATE 100

#ifdef __APPLE__
#import <sys/types.h>
#import <sys/sysctl.h>
#define MIBSIZE 4
#endif

#ifdef _WIN32

#if (_MSC_VER < 1600)
#include "stdint4win.h"
#endif
#include <winsock2.h>
#include <WS2tcpip.h> // otherwise socklen_t is not a recognized type
//#include <Windows.h> // otherwise int is not a recognized type
typedef int off_t;
typedef SSIZE_T ssize_t;
typedef UINT64 uint64_t;
typedef UINT16 uint16_t;
extern "C" {
#include <microhttpd.h>
}

#endif

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <getopt.h>
#include <iostream>
#include <string>
#include <map>
#include <fstream>
#include <iostream>
#include <sstream>
#include <zim/zim.h>
#include <zim/file.h>
#include <zim/article.h>
#include <zim/fileiterator.h>
#include <pthread.h>
#include <zlib.h>
#include <kiwix/reader.h>
#include <kiwix/manager.h>
#include <kiwix/xapianSearcher.h>
#include <pathTools.h>
#include <regexTools.h>
#include <stringTools.h>
#include <otherTools.h>
#include <resourceTools.h>

#ifndef _WIN32
#include <stdint.h>
#include <unistd.h>
#include <microhttpd.h>
#endif

using namespace std;

static bool nosearchbarFlag = false;
static string welcomeHTML;
static bool verboseFlag = false;
static std::map<std::string, kiwix::Reader*> readers;
static std::map<std::string, kiwix::Searcher*> searchers;
static pthread_mutex_t readerLock = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t mapLock = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t welcomeLock = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t searcherLock = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t compressorLock = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t resourceLock = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t verboseFlagLock = PTHREAD_MUTEX_INITIALIZER;

void introduceTaskbar(string &content, const string &humanReadableBookId) {
  pthread_mutex_lock(&resourceLock);
  if (!nosearchbarFlag) {
    content = appendToFirstOccurence(content, "<head>", getResourceAsString("jqueryui/include.html.part"));
    content = appendToFirstOccurence(content, "<head>", "<style>" +
				     getResourceAsString("server/taskbar.css") + "</style>");
    std::string HTMLDivRewrited = replaceRegex(getResourceAsString("server/taskbar.html.part"),
					       humanReadableBookId, "__CONTENT__");
    content = appendToFirstOccurence(content, "<body[^>]*>", HTMLDivRewrited);
  }
  pthread_mutex_unlock(&resourceLock);
}

/* Should display debug information? */
bool isVerbose() {
  bool value;
  pthread_mutex_lock(&verboseFlagLock);
  value = verboseFlag;
  pthread_mutex_unlock(&verboseFlagLock);
  return value;
}

/* For compression */
#define COMPRESSOR_BUFFER_SIZE 10000000
static Bytef *compr = (Bytef *)malloc(COMPRESSOR_BUFFER_SIZE);
static uLongf comprLen;

static int accessHandlerCallback(void *cls,
				 struct MHD_Connection * connection,
				 const char * url,
				 const char * method,
				 const char * version,
				 const char * upload_data,
				 size_t * upload_data_size,
				 void ** ptr) {

  /* Unexpected method */
  if (0 != strcmp(method, "GET") && 0 != strcmp(method, "POST"))
    return MHD_NO;

  /* The first time only the headers are valid, do not respond in the first round... */
  static int dummy;
  if (&dummy != *ptr) {
    *ptr = &dummy;
    return MHD_YES;
  }

  /* Debug */
  if (isVerbose()) {
    std::cout << "Requesting " << url << std::endl;
  }

  /* Check if the response can be compressed */
  const string acceptEncodingHeaderValue = MHD_lookup_connection_value(connection, MHD_HEADER_KIND, MHD_HTTP_HEADER_ACCEPT_ENCODING) ?
    MHD_lookup_connection_value(connection, MHD_HEADER_KIND, MHD_HTTP_HEADER_ACCEPT_ENCODING) : "";
  const bool acceptEncodingDeflate = !acceptEncodingHeaderValue.empty() && acceptEncodingHeaderValue.find("deflate") != string::npos;

  /* Check if range is requested */
  const string acceptRangeHeaderValue = MHD_lookup_connection_value(connection, MHD_HEADER_KIND, MHD_HTTP_HEADER_RANGE) ?
    MHD_lookup_connection_value(connection, MHD_HEADER_KIND, MHD_HTTP_HEADER_RANGE) : "";
  const bool acceptRange = !acceptRangeHeaderValue.empty();

  /* Prepare the variables */
  struct MHD_Response *response;
  std::string content;
  std::string mimeType;
  std::string httpRedirection;
  unsigned int contentLength = 0;
  bool found = true;
  int httpResponseCode = MHD_HTTP_OK;
  std::string urlStr = string(url);

  /* Get searcher and reader */
  std::string humanReadableBookId = "";
  if (!(urlStr.size() > 5 && urlStr.substr(0, 6) == "/skin/")) {
    if (!strcmp(url, "/search") || !strcmp(url, "/suggest")) {
      const char* tmpGetValue = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "content");
      humanReadableBookId = (tmpGetValue != NULL ? string(tmpGetValue) : "");
    } else {
      humanReadableBookId = urlStr.substr(1, urlStr.find("/", 1) != string::npos ? urlStr.find("/", 1) - 1 : urlStr.size() - 2);
      if (!humanReadableBookId.empty()) {
	urlStr = urlStr.substr(urlStr.find("/", 1) != string::npos ? urlStr.find("/", 1) : humanReadableBookId.size());
      }
    }
  }

  pthread_mutex_lock(&mapLock);
  kiwix::Searcher *searcher = searchers.find(humanReadableBookId) != searchers.end() ?
    searchers.find(humanReadableBookId)->second : NULL;
  kiwix::Reader *reader = readers.find(humanReadableBookId) != readers.end() ?
    readers.find(humanReadableBookId)->second : NULL;
  if (reader == NULL) {
    humanReadableBookId="";
  }

  pthread_mutex_unlock(&mapLock);

  /* Get suggestions */
  if (!strcmp(url, "/suggest") && reader != NULL) {
    unsigned int maxSuggestionCount = 10;
    unsigned int suggestionCount = 0;
    std::string suggestion;

    /* Get the suggestion pattern from the HTTP request */
    const char* cTerm = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "term");
    std::string term = cTerm == NULL ? "" : cTerm;
    if (isVerbose()) {
      std::cout << "Searching suggestions for: \"" << term << "\"" << endl;
    }

    /* Get the suggestions */
    content = "[";
    reader->searchSuggestionsSmart(term, maxSuggestionCount);
    while (reader->getNextSuggestion(suggestion)) {
      kiwix::stringReplacement(suggestion, "\"", "\\\"");
      content += (content == "[" ? "" : ",");
      content += "{\"value\":\"" + suggestion + "\",\"label\":\"" + suggestion + "\"}";
      suggestionCount++;
    }

    content += (suggestionCount == 0 ? "" : ",");
    content += "{\"value\":\"" + std::string(term) + " \", \"label\":\"containing '" + std::string(term) + "'...\"}]";
    mimeType = "text/x-json; charset=utf-8";
  }

  /* Get static skin stuff */
  else if (urlStr.size() > 5 && urlStr.substr(0, 6) == "/skin/") {
    content = getResourceAsString(urlStr.substr(6));
  }

  /* Display the search restults */
  else if (!strcmp(url, "/search")) {

    /* Retrieve the pattern to search */
    const char* pattern = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "pattern");
      if (pattern == NULL)
	pattern = "";
    std::string patternString = kiwix::urlDecode(string(pattern));

    /* Try first to load directly the article if exactly matching the pattern */
    std::string patternCorrespondingUrl;
    if (reader != NULL) {
      pthread_mutex_lock(&readerLock);
      reader->getPageUrlFromTitle(patternString, patternCorrespondingUrl);
      pthread_mutex_unlock(&readerLock);
      if (!patternCorrespondingUrl.empty()) {
	httpRedirection="/" + humanReadableBookId + "/" + patternCorrespondingUrl;
      }
    }

    /* Make the search */
    if (patternCorrespondingUrl.empty() && searcher != NULL) {
      const char* start = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "start");
      const char* end = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "end");
      unsigned int startNumber = 0;
      unsigned int endNumber = 25;

      if (start != NULL)
	startNumber = atoi(start);

      if (end != NULL)
	endNumber = atoi(end);

      /* Get the results */
      pthread_mutex_lock(&searcherLock);
      try {
	searcher->search(patternString, startNumber, endNumber, isVerbose());
	content = searcher->getHtml();
      } catch (const std::exception& e) {
	std::cerr << e.what() << std::endl;
      }
      pthread_mutex_unlock(&searcherLock);

      mimeType = "text/html; charset=utf-8";
    } else {
      content = "<!DOCTYPE html>\n<html><head><meta content=\"text/html;charset=UTF-8\" http-equiv=\"content-type\" /><title>Fulltext search unavailable</title></head><body><h1>Not Found</h1><p>There is no article with the title <b>\"" + patternString + "\"</b> and the fulltext search engine is not available for this content.</p></body></html>";
      mimeType = "text/html";
      httpResponseCode = MHD_HTTP_NOT_FOUND;
    }
  }

  /* Display the content of a ZIM article */
  else if (reader != NULL) {
    pthread_mutex_lock(&readerLock);
    try {
      found = reader->getContentByUrl(urlStr, content, contentLength, mimeType);
      if (found) {
	if (isVerbose()) {
	  cout << "Found " << urlStr << endl;
	  cout << "content size: " << contentLength << endl;
	  cout << "mimeType: " << mimeType << endl;
	}
      } else {
	if (isVerbose())
	  cout << "Failed to find " << urlStr << endl;

	content = "<!DOCTYPE html>\n<html><head><meta content=\"text/html;charset=UTF-8\" http-equiv=\"content-type\" /><title>Content not found</title></head><body><h1>Not Found</h1><p>The requested URL " + urlStr + " was not found on this server.</p></body></html>";
	mimeType = "text/html";
	httpResponseCode = MHD_HTTP_NOT_FOUND;
      }
    } catch (const std::exception& e) {
      std::cerr << e.what() << std::endl;
    }
    pthread_mutex_unlock(&readerLock);

    /* Special rewrite URL in case of ZIM file use intern *asbolute* url like /A/Kiwix */
    if (mimeType.find("text/html") != string::npos) {
      content = replaceRegex(content, "$1$2" + humanReadableBookId + "/$3/",
		   "(href|src)(=[\"|\']{0,1}/)([A-Z|\\-])/");
      content = replaceRegex(content, "$1$2" + humanReadableBookId + "/$3/",
		   "(@import[ ]+)([\"|\']{0,1}/)([A-Z|\\-])/");
    } else if (mimeType.find("text/css") != string::npos) {
      content = replaceRegex(content, "$1$2" + humanReadableBookId + "/$3/",
		   "(url|URL)(\\([\"|\']{0,1}/)([A-Z|\\-])/");
    }
  }

  /* Display the global Welcome page */
  else {
    pthread_mutex_lock(&welcomeLock);
    content = welcomeHTML;
    mimeType = "text/html; charset=utf-8";
    pthread_mutex_unlock(&welcomeLock);
  }

  /* Introduce Taskbar */
  if (!humanReadableBookId.empty() && mimeType.find("text/html") != string::npos) {
    introduceTaskbar(content, humanReadableBookId);
  }

  /* Compute the lengh */
  contentLength = content.size();

  /* Should be deflate */
  bool deflated =
    contentLength > KIWIX_MIN_CONTENT_SIZE_TO_DEFLATE &&
    contentLength < COMPRESSOR_BUFFER_SIZE &&
    acceptEncodingDeflate &&
    mimeType.find("text/") != string::npos;

  /* Compress the content if necessary */
  if (deflated) {
    pthread_mutex_lock(&compressorLock);
    comprLen = COMPRESSOR_BUFFER_SIZE;
    compress(compr, &comprLen, (const Bytef*)(content.data()), contentLength);

    if (comprLen > 2 && comprLen < contentLength) {

      /* /!\ Internet Explorer has a bug with deflate compression.
	 It can not handle the first two bytes (compression headers)
	 We need to chunk them off (move the content 2bytes)
	 It has no incidence on other browsers
	 See http://www.subbu.org/blog/2008/03/ie7-deflate-or-not and comments */
      compr += 2;

      content = string((char *)compr, comprLen);
      contentLength = comprLen;
    } else {
      deflated = false;
    }

    pthread_mutex_unlock(&compressorLock);
  }

  /* Create the response */
  response = MHD_create_response_from_data(contentLength,
					   (void *)content.data(),
					   MHD_NO,
					   MHD_YES);

  /* Make a redirection if necessary otherwise send the content */
  if (!httpRedirection.empty()) {
    MHD_add_response_header(response, MHD_HTTP_HEADER_LOCATION, httpRedirection.c_str());
    httpResponseCode = MHD_HTTP_FOUND;
  } else {
    /* Add if necessary the content-encoding */
    if (deflated) {
      MHD_add_response_header(response, MHD_HTTP_HEADER_CONTENT_ENCODING, "deflate");
    }

    /* Tell the client that byte ranges are accepted */
    MHD_add_response_header(response, MHD_HTTP_HEADER_ACCEPT_RANGES, "bytes");

    /* Specify the mime type */
    MHD_add_response_header(response, MHD_HTTP_HEADER_CONTENT_TYPE, mimeType.c_str());
  }

  /* clear context pointer */
  *ptr = NULL;

  /* Force to close the connection - cf. 100% CPU usage with v. 4.4 (in Lucid) */
  MHD_add_response_header(response, MHD_HTTP_HEADER_CONNECTION, "close");

  /* Force cache */
  MHD_add_response_header(response, MHD_HTTP_HEADER_CACHE_CONTROL, "max-age=87840, must-revalidate");

  /* Queue the response */
  int ret = MHD_queue_response(connection,
			   httpResponseCode,
			   response);
  MHD_destroy_response(response);

  return ret;
}

int main(int argc, char **argv) {
  struct MHD_Daemon *daemon;
  string zimPath;
  string libraryPath;
  string indexPath;
  string rootPath;
  int serverPort = 80;
  int daemonFlag = false;
  int libraryFlag = false;
  string PPIDString;
  unsigned int PPID = 0;
  kiwix::Manager libraryManager;

  /* Argument parsing */
  while (42) {

    static struct option long_options[] = {
      {"daemon", no_argument, 0, 'd'},
      {"verbose", no_argument, 0, 'v'},
      {"library", no_argument, 0, 'l'},
      {"nosearchbar", no_argument, 0, 'n'},
      {"index", required_argument, 0, 'i'},
      {"attachToProcess", required_argument, 0, 'a'},
      {"port", required_argument, 0, 'p'},
      {0, 0, 0, 0}
    };

    int option_index = 0;
    int c = getopt_long(argc, argv, "ndvli:a:p:", long_options, &option_index);

    if (c != -1) {

      switch (c) {
        case 'd':
	  daemonFlag = true;
	  break;

	case 'v':
	  verboseFlag = true;
	  break;

	case 'l':
	  libraryFlag = true;
	  break;

	case 'n':
	  nosearchbarFlag = true;
	  break;

	case 'i':
	  indexPath = optarg;
	  break;

	case 'p':
	  serverPort = atoi(optarg);
	  break;

        case 'a':
	  PPIDString = string(optarg);
	  PPID = atoi(optarg);
	  break;
      }
    } else {
      if (optind < argc) {
	if (libraryFlag)
	  libraryPath = argv[optind++];
	else
	  zimPath = argv[optind++];
      }
      break;
    }

  }

  /* Print usage)) if necessary */
  if (zimPath.empty() && libraryPath.empty()) {
    cerr << "Usage: kiwix-serve [--index=INDEX_PATH] [--port=PORT] [--verbose] [--nosearchbar] [--daemon] [--attachToProcess=PID] ZIM_PATH" << endl;
    cerr << "       kiwix-serve --library [--port=PORT] [--verbose] [--daemon] [--nosearchbar] [--attachToProcess=PID] LIBRARY_PATH" << endl;
    exit(1);
  }

  /* Setup the library manager and get the list of books */
  if (libraryFlag) {
    vector<string> libraryPaths = kiwix::split(libraryPath, ";");
    vector<string>::iterator itr;
    for ( itr = libraryPaths.begin(); itr != libraryPaths.end(); ++itr ) {
      if (!(*itr).empty()) {
	bool retVal = false;
	try {
	  retVal = libraryManager.readFile(*itr, true);
	} catch (...) {
	  retVal = false;
	}

	if (!retVal) {
	  cerr << "Unable to open the XML library file '" << *itr << "'." << endl;
	  exit(1);
	}
      }
    }

    /* Check if the library is not empty (or only remote books)*/
    if (libraryManager.getBookCount(true, false)==0) {
      cerr << "The XML library file '" << libraryPath << "' is empty (or has only remote books)." << endl;
    }
  } else {
    if (!libraryManager.addBookFromPath(zimPath, zimPath, "", false)) {
      cerr << "Unable to add the ZIM file '" << zimPath << "' to the internal library." << endl;
      exit(1);
    } else if (!indexPath.empty()) {
      vector<string> booksIds = libraryManager.getBooksIds();
      kiwix::supportedIndexType indexType = kiwix::UNKNOWN;

      /* Try with the XapianSearcher */
      try {
	new kiwix::XapianSearcher(indexPath);
	indexType = kiwix::XAPIAN;
      } catch (...) {
      }

      libraryManager.setBookIndex(booksIds[0], indexPath, indexType);
    }
  }

  /* Instance the readers and searcher and build the corresponding maps */
  vector<string> booksIds = libraryManager.getBooksIds();
  vector<string>::iterator itr;
  kiwix::Book currentBook;
  for ( itr = booksIds.begin(); itr != booksIds.end(); ++itr ) {
    bool zimFileOk = false;
    libraryManager.getBookById(*itr, currentBook);
    zimPath = currentBook.pathAbsolute;

    if (!zimPath.empty()) {
      indexPath = currentBook.indexPathAbsolute;

      /* Instanciate the ZIM file handler */
      kiwix::Reader *reader = NULL;
      try {
	reader = new kiwix::Reader(zimPath);
	zimFileOk = true;
      } catch (...) {
	cerr << "Unable to open the ZIM file '" << zimPath << "'." << endl;
      }

      if (zimFileOk) {
	string humanReadableId = currentBook.getHumanReadableIdFromPath();
	readers[humanReadableId] = reader;

	/* Instanciate the ZIM index (if necessary) */
	kiwix::Searcher *searcher = NULL;
	if (!indexPath.empty()) {
	  bool hasSearchIndex = false;

	  /* Try to load the search */
	  try {
	    if (currentBook.indexType == kiwix::XAPIAN) {
	      searcher = new kiwix::XapianSearcher(indexPath);
	    } else {
	      throw("Unknown index type");
	    }
	    hasSearchIndex = true;
	  } catch (...) {
	    cerr << "Unable to open the search index '" << indexPath << "'." << endl;
	  }

	  if (hasSearchIndex) {
	    searcher->setProtocolPrefix("/");
	    searcher->setSearchProtocolPrefix("/search?");
	    searcher->setContentHumanReadableId(humanReadableId);
	    searchers[humanReadableId] = searcher;
	  }
	}
      }
    }
  }

  /* Compute the Welcome HTML */
  string welcomeBooksHtml;
  for ( itr = booksIds.begin(); itr != booksIds.end(); ++itr ) {
    libraryManager.getBookById(*itr, currentBook);
    if (!currentBook.path.empty() && readers.find(currentBook.getHumanReadableIdFromPath()) != readers.end()) {
      welcomeBooksHtml += "<h3><a href=\"#\">" + currentBook.title + "</a></h3> \
                           <table style=\"overflow-x: hidden; overflow-y: hidden; margin-top: 0px; margin-bottom: 0px; padding-top: 0px; padding-bottom: 0px;\"><tr> \
                             <td style=\"background-repeat: no-repeat; background-image: url(data:" + currentBook.faviconMimeType+ ";base64," + currentBook.favicon + ")\"><div style=\"width: 50px\"></div></td> \
                             <td style=\"width: 100%;\">" + currentBook.description +
	                       "<br/><table style=\"font-size: small; color: grey; width: 100%;\">" +
	"<tr><td style=\"width: 50%\">Size: " + kiwix::beautifyInteger(atoi(currentBook.size.c_str())) + " MB (" + kiwix::beautifyInteger(atoi(currentBook.articleCount.c_str())) + " articles, " + kiwix::beautifyInteger(atoi(currentBook.mediaCount.c_str())) + " medias) \
                                  </td><td>Created: " + currentBook.date + "</td><td style=\"vertical-align: bottom;\" rowspan=\"3\"><form action=\"/" + currentBook.getHumanReadableIdFromPath() + "/\" method=\"GET\"><input style=\"align: right; right: 0px; float:right;\" type=\"submit\" value=\"Load\" /></form></td></tr>					\
                                  <tr><td>Author: " + currentBook.creator + "</td><td>Language: " + currentBook.language + "</td></tr> \
                                  <tr><td>Publisher: " + (currentBook.publisher.empty() ? "unknown" :  currentBook.publisher ) + "</td><td></td></tr> \
                                </table> \
                             </td></tr> \
                            </table>";
    }
  }

  welcomeHTML = replaceRegex(getResourceAsString("server/home.html.tmpl"), welcomeBooksHtml, "__BOOKS__");

#ifndef _WIN32
  /* Fork if necessary */
  if (daemonFlag) {
    pid_t pid;

    /* Fork off the parent process */
    pid = fork();
    if (pid < 0) {
      exit(1);
    }
    /* If we got a good PID, then
       we can exit the parent process. */
    if (pid > 0) {
      exit(0);
    }
  }
#endif

  /* Mutex init */
  pthread_mutex_init(&readerLock, NULL);
  pthread_mutex_init(&mapLock, NULL);
  pthread_mutex_init(&welcomeLock, NULL);
  pthread_mutex_init(&searcherLock, NULL);
  pthread_mutex_init(&compressorLock, NULL);
  pthread_mutex_init(&resourceLock, NULL);
  pthread_mutex_init(&verboseFlagLock, NULL);

  /* Start the HTTP daemon */
  void *page = NULL;
  daemon = MHD_start_daemon(MHD_USE_SELECT_INTERNALLY,
			    serverPort,
			    NULL,
			    NULL,
			    &accessHandlerCallback,
			    page,
			    MHD_OPTION_END);

  if (daemon == NULL) {
    cerr << "Unable to instanciate the HTTP daemon. The port " << serverPort << " is maybe already occupied or need more permissions to be open. Please try as root or with a port number higher or equal to 1024." << endl;
    exit(1);
  }

  /* Run endless */
  bool waiting = true;
  do {
    if (PPID > 0) {
#ifdef _WIN32
      HANDLE process = OpenProcess(SYNCHRONIZE, FALSE, PPID);
      DWORD ret = WaitForSingleObject(process, 0);
      CloseHandle(process);
      if (ret == WAIT_TIMEOUT) {
#elif __APPLE__
	int mib[MIBSIZE];
	struct kinfo_proc kp;
	size_t len = sizeof(kp);

	mib[0]=CTL_KERN;
	mib[1]=KERN_PROC;
	mib[2]=KERN_PROC_PID;
	mib[3]=PPID;

	int ret = sysctl(mib, MIBSIZE, &kp, &len, NULL, 0);
      if (ret != -1 && len > 0) {
#else /* Linux & co */
      string procPath = "/proc/" + string(PPIDString);
      if (access(procPath.c_str(), F_OK) != -1) {
#endif
      } else {
	waiting = false;
      }
    }

    kiwix::sleep(1000);
  } while (waiting);

  /* Stop the daemon */
  MHD_stop_daemon(daemon);

  /* Mutex destroy */
  pthread_mutex_destroy(&readerLock);
  pthread_mutex_destroy(&searcherLock);
  pthread_mutex_destroy(&compressorLock);
  pthread_mutex_destroy(&resourceLock);
  pthread_mutex_destroy(&mapLock);
  pthread_mutex_destroy(&welcomeLock);
  pthread_mutex_destroy(&verboseFlagLock);
  exit(0);
}
