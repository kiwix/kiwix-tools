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
#include <pthread.h>
#include <regex.h>
#include <kiwix/reader.h>

using namespace std;

static const string HTMLScripts = " \
<style type=\"text/css\"> \n \
\n \
#topbar{ \n \
  position: absolute; \n \
  right: 0%; \n \
  border: 1px solid #BBBBBB; \n \
  border-radius: 5px; \n \
  -moz-border-radius: 5px; \n \
  -webkit-border-radius: 5px; \n \
  margin-top: 0px; \n \
  margin-right: 5px;  \n \
  padding: 5px; \n \
  font-weight: bold; \n \
  background: #FFFFFF; \n \
  visibility: hidden; \n \
  z-index: 100; \n \
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
if (window.addEventListener) \n \
  window.addEventListener(\"load\", staticbar, false) \n \
else if (window.attachEvent) \n \
  window.attachEvent(\"onload\", staticbar) \n \
else if (document.getElementById) \n \
  window.onload=staticbar \n \
</script> \n \
";

static const string HTMLDiv = " \
<div id=\"topbar\"> \n \
Search <input type=\"textbox\" />\n \
</div> \n \
";

static kiwix::Reader* reader;
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

static void appendToFirstOccurence(string &content, const string regex, const string &replacement) {
  regmatch_t matchs[1];
  regex_t regexp;

  regcomp(&regexp, regex.data(), REG_ICASE);
  if (!regexec(&regexp, content.data(), 1, matchs, 0)) {
    if (matchs[0].rm_so > 0)
      content.insert(matchs[0].rm_eo, replacement);
  }

  regfree(&regexp);
}

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
  struct MHD_Response *response;
  string content = "";
  string mimeType = "";
  unsigned int contentLength = 0;

  /* Mutex lock */
  pthread_mutex_lock(&lock);

  /* Load the article from the ZIM file */
  cout << "Loading '" << url << "'... " << endl;
  try {
    reader->getContent(url, content, contentLength, mimeType);
    cout << "content size: " << contentLength << endl;
    cout << "mimeType: " << mimeType << endl;
    
    /* Rewrite the content */
    if (mimeType == "text/html") {
      appendToFirstOccurence(content, "<head>", HTMLScripts);
      appendToFirstOccurence(content, "<body[^>]*>", HTMLDiv);
      contentLength = content.size();
    }
  } catch (const std::exception& e) {
	std::cerr << e.what() << std::endl;
  }
  
  /* Mutex unlock */
  pthread_mutex_unlock(&lock);

  /* clear context pointer */
  *ptr = NULL;
  
  /* Create the response */
  response = MHD_create_response_from_data(contentLength,
					   (void *)content.data(),
					   MHD_NO,
					   MHD_YES);

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
  void *page;

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
    reader = new kiwix::Reader(zimPath);
  } catch (...) {
    cout << "Unable to open the ZIM file '" << zimPath << "'." << endl; 
    exit(1);
  }

  /* Mutex init */
  pthread_mutex_init(&lock, NULL);

  /* Run endless */
  while (42) sleep(1);
  
  /* Stop the daemon */
  MHD_stop_daemon(daemon);

  /* Mutex destroy */
  pthread_mutex_destroy(&lock);

  exit(0);
}
