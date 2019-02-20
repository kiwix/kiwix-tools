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
#include <kiwix/tools/otherTools.h>
#include <kiwix/tools/pathTools.h>
#include <kiwix/tools/regexTools.h>
#include <kiwix/tools/stringTools.h>
#include <kiwix/manager.h>
#include <kiwix/reader.h>
#include <kiwix/searcher.h>
#include <kiwix/opds_dumper.h>
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
#include <netinet/in.h>
#include <unistd.h>
#endif

#include "request_context.h"

#ifdef interface
#undef interface
#endif

using namespace std;

static bool noLibraryButtonFlag = false;
static bool noSearchBarFlag = false;
static string welcomeHTML;
static string catalogOpenSearchDescription;
static std::atomic_bool isVerbose(false);
static std::string rootLocation = "";
static std::map<std::string, std::string> extMimeTypes;
static std::map<std::string, kiwix::Reader*> readers;
static std::map<std::string, kiwix::Searcher*> searchers;
static kiwix::Searcher* globalSearcher = nullptr;
static kiwix::Library library;
static pthread_mutex_t searchLock = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t compressorLock = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t regexLock = PTHREAD_MUTEX_INITIALIZER;

std::pair<kiwix::Reader*, kiwix::Searcher*>
get_from_humanReadableBookId(const std::string& humanReadableBookId) {
  kiwix::Searcher* searcher
        = searchers.find(humanReadableBookId) != searchers.end()
              ? searchers.find(humanReadableBookId)->second
              : globalSearcher;
  kiwix::Reader* reader = readers.find(humanReadableBookId) != readers.end()
                                ? readers.find(humanReadableBookId)->second
                                : NULL;
  return std::pair<kiwix::Reader*, kiwix::Searcher*>(reader, searcher);
}

/* Try to get the mimeType from the file extension */
static std::string getMimeTypeForFile(const std::string& filename)
{
  std::string mimeType = "text/plain";
  auto pos = filename.find_last_of(".");

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


static bool startswith(const std::string& base, const std::string& start)
{
   return start.length() <= base.length()
        && std::equal(start.begin(), start.end(), base.begin());
}


void introduceTaskbar(string& content, const string& humanReadableBookId)
{
  string zimTitle;

  pthread_mutex_lock(&regexLock);

  if (!noSearchBarFlag) {
    content = appendToFirstOccurence(
        content,
        "<head>",
            RESOURCE::include_html_part + (noLibraryButtonFlag
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

      auto reader = get_from_humanReadableBookId(humanReadableBookId).first;
      if (reader != nullptr) {
        zimTitle = reader->getTitle();
      }
    }
  }

  content = replaceRegex(content, rootLocation, "__ROOT_LOCATION__");
  content = replaceRegex(content, replaceRegex(zimTitle, "%26", "&"), "__ZIM_TITLE__");
  content = replaceRegex(content, replaceRegex(humanReadableBookId, "%26", "&"), "__CONTENT_ESCAPED__");

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
  struct MHD_Response* response = MHD_create_response_from_buffer(
      length, const_cast<void*>(data), MHD_RESPMEM_MUST_COPY);

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


static struct MHD_Response* build_404(RequestContext* request,
                                      const std::string& humanReadableBookId) {
    std::string content
        = "<!DOCTYPE html>\n<html><head><meta "
          "content=\"text/html;charset=UTF-8\" http-equiv=\"content-type\" "
          "/><title>Content not found</title></head><body><h1>Not "
          "Found</h1><p>The requested URL \""
          + request->get_full_url() + "\" was not found on this server.</p></body></html>";
    auto mimeType = "text/html";
    request->httpResponseCode = MHD_HTTP_NOT_FOUND;
    introduceTaskbar(content, humanReadableBookId);
    bool deflated
        = request->can_compress() && compress_content(content, mimeType);
    return build_response(
        content.data(), content.size(), "", mimeType, deflated, false);
}

static struct MHD_Response* build_homepage(RequestContext* request)
{
  std::string content = welcomeHTML;

  std::string mimeType = "text/html; charset=utf-8";

  bool deflated = request->can_compress() && compress_content(content, mimeType);
  return build_response(
      content.data(), content.size(), "", mimeType, deflated, false);
}

struct RunningResponse {
   kiwix::Entry entry;
   int range_start;

   RunningResponse(kiwix::Entry entry,
                   int range_start) :
     entry(entry),
     range_start(range_start)
   {}
};


ssize_t callback_reader_from_entry(void* cls,
                                  uint64_t pos,
                                  char* buf,
                                  size_t max)
{
  RunningResponse* response = static_cast<RunningResponse*>(cls);

  size_t max_size_to_set = min<size_t>(
    max,
    response->entry.getSize() - pos - response->range_start);

  if (max_size_to_set <= 0) {
    return MHD_CONTENT_READER_END_WITH_ERROR;
  }

  zim::Blob blob = response->entry.getBlob(response->range_start+pos, max_size_to_set);
  memcpy(buf, blob.data(), max_size_to_set);
  return max_size_to_set;
}

void callback_free_response(void* cls)
{
  RunningResponse* response = static_cast<RunningResponse*>(cls);
  delete response;
}

static struct MHD_Response* build_callback_response_from_entry(
   kiwix::Entry entry, int range_start, int range_len, const std::string& mimeType)
{
  RunningResponse* run_response =
      new RunningResponse(entry, range_start);

  struct MHD_Response* response
      = MHD_create_response_from_callback(entry.getSize(),
                                          16384,
                                          callback_reader_from_entry,
                                          run_response,
                                          callback_free_response);
  /* Tell the client that byte ranges are accepted */
  MHD_add_response_header(response, MHD_HTTP_HEADER_ACCEPT_RANGES, "bytes");
  std::ostringstream oss;
  oss << "bytes " << range_start << "-" << range_start + range_len - 1 << "/" << entry.getSize();

  MHD_add_response_header(response, MHD_HTTP_HEADER_CONTENT_RANGE, oss.str().c_str());

  MHD_add_response_header(response, MHD_HTTP_HEADER_CONTENT_LENGTH,
    kiwix::to_string(range_len).c_str());

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

static struct MHD_Response* handle_meta(RequestContext* request)
{
  std::string humanReadableBookId;
  std::string meta_name;
  try {
    humanReadableBookId = request->get_argument("content");
    meta_name = request->get_argument("name");
  } catch (const std::out_of_range& e) {
    return build_404(request, humanReadableBookId);
  }

  auto reader = get_from_humanReadableBookId(humanReadableBookId).first;
  if (reader == nullptr) {
    return build_404(request, humanReadableBookId);
  }

  std::string content;
  std::string mimeType = "text";

  if (meta_name == "title") {
    content = reader->getTitle();
  } else if (meta_name == "description") {
    content = reader->getDescription();
  } else if (meta_name == "language") {
    content = reader->getLanguage();
  } else if (meta_name == "name") {
    content = reader->getName();
  } else if (meta_name == "tags") {
    content = reader->getTags();
  } else if (meta_name == "date") {
    content = reader->getDate();
  } else if (meta_name == "creator") {
    content = reader->getCreator();
  } else if (meta_name == "publisher") {
    content = reader->getPublisher();
  } else if (meta_name == "favicon") {
    reader->getFavicon(content, mimeType);
  } else {
    return build_404(request, humanReadableBookId);
  }

  return build_response(content.data(), content.size(), "", mimeType, false, true);
}

static struct MHD_Response* handle_suggest(RequestContext* request)
{
  if (isVerbose.load()) {
    printf("** running handle_suggest\n");
  }

  std::string content;
  std::string mimeType;
  unsigned int maxSuggestionCount = 10;
  unsigned int suggestionCount = 0;
  std::string suggestion;

  std::string humanReadableBookId;
  std::string term;
  try {
    humanReadableBookId = request->get_argument("content");
    term = request->get_argument("term");
  } catch (const std::out_of_range&) {
    return build_404(request, "");
  }

  if (isVerbose.load()) {
    printf("Searching suggestions for: \"%s\"\n", term.c_str());
  }

  auto reader_searcher = get_from_humanReadableBookId(humanReadableBookId);
  auto reader = reader_searcher.first;
  auto searcher = reader_searcher.second;

  pthread_mutex_lock(&searchLock);
  /* Get the suggestions */
  content = "[";
  if (reader != nullptr) {
    /* Get the suggestions */
    reader->searchSuggestionsSmart(term, maxSuggestionCount);
    while (reader->getNextSuggestion(suggestion)) {
      kiwix::stringReplacement(suggestion, "\"", "\\\"");
      content += (content == "[" ? "" : ",");
      content += "{\"value\":\"" + suggestion + "\",\"label\":\"" + suggestion
                 + "\"}";
      suggestionCount++;
    }
  }
  pthread_mutex_unlock(&searchLock);

  /* Propose the fulltext search if possible */
  if (searcher != NULL) {
    content += (suggestionCount == 0 ? "" : ",");
    content += "{\"value\":\"" + term
               + " \", \"label\":\"containing '" + term
               + "'...\"}";
  }

  content += "]";
  mimeType = "application/json; charset=utf-8";
  bool deflated = request->can_compress() && compress_content(content, mimeType);
  return build_response(
      content.data(), content.size(), "", mimeType, deflated, true);
}

static struct MHD_Response* handle_skin(RequestContext* request)
{
  if (isVerbose.load()) {
    printf("** running handle_skin\n");
  }

  std::string content;
  auto resourceName = request->get_url().substr(6);
  try {
    content = getResource(resourceName);
  } catch (const ResourceNotFound& e) {
    return build_404(request, "");
  }
  std::string mimeType = getMimeTypeForFile(resourceName);
  bool deflated = request->can_compress() && compress_content(content, mimeType);
  return build_response(
      content.data(), content.size(), "", mimeType, deflated, true);
}

static struct MHD_Response* handle_search(RequestContext* request)
{
  if (isVerbose.load()) {
    printf("** running handle_search\n");
  }

  std::string content;
  std::string mimeType;
  std::string httpRedirection;

  std::string humanReadableBookId;
  std::string patternString;
  try {
    humanReadableBookId = request->get_argument("content");
  } catch (const std::out_of_range&) {}

  try {
    patternString = request->get_argument("pattern");
  } catch (const std::out_of_range&) {}

  /* Retrive geo search */
  bool has_geo_query = false;
  float latitude = 0;
  float longitude = 0;
  float distance = 0;
  try {
    latitude = request->get_argument<float>("latitude");
    longitude = request->get_argument<float>("longitude");
    distance = request->get_argument<float>("distance");
    has_geo_query = true;
  } catch(const std::out_of_range&) {}
    catch(const std::invalid_argument&) {}

  /* Search results for searches from the welcome page should not
     be cached
  */
  auto reader_searcher = get_from_humanReadableBookId(humanReadableBookId);
  auto reader = reader_searcher.first;
  auto searcher = reader_searcher.second;
  bool cacheEnabled = !(searcher == globalSearcher);

  /* Try first to load directly the article */
  if (reader != nullptr && !patternString.empty()) {
    std::string patternCorrespondingUrl;
    auto variants = reader->getTitleVariants(patternString);
    auto variantsItr = variants.begin();

    while (patternCorrespondingUrl.empty() && variantsItr != variants.end()) {
      try {
        auto entry = reader->getEntryFromTitle(*variantsItr);
        entry = entry.getFinalEntry();
        patternCorrespondingUrl = entry.getPath();
        break;
      } catch(kiwix::NoEntry& e) {
        variantsItr++;
      }
    }

    /* If article found then redirect directly to it */
    if (!patternCorrespondingUrl.empty()) {
      httpRedirection
          = rootLocation + "/" + humanReadableBookId + "/" + patternCorrespondingUrl;
      request->httpResponseCode = MHD_HTTP_FOUND;
      return build_response("", 0, httpRedirection, "", false, true);
    }
  }

  /* Make the search */
  if (searcher != nullptr &&
      (!patternString.empty() || has_geo_query)) {
    auto start = 0;
    try {
      start = request->get_argument<unsigned int>("start");
    } catch (const std::exception&) {}
    auto end = 25;
    try {
      end = request->get_argument<unsigned int>("end");
    } catch (const std::exception&) {}

    /* Get the results */
    pthread_mutex_lock(&searchLock);
    try {
      if (patternString.empty()) {
        searcher->geo_search(latitude, longitude, distance,
                             start, end, isVerbose.load());
      } else {
        searcher->search(patternString,
                         start, end, isVerbose.load());
      }
      content = searcher->getHtml();
    } catch (const std::exception& e) {
      std::cerr << e.what() << std::endl;
    }
    pthread_mutex_unlock(&searchLock);
  } else {
    content = "<!DOCTYPE html>\n<html><head><meta content=\"text/html;charset=UTF-8\" http-equiv=\"content-type\" /><title>Fulltext search unavailable</title></head><body><h1>Not Found</h1><p>There is no article with the title <b>\"" + kiwix::encodeDiples(patternString) + "\"</b> and the fulltext search engine is not available for this content.</p></body></html>";
    request->httpResponseCode = MHD_HTTP_NOT_FOUND;
  }

  mimeType = "text/html; charset=utf-8";

  introduceTaskbar(content, humanReadableBookId);

  bool deflated = request->can_compress() && compress_content(content, mimeType);
  return build_response(content.data(),
                        content.size(),
                        httpRedirection,
                        mimeType,
                        deflated,
                        cacheEnabled);
}

static struct MHD_Response* handle_random(RequestContext* request)
{
  if (isVerbose.load()) {
    printf("** running handle_random\n");
  }

  std::string httpRedirection;
  request->httpResponseCode = MHD_HTTP_FOUND;
  std::string humanReadableBookId;
  try {
    humanReadableBookId = request->get_argument("content");
  } catch (const std::out_of_range&) {
    return build_404(request, "");
  }

  auto reader = get_from_humanReadableBookId(humanReadableBookId).first;
  if (reader == nullptr) {
    return build_404(request, "");
  }

  try {
    auto entry = reader->getRandomPage();
    entry = entry.getFinalEntry();
    httpRedirection
      = rootLocation + "/" + humanReadableBookId + "/" + kiwix::urlEncode(entry.getPath());
    return build_response("", 0, httpRedirection, "", false, false);
  } catch(kiwix::NoEntry& e) {
    return build_404(request, humanReadableBookId);
  }
}

static struct MHD_Response* handle_catalog(RequestContext* request)
{
  if (isVerbose.load()) {
    printf("** running handle_catalog");
  }

  std::string host;
  std::string url;
  try {
    host = request->get_header("Host");
    url  = request->get_url_part(1);
  } catch (const std::out_of_range&) {
    return build_404(request, "");
  }

  std::string content;
  std::string mimeType;

  if (url == "searchdescription.xml") {
    content = catalogOpenSearchDescription;
    mimeType = "application/opensearchdescription+xml";
  } else {
    zim::Uuid uuid;
    kiwix::OPDSDumper opdsDumper;
    opdsDumper.setRootLocation(rootLocation);
    opdsDumper.setSearchDescriptionUrl("catalog/searchdescription.xml");
    opdsDumper.setId(kiwix::to_string(uuid));
    opdsDumper.setLibrary(&library);
    mimeType = "application/atom+xml;profile=opds-catalog;kind=acquisition; charset=utf-8";
    std::vector<std::string> bookIdsToDump;
    if (url == "root.xml") {
      opdsDumper.setTitle("All zims");
      uuid = zim::Uuid::generate(host);

      bookIdsToDump = library.listBooksIds(
        kiwix::VALID|kiwix::LOCAL|kiwix::REMOTE);
    } else if (url == "search") {
      std::string query;
      std::string language;
      size_t count(10);
      size_t startIndex(0);
      try {
        query = request->get_argument("q");
      } catch (const std::out_of_range&) {}
      try {
        language = request->get_argument("lang");
      } catch (const std::out_of_range&) {}
      try {
        count = stoul(request->get_argument("count"));
      } catch (...) {}
      try {
        startIndex = stoul(request->get_argument("start"));
      } catch (...) {}
      opdsDumper.setTitle("Search result for " + query);
      uuid = zim::Uuid::generate();
      bookIdsToDump = library.listBooksIds(
        kiwix::VALID|kiwix::LOCAL|kiwix::REMOTE,
        kiwix::UNSORTED,
        query,
        language);
      auto totalResults = bookIdsToDump.size();
      bookIdsToDump.erase(bookIdsToDump.begin(), bookIdsToDump.begin()+startIndex);
      if (count>0 && bookIdsToDump.size() > count) {
        bookIdsToDump.resize(count);
      }
      opdsDumper.setOpenSearchInfo(totalResults, startIndex, bookIdsToDump.size());
    } else {
      return build_404(request, "");
    }

    content = opdsDumper.dumpOPDSFeed(bookIdsToDump);
  }

  bool deflated = request->can_compress() && compress_content(content, mimeType);
  return build_response(
      content.data(), content.size(), "", mimeType, deflated, false);
}

static struct MHD_Response* handle_content(RequestContext* request)
{
  if (isVerbose.load()) {
    printf("** running handle_content\n");
  }

  std::string baseUrl;
  std::string content;
  std::string mimeType;

  kiwix::Entry entry;

  std::string humanReadableBookId;
  try {
    humanReadableBookId = request->get_url_part(0);
  } catch (const std::out_of_range& e) {
    return build_homepage(request);
  }

  auto reader = get_from_humanReadableBookId(humanReadableBookId).first;
  if (reader == nullptr) {
    if (humanReadableBookId.size() == 0){
        return build_homepage(request);
    } else {
        return build_404(request, "");
    }
  }

  auto urlStr = request->get_url().substr(humanReadableBookId.size()+1);
  if (urlStr[0] == '/') {
    urlStr = urlStr.substr(1);
  }

  try {
    entry = reader->getEntryFromPath(urlStr);
    if (entry.isRedirect() || urlStr.empty()) {
      // If urlStr is empty, we want to mainPage.
      // We must do a redirection to the real page.
      entry = entry.getFinalEntry();
      std::string httpRedirection = (
        rootLocation + "/" + humanReadableBookId + "/" +
        kiwix::urlEncode(entry.getPath()));
      request->httpResponseCode = MHD_HTTP_FOUND;
      return build_response("", 0, httpRedirection, "", false, false);
    }
  } catch(kiwix::NoEntry& e) {
    if (isVerbose.load())
      printf("Failed to find %s\n", urlStr.c_str());

    return build_404(request, humanReadableBookId);
  }

  try {
    mimeType = entry.getMimetype();
  } catch (exception& e) {
    mimeType = "application/octet-stream";
  }

  if (isVerbose.load()) {
    printf("Found %s\n", urlStr.c_str());
    printf("mimeType: %s\n", mimeType.c_str());
  }

  if (mimeType.find("text/") != string::npos
      || mimeType.find("application/javascript") != string::npos
      || mimeType.find("application/json") != string::npos) {
    zim::Blob raw_content = entry.getBlob();
    content = string(raw_content.data(), raw_content.size());

    if (mimeType.find("text/html") != string::npos) {
      introduceTaskbar(content, humanReadableBookId);
    }

    bool deflated
        = request->can_compress() && compress_content(content, mimeType);
    return build_response(
        content.data(), content.size(), "", mimeType, deflated, true);
  } else {
    int range_len;
    if (request->get_range().second == -1) {
       range_len = entry.getSize() - request->get_range().first;
    } else {
       range_len = request->get_range().second - request->get_range().first;
    }
    return build_callback_response_from_entry(
      entry,
      request->get_range().first,
      range_len,
      mimeType);
  }
}

int print_key_value (void *cls, enum MHD_ValueKind kind,
                   const char *key, const char *value)
{
  printf (" - %s: '%s'\n", key, value);
  return MHD_YES;
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
  if (isVerbose.load() ) {
    printf("======================\n");
    printf("Requesting : \n");
    printf("full_url  : %s\n", url);
  }
  RequestContext request(connection, rootLocation, url, method, version);

  if (isVerbose.load() ) {
    request.print_debug_info();
  }
  /* Unexpected method */
  if (request.get_method() != RequestMethod::GET && request.get_method() != RequestMethod::POST) {
    printf("Reject request because of unhandled request method.\n");
    printf("----------------------\n");
    return MHD_NO;
  }

  /* Prepare the variables */
  struct MHD_Response* response;
  request.httpResponseCode = request.has_range() ? MHD_HTTP_PARTIAL_CONTENT : MHD_HTTP_OK;

  if (! request.is_valid_url()) {
    response = build_404(&request, "");
  } else {
    if (startswith(request.get_url(), "/skin/")) {
      response = handle_skin(&request);
    } else if (startswith(request.get_url(), "/catalog")) {
      response = handle_catalog(&request);
    } else if (request.get_url() == "/meta") {
      response = handle_meta(&request);
    } else if (request.get_url() == "/search") {
      response = handle_search(&request);
    } else if (request.get_url() == "/suggest") {
      response = handle_suggest(&request);
    } else if (request.get_url() == "/random") {
      response = handle_random(&request);
    } else {
      response = handle_content(&request);
    }
  }

  /* Queue the response */
  if (isVerbose.load()) {
    printf("Response :\n");
    printf("httpResponseCode : %d\n", request.httpResponseCode);
    printf("headers :\n");
    MHD_get_response_headers(response, print_key_value, nullptr);
    printf("----------------------\n");
  }
  int ret = MHD_queue_response(connection, request.httpResponseCode, response);
  MHD_destroy_response(response);

  return ret;
}

int main(int argc, char** argv)
{
  struct MHD_Daemon* daemon;
  vector<string> zimPathes;
  string libraryPath;
  string rootPath;
  string interface;
  int serverPort = 80;
  int daemonFlag [[gnu::unused]] = false;
  int libraryFlag = false;
  string PPIDString;
  unsigned int PPID = 0;
  unsigned int nb_threads = std::thread::hardware_concurrency();

  static struct option long_options[]
      = {{"daemon", no_argument, 0, 'd'},
         {"verbose", no_argument, 0, 'v'},
         {"library", no_argument, 0, 'l'},
         {"nolibrarybutton", no_argument, 0, 'm'},
         {"nosearchbar", no_argument, 0, 'n'},
         {"attachToProcess", required_argument, 0, 'a'},
         {"port", required_argument, 0, 'p'},
         {"interface", required_argument, 0, 'f'},
         {"threads", required_argument, 0, 't'},
         {"urlRootLocation", required_argument, 0, 'r'},
         {0, 0, 0, 0}};

  /* Argument parsing */
  while (true) {
    int option_index = 0;
    int c
        = getopt_long(argc, argv, "mndvla:p:f:t:r:", long_options, &option_index);

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
        case 'r':
          rootLocation = string(optarg);

          /* prepend prefix "/" if not provided*/
          if (rootLocation[0] != '/'){
              rootLocation = "/" + rootLocation;
          }

          /* remove the trailing slash if provided*/
          if (rootLocation.back() == '/'){
              rootLocation.erase(rootLocation.size() - 1);
          }

          break;
      }
    } else {
      if (optind < argc) {
        if (libraryFlag) {
          libraryPath = argv[optind++];
        } else {
          while (optind < argc)
            zimPathes.push_back(std::string(argv[optind++]));
        }
      }
      break;
    }
  }

  /* Print usage)) if necessary */
  if (zimPathes.empty() && libraryPath.empty()) {
    cerr << "Usage: kiwix-serve [--index=INDEX_PATH] [--port=PORT] [--verbose] "
            "[--nosearchbar] [--nolibrarybutton] [--daemon] "
            "[--attachToProcess=PID] [--interface=IF_NAME] "
            "[--urlRootLocation=/URL_ROOT] "
            "[--threads=NB_THREAD(" << nb_threads << ")] ZIM_PATH+"
         << endl;
    cerr << "       kiwix-serve --library [--port=PORT] [--verbose] [--daemon] "
            "[--nosearchbar] [--nolibrarybutton] [--attachToProcess=PID] "
            "[--interface=IF_NAME] [--urlRootLocation=/URL_ROOT] "
            "[--threads=NB_THREAD(" << nb_threads << ")] LIBRARY_PATH "
         << endl;
    cerr << "\n      If you set more than one ZIM_PATH, you cannot set a "
            "INDEX_PATH."
         << endl;
    exit(1);
  }

  /* Setup the library manager and get the list of books */
  kiwix::Manager manager(&library);
  if (libraryFlag) {
    vector<string> libraryPaths = kiwix::split(libraryPath, ";");
    vector<string>::iterator itr;

    for (itr = libraryPaths.begin(); itr != libraryPaths.end(); ++itr) {
      if (!itr->empty()) {
        bool retVal = false;

        try {
          string libraryPath
              = isRelativePath(*itr)
                    ? computeAbsolutePath(getCurrentDirectory(), *itr)
                    : *itr;
          retVal = manager.readFile(libraryPath, true);
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
    if (library.getBookCount(true, false) == 0) {
      cerr << "The XML library file '" << libraryPath
           << "' is empty (or has only remote books)." << endl;
    }
  } else {
    std::vector<std::string>::iterator it;
    for (it = zimPathes.begin(); it != zimPathes.end(); it++) {
      if (!manager.addBookFromPath(*it, *it, "", false)) {
        cerr << "Unable to add the ZIM file '" << *it
             << "' to the internal library." << endl;
        exit(1);
      }
    }
  }

  /* Instance the readers and searcher and build the corresponding maps */
  vector<string> booksIds = library.listBooksIds(kiwix::LOCAL);
  globalSearcher = new kiwix::Searcher();
  globalSearcher->setProtocolPrefix(rootLocation + "/");
  globalSearcher->setSearchProtocolPrefix(rootLocation + "/" + "search?");
  for (auto& bookId: booksIds) {
    auto& currentBook = library.getBookById(bookId);
    auto zimPath = currentBook.getPath();

    /* Instanciate the ZIM file handler */
    kiwix::Reader* reader = NULL;
    try {
      reader = new kiwix::Reader(zimPath);
    } catch (...) {
      cerr << "Unable to open the ZIM file '" << zimPath << "'." << endl;
      continue;
    }

    auto humanReadableId = currentBook.getHumanReadableIdFromPath();
    readers[humanReadableId] = reader;

    if (reader->hasFulltextIndex()) {
      kiwix::Searcher* searcher = new kiwix::Searcher(humanReadableId);
      searcher->setProtocolPrefix(rootLocation + "/");
      searcher->setSearchProtocolPrefix(rootLocation + "/" + "search?");
      searcher->add_reader(reader, humanReadableId);
      globalSearcher->add_reader(reader, humanReadableId);
      searchers[humanReadableId] = searcher;
    } else {
        searchers[humanReadableId] = nullptr;
    }
  }

  /* Compute the Welcome HTML */
  string welcomeBooksHtml
      = ""
        "<div class='book__list'>";
  for (auto& bookId: booksIds) {
    auto& currentBook = library.getBookById(bookId);

    if (!currentBook.getPath().empty()
        && readers.find(currentBook.getHumanReadableIdFromPath())
               != readers.end()) {
      welcomeBooksHtml += ""
"<a href='" + rootLocation + "/" + currentBook.getHumanReadableIdFromPath() + "/'>"
    "<div class='book'>"
        "<div class='book__background' style=\"background-image: url('/meta?content="
             + currentBook.getHumanReadableIdFromPath() + "&name=favicon');\">"
            "<div class='book__title' title='" + currentBook.getTitle() + "'>" + currentBook.getTitle() + "</div>"
            "<div class='book__description' title='" + currentBook.getDescription() + "'>" + currentBook.getDescription() + "</div>"
            "<div class='book__info'>"
                "" + kiwix::beautifyInteger(currentBook.getArticleCount()) + " articles, " + kiwix::beautifyInteger(currentBook.getMediaCount()) + " medias"
            "</div>"
        "</div>"
    "</div>"
"</a>\n";
    }
  }
  welcomeBooksHtml += ""
"</div>";

  pthread_mutex_lock(&regexLock);
  welcomeHTML
      = replaceRegex(RESOURCE::home_html_tmpl, welcomeBooksHtml, "__BOOKS__");
  pthread_mutex_unlock(&regexLock);

  introduceTaskbar(welcomeHTML, "");

  /* Compute the OpenSearch description */
  catalogOpenSearchDescription = RESOURCE::opensearchdescription_xml;
  catalogOpenSearchDescription = replaceRegex(catalogOpenSearchDescription, rootLocation, "__ROOT_LOCATION__");


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
    cerr << "Unable to instantiate the HTTP daemon. The port " << serverPort
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
