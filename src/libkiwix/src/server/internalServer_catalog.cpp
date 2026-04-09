/*
 * Copyright 2021 Veloman Yunkan <veloman.yunkan@gmail.com>
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

#include "library.h"
#include "opds_dumper.h"
#include "request_context.h"
#include "response.h"
#include "tools/otherTools.h"
#include "libkiwix-resources.h"

#include <mustache.hpp>

#include <string>
#include <vector>

namespace kiwix {

namespace
{

enum OPDSResponseKind
{
  OPDS_ENTRY,
  OPDS_NAVIGATION_FEED,
  OPDS_ACQUISITION_FEED
};

const std::string opdsMimeType[] = {
  "application/atom+xml;type=entry;profile=opds-catalog;charset=utf-8",
  "application/atom+xml;profile=opds-catalog;kind=navigation;charset=utf-8",
  "application/atom+xml;profile=opds-catalog;kind=acquisition;charset=utf-8"
};

} // unnamed namespace

OPDSDumper InternalServer::getOPDSDumper() const
{
  kiwix::OPDSDumper opdsDumper(mp_library.get(), mp_nameMapper.get());
  opdsDumper.setRootLocation(m_root);
  opdsDumper.setLibraryId(getLibraryId());
  setContentAccessUrl(opdsDumper);
  return opdsDumper;
}

std::unique_ptr<Response> InternalServer::handle_catalog(const RequestContext& request)
{
  if (m_verbose.load()) {
    printf("** running handle_catalog");
  }

  std::string host;
  std::string url;
  try {
    host = request.get_header("Host");
    url  = request.get_url_part(1);
  } catch (const std::out_of_range&) {
    return UrlNotFoundResponse(request);
  }

  if (url == "v2") {
    return handle_catalog_v2(request);
  }

  if (url != "searchdescription.xml" && url != "root.xml" && url != "search") {
    return UrlNotFoundResponse(request);
  }

  if (url == "searchdescription.xml") {
    auto response = ContentResponse::build(RESOURCE::opensearchdescription_xml, get_default_data(), "application/opensearchdescription+xml");
    return std::move(response);
  }

  zim::Uuid uuid;
  kiwix::OPDSDumper opdsDumper = getOPDSDumper();
  std::vector<std::string> bookIdsToDump;
  if (url == "root.xml") {
    uuid = zim::Uuid::generate(host);
    bookIdsToDump = mp_library->filter(kiwix::Filter().valid(true).local(true).remote(true));
  } else if (url == "search") {
    bookIdsToDump = search_catalog(request, opdsDumper);
    uuid = zim::Uuid::generate();
  }

  auto response = ContentResponse::build(
      opdsDumper.dumpOPDSFeed(bookIdsToDump, request.get_query()),
      opdsMimeType[OPDS_ACQUISITION_FEED]);
  return std::move(response);
}

std::unique_ptr<Response> InternalServer::handle_catalog_v2(const RequestContext& request)
{
  if (m_verbose.load()) {
    printf("** running handle_catalog_v2");
  }

  std::string url;
  try {
    url  = request.get_url_part(2);
  } catch (const std::out_of_range&) {
    return UrlNotFoundResponse(request);
  }

  if (url == "root.xml") {
    return handle_catalog_v2_root(request);
  } else if (url == "searchdescription.xml") {
    const std::string endpoint_root = m_root + "/catalog/v2";
    return ContentResponse::build(
        RESOURCE::catalog_v2_searchdescription_xml,
        kainjow::mustache::object({{"endpoint_root", endpoint_root}}),
        "application/opensearchdescription+xml"
    );
  } else if (url == "entry") {
    const std::string entryId  = request.get_url_part(3);
    return handle_catalog_v2_complete_entry(request, entryId);
  } else if (url == "entries") {
    return handle_catalog_v2_entries(request, /*partial=*/false);
  } else if (url == "partial_entries") {
    return handle_catalog_v2_entries(request, /*partial=*/true);
  } else if (url == "categories") {
    return handle_catalog_v2_categories(request);
  } else if (url == "languages") {
    return handle_catalog_v2_languages(request);
  } else if (url == "illustration") {
    return handle_catalog_v2_illustration(request);
  } else {
    return UrlNotFoundResponse(request);
  }
}

std::unique_ptr<Response> InternalServer::handle_catalog_v2_root(const RequestContext& request)
{
  const std::string libraryId = getLibraryId();
  return ContentResponse::build(
             RESOURCE::templates::catalog_v2_root_xml,
             kainjow::mustache::object{
               {"date", gen_date_str()},
               {"endpoint_root", m_root + "/catalog/v2"},
               {"feed_id", gen_uuid(libraryId)},
               {"all_entries_feed_id", gen_uuid(libraryId + "/entries")},
               {"partial_entries_feed_id", gen_uuid(libraryId + "/partial_entries")},
               {"category_list_feed_id", gen_uuid(libraryId + "/categories")},
               {"language_list_feed_id", gen_uuid(libraryId + "/languages")}
             },
             opdsMimeType[OPDS_NAVIGATION_FEED]
  );
}

std::unique_ptr<Response> InternalServer::handle_catalog_v2_entries(const RequestContext& request, bool partial)
{
  kiwix::OPDSDumper opdsDumper = getOPDSDumper();
  const auto bookIds = search_catalog(request, opdsDumper);
  const auto opdsFeed = opdsDumper.dumpOPDSFeedV2(bookIds, request.get_query(), partial);
  return ContentResponse::build(
             opdsFeed,
             opdsMimeType[OPDS_ACQUISITION_FEED]
  );
}

std::unique_ptr<Response> InternalServer::handle_catalog_v2_complete_entry(const RequestContext& request, const std::string& entryId)
{
  try {
    mp_library->getBookById(entryId);
  } catch (const std::out_of_range&) {
    return UrlNotFoundResponse(request);
  }

  kiwix::OPDSDumper opdsDumper = getOPDSDumper();
  const auto opdsFeed = opdsDumper.dumpOPDSCompleteEntry(entryId);
  return ContentResponse::build(
             opdsFeed,
             opdsMimeType[OPDS_ENTRY]
  );
}

std::unique_ptr<Response> InternalServer::handle_catalog_v2_categories(const RequestContext& request)
{
  kiwix::OPDSDumper opdsDumper = getOPDSDumper();
  return ContentResponse::build(
             opdsDumper.categoriesOPDSFeed(),
             opdsMimeType[OPDS_NAVIGATION_FEED]
  );
}

std::unique_ptr<Response> InternalServer::handle_catalog_v2_languages(const RequestContext& request)
{
  kiwix::OPDSDumper opdsDumper = getOPDSDumper();
  return ContentResponse::build(
             opdsDumper.languagesOPDSFeed(),
             opdsMimeType[OPDS_NAVIGATION_FEED]
  );
}

std::unique_ptr<Response> InternalServer::handle_catalog_v2_illustration(const RequestContext& request)
{
  try {
    const auto bookId  = request.get_url_part(3);
    auto book = mp_library->getBookByIdThreadSafe(bookId);
    auto size = request.get_argument<unsigned int>("size");
    auto illustration = book.getIllustration(size);
    return ContentResponse::build(
               illustration->getData(),
               illustration->mimeType
    );
  } catch(...) {
    return UrlNotFoundResponse(request);
  }
}

} // namespace kiwix
