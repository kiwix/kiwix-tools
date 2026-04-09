/*
 * Copyright 2017 Matthieu Gautier <mgautier@kymeria.fr>
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

#include "opds_dumper.h"
#include "book.h"

#include "libkiwix-resources.h"
#include <mustache.hpp>

#include "tools/stringTools.h"
#include "tools/otherTools.h"

namespace kiwix
{

/* Constructor */
OPDSDumper::OPDSDumper(const Library* library, const NameMapper* nameMapper)
  : LibraryDumper(library, nameMapper)
{
}
/* Destructor */
OPDSDumper::~OPDSDumper()
{
}

namespace
{

const std::string XML_HEADER(R"(<?xml version="1.0" encoding="UTF-8"?>)");

typedef kainjow::mustache::data MustacheData;
typedef kainjow::mustache::list BooksData;
typedef kainjow::mustache::list IllustrationInfo;

IllustrationInfo getBookIllustrationInfo(const Book& book)
{
    kainjow::mustache::list illustrations;
    for ( const auto& illustration : book.getIllustrations() ) {
      // For now, we are handling only sizexsize@1 illustration.
      // So we can simply pass one size to mustache.
      illustrations.push_back(kainjow::mustache::object{
        {"icon_size", to_string(illustration->width)},
        {"icon_mimetype", illustration->mimeType}
      });
    }
    return illustrations;
}

std::string fullEntryXML(const Book& book,
                         const std::string& rootLocation,
                         const std::string& contentAccessUrl,
                         const std::string& contentId)
{
    const auto bookDate = book.getDate() + "T00:00:00Z";
    const kainjow::mustache::object data{
      {"root",  rootLocation},
      {"contentAccessUrl",  onlyAsNonEmptyMustacheValue(contentAccessUrl)},
      {"id", book.getId()},
      {"name", book.getName()},
      {"title", book.getTitle()},
      {"description", book.getDescription()},
      {"language", book.getCommaSeparatedLanguages()},
      {"content_id",  urlEncode(contentId)},
      {"updated", bookDate}, // XXX: this should be the entry update datetime
      {"book_date", bookDate},
      {"category", book.getCategory()},
      {"flavour", book.getFlavour()},
      {"tags", book.getTags()},
      {"article_count", to_string(book.getArticleCount())},
      {"media_count", to_string(book.getMediaCount())},
      {"author_name", book.getCreator()},
      {"publisher_name", book.getPublisher()},
      {"url", onlyAsNonEmptyMustacheValue(book.getUrl())},
      {"size", to_string(book.getSize())},
      {"icons", getBookIllustrationInfo(book)},
    };
    return render_template(RESOURCE::templates::catalog_v2_entry_xml, data);
}

std::string partialEntryXML(const Book& book, const std::string& rootLocation)
{
    const auto bookDate = book.getDate() + "T00:00:00Z";
    const kainjow::mustache::object data{
      {"root",  rootLocation},
      {"endpoint_root", rootLocation + "/catalog/v2"},
      {"id", book.getId()},
      {"title", book.getTitle()},
      {"updated", bookDate}, // XXX: this should be the entry update datetime
    };
    const auto xmlTemplate = RESOURCE::templates::catalog_v2_partial_entry_xml;
    return render_template(xmlTemplate, data);
}

BooksData getBooksData(const Library* library,
                       const NameMapper* nameMapper,
                       const std::vector<std::string>& bookIds,
                       const std::string& rootLocation,
                       const std::string& contentAccessUrl,
                       bool partial)
{
  BooksData booksData;
  for ( const auto& bookId : bookIds ) {
    try {
      const Book book = library->getBookByIdThreadSafe(bookId);
      const std::string contentId = nameMapper->getNameForId(bookId);
      const auto entryXML = partial
                          ? partialEntryXML(book, rootLocation)
                          : fullEntryXML(book, rootLocation, contentAccessUrl, contentId);
      booksData.push_back(kainjow::mustache::object{ {"entry", entryXML} });
    } catch ( const std::out_of_range& ) {
      // the book was removed from the library since its id was obtained
      // ignore it
    }
  }

  return booksData;
}

} // unnamed namespace

string OPDSDumper::dumpOPDSFeed(const std::vector<std::string>& bookIds, const std::string& query) const
{
  const auto booksData = getBooksData(library, nameMapper, bookIds, rootLocation, contentAccessUrl, false);
  const kainjow::mustache::object template_data{
     {"date", gen_date_str()},
     {"root", rootLocation},
     {"feed_id", gen_uuid(libraryId + "/catalog/search?"+query)},
     {"filter", onlyAsNonEmptyMustacheValue(query)},
     {"totalResults", to_string(m_totalResults)},
     {"startIndex", to_string(m_startIndex)},
     {"itemsPerPage", to_string(m_count)},
     {"books", booksData }
  };

  return render_template(RESOURCE::templates::catalog_entries_xml, template_data);
}

string OPDSDumper::dumpOPDSFeedV2(const std::vector<std::string>& bookIds, const std::string& query, bool partial) const
{
  const auto endpointRoot = rootLocation + "/catalog/v2";
  const auto booksData = getBooksData(library, nameMapper, bookIds, rootLocation, contentAccessUrl, partial);

  const char* const endpoint = partial ? "/partial_entries" : "/entries";
  const std::string url = endpoint + (query.empty() ? "" : "?" + query);
  const kainjow::mustache::object template_data{
     {"date", gen_date_str()},
     {"endpoint_root", endpointRoot},
     {"feed_id", gen_uuid(libraryId + endpoint + "?" + query)},
     {"filter", onlyAsNonEmptyMustacheValue(query)},
     {"self_url", url},
     {"totalResults", to_string(m_totalResults)},
     {"startIndex", to_string(m_startIndex)},
     {"itemsPerPage", to_string(m_count)},
     {"books", booksData }
  };

  return render_template(RESOURCE::templates::catalog_v2_entries_xml, template_data);
}

std::string OPDSDumper::dumpOPDSCompleteEntry(const std::string& bookId) const
{
  const auto book = library->getBookById(bookId);
  const std::string contentId = nameMapper->getNameForId(bookId);
  return XML_HEADER
         + "\n"
         + fullEntryXML(book, rootLocation, contentAccessUrl, contentId);
}

std::string OPDSDumper::categoriesOPDSFeed() const
{
  const auto now = gen_date_str();
  kainjow::mustache::list categoryData = getCategoryData();
  return render_template(
             RESOURCE::templates::catalog_v2_categories_xml,
             kainjow::mustache::object{
               {"date", now},
               {"endpoint_root", rootLocation + "/catalog/v2"},
               {"feed_id", gen_uuid(libraryId + "/categories")},
               {"categories", categoryData }
             }
  );
}

std::string OPDSDumper::languagesOPDSFeed() const
{
  const auto now = gen_date_str();
  kainjow::mustache::list languageData = getLanguageData();
  return render_template(
             RESOURCE::templates::catalog_v2_languages_xml,
             kainjow::mustache::object{
               {"date", now},
               {"endpoint_root", rootLocation + "/catalog/v2"},
               {"feed_id", gen_uuid(libraryId + "/languages")},
               {"languages", languageData }
             }
  );
}

}
