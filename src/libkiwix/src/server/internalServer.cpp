/*
 * Copyright 2019 Matthieu Gautier <mgautier@kymeria.fr>
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

#include "internalServer.h"

#ifndef _WIN32
#include <netinet/in.h>
#endif

#ifdef _WIN32
# if !defined(__MINGW32__) && (_MSC_VER < 1600)
#   include "stdint4win.h"
# endif
# include <winsock2.h>
# include <ws2tcpip.h>
# ifdef __GNUC__
  // inet_pton is not declared in mingw, even if the function exists.
  extern "C" {
    WINSOCK_API_LINKAGE  INT WSAAPI inet_pton( INT Family, PCSTR pszAddrString, PVOID pAddrBuf);
  }
# endif
  typedef UINT64 uint64_t;
  typedef UINT16 uint16_t;
#endif

extern "C" {
#include "microhttpd_wrapper.h"
}

#include "tools.h"
#include "tools/pathTools.h"
#include "tools/regexTools.h"
#include "tools/stringTools.h"
#include "tools/archiveTools.h"
#include "tools/networkTools.h"
#include "library.h"
#include "name_mapper.h"
#include "search_renderer.h"
#include "opds_dumper.h"
#include "html_dumper.h"
#include "i18n_utils.h"

#include <zim/uuid.h>
#include <zim/error.h>
#include <zim/entry.h>
#include <zim/item.h>
#include <zim/suggestion.h>

#include <mustache.hpp>

#include <atomic>
#include <string>
#include <vector>
#include <chrono>
#include <fstream>
#include "libkiwix-resources.h"

#ifndef _WIN32
# include <arpa/inet.h>
#endif

#include "request_context.h"
#include "response.h"

#define DEFAULT_CACHE_SIZE 2

namespace kiwix {

namespace
{

void error(const std::string& msg) {
  throw std::runtime_error(msg);
}

bool ipAvailable(const std::string addr)
{
  auto interfaces = kiwix::getNetworkInterfacesIPv4Or6();

  for (const auto& kv : interfaces) {
    const auto& interfaceIps = kv.second;
    if ((interfaceIps.addr == addr) || (interfaceIps.addr6 == addr)) {
      return true;
    }
  }

  return false;
}

class InSockAddr
{
private: // data
  struct sockaddr_in  v4 = {0};
  struct sockaddr_in6 v6 = {0};

public: // functions
  explicit InSockAddr(int port)
  {
    v4.sin_family  = AF_INET;
    v4.sin_port    = htons(port);
    v6.sin6_family = AF_INET6;
    v6.sin6_port   = htons(port);
  }

  IpAddress setAnyAddress(IpMode ipMode)
  {
    v6.sin6_addr = in6addr_any;
    v4.sin_addr.s_addr = htonl(INADDR_ANY);

    IpAddress a = kiwix::getBestPublicIps();
    if (ipMode == IpMode::IPV6) {
      a.addr = "";
    } else if (ipMode == IpMode::IPV4) {
      a.addr6 = "";
    }

    return a;
  }

  IpMode setAddress(const IpAddress& a)
  {
    const std::string addrStr = !a.addr.empty() ? a.addr : a.addr6;

    const int r1 = inet_pton(AF_INET,  a.addr.c_str(),  &v4.sin_addr.s_addr);
    const int r2 = inet_pton(AF_INET6, a.addr6.c_str(), &v6.sin6_addr.s6_addr);

    if ( r1 != 1 && r2 != 1 ) {
      error("invalid IP address: " + addrStr);
    }

    if ( !ipAvailable(addrStr) ) {
      error("IP address is not available on this system: " + addrStr);
    }

    return !a.addr.empty() ? IpMode::IPV4 : IpMode::IPV6;
  }

  struct sockaddr* sockaddr(IpMode ipMode) const
  {
    return (ipMode==IpMode::ALL || ipMode==IpMode::IPV6)
         ? (struct sockaddr*)&v6
         : (struct sockaddr*)&v4;
  }

};

int getMHDFlags(IpMode ipMode, bool verbose)
{
#ifdef _WIN32
  int flags = MHD_USE_SELECT_INTERNALLY;
#else
  int flags = MHD_USE_POLL_INTERNALLY;
#endif

  if (ipMode == IpMode::ALL) {
    flags |= MHD_USE_DUAL_STACK;
  } else if (ipMode == IpMode::IPV6) {
    flags |= MHD_USE_IPv6;
  }

  if (verbose) {
    flags |= MHD_USE_DEBUG;
  }

  return flags;
}

std::string
fullURL2LocalURL(const std::string& fullUrl, const std::string& rootLocation)
{
  if ( kiwix::startsWith(fullUrl, rootLocation) ) {
    return fullUrl.substr(rootLocation.size());
  } else {
    return "INVALID URL";
  }
}

std::string getSearchComponent(const RequestContext& request)
{
    const std::string query = request.get_query();
    return query.empty() ? query : "?" + query;
}

Filter get_search_filter(const RequestContext& request, const std::string& prefix="", bool catalogOnlyMode = false)
{
    auto filter = kiwix::Filter();
    if ( !catalogOnlyMode ) {
      filter.valid(true).local(true);
    }
    try {
      filter.query(request.get_argument(prefix+"q"));
    } catch (const std::out_of_range&) {}
    try {
      filter.maxSize(request.get_argument<unsigned long>(prefix+"maxsize"));
    } catch (...) {}
    try {
      filter.name(request.get_argument(prefix+"name"));
    } catch (const std::out_of_range&) {}
    try {
      filter.category(request.get_argument(prefix+"category"));
    } catch (const std::out_of_range&) {}
    try {
      filter.lang(request.get_argument(prefix+"lang"));
    } catch (const std::out_of_range&) {}
    try {
      filter.acceptTags(kiwix::split(request.get_argument(prefix+"tag"), ";"));
    } catch (...) {}
    try {
      filter.rejectTags(kiwix::split(request.get_argument(prefix+"notag"), ";"));
    } catch (...) {}
    return filter;
}

template<class T>
std::vector<T> subrange(const std::vector<T>& v, size_t s, size_t n)
{
  const size_t e = std::min(v.size(), s+n);
  return std::vector<T>(v.begin()+std::min(v.size(), s), v.begin()+e);
}

std::string renderUrl(const std::string& root, const std::string& urlTemplate)
{
  MustacheData data;
  data.set("root", root);
  auto url = kainjow::mustache::mustache(urlTemplate).render(data);
  if ( url.back() == '\n' )
    url.pop_back();
  return url;
}

ParameterizedMessage noSuchBookErrorMsg(const std::string& bookName)
{
  return ParameterizedMessage("no-such-book", { {"BOOK_NAME", bookName} });
}

ParameterizedMessage invalidRawAccessMsg(const std::string& dt)
{
  return ParameterizedMessage("invalid-raw-data-type", { {"DATATYPE", dt} });
}

ParameterizedMessage noValueForArgMsg(const std::string& argument)
{
  return ParameterizedMessage("no-value-for-arg", { {"ARGUMENT", argument} });
}

ParameterizedMessage rawEntryNotFoundMsg(const std::string& dt, const std::string& entry)
{
  return ParameterizedMessage("raw-entry-not-found",
                              {
                                {"DATATYPE", dt},
                                {"ENTRY", entry},
                              }
  );
}

ParameterizedMessage tooManyBooksMsg(size_t nbBooks, size_t limit)
{
  return ParameterizedMessage("too-many-books",
                              {
                                {"NB_BOOKS", beautifyInteger(nbBooks)},
                                {"LIMIT", beautifyInteger(limit)},
                              }
  );
}

struct Error : public std::runtime_error {
  explicit Error(const ParameterizedMessage& message)
    : std::runtime_error("Error while handling request"),
      _message(message)
  {}

  const ParameterizedMessage& message() const
  {
    return _message;
  }

  const ParameterizedMessage _message;
};

void checkBookNumber(const Library::BookIdSet& bookIds, size_t limit) {
  if (bookIds.empty()) {
    throw Error(nonParameterizedMessage("no-book-found"));
  }
  if (limit > 0 && bookIds.size() > limit) {
    throw Error(tooManyBooksMsg(bookIds.size(), limit));
  }
}

typedef std::set<std::string> Languages;

Languages getLanguages(const Library& lib, const Library::BookIdSet& bookIds) {
  Languages langs;
  for ( const auto& b : bookIds ) {
    const auto bookLangs = lib.getBookById(b).getLanguages();
    langs.insert(bookLangs.begin(), bookLangs.end());
  }
  return langs;
}

struct CustomizedResourceData
{
  std::string mimeType;
  std::string resourceFilePath;
};

bool responseMustBeETaggedWithLibraryId(const Response& response, const RequestContext& request)
{
  return response.getReturnCode() == MHD_HTTP_OK
      && response.get_kind() == Response::DYNAMIC_CONTENT
      && request.get_url() != "/random";
}

ETag
get_matching_if_none_match_etag(const RequestContext& r, const std::string& etagBody)
{
  try {
    const std::string etag_list = r.get_header(MHD_HTTP_HEADER_IF_NONE_MATCH);
    return ETag::match(etag_list, etagBody);
  } catch (const std::out_of_range&) {
    return ETag();
  }
}

struct NoDelete
{
  template<class T> void operator()(T*) {}
};

} // unnamed namespace

std::pair<std::string, Library::BookIdSet> InternalServer::selectBooks(const RequestContext& request) const
{
  // Try old API
  try {
    auto bookName = request.get_argument("content");
    try {
      const auto bookIds = Library::BookIdSet{mp_nameMapper->getIdForName(bookName)};
      const auto queryString = request.get_query([&](const std::string& key){return key == "content";}, true);
      return {queryString, bookIds};
    } catch (const std::out_of_range&) {
      throw Error(noSuchBookErrorMsg(bookName));
    }
  } catch(const std::out_of_range&) {
    // We've catch the out_of_range of get_argument
    // continue
  }

  // Does user directly gives us ids ?
  try {
    auto id_vec = request.get_arguments("books.id");
    if (id_vec.empty()) {
      throw Error(noValueForArgMsg("books.id"));
    }
    for(const auto& bookId: id_vec) {
      try {
        // This is a silly way to check that bookId exists
        mp_nameMapper->getNameForId(bookId);
      } catch (const std::out_of_range&) {
        throw Error(noSuchBookErrorMsg(bookId));
      }
    }
    const auto bookIds = Library::BookIdSet(id_vec.begin(), id_vec.end());
    const auto queryString = request.get_query([&](const std::string& key){return key == "books.id";}, true);
    return {queryString, bookIds};
  } catch(const std::out_of_range&) {}

  // Use the names
  try {
    auto name_vec = request.get_arguments("books.name");
    if (name_vec.empty()) {
      throw Error(noValueForArgMsg("books.name"));
    }
    Library::BookIdSet bookIds;
    for(const auto& bookName: name_vec) {
      try {
        bookIds.insert(mp_nameMapper->getIdForName(bookName));
      } catch(const std::out_of_range&) {
        throw Error(noSuchBookErrorMsg(bookName));
      }
    }
    const auto queryString = request.get_query([&](const std::string& key){return key == "books.name";}, true);
    return {queryString, bookIds};
  } catch(const std::out_of_range&) {}

  // Check for filtering
  Filter filter = get_search_filter(request, "books.filter.");
  auto id_vec = mp_library->filter(filter);
  if (id_vec.empty()) {
    throw Error(nonParameterizedMessage("no-book-found"));
  }
  const auto bookIds = Library::BookIdSet(id_vec.begin(), id_vec.end());
  const auto queryString = request.get_query([&](const std::string& key){return startsWith(key, "books.filter.");}, true);
  return {queryString, bookIds};
}

SearchInfo InternalServer::getSearchInfo(const RequestContext& request) const
{
  auto bookIds = selectBooks(request);
  checkBookNumber(bookIds.second, m_multizimSearchLimit);
  if ( getLanguages(*mp_library, bookIds.second).size() != 1 ) {
    throw Error(nonParameterizedMessage("confusion-of-tongues"));
  }

  auto pattern = request.get_optional_param<std::string>("pattern", "");
  GeoQuery geoQuery;

  /* Retrive geo search */
  try {
    auto latitude = request.get_argument<float>("latitude");
    auto longitude = request.get_argument<float>("longitude");
    auto distance = request.get_argument<float>("distance");
    geoQuery = GeoQuery(latitude, longitude, distance);
  } catch(const std::out_of_range&) {}
    catch(const std::invalid_argument&) {}
  return SearchInfo(pattern, geoQuery, bookIds.second, bookIds.first);
}

SearchInfo::SearchInfo(const std::string& pattern, GeoQuery geoQuery, const Library::BookIdSet& bookIds, const std::string& bookFilterQuery)
  : pattern(pattern),
    geoQuery(geoQuery),
    bookIds(bookIds),
    bookFilterQuery(bookFilterQuery)
{}

zim::Query SearchInfo::getZimQuery(bool verbose) const {
  zim::Query query;
  if (verbose) {
    std::cout << "Performing query '" << pattern<< "'";
  }
  query.setQuery(kiwix::trim(pattern));
  if (geoQuery) {
    if (verbose) {
      std::cout << " with geo query '" << geoQuery.distance << "&(" << geoQuery.latitude << ";" << geoQuery.longitude << ")'";
    }
    query.setGeorange(geoQuery.latitude, geoQuery.longitude, geoQuery.distance);
  }
  if (verbose) {
    std::cout << std::endl;
  }
  return query;
}

static IdNameMapper defaultNameMapper;

static MHD_Result staticHandlerCallback(void* cls,
                                        struct MHD_Connection* connection,
                                        const char* url,
                                        const char* method,
                                        const char* version,
                                        const char* upload_data,
                                        size_t* upload_data_size,
                                        void** cont_cls);

class InternalServer::CustomizedResources : public std::map<std::string, CustomizedResourceData>
{
public:
  CustomizedResources()
  {
    const char* fname = ::getenv("KIWIX_SERVE_CUSTOMIZED_RESOURCES");
    if ( fname )
    {
      std::cout << "Populating customized resources" << std::endl;
      std::ifstream file(fname);
      std::string url, mimeType, resourceFilePath;
      while ( file >> url >> mimeType >> resourceFilePath )
      {
        std::cout << "Got " << url << " " << mimeType << " " << resourceFilePath << std::endl;
        (*this)[url] = CustomizedResourceData{mimeType, resourceFilePath};
      }
      std::cout << "Done populating customized resources" << std::endl;
    }
  }
};


InternalServer::InternalServer(LibraryPtr library,
                               std::shared_ptr<NameMapper> nameMapper,
                               IpAddress addr,
                               int port,
                               std::string root,
                               int nbThreads,
                               unsigned int multizimSearchLimit,
                               bool verbose,
                               bool withTaskbar,
                               bool withLibraryButton,
                               bool blockExternalLinks,
                               IpMode ipMode,
                               std::string indexTemplateString,
                               int ipConnectionLimit,
                               bool catalogOnlyMode,
                               std::string contentServerUrl) :
  m_addr(addr),
  m_port(port),
  m_root(root),
  m_rootPrefixOfDecodedURL(m_root),
  m_nbThreads(nbThreads),
  m_multizimSearchLimit(multizimSearchLimit),
  m_verbose(verbose),
  m_withTaskbar(withTaskbar),
  m_withLibraryButton(withLibraryButton),
  m_blockExternalLinks(blockExternalLinks),
  m_ipMode(ipMode),
  m_indexTemplateString(indexTemplateString.empty() ? RESOURCE::templates::index_html : indexTemplateString),
  m_ipConnectionLimit(ipConnectionLimit),
  mp_daemon(nullptr),
  mp_library(library),
  mp_nameMapper(nameMapper ? nameMapper : std::shared_ptr<NameMapper>(&defaultNameMapper, NoDelete())),
  searchCache(getEnvVar<int>("KIWIX_SEARCH_CACHE_SIZE", DEFAULT_CACHE_SIZE)),
  suggestionSearcherCache(getEnvVar<int>("KIWIX_SUGGESTION_SEARCHER_CACHE_SIZE", std::max((unsigned int) (mp_library->getBookCount(true, true)*0.1), 1U))),
  m_customizedResources(new CustomizedResources),
  m_catalogOnlyMode(catalogOnlyMode),
  m_contentServerUrl(contentServerUrl)
{
  m_root = urlEncode(m_root);
}

InternalServer::~InternalServer() = default;

struct MHD_Daemon* InternalServer::startMHD(int flags,
                                            struct sockaddr* sockaddr)
{
  return MHD_start_daemon(flags,
                          m_port,
                          NULL,
                          NULL,
                          &staticHandlerCallback,
                          this,
                          MHD_OPTION_SOCK_ADDR, sockaddr,
                          MHD_OPTION_THREAD_POOL_SIZE, m_nbThreads,
                          MHD_OPTION_PER_IP_CONNECTION_LIMIT, m_ipConnectionLimit,
                          MHD_OPTION_END);
}

void InternalServer::startMHD() {
  InSockAddr inSockAddr(m_port);
  if (m_addr.addr.empty() && m_addr.addr6.empty()) { // No ip address provided
    if (m_ipMode == IpMode::AUTO) m_ipMode = IpMode::ALL;
    m_addr = inSockAddr.setAnyAddress(m_ipMode);
  } else {
    if (m_ipMode != kiwix::IpMode::AUTO) {
      error("When an IP address is provided the IP mode must not be set");
    }

    m_ipMode = inSockAddr.setAddress(m_addr);
  }

  const int flags = getMHDFlags(m_ipMode, m_verbose.load());
  mp_daemon = startMHD(flags, inSockAddr.sockaddr(m_ipMode));
  if (mp_daemon == nullptr && m_ipMode == IpMode::ALL) {
    // MHD_USE_DUAL_STACK (set in IpMode::ALL case) fails on systems with IPv6
    // disabled. Let's retry in IPv4-only mode.
    m_ipMode = IpMode::IPV4;
    m_addr.addr6 = "";
    const int flags = getMHDFlags(m_ipMode, m_verbose.load());
    mp_daemon = startMHD(flags, inSockAddr.sockaddr(m_ipMode));
  }

  if (mp_daemon == nullptr) {
    error("Unable to instantiate the HTTP daemon. "
          "The port " + kiwix::to_string(m_port) + " is maybe already occupied"
          " or need more permissions to be open. "
          "Please try as root or with a port number higher or equal to 1024.");
  }
}

bool InternalServer::start() {
  try {
    startMHD();
  } catch (const std::runtime_error& err ) {
    std::cerr << "ERROR: " << err.what() << std::endl;
    return false;
  }

  auto server_start_time = std::chrono::system_clock::now().time_since_epoch();
  m_server_id = kiwix::to_string(server_start_time.count());
  return true;
}

void InternalServer::stop()
{
  MHD_stop_daemon(mp_daemon);
}

static MHD_Result staticHandlerCallback(void* cls,
                                        struct MHD_Connection* connection,
                                        const char* url,
                                        const char* method,
                                        const char* version,
                                        const char* upload_data,
                                        size_t* upload_data_size,
                                        void** cont_cls)
{
  InternalServer* _this = static_cast<InternalServer*>(cls);

  return _this->handlerCallback(connection,
                                url,
                                method,
                                version,
                                upload_data,
                                upload_data_size,
                                cont_cls);
}

namespace
{

MHD_Result add_name_value_pair(void *nvp, enum MHD_ValueKind kind,
                               const char *key, const char *value)
{
  auto& nameValuePairs = *reinterpret_cast<RequestContext::NameValuePairs*>(nvp);
  nameValuePairs.push_back({key, value});
  return MHD_YES;
}

} // unnamed namespace

MHD_Result InternalServer::handlerCallback(struct MHD_Connection* connection,
                                           const char* fullUrl,
                                           const char* method,
                                           const char* version,
                                           const char* upload_data,
                                           size_t* upload_data_size,
                                           void** cont_cls)
{
  auto start_time = std::chrono::steady_clock::now();
  if (m_verbose.load() ) {
    printf("======================\n");
    printf("Requesting : \n");
    printf("full_url  : %s\n", fullUrl);
  }

  const auto url = fullURL2LocalURL(fullUrl, m_rootPrefixOfDecodedURL);
  RequestContext::NameValuePairs headers, queryArgs;
  MHD_get_connection_values(connection, MHD_HEADER_KIND, add_name_value_pair, &headers);
  MHD_get_connection_values(connection, MHD_GET_ARGUMENT_KIND, add_name_value_pair, &queryArgs);
  RequestContext request(m_root, url, method, version, headers, queryArgs);

  if (m_verbose.load() ) {
    request.print_debug_info();
  }
  /* Unexpected method */
  if (request.get_method() != RequestMethod::GET
   && request.get_method() != RequestMethod::POST
   && request.get_method() != RequestMethod::HEAD) {
    printf("Reject request because of unhandled request method.\n");
    printf("----------------------\n");
    return MHD_NO;
  }

  auto response = handle_request(request);

  if (response->getReturnCode() == MHD_HTTP_INTERNAL_SERVER_ERROR) {
    printf("========== INTERNAL ERROR !! ============\n");
    if (!m_verbose.load()) {
      printf("Requesting : \n");
      printf("full_url : %s\n", fullUrl);
      request.print_debug_info();
    }
  }

  if ( responseMustBeETaggedWithLibraryId(*response, request) ) {
    response->set_etag_body(getLibraryId());
  }

  auto ret = response->send(request, m_verbose.load(), connection);
  auto end_time = std::chrono::steady_clock::now();
  auto time_span = std::chrono::duration_cast<std::chrono::duration<double>>(end_time - start_time);
  if (m_verbose.load()) {
    printf("Request time : %fs\n", time_span.count());
    printf("----------------------\n");
  }
  return ret;
}

namespace
{

bool isEndpointUrl(const std::string& url, const std::string& endpoint)
{
  return startsWith(url, "/" + endpoint + "/") || url == "/" + endpoint;
};

} // unnamed namespace

std::string InternalServer::getLibraryId() const
{
  return m_server_id + "." + kiwix::to_string(mp_library->getRevision());
}

std::unique_ptr<Response> InternalServer::handle_request(const RequestContext& request)
{
  try {
    if (! request.is_valid_url()) {
      return UrlNotFoundResponse(request);
    }

    if ( request.get_url() == "" ) {
      // Redirect /ROOT_LOCATION to /ROOT_LOCATION/ (note the added slash)
      // so that relative URLs are resolved correctly
      const std::string query = getSearchComponent(request);
      return Response::build_redirect(m_root + "/" + query);
    }

    const ETag etag = get_matching_if_none_match_etag(request, getLibraryId());
    if ( etag )
      return Response::build_304(etag);

    const auto url = request.get_url();
    if ( isLocallyCustomizedResource(url) )
      return handle_locally_customized_resource(request);

    if (url == "/" )
      return build_homepage(request);

    if (isEndpointUrl(url, "viewer") || isEndpointUrl(url, "skin"))
      return handle_skin(request);

    if (url == "/viewer_settings.js")
      return handle_viewer_settings(request);

    if (isEndpointUrl(url, "content"))
      return handle_content(request);

    if (isEndpointUrl(url, "catalog"))
      return handle_catalog(request);

    if (isEndpointUrl(url, "raw"))
      return handle_raw(request);

    if (isEndpointUrl(url, "search"))
      return handle_search(request);

    if (isEndpointUrl(url, "nojs"))
      return handle_no_js(request);

    if (isEndpointUrl(url, "suggest"))
     return handle_suggest(request);

    if (isEndpointUrl(url, "random"))
      return handle_random(request);

    if (isEndpointUrl(url, "catch"))
      return handle_catch(request);

    const std::string contentUrl = m_root + "/content" + urlEncode(url);
    const std::string query = getSearchComponent(request);
    return Response::build_redirect(contentUrl + query);
  } catch (std::exception& e) {
    fprintf(stderr, "===== Unhandled error : %s\n", e.what());
    return HTTP500Response(request, m_root, request.get_full_url(), e.what());
  } catch (...) {
    fprintf(stderr, "===== Unhandled unknown error\n");
    return HTTP500Response(request, m_root, request.get_full_url());
  }
}

MustacheData InternalServer::get_default_data() const
{
  MustacheData data;
  data.set("root", m_root);
  return data;
}

std::unique_ptr<Response> InternalServer::build_homepage(const RequestContext& request)
{
  return ContentResponse::build(m_indexTemplateString, get_default_data(), "text/html; charset=utf-8");
}

/**
 * Archive and Zim handlers begin
 **/

class InternalServer::LockableSuggestionSearcher : public zim::SuggestionSearcher
{
  public:
    explicit LockableSuggestionSearcher(const zim::Archive& archive)
      : zim::SuggestionSearcher(archive)
    {}

    std::unique_lock<std::mutex> getLock() {
      return std::unique_lock<std::mutex>(m_mutex);
    }
    virtual ~LockableSuggestionSearcher() = default;
  private:
    std::mutex m_mutex;
};

std::unique_ptr<Response> InternalServer::handle_suggest(const RequestContext& request)
{
  if (m_verbose.load()) {
    printf("** running handle_suggest\n");
  }

  if ( startsWith(request.get_url(), "/suggest/") ) {
    return UrlNotFoundResponse(request);
  }

  std::string bookName, bookId;
  std::shared_ptr<zim::Archive> archive;
  try {
    bookName = request.get_argument("content");
    bookId = mp_nameMapper->getIdForName(bookName);
    archive = mp_library->getArchiveById(bookId);
  } catch (const std::out_of_range&) {
    // error handled by the archive == nullptr check below
  }

  if (archive == nullptr) {
    return HTTP404Response(request)
           + noSuchBookErrorMsg(bookName);
  }

  const auto queryString = request.get_optional_param("term", std::string());
  const auto start = request.get_optional_param<unsigned int>("start", 0);
  unsigned int count = request.get_optional_param<unsigned int>("count", 10);
  if (count == 0) {
    count = 10;
  }

  if (m_verbose.load()) {
    printf("Searching suggestions for: \"%s\"\n", queryString.c_str());
  }

  Suggestions results;

  /* Get the suggestions */
  auto searcher = suggestionSearcherCache.getOrPut(bookId,
    [=](){ return make_shared<LockableSuggestionSearcher>(*archive); }
  );
  const auto lock(searcher->getLock());
  auto search = searcher->suggest(queryString);
  auto srs = search.getResults(start, count);

  for(auto& suggestion: srs) {
    results.add(suggestion);
  }


  /* Propose the fulltext search if possible */
  if (archive->hasFulltextIndex()) {
    results.addFTSearchSuggestion(request.get_user_language(), queryString);
  }

  return ContentResponse::build(results.getJSON(), "application/json; charset=utf-8");
}

std::unique_ptr<Response> InternalServer::handle_viewer_settings(const RequestContext& request)
{
  if (m_verbose.load()) {
    printf("** running handle_viewer_settings\n");
  }

  const kainjow::mustache::object data{
    {"enable_toolbar", m_withTaskbar ? "true" : "false" },
    {"enable_link_blocking", m_blockExternalLinks ? "true" : "false" },
    {"enable_library_button", m_withLibraryButton ? "true" : "false" }
  };
  return ContentResponse::build(RESOURCE::templates::viewer_settings_js, data, "application/javascript; charset=utf-8");
}

std::string InternalServer::getNoJSDownloadPageHTML(const std::string& bookId, const std::string& userLang) const
{
  const auto book = mp_library->getBookById(bookId);
  auto bookUrl = kiwix::stripSuffix(book.getUrl(), ".meta4");
  auto getTranslation = i18n::GetTranslatedStringWithMsgId(userLang);
  const auto translations = kainjow::mustache::object{
                            getTranslation("download-links-heading", {{"BOOK_TITLE", book.getTitle()}}),
                            getTranslation("download-links-title"),
                            getTranslation("direct-download-link-text"),
                            getTranslation("hash-download-link-text"),
                            getTranslation("magnet-link-text"),
                            getTranslation("torrent-download-link-text")
  };

  return render_template(
             RESOURCE::templates::no_js_download_html,
             kainjow::mustache::object{
               {"url", bookUrl},
               {"translations", translations}
             }
  );
}

void InternalServer::setContentAccessUrl(LibraryDumper& libDumper) const
{
  if ( !m_contentServerUrl.empty() ) {
    libDumper.setContentAccessUrl(m_contentServerUrl + "/content");
  } else if ( !m_catalogOnlyMode ) {
    libDumper.setContentAccessUrl(m_root + "/content");
  }
}

std::unique_ptr<Response> InternalServer::handle_no_js(const RequestContext& request)
{
  const auto url = request.get_url();
  const auto urlParts = kiwix::split(url, "/", true, false);
  HTMLDumper htmlDumper(mp_library.get(), mp_nameMapper.get());
  htmlDumper.setRootLocation(m_root);
  htmlDumper.setLibraryId(getLibraryId());
  setContentAccessUrl(htmlDumper);
  auto userLang = request.get_user_language();
  htmlDumper.setUserLanguage(userLang);
  std::string content;

  if (urlParts.size() == 1) {
    auto filter = get_search_filter(request, "", m_catalogOnlyMode);
    try {
      if (request.get_argument("category") == "") {
        filter.clearCategory();
      }
    } catch (...) {}
    try {
      if (request.get_argument("lang") == "") {
        filter.clearLang();
      }
    } catch (...) {}
    content = htmlDumper.dumpPlainHTML(filter);
  } else if ((urlParts.size() == 3) && (urlParts[1] == "download")) {
    try {
      const auto bookId = mp_nameMapper->getIdForName(urlParts[2]);
      content = getNoJSDownloadPageHTML(bookId, userLang);
    } catch (const std::out_of_range&) {
      return UrlNotFoundResponse(request);
    }
  } else {
    return UrlNotFoundResponse(request);
  }

  return ContentResponse::build(content, "text/html; charset=utf-8");
}

namespace
{

Response::Kind staticResourceAccessType(const RequestContext& req, const char* expectedCacheid)
{
  if ( expectedCacheid == nullptr )
    return Response::DYNAMIC_CONTENT;

  try {
    if ( expectedCacheid != req.get_argument("cacheid") )
      throw ResourceNotFound("Wrong cacheid");
    return Response::STATIC_RESOURCE;
  } catch( const std::out_of_range& ) {
    return Response::DYNAMIC_CONTENT;
  }
}

} // unnamed namespace

std::unique_ptr<Response> InternalServer::handle_skin(const RequestContext& request)
{
  if (m_verbose.load()) {
    printf("** running handle_skin\n");
  }

  const bool isRequestForViewer = request.get_url() == "/viewer";
  auto resourceName = isRequestForViewer
                    ? "viewer.html"
                    : request.get_url().substr(1);

  const char* const resourceCacheId = getResourceCacheId(resourceName);

  try {
    const auto accessType = staticResourceAccessType(request, resourceCacheId);
    auto response = ContentResponse::build(
        getResource(resourceName),
        getMimeTypeForFile(resourceName));
    response->set_kind(accessType);
    return std::move(response);
  } catch (const ResourceNotFound& e) {
    return UrlNotFoundResponse(request);
  }
}

std::unique_ptr<Response> InternalServer::handle_search(const RequestContext& request)
{
  if (m_verbose.load()) {
    printf("** running handle_search\n");
  }

  if ( startsWith(request.get_url(), "/search/") ) {
    if (request.get_url() == "/search/searchdescription.xml") {
      return ContentResponse::build(
        RESOURCE::ft_opensearchdescription_xml,
        get_default_data(),
        "application/opensearchdescription+xml");
    }
    return UrlNotFoundResponse(request);
  }

  try {
    return handle_search_request(request);
  } catch (const Error& e) {
    return HTTP400Response(request)
      + e.message();
  }
}

namespace
{

unsigned getSearchPageSize(const RequestContext& r)
{
  const auto DEFAULT_PAGE_LEN = 25u;
  const auto MAX_PAGE_LEN = 140u;

  const auto pageLength = r.get_optional_param("pageLength", DEFAULT_PAGE_LEN);
  return pageLength == 0
       ? DEFAULT_PAGE_LEN
       : min(MAX_PAGE_LEN, pageLength);
}

} // unnamed namespace

std::unique_ptr<Response> InternalServer::handle_search_request(const RequestContext& request)
{
  auto searchInfo = getSearchInfo(request);
  auto bookIds = searchInfo.getBookIds();

  /* Make the search */
  // Try to get a search from the searchInfo, else build it
  auto searcher = mp_library->getSearcherByIds(bookIds);
  auto lock(searcher->getLock());

  std::shared_ptr<zim::Search> search;
  try {
    search = searchCache.getOrPut(searchInfo,
      [=](){
        return make_shared<zim::Search>(searcher->search(searchInfo.getZimQuery(m_verbose.load())));
      }
    );
  } catch(std::runtime_error& e) {
    // Searcher->search will throw a runtime error if there is no valid xapian database to do the search.
    // (in case of zim file not containing a index)
    const auto cssUrl = renderUrl(m_root, RESOURCE::templates::url_of_search_results_css_tmpl);
    HTTPErrorResponse response(request, MHD_HTTP_NOT_FOUND,
                               "fulltext-search-unavailable",
                               "404-page-heading",
                               cssUrl,
                               /*includeKiwixResponseData=*/true);
    response += nonParameterizedMessage("no-search-results");
    // XXX: Now this has to be handled by the iframe-based viewer which
    // XXX: has to resolve if the book selection resulted in a single book.
    /*
    if(bookIds.size() == 1) {
      auto bookId = *bookIds.begin();
      auto bookName = mp_nameMapper->getNameForId(bookId);
      response += TaskbarInfo(bookName, mp_library->getArchiveById(bookId).get());
    }
    */
    return response;
  }

  const auto start = max(0u, request.get_optional_param("start", 0u));
  const auto pageLength = getSearchPageSize(request);

  /* Get the results */
  SearchRenderer renderer(search->getResults(start, pageLength), start,
                          search->getEstimatedMatches());
  renderer.setSearchPattern(searchInfo.pattern);
  renderer.setSearchBookQuery(searchInfo.bookFilterQuery);
  renderer.setProtocolPrefix(m_root + "/content/");
  renderer.setSearchProtocolPrefix(m_root + "/search");
  renderer.setPageLength(pageLength);
  renderer.setUserLang(request.get_user_language());
  if (request.get_requested_format() == "xml") {
    return ContentResponse::build(
      renderer.getXml(*mp_nameMapper, mp_library.get()),
      "application/rss+xml; charset=utf-8"
    );
  }
  auto response = ContentResponse::build(
    renderer.getHtml(*mp_nameMapper, mp_library.get()),
    "text/html; charset=utf-8"
  );
  // XXX: Now this has to be handled by the iframe-based viewer which
  // XXX: has to resolve if the book selection resulted in a single book.
  /*
  if(bookIds.size() == 1) {
    auto bookId = *bookIds.begin();
    auto bookName = mp_nameMapper->getNameForId(bookId);
    response->set_taskbar(bookName, mp_library->getArchiveById(bookId).get());
  }
  */
  return std::move(response);
}

std::unique_ptr<Response> InternalServer::handle_random(const RequestContext& request)
{
  if (m_verbose.load()) {
    printf("** running handle_random\n");
  }

  if ( startsWith(request.get_url(), "/random/") ) {
    return UrlNotFoundResponse(request);
  }

  std::string bookName;
  std::shared_ptr<zim::Archive> archive;
  try {
    bookName = request.get_argument("content");
    const std::string bookId = mp_nameMapper->getIdForName(bookName);
    archive = mp_library->getArchiveById(bookId);
  } catch (const std::out_of_range&) {
    // error handled by the archive == nullptr check below
  }

  if (archive == nullptr) {
    return HTTP404Response(request)
           + noSuchBookErrorMsg(bookName);
  }

  try {
    auto entry = archive->getRandomEntry();
    return build_redirect(bookName, getFinalItem(*archive, entry));
  } catch(zim::EntryNotFound& e) {
    return HTTP404Response(request)
           + nonParameterizedMessage("random-article-failure");
  }
}

std::unique_ptr<Response> InternalServer::handle_captured_external(const RequestContext& request)
{
  std::string source = "";
  try {
    source = kiwix::urlDecode(request.get_argument("source"));
  } catch (const std::out_of_range& e) {}

  if (source.empty()) {
    return UrlNotFoundResponse(request);
  }

  return BlockExternalLinkResponse(request, m_root, source);
}

std::unique_ptr<Response> InternalServer::handle_catch(const RequestContext& request)
{
  if (m_verbose.load()) {
    printf("** running handle_catch\n");
  }

  if ( request.get_url() == "/catch/external" ) {
    return handle_captured_external(request);
  }

  return UrlNotFoundResponse(request);
}

std::vector<std::string>
InternalServer::search_catalog(const RequestContext& request,
                               kiwix::OPDSDumper& opdsDumper)
{
    const auto filter = get_search_filter(request, "", m_catalogOnlyMode);
    std::vector<std::string> bookIdsToDump = mp_library->filter(filter);
    const auto totalResults = bookIdsToDump.size();
    const long count = request.get_optional_param("count", 10L);
    const size_t startIndex = request.get_optional_param("start", 0UL);
    const size_t intendedCount = count >= 0 ? count : bookIdsToDump.size();
    bookIdsToDump = subrange(bookIdsToDump, startIndex, intendedCount);
    opdsDumper.setOpenSearchInfo(totalResults, startIndex, bookIdsToDump.size());
    return bookIdsToDump;
}

namespace
{

///////////////////////////////////////////////////////////////////////////////
// The content security policy below is set on responses to the /content
// endpoint in order to prevent the ZIM content from interfering with the
// viewer (e.g. breaking out of the viewer iframe by performing top-level
// navigation).
const std::string CONTENT_CSP_HEADER =
    "default-src 'self' "
                "data: "
                "blob: "
                "about: "
                "'unsafe-inline' "
                "'unsafe-eval'; "

    "sandbox allow-scripts "
            "allow-same-origin "
            "allow-modals "
            "allow-popups "
            "allow-popups-to-escape-sandbox "
            "allow-forms "
            "allow-downloads;";

// End of content security policy
///////////////////////////////////////////////////////////////////////////////

} // unnamed namespace

std::unique_ptr<Response>
InternalServer::build_redirect(const std::string& bookName, const zim::Item& item) const
{
  const auto contentPath = "/content/" + bookName + "/" + item.getPath();
  const auto url = m_root + kiwix::urlEncode(contentPath);
  return Response::build_redirect(url);
}

std::unique_ptr<Response> InternalServer::handle_content(const RequestContext& request)
{
  const std::string url = request.get_url();
  const std::string pattern = url.substr((url.find_last_of('/'))+1);
  if (m_verbose.load()) {
    printf("** running handle_content\n");
  }

  const std::string contentPrefix = "/content/";
  const bool isContentPrefixedUrl = startsWith(url, contentPrefix);
  const size_t prefixLength = isContentPrefixedUrl ? contentPrefix.size() : 1;
  const std::string bookName = request.get_url_part(isContentPrefixedUrl);

  std::shared_ptr<zim::Archive> archive;
  try {
    const std::string bookId = mp_nameMapper->getIdForName(bookName);
    archive = mp_library->getArchiveById(bookId);
  } catch (const std::out_of_range& e) {}

  if (archive == nullptr) {
    return NewHTTP404Response(request, m_root, m_root + url);
  }

  const std::string archiveUuid(archive->getUuid());
  const ETag etag = get_matching_if_none_match_etag(request, archiveUuid);
  if ( etag )
    return Response::build_304(etag);

  auto urlStr = url.substr(prefixLength + bookName.size());
  if (urlStr[0] == '/') {
    urlStr = urlStr.substr(1);
  }

  try {
    auto entry = getEntryFromPath(*archive, urlStr);
    if (entry.isRedirect() || urlStr != entry.getPath()) {
      // In the condition above, the second case (an entry with a different
      // URL was returned) can occur in the following situations:
      // 1. urlStr is empty or equal to "/" and the ZIM file doesn't contain
      //    such an entry, in which case the main entry is returned instead.
      // 2. The ZIM file uses old namespace scheme, and the resource at urlStr
      //    is not present but can be found under one of the 'A', 'I', 'J' or
      //    '-' namespaces, in which case that resource is returned instead.
      return build_redirect(bookName, getFinalItem(*archive, entry));
    }
    auto response = ItemResponse::build(request, entry.getItem());
    response->set_etag_body(archiveUuid);

    if ( !startsWith(entry.getItem().getMimetype(), "application/pdf") ) {
      // NOTE: Content security policy is not applied to PDF content so that
      // NOTE: it can be displayed in the viewer in Chromium-based browsers.
      response->add_header("Content-Security-Policy", CONTENT_CSP_HEADER);
      response->add_header("Referrer-Policy", "no-referrer");
    }

    if (m_verbose.load()) {
      printf("Found %s\n", entry.getPath().c_str());
      printf("mimeType: %s\n", entry.getItem(true).getMimetype().c_str());
    }

    return response;
  } catch(zim::EntryNotFound& e) {
    if (m_verbose.load())
      printf("Failed to find %s\n", urlStr.c_str());

    return NewHTTP404Response(request, m_root, m_root + url);
  }
}


std::unique_ptr<Response> InternalServer::handle_raw(const RequestContext& request)
{
  if (m_verbose.load()) {
    printf("** running handle_raw\n");
  }

  std::string bookName;
  std::string kind;
  try {
     bookName = request.get_url_part(1);
     kind = request.get_url_part(2);
  } catch (const std::out_of_range& e) {
    return UrlNotFoundResponse(request);
  }

  if (kind != "meta" && kind!= "content") {
    return UrlNotFoundResponse(request)
           + invalidRawAccessMsg(kind);
  }

  std::shared_ptr<zim::Archive> archive;
  try {
    const std::string bookId = mp_nameMapper->getIdForName(bookName);
    archive = mp_library->getArchiveById(bookId);
  } catch (const std::out_of_range& e) {}

  if (archive == nullptr) {
    return UrlNotFoundResponse(request)
           + noSuchBookErrorMsg(bookName);
  }

  const std::string archiveUuid(archive->getUuid());
  const ETag etag = get_matching_if_none_match_etag(request, archiveUuid);
  if ( etag )
    return Response::build_304(etag);

  // Remove the beggining of the path:
  // /raw/<bookName>/<kind>/foo
  // ^^^^^          ^      ^
  //   5      +     1  +   1   = 7
  auto itemPath = request.get_url().substr(bookName.size()+kind.size()+7);

  try {
    if (kind == "meta") {
      auto item = archive->getMetadataItem(itemPath);
      auto response = ItemResponse::build(request, item);
      response->set_etag_body(archiveUuid);
      return response;
    } else {
      auto entry = archive->getEntryByPath(itemPath);
      if (entry.isRedirect()) {
        return build_redirect(bookName, entry.getItem(true));
      }
      auto response = ItemResponse::build(request, entry.getItem());
      response->set_etag_body(archiveUuid);
      return response;
    }
  } catch (zim::EntryNotFound& e ) {
    if (m_verbose.load()) {
      printf("Failed to find %s\n", itemPath.c_str());
    }
    return UrlNotFoundResponse(request)
           + rawEntryNotFoundMsg(kind, itemPath);
  }
}

bool InternalServer::isLocallyCustomizedResource(const std::string& url) const
{
  return m_customizedResources->find(url) != m_customizedResources->end();
}

std::unique_ptr<Response> InternalServer::handle_locally_customized_resource(const RequestContext& request)
{
  if (m_verbose.load()) {
    printf("** running handle_locally_customized_resource\n");
  }

  const CustomizedResourceData& crd = m_customizedResources->at(request.get_url());

  if (m_verbose.load()) {
    std::cout << "Reading " <<  crd.resourceFilePath << std::endl;
  }
  const auto resourceData = getFileContent(crd.resourceFilePath);

  auto byteRange = request.get_range().resolve(resourceData.size());
  if (byteRange.kind() != ByteRange::RESOLVED_FULL_CONTENT) {
    return Response::build_416(resourceData.size());
  }

  return ContentResponse::build(resourceData, crd.mimeType);
}

}
