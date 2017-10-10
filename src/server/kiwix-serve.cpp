/*
 * Copyright 2009-2016 Emmanuel Engelhart <kelson@kiwix.org>
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
#import <sys/sysctl.h>
#import <sys/types.h>
#define MIBSIZE 4
#endif

#ifdef _WIN32

#if !defined(__MINGW32__) && (_MSC_VER < 1600)
#include "stdint4win.h"
#endif
#include <winsock2.h>
#include <ws2tcpip.h>  // otherwise socklen_t is not a recognized type
//#include <Windows.h> // otherwise int is not a recognized type
// typedef int off_t;
// typedef SSIZE_T ssize_t;
typedef UINT64 uint64_t;
typedef UINT16 uint16_t;
extern "C" {
#include <microhttpd.h>
}

#endif

#include <getopt.h>
#include <kiwix/common/otherTools.h>
#include <kiwix/common/pathTools.h>
#include <kiwix/common/regexTools.h>
#include <kiwix/common/stringTools.h>
#include <kiwix/manager.h>
#include <kiwix/reader.h>
#include <kiwix/searcher.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zim/article.h>
#include <zim/file.h>
#include <zim/fileiterator.h>
#include <zim/zim.h>
#include <zlib.h>
#include <atomic>
#include <fstream>
#include <iostream>
#include <iostream>
#include <map>
#include <sstream>
#include <thread>
#include <string>
#include <vector>
#include "server-resources.h"

#ifndef _WIN32
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <microhttpd.h>
#include <netdb.h>
#include <stdint.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

#ifdef interface
#undef interface
#endif

using namespace std;

static bool noLibraryButtonFlag = false;
static bool noSearchBarFlag = false;
static string welcomeHTML;
static std::atomic_bool isVerbose(false);
static std::map<std::string, std::string> extMimeTypes;
static std::map<std::string, kiwix::Reader*> readers;
static std::map<std::string, kiwix::Searcher*> searchers;
static kiwix::Searcher* globalSearcher = nullptr;
static pthread_mutex_t searchLock = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t compressorLock = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t regexLock = PTHREAD_MUTEX_INITIALIZER;

/* Try to get the mimeType from the file extension */
static std::string getMimeTypeForFile(const std::string& filename)
{
  std::string mimeType = "text/plain";
  unsigned int pos = filename.find_last_of(".");

  if (pos != std::string::npos) {
    std::string extension = filename.substr(pos + 1);

    auto it = extMimeTypes.find(extension);
    if (it != extMimeTypes.end()) {
      mimeType = it->second;
    } else {
      it = extMimeTypes.find(kiwix::lcAll(extension));
      if (it != extMimeTypes.end()) {
        mimeType = it->second;
      }
    }
  }

  return mimeType;
}


struct RequestContext {
    struct MHD_Connection* connection;
    int httpResponseCode;
    kiwix::Reader* reader;
    kiwix::Searcher* searcher;
    const std::string urlStr;
    const std::string humanReadableBookId;
    bool acceptEncodingDeflate;
    bool acceptRange;
    int range_start;
    int range_end;

    RequestContext(struct MHD_Connection* connection, int httpResponseCode,
                   kiwix::Reader* reader, kiwix::Searcher* searcher,
                   const std::string& urlStr, const std::string& humanReadableBookId,
                   bool acceptEncodingDeflate,
                   bool acceptRange, int range_start, int range_end) :
        connection(connection),
        httpResponseCode(httpResponseCode),
        reader(reader),
        searcher(searcher),
        urlStr(urlStr),
        humanReadableBookId(humanReadableBookId),
        acceptEncodingDeflate(acceptEncodingDeflate),
        acceptRange(acceptRange),
        range_start(range_start),
        range_end(range_end)
    {}
};


void introduceTaskbar(string& content, const string& humanReadableBookId)
{
  pthread_mutex_lock(&regexLock);
  if (!noSearchBarFlag) {
    content = appendToFirstOccurence(
        content,
        "<head>",
        replaceRegex(
            RESOURCE::include_html_part, humanReadableBookId, "__CONTENT__")
        + (noLibraryButtonFlag
            ? "<style>#kiwix_serve_taskbar_library_button { display: none }</style>"
            : "")
    );
    if ( humanReadableBookId.empty() ) {
      content = appendToFirstOccurence(
          content,
          "<body[^>]*>",
          RESOURCE::global_taskbar_html_part);
    } else {
      content = appendToFirstOccurence(
          content,
          "<body[^>]*>",
          replaceRegex(
             RESOURCE::taskbar_html_part,
             humanReadableBookId,
             "__CONTENT__"));
    }
  }
  pthread_mutex_unlock(&regexLock);
}

static bool compress_content(string& content, const string& mimeType)
{
  static std::vector<Bytef> compr_buffer;

  /* Should be deflate */
  bool deflated = mimeType.find("text/") != string::npos
                  || mimeType.find("application/javascript") != string::npos
                  || mimeType.find("application/json") != string::npos;

  if (!deflated)
    return false;

  /* Compute the lengh */
  unsigned int contentLength = content.size();

  /* If the content is too short, no need to compress it */
  if (contentLength <= KIWIX_MIN_CONTENT_SIZE_TO_DEFLATE)
    return false;

  uLong bufferBound = compressBound(contentLength);

  /* Compress the content if necessary */
  pthread_mutex_lock(&compressorLock);
  compr_buffer.reserve(bufferBound);
  uLongf comprLen = compr_buffer.capacity();
  int err = compress(&compr_buffer[0],
                     &comprLen,
                     (const Bytef*)(content.data()),
                     contentLength);

  if (err == Z_OK && comprLen > 2 && comprLen < (contentLength + 2)) {
    /* /!\ Internet Explorer has a bug with deflate compression.
       It can not handle the first two bytes (compression headers)
       We need to chunk them off (move the content 2bytes)
       It has no incidence on other browsers
       See http://www.subbu.org/blog/2008/03/ie7-deflate-or-not and comments */
    content = string((char*)&compr_buffer[2], comprLen - 2);
  } else {
    deflated = false;
  }

  pthread_mutex_unlock(&compressorLock);
  return deflated;
}


static struct MHD_Response* build_response(const void* data,
                                           unsigned int length,
                                           const std::string& httpRedirection,
                                           const std::string& mimeType,
                                           bool deflated,
                                           bool cacheEnabled)
{
  /* Create the response */
  struct MHD_Response* response = MHD_create_response_from_data(
      length, const_cast<void*>(data), MHD_NO, MHD_YES);

  /* Make a redirection if necessary otherwise send the content */
  if (!httpRedirection.empty()) {
    MHD_add_response_header(
        response, MHD_HTTP_HEADER_LOCATION, httpRedirection.c_str());
  } else {
    /* Add if necessary the content-encoding */
    if (deflated) {
      MHD_add_response_header(
          response, MHD_HTTP_HEADER_VARY, "Accept-Encoding");
      MHD_add_response_header(
          response, MHD_HTTP_HEADER_CONTENT_ENCODING, "deflate");
    }

    /* Tell the client that byte ranges are accepted */
    MHD_add_response_header(response, MHD_HTTP_HEADER_ACCEPT_RANGES, "bytes");

    /* Specify the mime type */
    MHD_add_response_header(
        response, MHD_HTTP_HEADER_CONTENT_TYPE, mimeType.c_str());
  }

  /* Force to close the connection - cf. 100% CPU usage with v. 4.4 (in Lucid)
   */
  // MHD_add_response_header(response, MHD_HTTP_HEADER_CONNECTION, "close");

  /* Allow cross-domain requests */
  // MHD_add_response_header(response,
  // MHD_HTTP_HEADER_ACCESS_CONTROL_ALLOW_ORIGIN, "*");
  MHD_add_response_header(response, "Access-Control-Allow-Origin", "*");

  if (cacheEnabled) { /* Force cache */
    MHD_add_response_header(
        response, MHD_HTTP_HEADER_CACHE_CONTROL, "max-age=2723040, public");
  } else { /* Prevent cache (for random page) */
    MHD_add_response_header(response,
                            MHD_HTTP_HEADER_CACHE_CONTROL,
                            "no-cache, no-store, must-revalidate");
  }
  return response;
}


static struct MHD_Response* build_404(RequestContext* request_context) {
    std::string content
        = "<!DOCTYPE html>\n<html><head><meta "
          "content=\"text/html;charset=UTF-8\" http-equiv=\"content-type\" "
          "/><title>Content not found</title></head><body><h1>Not "
          "Found</h1><p>The requested URL \""
          + request_context->urlStr + "\" was not found on this server.</p></body></html>";
    auto mimeType = "text/html";
    request_context->httpResponseCode = MHD_HTTP_NOT_FOUND;
    introduceTaskbar(content, request_context->humanReadableBookId);
    bool deflated
        = request_context->acceptEncodingDeflate && compress_content(content, mimeType);
    return build_response(
        content.data(), content.size(), "", mimeType, deflated, false);
}


ssize_t callback_reader_from_blob(void* cls,
                                  uint64_t pos,
                                  char* buf,
                                  size_t max)
{
  zim::Blob* blob = static_cast<zim::Blob*>(cls);
  size_t max_size_to_set = min<size_t>(max, blob->size() - pos);

  if (max_size_to_set <= 0) {
    return MHD_CONTENT_READER_END_WITH_ERROR;
  }

  memcpy(buf, blob->data() + pos, max_size_to_set);
  return max_size_to_set;
}

void callback_free_blob(void* cls)
{
  zim::Blob* blob = static_cast<zim::Blob*>(cls);
  delete blob;
}

static struct MHD_Response* build_callback_response_from_blob(
    zim::Blob& blob, const std::string& mimeType)
{
  zim::Blob* p_blob = new zim::Blob(blob);
  struct MHD_Response* response
      = MHD_create_response_from_callback(blob.size(),
                                          16384,
                                          callback_reader_from_blob,
                                          p_blob,
                                          callback_free_blob);
  /* Tell the client that byte ranges are accepted */
  MHD_add_response_header(response, MHD_HTTP_HEADER_ACCEPT_RANGES, "bytes");

  /* Specify the mime type */
  MHD_add_response_header(
      response, MHD_HTTP_HEADER_CONTENT_TYPE, mimeType.c_str());

  /* Allow cross-domain requests */
  // MHD_add_response_header(response,
  // MHD_HTTP_HEADER_ACCESS_CONTROL_ALLOW_ORIGIN, "*");
  MHD_add_response_header(response, "Access-Control-Allow-Origin", "*");

  MHD_add_response_header(
      response, MHD_HTTP_HEADER_CACHE_CONTROL, "max-age=2723040, public");

  return response;
}

static struct MHD_Response* handle_suggest(RequestContext* request_context)
{
  std::string content;
  std::string mimeType;
  unsigned int maxSuggestionCount = 10;
  unsigned int suggestionCount = 0;
  std::string suggestion;

  /* Get the suggestion pattern from the HTTP request */
  const char* cTerm = MHD_lookup_connection_value(
    request_context->connection, MHD_GET_ARGUMENT_KIND, "term");
  std::string term = cTerm == NULL ? "" : cTerm;
  if (isVerbose.load()) {
    printf("Searching suggestions for: \"%s\"\n", term.c_str());
  }

  pthread_mutex_lock(&searchLock);
  /* Get the suggestions */
  content = "[";
  if (request_context->reader != NULL) {
    /* Get the suggestions */
    request_context->reader->searchSuggestionsSmart(term, maxSuggestionCount);
    while (request_context->reader->getNextSuggestion(suggestion)) {
      kiwix::stringReplacement(suggestion, "\"", "\\\"");
      content += (content == "[" ? "" : ",");
      content += "{\"value\":\"" + suggestion + "\",\"label\":\"" + suggestion
                 + "\"}";
      suggestionCount++;
    }
  }
  pthread_mutex_unlock(&searchLock);

  /* Propose the fulltext search if possible */
  if (request_context->searcher != NULL) {
    content += (suggestionCount == 0 ? "" : ",");
    content += "{\"value\":\"" + std::string(term)
               + " \", \"label\":\"containing '" + std::string(term)
               + "'...\"}";
  }

  content += "]";
  mimeType = "application/json; charset=utf-8";
  bool deflated = request_context->acceptEncodingDeflate && compress_content(content, mimeType);
  return build_response(
      content.data(), content.size(), "", mimeType, deflated, true);
}

static struct MHD_Response* handle_skin(RequestContext* request_context)
{
  std::string content;
  try {
    content = getResource(request_context->urlStr.substr(6));
  } catch (const ResourceNotFound& e) {
    return build_404(request_context);
  }
  std::string mimeType = getMimeTypeForFile(request_context->urlStr);
  bool deflated = request_context->acceptEncodingDeflate && compress_content(content, mimeType);
  return build_response(
      content.data(), content.size(), "", mimeType, deflated, true);
}

static struct MHD_Response* handle_search(RequestContext* request_context)
{
  std::string content;
  std::string mimeType;
  std::string httpRedirection;

  /* Retrieve the pattern to search */
  const char* pattern = MHD_lookup_connection_value(
      request_context->connection, MHD_GET_ARGUMENT_KIND, "pattern");
  std::string patternString
      = kiwix::urlDecode(pattern == NULL ? "" : string(pattern));
  std::string patternCorrespondingUrl;

  /* Try first to load directly the article */
  if (request_context->reader != NULL) {
    std::vector<std::string> variants = request_context->reader->getTitleVariants(patternString);
    std::vector<std::string>::iterator variantsItr = variants.begin();

    while (patternCorrespondingUrl.empty() && variantsItr != variants.end()) {
      request_context->reader->getPageUrlFromTitle(*variantsItr, patternCorrespondingUrl);
      variantsItr++;
    }

    /* If article found then redirect directly to it */
    if (!patternCorrespondingUrl.empty()) {
      httpRedirection
          = "/" + request_context->humanReadableBookId + "/" + patternCorrespondingUrl;
      request_context->httpResponseCode = MHD_HTTP_FOUND;
      return build_response("", 0, httpRedirection, "", false, true);
    }
  }

  /* Make the search */
  if (request_context->searcher != NULL) {
    const char* start = MHD_lookup_connection_value(
        request_context->connection, MHD_GET_ARGUMENT_KIND, "start");
    const char* end = MHD_lookup_connection_value(
        request_context->connection, MHD_GET_ARGUMENT_KIND, "end");
    unsigned int startNumber = start != NULL ? atoi(start) : 0;
    unsigned int endNumber = end != NULL ? atoi(end) : 25;

    /* Get the results */
    pthread_mutex_lock(&searchLock);
    try {
      request_context->searcher->search(patternString, startNumber, endNumber, isVerbose.load());
      content = request_context->searcher->getHtml();
    } catch (const std::exception& e) {
      std::cerr << e.what() << std::endl;
    }
    pthread_mutex_unlock(&searchLock);
  } else {
    content = "<!DOCTYPE html>\n<html><head><meta content=\"text/html;charset=UTF-8\" http-equiv=\"content-type\" /><title>Fulltext search unavailable</title></head><body><h1>Not Found</h1><p>There is no article with the title <b>\"" + kiwix::encodeDiples(patternString) + "\"</b> and the fulltext search engine is not available for this content.</p></body></html>";
    request_context->httpResponseCode = MHD_HTTP_NOT_FOUND;
  }

  mimeType = "text/html; charset=utf-8";

  introduceTaskbar(content, request_context->humanReadableBookId);

  bool deflated = request_context->acceptEncodingDeflate && compress_content(content, mimeType);
  return build_response(content.data(),
                        content.size(),
                        httpRedirection,
                        mimeType,
                        deflated,
                        true);
}

static struct MHD_Response* handle_random(RequestContext* request_context)
{
  std::string httpRedirection;
  request_context->httpResponseCode = MHD_HTTP_FOUND;
  if (request_context->reader != NULL) {
    std::string randomUrl = request_context->reader->getRandomPageUrl();
    httpRedirection
        = "/" + request_context->humanReadableBookId + "/" + kiwix::urlEncode(randomUrl);
  }
  return build_response("", 0, httpRedirection, "", false, false);
}

static struct MHD_Response* handle_content(RequestContext* request_context)
{
  std::string baseUrl;
  std::string content;
  std::string mimeType;

  bool found = false;
  zim::Article article;
  try {
    found = request_context->reader->getArticleObjectByDecodedUrl(request_context->urlStr, article);

    if (found) {
      /* If redirect */
      unsigned int loopCounter = 0;
      while (article.isRedirect() && loopCounter++ < 42) {
        article = article.getRedirectArticle();
      }

      /* To many loop */
      if (loopCounter == 42)
        found = false;
    }
  } catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
    found = false;
  }

  if (!found) {
    if (isVerbose.load())
      printf("Failed to find %s\n", request_context->urlStr.c_str());

    return build_404(request_context);
  }

  try {
    mimeType = article.getMimeType();
  } catch (exception& e) {
    mimeType = "application/octet-stream";
  }

  if (isVerbose.load()) {
    printf("Found %s\n", request_context->urlStr.c_str());
    printf("mimeType: %s\n", mimeType.c_str());
  }

  zim::Blob raw_content = article.getData();

  if (mimeType.find("text/") != string::npos
      || mimeType.find("application/javascript") != string::npos
      || mimeType.find("application/json") != string::npos) {
    content = string(raw_content.data(), raw_content.size());

    /* Special rewrite URL in case of ZIM file use intern *asbolute* url like
     * /A/Kiwix */
    if (mimeType.find("text/html") != string::npos) {
      baseUrl = "/" + std::string(1, article.getNamespace()) + "/"
                + article.getUrl();
      pthread_mutex_lock(&regexLock);
      content = replaceRegex(content,
                             "$1$2" + request_context->humanReadableBookId + "/$3/",
                             "(href|src)(=[\"|\']{0,1}/)([A-Z|\\-])/");
      content = replaceRegex(content,
                             "$1$2" + request_context->humanReadableBookId + "/$3/",
                             "(@import[ ]+)([\"|\']{0,1}/)([A-Z|\\-])/");
      content = replaceRegex(
          content,
          "<head><base href=\"/" + request_context->humanReadableBookId + baseUrl + "\" />",
          "<head>");
      pthread_mutex_unlock(&regexLock);
      introduceTaskbar(content, request_context->humanReadableBookId);
    } else if (mimeType.find("text/css") != string::npos) {
      pthread_mutex_lock(&regexLock);
      content = replaceRegex(content,
                             "$1$2" + request_context->humanReadableBookId + "/$3/",
                             "(url|URL)(\\([\"|\']{0,1}/)([A-Z|\\-])/");
      pthread_mutex_unlock(&regexLock);
    }

    bool deflated
        = request_context->acceptEncodingDeflate && compress_content(content, mimeType);
    return build_response(
        content.data(), content.size(), "", mimeType, deflated, true);
  } else {
    return build_callback_response_from_blob(raw_content, mimeType);
  }
}

static struct MHD_Response* handle_default(RequestContext* request_context)
{
  std::string content = welcomeHTML;

  std::string mimeType = "text/html; charset=utf-8";

  bool deflated = request_context->acceptEncodingDeflate && compress_content(content, mimeType);
  return build_response(
      content.data(), content.size(), "", mimeType, deflated, true);
}

static int accessHandlerCallback(void* cls,
                                 struct MHD_Connection* connection,
                                 const char* url,
                                 const char* method,
                                 const char* version,
                                 const char* upload_data,
                                 size_t* upload_data_size,
                                 void** ptr)
{
  /* Unexpected method */
  if (0 != strcmp(method, "GET") && 0 != strcmp(method, "POST"))
    return MHD_NO;

  /* The first time only the headers are valid, do not respond in the first
   * round... */
  static int dummy;
  if (&dummy != *ptr) {
    *ptr = &dummy;
    return MHD_YES;
  }

  /* clear context pointer */
  *ptr = NULL;

  /* Debug */
  if (isVerbose.load()) {
    printf("Requesting %s\n", url);
  }

  /* Check if the response can be compressed */
  const char* acceptEncodingHeaderValue = MHD_lookup_connection_value(
      connection, MHD_HEADER_KIND, MHD_HTTP_HEADER_ACCEPT_ENCODING);
  const bool acceptEncodingDeflate
      = acceptEncodingHeaderValue
        && string(acceptEncodingHeaderValue).find("deflate") != string::npos;

  /* Check if range is requested. [TODO] Handle this somehow */
  const char* acceptRangeHeaderValue = MHD_lookup_connection_value(
      connection, MHD_HEADER_KIND, MHD_HTTP_HEADER_RANGE);
  const bool acceptRange = acceptRangeHeaderValue != NULL;

  /* Prepare the variables */
  struct MHD_Response* response;
  int httpResponseCode = MHD_HTTP_OK;
  std::string urlStr = string(url);

  /* Get searcher and reader */
  std::string humanReadableBookId = "";
  if (!(urlStr.size() > 5 && urlStr.substr(0, 6) == "/skin/")) {
    if (!strcmp(url, "/search") || !strcmp(url, "/suggest")
        || !strcmp(url, "/random")) {
      const char* tmpGetValue = MHD_lookup_connection_value(
          connection, MHD_GET_ARGUMENT_KIND, "content");
      humanReadableBookId = (tmpGetValue != NULL ? string(tmpGetValue) : "");
    } else {
      humanReadableBookId = urlStr.substr(1,
                                          urlStr.find("/", 1) != string::npos
                                              ? urlStr.find("/", 1) - 1
                                              : urlStr.size() - 2);
      if (!humanReadableBookId.empty()) {
        urlStr = urlStr.substr(urlStr.find("/", 1) != string::npos
                                   ? urlStr.find("/", 1)
                                   : humanReadableBookId.size());
      }
    }
  }

  kiwix::Searcher* searcher
      = searchers.find(humanReadableBookId) != searchers.end()
            ? searchers.find(humanReadableBookId)->second
            : globalSearcher;
  kiwix::Reader* reader = readers.find(humanReadableBookId) != readers.end()
                              ? readers.find(humanReadableBookId)->second
                              : NULL;
  if (reader == NULL) {
    humanReadableBookId = "";
  }

  RequestContext request_context(connection, httpResponseCode,
                                 reader, searcher,
                                 urlStr, humanReadableBookId,
                                 acceptEncodingDeflate,
                                 acceptRange, range_start, range_end);


  /* Get suggestions */
  if (!strcmp(url, "/suggest") && reader != NULL) {
    response = handle_suggest(&request_context);
  }

  /* Get static skin stuff */
  else if (urlStr.substr(0, 6) == "/skin/") {
    response = handle_skin(&request_context);
  }

  /* Display the search restults */
  else if (!strcmp(url, "/search")) {
    response = handle_search(&request_context);
  }

  /* Display a random article */
  else if (!strcmp(url, "/random")) {
    response = handle_random(&request_context);
  }

  /* Display the content of a ZIM content (article, image, ...) */
  else if (reader != NULL) {
    response = handle_content(&request_context);
  }

  /* Display the global Welcome page */
  else {
    response = handle_default(&request_context);
  }

  /* Queue the response */
  int ret = MHD_queue_response(connection, request_context.httpResponseCode, response);
  MHD_destroy_response(response);

  return ret;
}

int main(int argc, char** argv)
{
  struct MHD_Daemon* daemon;
  vector<string> zimPathes;
  string libraryPath;
  string indexPath;
  string rootPath;
  string interface;
  int serverPort = 80;
  int daemonFlag = false;
  int libraryFlag = false;
  string PPIDString;
  unsigned int PPID = 0;
  unsigned int nb_threads = std::thread::hardware_concurrency();
  kiwix::Manager libraryManager;

  static struct option long_options[]
      = {{"daemon", no_argument, 0, 'd'},
         {"verbose", no_argument, 0, 'v'},
         {"library", no_argument, 0, 'l'},
         {"nolibrarybutton", no_argument, 0, 'm'},
         {"nosearchbar", no_argument, 0, 'n'},
         {"index", required_argument, 0, 'i'},
         {"attachToProcess", required_argument, 0, 'a'},
         {"port", required_argument, 0, 'p'},
         {"interface", required_argument, 0, 'f'},
         {"threads", required_argument, 0, 't'},
         {0, 0, 0, 0}};

  /* Argument parsing */
  while (true) {
    int option_index = 0;
    int c
        = getopt_long(argc, argv, "mndvli:a:p:f:t:", long_options, &option_index);

    if (c != -1) {
      switch (c) {
        case 'd':
          daemonFlag = true;
          break;
        case 'v':
          isVerbose.store(true);
          break;
        case 'l':
          libraryFlag = true;
          break;
        case 'n':
          noSearchBarFlag = true;
          break;
        case 'm':
          noLibraryButtonFlag = true;
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
        case 'f':
          interface = string(optarg);
          break;
        case 't':
          nb_threads = atoi(optarg);
          break;
      }
    } else {
      if (optind <= argc) {
        if (libraryFlag) {
          libraryPath = argv[optind++];
        } else {
          while (optind < argc)
            zimPathes.push_back(std::string(argv[optind++]));
        }
        break;
      }
    }
  }

  /* Print usage)) if necessary */
  if (zimPathes.empty() && libraryPath.empty()) {
    cerr << "Usage: kiwix-serve [--index=INDEX_PATH] [--port=PORT] [--verbose] "
            "[--nosearchbar] [--nolibrarybutton] [--daemon] "
            "[--attachToProcess=PID] [--interface=IF_NAME] "
            "[--threads=NB_THREAD(" << nb_threads << ")] ZIM_PATH+"
         << endl;
    cerr << "       kiwix-serve --library [--port=PORT] [--verbose] [--daemon] "
            "[--nosearchbar] [--nolibrarybutton] [--attachToProcess=PID] "
            "[--interface=IF_NAME] [--threads=NB_THREAD(" << nb_threads << ")] "
            "LIBRARY_PATH"
         << endl;
    cerr << "\n      If you set more than one ZIM_PATH, you cannot set a "
            "INDEX_PATH."
         << endl;
    exit(1);
  }

  if ((zimPathes.size() > 1) && !indexPath.empty()) {
    cerr << "You cannot set a indexPath if you also set several zimPathes";
    exit(1);
  }

  /* Setup the library manager and get the list of books */
  if (libraryFlag) {
    vector<string> libraryPaths = kiwix::split(libraryPath, ";");
    vector<string>::iterator itr;

    for (itr = libraryPaths.begin(); itr != libraryPaths.end(); ++itr) {
      if (!itr->empty()) {
        bool retVal = false;

        try {
          string libraryPath
              = isRelativePath(*itr)
                    ? computeAbsolutePath(removeLastPathElement(
                                              getExecutablePath(), true, false),
                                          *itr)
                    : *itr;
          retVal = libraryManager.readFile(libraryPath, true);
        } catch (...) {
          retVal = false;
        }

        if (!retVal) {
          cerr << "Unable to open the XML library file '" << *itr << "'."
               << endl;
          exit(1);
        }
      }
    }

    /* Check if the library is not empty (or only remote books)*/
    if (libraryManager.getBookCount(true, false) == 0) {
      cerr << "The XML library file '" << libraryPath
           << "' is empty (or has only remote books)." << endl;
    }
  } else {
    std::vector<std::string>::iterator it;
    for (it = zimPathes.begin(); it != zimPathes.end(); it++) {
      if (!libraryManager.addBookFromPath(*it, *it, "", false)) {
        cerr << "Unable to add the ZIM file '" << *it
             << "' to the internal library." << endl;
        exit(1);
      }
    }
    if (!indexPath.empty()) {
      libraryManager.setBookIndex(libraryManager.getBooksIds()[0], indexPath);
    }
  }

  /* Instance the readers and searcher and build the corresponding maps */
  vector<string> booksIds = libraryManager.getBooksIds();
  vector<string>::iterator itr;
  kiwix::Book currentBook;
  globalSearcher = new kiwix::Searcher();
  globalSearcher->setProtocolPrefix("/");
  globalSearcher->setSearchProtocolPrefix("/search?");
  for (itr = booksIds.begin(); itr != booksIds.end(); ++itr) {
    bool zimFileOk = false;
    libraryManager.getBookById(*itr, currentBook);
    std::string zimPath = currentBook.pathAbsolute;

    if (!zimPath.empty()) {
      indexPath = currentBook.indexPathAbsolute;

      /* Instanciate the ZIM file handler */
      kiwix::Reader* reader = NULL;
      try {
        reader = new kiwix::Reader(zimPath);
        zimFileOk = true;
      } catch (...) {
        cerr << "Unable to open the ZIM file '" << zimPath << "'." << endl;
      }

      if (zimFileOk) {
        string humanReadableId = currentBook.getHumanReadableIdFromPath();
        readers[humanReadableId] = reader;

        if ( reader->hasFulltextIndex()) {
          kiwix::Searcher* searcher = new kiwix::Searcher();
          searcher->setProtocolPrefix("/");
          searcher->setSearchProtocolPrefix("/search?");
          searcher->add_reader(reader, humanReadableId);
          globalSearcher->add_reader(reader, humanReadableId);
          searchers[humanReadableId] = searcher;
        } else if ( !indexPath.empty() ) {
          try {
            kiwix::Searcher* searcher = new kiwix::Searcher(indexPath, reader, humanReadableId);
            searcher->setProtocolPrefix("/");
            searcher->setSearchProtocolPrefix("/search?");
            searchers[humanReadableId] = searcher;
          } catch (...) {
            cerr << "Unable to open the search index '" << indexPath << "'."
                 << endl;
            searchers[humanReadableId] = nullptr;
          }
        } else {
            searchers[humanReadableId] = nullptr;
        }
      }
    }
  }

  /* Compute the Welcome HTML */
  string welcomeBooksHtml
      = ""
        "<div class='book__list'>";
  for (itr = booksIds.begin(); itr != booksIds.end(); ++itr) {
    libraryManager.getBookById(*itr, currentBook);

    if (!currentBook.path.empty()
        && readers.find(currentBook.getHumanReadableIdFromPath())
               != readers.end()) {
      welcomeBooksHtml += ""
"<a href='/" + currentBook.getHumanReadableIdFromPath() + "/'>"
    "<div class='book'>"
        "<div class='book__background' style='background-image: url(data:" + currentBook.faviconMimeType+ ";base64," + currentBook.favicon + ");'>"
            "<div class='book__title' title='" + currentBook.title + "'>" + currentBook.title + "</div>"
            "<div class='book__description' title='" + currentBook.description + "'>" + currentBook.description + "</div>"
            "<div class='book__info'>"
                "" + kiwix::beautifyInteger(atoi(currentBook.articleCount.c_str())) + " articles, " + kiwix::beautifyInteger(atoi(currentBook.mediaCount.c_str())) + " medias"
            "</div>"
        "</div>"
    "</div>"
"</a>";
    }
  }
  welcomeBooksHtml += ""
"</div>";

  pthread_mutex_lock(&regexLock);
  welcomeHTML
      = replaceRegex(RESOURCE::home_html_tmpl, welcomeBooksHtml, "__BOOKS__");
  pthread_mutex_unlock(&regexLock);

  introduceTaskbar(welcomeHTML, "");

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
  pthread_mutex_init(&searchLock, NULL);
  pthread_mutex_init(&compressorLock, NULL);
  pthread_mutex_init(&regexLock, NULL);

  /* Hard coded mimetypes */
  extMimeTypes["html"] = "text/html";
  extMimeTypes["htm"] = "text/html";
  extMimeTypes["png"] = "image/png";
  extMimeTypes["tiff"] = "image/tiff";
  extMimeTypes["tif"] = "image/tiff";
  extMimeTypes["jpeg"] = "image/jpeg";
  extMimeTypes["jpg"] = "image/jpeg";
  extMimeTypes["gif"] = "image/gif";
  extMimeTypes["svg"] = "image/svg+xml";
  extMimeTypes["txt"] = "text/plain";
  extMimeTypes["xml"] = "text/xml";
  extMimeTypes["pdf"] = "application/pdf";
  extMimeTypes["ogg"] = "application/ogg";
  extMimeTypes["js"] = "application/javascript";
  extMimeTypes["css"] = "text/css";
  extMimeTypes["otf"] = "application/vnd.ms-opentype";
  extMimeTypes["ttf"] = "application/font-ttf";
  extMimeTypes["woff"] = "application/font-woff";
  extMimeTypes["vtt"] = "text/vtt";

  /* Start the HTTP daemon */
  void* page = NULL;
  if (interface.length() > 0) {
#ifndef _WIN32

    /* TBD IPv6 support */
    struct sockaddr_in sockAddr;
    struct ifaddrs *ifaddr, *ifa;
    int family, n;

    /* Search all available interfaces */
    if (getifaddrs(&ifaddr) == -1) {
      cerr << "Getifaddrs() failed while searching for '" << interface << "'"
           << endl;
      exit(1);
    }

    /* Init 'sockAddr' with zeros */
    memset(&sockAddr, 0, sizeof(sockAddr));

    /* Try to find interfaces in the list of available interfaces */
    for (ifa = ifaddr, n = 0; ifa != NULL; ifa = ifa->ifa_next, n++) {
      /* Ignore if no IP attributed to the interface */
      if (ifa->ifa_addr == NULL)
        continue;

      /* Check if the interface is the right one */
      family = ifa->ifa_addr->sa_family;
      if (family == AF_INET) {
        if (strcasecmp(ifa->ifa_name, interface.c_str()) == 0) {
          sockAddr.sin_family = family;
          sockAddr.sin_port = htons(serverPort);
          sockAddr.sin_addr.s_addr
              = ((struct sockaddr_in*)ifa->ifa_addr)->sin_addr.s_addr;
          break;
        }
      }
    }

    /* Free 'ifaddr' */
    freeifaddrs(ifaddr);

    /* Dies if interface was not found in the list */
    if (sockAddr.sin_family == 0) {
      cerr << "Unable to find interface '" << interface << "'" << endl;
      exit(1);
    }

    daemon = MHD_start_daemon(MHD_USE_POLL_INTERNALLY,
                              serverPort,
                              NULL,
                              NULL,
                              &accessHandlerCallback,
                              page,
                              MHD_OPTION_SOCK_ADDR, &sockAddr,
                              MHD_OPTION_THREAD_POOL_SIZE, nb_threads,
                              MHD_OPTION_END);
#else
    cerr << "Setting 'interface' not yet implemented for Windows" << endl;
    exit(1);
#endif

  } else {
    daemon = MHD_start_daemon(MHD_USE_POLL_INTERNALLY,
                              serverPort,
                              NULL,
                              NULL,
                              &accessHandlerCallback,
                              page,
                              MHD_OPTION_THREAD_POOL_SIZE, nb_threads,
                              MHD_OPTION_END);
  }

  if (daemon == NULL) {
    cerr << "Unable to instanciate the HTTP daemon. The port " << serverPort
         << " is maybe already occupied or need more permissions to be open. "
            "Please try as root or with a port number higher or equal to 1024."
         << endl;
    exit(1);
  }

  /* Run endless (until PPID dies) */
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

      mib[0] = CTL_KERN;
      mib[1] = KERN_PROC;
      mib[2] = KERN_PROC_PID;
      mib[3] = PPID;

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

  delete globalSearcher;

  /* Stop the daemon */
  MHD_stop_daemon(daemon);

  /* Mutex destroy */
  pthread_mutex_destroy(&searchLock);
  pthread_mutex_destroy(&compressorLock);
  pthread_mutex_destroy(&regexLock);
  exit(0);
}
