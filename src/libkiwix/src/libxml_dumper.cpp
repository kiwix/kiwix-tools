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

#include "libxml_dumper.h"
#include "book.h"

#include "tools.h"
#include "tools/base64.h"
#include "tools/stringTools.h"
#include "tools/otherTools.h"

namespace kiwix
{
/* Constructor */
LibXMLDumper::LibXMLDumper(const Library* library)
  : library(library)
{
}
/* Destructor */
LibXMLDumper::~LibXMLDumper()
{
}

#define ADD_ATTRIBUTE(node, name, value) { (node).append_attribute((name)) = (value).c_str(); }
#define ADD_ATTR_NOT_EMPTY(node, name, value) { if (!(value).empty()) ADD_ATTRIBUTE(node, name, value); }

void LibXMLDumper::handleBook(Book book, pugi::xml_node root_node) {
  if (book.readOnly())
    return;

  auto entry_node = root_node.append_child("book");
  ADD_ATTRIBUTE(entry_node, "id", book.getId());

  if (!book.getPath().empty()) {
    ADD_ATTRIBUTE(entry_node, "path", computeRelativePath(baseDir, book.getPath()));
  }

  if (book.getOrigId().empty()) {
    ADD_ATTR_NOT_EMPTY(entry_node, "title", book.getTitle());
    ADD_ATTR_NOT_EMPTY(entry_node, "description", book.getDescription());
    ADD_ATTR_NOT_EMPTY(entry_node, "language", book.getCommaSeparatedLanguages());
    ADD_ATTR_NOT_EMPTY(entry_node, "creator", book.getCreator());
    ADD_ATTR_NOT_EMPTY(entry_node, "publisher", book.getPublisher());
    ADD_ATTR_NOT_EMPTY(entry_node, "name", book.getName());
    ADD_ATTR_NOT_EMPTY(entry_node, "flavour", book.getFlavour());
    ADD_ATTR_NOT_EMPTY(entry_node, "tags", book.getTags());
    try {
      auto defaultIllustration = book.getIllustration(48);
      ADD_ATTR_NOT_EMPTY(entry_node, "faviconMimeType", defaultIllustration->mimeType);
      ADD_ATTR_NOT_EMPTY(entry_node, "faviconUrl", defaultIllustration->url);
      if (!defaultIllustration->getData().empty())
        ADD_ATTRIBUTE(entry_node, "favicon", base64_encode(defaultIllustration->getData()));
    } catch(...) {}
  } else {
    ADD_ATTRIBUTE(entry_node, "origId", book.getOrigId());
  }

  ADD_ATTR_NOT_EMPTY(entry_node, "date", book.getDate());
  ADD_ATTR_NOT_EMPTY(entry_node, "url", book.getUrl());

  if (book.getArticleCount())
    ADD_ATTRIBUTE(entry_node, "articleCount", to_string(book.getArticleCount()));

  if (book.getMediaCount())
    ADD_ATTRIBUTE(entry_node, "mediaCount", to_string(book.getMediaCount()));

  if (book.getSize())
    ADD_ATTRIBUTE(entry_node, "size", to_string(book.getSize()>>10));

  ADD_ATTR_NOT_EMPTY(entry_node, "downloadId", book.getDownloadId());
}

#define ADD_TEXT_ENTRY(node, child, value) (node).append_child((child)).append_child(pugi::node_pcdata).set_value((value).c_str())

void LibXMLDumper::handleBookmark(Bookmark bookmark, pugi::xml_node root_node) {

  auto entry_node = root_node.append_child("bookmark");
  auto book_node = entry_node.append_child("book");

  try {
    auto book = library->getBookByIdThreadSafe(bookmark.getBookId());
    ADD_TEXT_ENTRY(book_node, "id", book.getId());
    ADD_TEXT_ENTRY(book_node, "title", book.getTitle());
    ADD_TEXT_ENTRY(book_node, "name", book.getName());
    ADD_TEXT_ENTRY(book_node, "flavour", book.getFlavour());
    ADD_TEXT_ENTRY(book_node, "language", book.getCommaSeparatedLanguages());
    ADD_TEXT_ENTRY(book_node, "date", book.getDate());
  } catch (...) {
    ADD_TEXT_ENTRY(book_node, "id", bookmark.getBookId());
    ADD_TEXT_ENTRY(book_node, "title", bookmark.getBookTitle());
    ADD_TEXT_ENTRY(book_node, "name", bookmark.getBookName());
    ADD_TEXT_ENTRY(book_node, "flavour", bookmark.getBookFlavour());
    ADD_TEXT_ENTRY(book_node, "language", bookmark.getLanguage());
    ADD_TEXT_ENTRY(book_node, "date", bookmark.getDate());
  }
  ADD_TEXT_ENTRY(entry_node, "title", bookmark.getTitle());
  ADD_TEXT_ENTRY(entry_node, "url", bookmark.getUrl());
}


std::string LibXMLDumper::dumpLibXMLContent(const std::vector<std::string>& bookIds)
{
  pugi::xml_document doc;

  /* Add the library node */
  pugi::xml_node libraryNode = doc.append_child("library");

  libraryNode.append_attribute("version") = KIWIX_LIBRARY_VERSION;

  if (library) {
    for (auto& bookId: bookIds) {
      handleBook(library->getBookById(bookId), libraryNode);
    }
  }
  return nodeToString(libraryNode);
}

std::string LibXMLDumper::dumpLibXMLBookmark()
{
  pugi::xml_document doc;

  /* Add the library node */
  pugi::xml_node bookmarksNode = doc.append_child("bookmarks");

  if (library) {
    for (auto& bookmark: library->getBookmarks(false)) {
      handleBookmark(bookmark, bookmarksNode);
    }
  }
  return nodeToString(bookmarksNode);
}

}
