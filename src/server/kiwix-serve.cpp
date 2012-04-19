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

#ifdef __APPLE__
#import <sys/types.h>
#import <sys/sysctl.h>
#define MIBSIZE 4
#endif

#ifdef _WIN32
#include <Windows.h>
#include <stdint4win.h>
#include <winsock2.h>
#include <ws2tcpip.h>
typedef SSIZE_T ssize_t;
typedef int off_t;
#else
#include <stdint.h>
#include <unistd.h>
#include <kiwix/cluceneSearcher.h>
#endif

#include <microhttpd.h>
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
#include <splitString.h>

using namespace std;

static string welcomeHTML;

static const string HTMLScripts = " \
<style type=\"text/css\"> \n \
\n \
#topbar{ \n \
  border: 1px solid #BBBBBB; \n \
  border-radius: 5px; \n \
  -moz-border-radius: 5px; \n \
  -webkit-border-radius: 5px; \n \
  margin: 5px;  \n \
  padding: 5px; \n \
  background: #FFFFFF; \n \
} \n \
\n \
</style> \n \
<script type=\"text/javascript\"> \n \
var persistclose=0 //set to 0 or 1. 1 means once the bar is manually closed, it will remain closed for browser session \n \
//var startX = 30 //set x offset of bar in pixels \n \
var startY = 5 //set y offset of bar in pixels \n \
var verticalpos=\"fromtop\" //enter \"fromtop\" or \"frombottom\" \n \
 \n \
function iecompattest(){ \n \
  return (document.compatMode && document.compatMode!=\"BackCompat\")? document.documentElement : document.body \n \
} \n \
\n \
function staticbar(){ \n \
  barheight=document.getElementById(\"topbar\").offsetHeight \n \
  var ns = (navigator.appName.indexOf(\"Netscape\") != -1) || window.opera; \n \
  var d = document; \n \
  \n \
  function ml(id){ \n \
    var el=d.getElementById(id); \n \
    if (!persistclose || persistclose && get_cookie(\"remainclosed\")==\"\") \n \
      el.style.visibility=\"visible\" \n \
    if(d.layers) \n \
      el.style=el; \n \
    el.sP=function(x,y) { \n \
      this.style.left=x+\"px\"; \n \
      this.style.top=y+\"px\"; \n \
    }; \n \
    el.x = window.innerWidth - el.width - 5; \n \
    if (verticalpos==\"fromtop\") \n \
      el.y = startY; \n \
    else{ \n \
      el.y = ns ? pageYOffset + innerHeight : iecompattest().scrollTop + iecompattest().clientHeight; \n \
      el.y -= startY; \n \
    } \n \
    return el; \n \
  } \n \
  window.stayTopLeft=function(){ \n \
  if (verticalpos==\"fromtop\"){ \n \
    var pY = ns ? pageYOffset : iecompattest().scrollTop; \n \
    ftlObj.y += (pY + startY - ftlObj.y)/8; \n \
  } \n \
  else{ \n \
    var pY = ns ? pageYOffset + innerHeight - barheight: iecompattest().scrollTop + iecompattest().clientHeight - barheight; \n \
    ftlObj.y += (pY - startY - ftlObj.y)/8; \n \
  } \n \
  ftlObj.sP(ftlObj.x, ftlObj.y); \n \
  setTimeout(\"stayTopLeft()\", 10); \n \
} \n \
ftlObj = ml(\"topbar\"); \n \
stayTopLeft(); \n \
} \n \
\n \
/* if (window.addEventListener) \n \
  window.addEventListener(\"load\", staticbar, false) \n \
else if (window.attachEvent) \n \
  window.attachEvent(\"onload\", staticbar) \n \
else if (document.getElementById) \n \
  window.onload=staticbar */\n	     \
</script> \n \
";

static const string HTMLDiv = " \
<div id=\"topbar\"></b><form method=\"GET\" action=\"/search\"><input type=\"textbox\" name=\"pattern\" /><input type=\"hidden\" name=\"content\" value=\"__CONTENT__\"><input type=\"submit\" value=\"Search\" /></form></div> \n \
";

// Urlencode

//based on javascript encodeURIComponent()

string char2hex( char dec )
{
  char dig1 = (dec&0xF0)>>4;
  char dig2 = (dec&0x0F);
  if ( 0<= dig1 && dig1<= 9) dig1+=48;    //0,48inascii
  if (10<= dig1 && dig1<=15) dig1+=97-10; //a,97inascii
  if ( 0<= dig2 && dig2<= 9) dig2+=48;
  if (10<= dig2 && dig2<=15) dig2+=97-10;

  string r;
  r.append( &dig1, 1);
  r.append( &dig2, 1);
  return r;
}

string urlEncode(const string &c) {
  string escaped="";
  int max = c.length();
  for(int i=0; i<max; i++)
    {
      if ( (48 <= c[i] && c[i] <= 57) ||//0-9
	   (65 <= c[i] && c[i] <= 90) ||//abc...xyz
	   (97 <= c[i] && c[i] <= 122) || //ABC...XYZ
	   (c[i]=='~' || c[i]=='!' || c[i]=='*' || c[i]=='(' || c[i]==')' || c[i]=='\'')
	   )
        {
	  escaped.append( &c[i], 1);
        }
      else
        {
	  escaped.append("%");
	  escaped.append( char2hex(c[i]) );//converts char 255 to string "ff"
        }
    }
  return escaped;
}

static bool verboseFlag = false;
static std::map<std::string, kiwix::Reader*> readers;
static std::map<std::string, kiwix::Searcher*> searchers;
static pthread_mutex_t readerLock = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t mapLock = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t welcomeLock = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t searcherLock = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t compressorLock = PTHREAD_MUTEX_INITIALIZER;

/* For compression */
#define COMPRESSOR_BUFFER_SIZE 5000000
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
  if (0 != strcmp(method, "GET"))
    return MHD_NO;
  
  /* The first time only the headers are valid, do not respond in the first round... */
  static int dummy;
  if (&dummy != *ptr) {
    *ptr = &dummy;
    return MHD_YES;
  }

  /* Check if the response can be compressed */
  const string acceptEncodingHeaderValue = MHD_lookup_connection_value(connection, MHD_HEADER_KIND, MHD_HTTP_HEADER_ACCEPT_ENCODING) ? 
    MHD_lookup_connection_value(connection, MHD_HEADER_KIND, MHD_HTTP_HEADER_ACCEPT_ENCODING) : "";
  const bool acceptEncodingDeflate = !acceptEncodingHeaderValue.empty() && acceptEncodingHeaderValue.find("deflate") != string::npos;

  /* Prepare the variables */
  struct MHD_Response *response;
  string content = "";
  string mimeType = "";
  unsigned int contentLength = 0;
  bool found = true;
  int httpResponseCode = MHD_HTTP_OK;
  std::string urlStr = string(url);

  /* Get searcher and reader */
  std::string humanReadableBookId = "";
  if (!strcmp(url, "/search")) {
    const char* tmpGetValue = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "content");
    humanReadableBookId = (tmpGetValue != NULL ? string(tmpGetValue) : "");
    cout << humanReadableBookId << endl;
  } else {
    humanReadableBookId = urlStr.substr(1, urlStr.find("/", 1) != string::npos ? urlStr.find("/", 1) - 1 : urlStr.size() - 2);
    if (!humanReadableBookId.empty()) {
      urlStr = urlStr.substr(urlStr.find("/", 1) != string::npos ? urlStr.find("/", 1) : humanReadableBookId.size());
    }
  }

  pthread_mutex_lock(&mapLock);
  kiwix::Searcher *searcher = searchers.find(humanReadableBookId) != searchers.end() ? 
    searchers.find(humanReadableBookId)->second : NULL;
  kiwix::Reader *reader = readers.find(humanReadableBookId) != readers.end() ? 
    readers.find(humanReadableBookId)->second : NULL;
  pthread_mutex_unlock(&mapLock);
  
  /* Display the search restults */
  if (!strcmp(url, "/search") && searcher != NULL) {
    if (searcher != NULL) {
      const char* pattern = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "pattern");
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
	std::string patternString = string(pattern);
	searcher->search(patternString, startNumber, endNumber, verboseFlag);
	content = "<html><head><title>Kiwix search results</title></head><body>\n";
	content += searcher->getHtml();
      } catch (const std::exception& e) {
	std::cerr << e.what() << std::endl;
      }
      pthread_mutex_unlock(&searcherLock);
      
      content += "</body></html>\n";
      mimeType = "text/html; charset=utf-8";
    } else {
      content = "<html><head><title>Error</title></head><body><h1>Unable to find a full text search index for this content.</h1></body></html>";
    }
  } 

  /* Display the content of a ZIM article */
  else if (reader != NULL) {    
    pthread_mutex_lock(&readerLock);
    try {
      found = reader->getContentByUrl(urlStr, content, contentLength, mimeType);
      
      if (found) {
	if (verboseFlag) {
	  cout << "Found " << urlStr << endl;
	  cout << "content size: " << contentLength << endl;
	  cout << "mimeType: " << mimeType << endl;
	}
      } else {
	if (verboseFlag)
	  cout << "Failed to find " << urlStr << endl;
	
	content = "<html><head><title>Content not found</title></head><body><h1>Not Found</h1><p>The requested URL " + urlStr + " was not found on this server.</p></body></html>";
	mimeType = "text/html";
	httpResponseCode = MHD_HTTP_NOT_FOUND;
      }
    } catch (const std::exception& e) {
      std::cerr << e.what() << std::endl;
    }
    pthread_mutex_unlock(&readerLock);

    /* Rewrite the content (add the search box) */
    if (mimeType.find("text/html") != string::npos) {
      /* Special rewrite URL in case of ZIM file use intern *asbolute* url like /A/Kiwix */
      replaceRegex(content, "$1=\"/" + humanReadableBookId + "/$3/", "(href|src)(=\"/)([A-Z|\\-])/");

      if (searcher != NULL) {
	std::string HTMLDivRewrited = HTMLDiv;
	replaceRegex(HTMLDivRewrited, humanReadableBookId, "__CONTENT__");
	appendToFirstOccurence(content, "<head>", HTMLScripts);
	appendToFirstOccurence(content, "<body[^>]*>", HTMLDivRewrited);
      }
    }
  }

  /* Display the global Welcome page */
  else {
    pthread_mutex_lock(&welcomeLock);
    content = welcomeHTML;
    pthread_mutex_unlock(&welcomeLock);
  }
  
  /* Compute the lengh */
  contentLength = content.size();
  
  /* Compress the content if necessary */
  if (acceptEncodingDeflate && mimeType.find("text/html") != string::npos) {
    pthread_mutex_lock(&compressorLock);
    comprLen = COMPRESSOR_BUFFER_SIZE;
    
    compress(compr, &comprLen, (const Bytef*)(content.data()), contentLength);
    
    content = string((char *)compr, comprLen);
    contentLength = comprLen;
    
    pthread_mutex_unlock(&compressorLock);
  }
  
  /* Create the response */
  response = MHD_create_response_from_data(contentLength,
					   (void *)content.data(),
					   MHD_NO,
					   MHD_YES);
  
  /* Add if necessary the content-encoding */
  if (acceptEncodingDeflate && mimeType.find("text/html") != string::npos) {
    MHD_add_response_header(response, "Content-encoding", "deflate");
  }  
  
  /* Specify the mime type */
  MHD_add_response_header(response, "Content-Type", mimeType.c_str());

  /* clear context pointer */
  *ptr = NULL;

  /* Force to close the connection - cf. 100% CPU usage with v. 4.4 (in Lucid) */
  MHD_add_response_header(response, "Connection", "close");

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
  string templatePath;
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
      {"index", required_argument, 0, 'i'},
      {"attachToProcess", required_argument, 0, 'a'},
      {"port", required_argument, 0, 'p'},
      {0, 0, 0, 0}
    };
    
    int option_index = 0;
    int c = getopt_long(argc, argv, "dvli:a:p:", long_options, &option_index);

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
    cerr << "Usage: kiwix-serve [--index=INDEX_PATH] [--port=PORT] [--verbose] [--daemon] [--attachToProcess=PID] ZIM_PATH" << endl;
    cerr << "       kiwix-serve --library [--port=PORT] [--verbose] [--daemon] [--attachToProcess=PID] LIBRARY_PATH" << endl;
    exit(1);
  }
  
  /* Setup the library manager and get the list of books */
  if (libraryFlag) {
    vector<string> libraryPaths = split(libraryPath, ":");
    vector<string>::iterator itr;
    for ( itr = libraryPaths.begin(); itr != libraryPaths.end(); ++itr ) {
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

    /* Check if the library is not empty (or only remote books)*/
    if (libraryManager.getBookCount(true, false)==0) {
      cerr << "The XML library file '" << libraryPath << "' is empty (or has only remote books)." << endl; 
    }
  } else {
    if (!libraryManager.addBookFromPath(zimPath, zimPath, "", false)) {
      cerr << "Unable to add the ZIM file '" << libraryPath << "' to the internal library." << endl; 
      exit(1);
    }
  }

  /* Try to load the result template */

  /* Change the current dir to binary dir */
  /* Non portable linux solution */
  rootPath = getExecutablePath();
  
#ifndef _WIN32
  chdir(removeLastPathElement(rootPath).c_str());
#endif      

  try {
    
#ifdef _WIN32
    const char* pathArray[] = {"chrome\\static\\results.tmpl"};
    std::vector<std::string> templatePaths(pathArray, pathArray+1);
#else
    const char* pathArray[] = {"../share/kiwix/static/results.tmpl", "../../static/results.tmpl", "results.tmpl"};
    std::vector<std::string> templatePaths(pathArray, pathArray+3);
#endif	
    vector<string>::const_iterator templatePathsIt;
    bool templateFound = false;
    for(templatePathsIt=templatePaths.begin(); !templateFound && templatePathsIt != templatePaths.end(); templatePathsIt++) {
      templatePath = computeAbsolutePath(removeLastPathElement(rootPath), *templatePathsIt);
      if (fileExists(templatePath)) {
	templateFound = true;
      }
    }
    if (!templateFound) {
      throw("Unable to find a valid template file.");
    }
  } catch (...) {
    cerr << "Unable to find/open the result template file." << endl; 
    exit(1);
  }

  /* Instance the readers and searcher and build the corresponding maps */
  vector<string> booksIds = libraryManager.getBooksIds();
  vector<string>::iterator itr;
  kiwix::Book currentBook;
  for ( itr = booksIds.begin(); itr != booksIds.end(); ++itr ) {
    libraryManager.getBookById(*itr, currentBook);
    string humanReadableId = currentBook.getHumanReadableIdFromPath();
    zimPath = currentBook.path;

    if (!zimPath.empty()) {
      indexPath = currentBook.indexPath; 
      
      /* Instanciate the ZIM file handler */
      kiwix::Reader *reader = NULL;
      try {
	reader = new kiwix::Reader(zimPath);
      } catch (...) {
	cerr << "Unable to open the ZIM file '" << zimPath << "'." << endl; 
	exit(1);
      }
      readers[humanReadableId] = reader;
      
      /* Instanciate the ZIM index (if necessary) */
      kiwix::Searcher *searcher = NULL;
      if (indexPath != "") {
	bool hasSearchIndex = false;
	
	/* Try with the XapianSearcher */
	try {
	  searcher = new kiwix::XapianSearcher(indexPath);
	  hasSearchIndex = true;
	} catch (...) {
	  cerr << "Unable to open the search index '" << zimPath << "' with the XapianSearcher." << endl; 
	}
	
#ifndef _WIN32
	/* Try with the CluceneSearcher */
	if (!hasSearchIndex) {
	  try {
	    searcher = new kiwix::CluceneSearcher(indexPath);
	  } catch (...) {
	    cerr << "Unable to open the search index '" << zimPath << "' with the CluceneSearcher." << endl; 
	    exit(1);
	  }
	}
#endif
	searcher->setProtocolPrefix("/");
	searcher->setSearchProtocolPrefix("/search");
	searcher->setContentHumanReadableId(humanReadableId);
	searcher->setResultTemplatePath(templatePath);
	searchers[humanReadableId] = searcher;
      }
    }
  }

  /* Compute the Welcome HTML */
  welcomeHTML = "<html><head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\" /><title>Welcome to Kiwix Server</title></head><body>";
  for ( itr = booksIds.begin(); itr != booksIds.end(); ++itr ) {
    libraryManager.getBookById(*itr, currentBook);
    string humanReadableId = currentBook.getHumanReadableIdFromPath();

    if (!currentBook.path.empty()) {
      welcomeHTML += "<p>";
      welcomeHTML += "<h1><a href='/" + humanReadableId + "/'>" + currentBook.title + "</a>(" + currentBook.creator + "/" + currentBook.publisher + ")</h1>";
      welcomeHTML += "<p>" + currentBook.description + "</p>";
      welcomeHTML += "<p><ul>";
      welcomeHTML += "<li>Number of articles: " + currentBook.articleCount + "</li>";
      welcomeHTML += "<li>Number of pictures: " + currentBook.mediaCount + "</li>";
      welcomeHTML += "</ul></p>";
      welcomeHTML += "</p><hr/>";
    }
  }
  welcomeHTML += "</body></html>";

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

#ifdef _WIN32
    Sleep(1000);
#else
    sleep(1);
#endif
  } while (waiting);

  /* Stop the daemon */
  MHD_stop_daemon(daemon);

  /* Mutex destroy */
  pthread_mutex_destroy(&readerLock);
  pthread_mutex_destroy(&searcherLock);
  pthread_mutex_destroy(&compressorLock);

  exit(0);
}
