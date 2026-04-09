/*
 * Copyright 2018 Matthieu Gautier <mgautier@kymeria.fr>
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

#ifndef KIWIX_LIBXML_DUMPER_H
#define KIWIX_LIBXML_DUMPER_H

#include <string>
#include <vector>

#include <pugixml.hpp>

#include "library.h"

namespace kiwix
{

/**
 * A tool to dump a `Library` into a basic library.xml
 *
 */
class LibXMLDumper
{
 public:
  LibXMLDumper() = default;
  LibXMLDumper(const Library* library);
  ~LibXMLDumper();

  /**
   * Dump the library.xml
   *
   * @param id The id of the library.
   * @return The library.xml content.
   */
  std::string dumpLibXMLContent(const std::vector<std::string>& bookIds);


  /**
   * Dump the bookmark of the library.
   *
   * @return The bookmark.xml content.
   */
  std::string dumpLibXMLBookmark();

  /**
   * Set the base directory used.
   *
   * @param baseDir the base directory to use.
   */
  void setBaseDir(const std::string& baseDir) { this->baseDir = baseDir; }

  /**
   * Set the library to dump.
   *
   * @param library The library to dump.
   */
  void setLibrary(const Library* library) { this->library = library; }

 protected:
  const kiwix::Library* library;
  std::string baseDir;
 private:
  void handleBook(Book book, pugi::xml_node root_node);
  void handleBookmark(Bookmark bookmark, pugi::xml_node root_node);
};
}

#endif // KIWIX_OPDS_DUMPER_H
