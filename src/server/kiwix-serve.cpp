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

#ifdef _WIN32
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
#include <kiwix/xapianSearcher.h>
#include <pathTools.h>
#include <regexTools.h>

using namespace std;

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
<div id=\"topbar\"><form method=\"GET\" action=\"/search\"><input type=\"textbox\" name=\"pattern\" /><input type=\"submit\" value=\"Search\" /></form></div> \n \
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
static kiwix::Reader* reader;
static kiwix::Searcher* searcher;
static pthread_mutex_t readerLock = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t searcherLock = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t compressorLock = PTHREAD_MUTEX_INITIALIZER;
static bool hasSearchIndex = false;

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
  const bool acceptEncodingDeflate = (!acceptEncodingHeaderValue.empty() && acceptEncodingHeaderValue.find("deflate") != string::npos ? true : false );

  /* Prepare the variables */
  struct MHD_Response *response;
  string content = "";
  string mimeType = "";
  unsigned int contentLength = 0;
  bool found = true;
  int httpResponseCode = MHD_HTTP_OK;

  if (!strcmp(url, "/search") && hasSearchIndex) {
    const char* pattern = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "pattern");
    const char* start = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "start");
    const char* end = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "end");
    unsigned int startNumber = 0;
    unsigned int endNumber = 25;

    if (start != NULL)
      startNumber = atoi(start);

    if (end != NULL)
      endNumber = atoi(end);

    std::string urlStr;
    std::string titleStr;
    unsigned int scoreInt;
    char scoreStr[4];

    /* Mutex lock */
    pthread_mutex_lock(&searcherLock);

    try {
      std::string patternString = string(pattern);
      searcher->search(patternString, startNumber, endNumber, verboseFlag);
      content = "<html><head><title>Kiwix search results</title></head><body>\n";
      content += searcher->getHtml();
    } catch (const std::exception& e) {
      std::cerr << e.what() << std::endl;
    }

    content += "</body></html>\n";

    mimeType = "text/html; charset=utf-8";

    /* Mutex unlock */
    pthread_mutex_unlock(&searcherLock);

  } else {

    /* urlstr */
    std::string urlStr = string(url);

    /* Mutex Lock */
    pthread_mutex_lock(&readerLock);
    
    /* Load the article from the ZIM file */
    if (verboseFlag)
      cout << "Loading '" << urlStr << "'... " << endl;

    try {
      found = reader->getContentByUrl(urlStr, content, contentLength, mimeType);

      if (verboseFlag) {
	if (found) {
	  cout << "Found " << urlStr << endl;
	  cout << "content size: " << contentLength << endl;
	  cout << "mimeType: " << mimeType << endl;
	} else {
	  cout << "Failed to find " << urlStr << endl;
	  content = "<h1>Not Found</h1><p>The requested URL " + urlStr + " was not found on this server.</p>" ;
	  mimeType = "text/html";
	  httpResponseCode = MHD_HTTP_NOT_FOUND;
	}
      }
    } catch (const std::exception& e) {
      std::cerr << e.what() << std::endl;
    }
    
    /* Mutex unlock */
    pthread_mutex_unlock(&readerLock);
  }

  /* Rewrite the content (add the search box) */
#ifdef _WIN32
  if (hasSearchIndex && mimeType.find("text/html") != string::npos) {
    appendToFirstOccurence(content, "<head>", HTMLScripts);
    appendToFirstOccurence(content, "<body[^>]*>", HTMLDiv);
  }
#endif  

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
  string indexPath;
  string rootPath;
  int serverPort = 80;
  int daemonFlag = false;

  /* Argument parsing */
  while (42) {

    static struct option long_options[] = {
      {"daemon", no_argument, 0, 'd'},
      {"verbose", no_argument, 0, 'v'},
      {"index", required_argument, 0, 'i'},
      {"port", required_argument, 0, 'p'},
      {0, 0, 0, 0}
    };
    
    int option_index = 0;
    int c = getopt_long(argc, argv, "dvi:p:", long_options, &option_index);

    if (c != -1) {

      switch (c) {
        case 'd':
	  daemonFlag = true;
	  break;
	  
	case 'v':
	  verboseFlag = true;
	  break;
	  
	case 'i':
	  indexPath = optarg;
	  break;
	  
	case 'p':
	  serverPort = atoi(optarg);
	  break;
      }
    } else {
      if (optind < argc) {
	zimPath = argv[optind++];
      }
      break;
    }
    
  }

  /* Print usage)) if necessary */
  if (zimPath == "") {
    cerr << "Usage: kiwix-serve [--index=INDEX_PATH] [--port=PORT] [--verbose] [--daemon] ZIM_PATH" << endl;
    exit(1);
  }
  
  void *page;

  /* Instanciate the ZIM file handler */
  try {
    reader = new kiwix::Reader(zimPath);
  } catch (...) {
    cerr << "Unable to open the ZIM file '" << zimPath << "'." << endl; 
    exit(1);
  }

  /* Instanciate the ZIM index (if necessary) */
  if (indexPath != "") {

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
	hasSearchIndex = true;
      } catch (...) {
	cerr << "Unable to open the search index '" << zimPath << "' with the CluceneSearcher." << endl; 
	exit(1);
      }
    }
#endif

    /* searcher configuration */
    if (hasSearchIndex) {

      /* Change the current dir to binary dir */
      /* Non portable linux solution */
      rootPath = getExecutablePath();

#ifndef _WIN32
      chdir(removeLastPathElement(rootPath).c_str());
#endif      

      /* Try to load the result template */
      try {

#ifdef _WIN32
	const char* pathArray[] = {"chrome\\static\\results.tmpl"};
	std::vector<std::string> templatePaths(pathArray, pathArray+1);
#elif APPLE
#else
	const char* pathArray[] = {"../share/kiwix/static/results.tmpl", "../../static/results.tmpl"};
	std::vector<std::string> templatePaths(pathArray, pathArray+2);
#endif	
	vector<string>::const_iterator templatePathsIt;
	bool templateFound = false;
	for(templatePathsIt=templatePaths.begin(); !templateFound && templatePathsIt != templatePaths.end(); templatePathsIt++) {
	  string templatePath = computeAbsolutePath(removeLastPathElement(rootPath), *templatePathsIt);
	  if (fileExists(templatePath)) {
	    searcher->setResultTemplatePath(templatePath);
	    templateFound = true;
	  }
	}
	if (!templateFound) {
	  throw("Unable to find a valid template file.");
	}
      } catch (...) {
	cerr << "Unable to open the result template file." << endl; 
	exit(1);
      }
      
      searcher->setProtocolPrefix("/");
      searcher->setSearchProtocolPrefix("/search?");
    }

  } else {
    hasSearchIndex = false;
  }

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
  pthread_mutex_init(&searcherLock, NULL);
  pthread_mutex_init(&compressorLock, NULL);

  /* Start the HTTP daemon */
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
  while (42) {
#ifdef _WIN32
    Sleep(1000);
#else
    sleep(1);
#endif
  }

  /* Stop the daemon */
  MHD_stop_daemon(daemon);

  /* Mutex destroy */
  pthread_mutex_destroy(&readerLock);
  pthread_mutex_destroy(&searcherLock);
  pthread_mutex_destroy(&compressorLock);

  exit(0);
}
