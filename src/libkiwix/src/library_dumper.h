/*
 * Copyright 2023 Nikhil Tanwar <2002nikhiltanwar@gmail.com>
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

#ifndef KIWIX_LIBRARY_DUMPER_H
#define KIWIX_LIBRARY_DUMPER_H

#include <string>


#include "library.h"
#include "name_mapper.h"
#include <mustache.hpp>

namespace kiwix
{

/**
 * A base class to dump Library in various formats.
 *
 */
class LibraryDumper
{
 public:
  LibraryDumper(const Library* library, const NameMapper* NameMapper);
  ~LibraryDumper();

  void setLibraryId(const std::string& id) { this->libraryId = id;}

  /**
   * Set the root location used when generating url.
   *
   * @param rootLocation the root location to use.
   */
  void setRootLocation(const std::string& rootLocation) { this->rootLocation = rootLocation; }

  /**
   * Set the URL for accessing book content
   *
   * @param url the URL of the /content endpoint of the content server
   */
  void setContentAccessUrl(const std::string& url) { this->contentAccessUrl = url; }

  /**
   * Set some informations about the search results.
   *
   * @param totalResult the total number of results of the search.
   * @param startIndex the start index of the result.
   * @param count the number of result of the current set (or page).
   */
  void setOpenSearchInfo(int totalResult, int startIndex, int count);

  /**
   * Sets user default language
   *
   * @param userLang the user language to be set
   */
  void setUserLanguage(std::string userLang) { this->m_userLang = userLang; }

  /**
   * Get the data of categories
   */
  kainjow::mustache::list getCategoryData() const;

  /**
   * Get the data of languages
   */
  kainjow::mustache::list getLanguageData() const;

 protected:
  const kiwix::Library* const library;
  const kiwix::NameMapper* const nameMapper;
  std::string libraryId;
  std::string rootLocation;
  std::string contentAccessUrl;
  std::string m_userLang;
  int m_totalResults;
  int m_startIndex;
  int m_count;
};
}

#endif // KIWIX_LIBRARY_DUMPER_H
