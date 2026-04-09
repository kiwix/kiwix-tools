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

#ifndef KIWIX_MANAGER_H
#define KIWIX_MANAGER_H

#include "book.h"
#include "library.h"

#include <string>
#include <vector>
#include <memory>

namespace pugi {
class xml_document;
}

namespace kiwix
{

class LibraryManipulator
{
 public: // functions
  explicit LibraryManipulator(LibraryPtr library);
  virtual ~LibraryManipulator();

  LibraryPtr getLibrary() const { return library; }

  bool addBookToLibrary(const Book& book);
  void addBookmarkToLibrary(const Bookmark& bookmark);
  uint32_t removeBooksNotUpdatedSince(Library::Revision rev);

 protected: // overrides
  virtual void bookWasAddedToLibrary(const Book& book);
  virtual void bookmarkWasAddedToLibrary(const Bookmark& bookmark);
  virtual void booksWereRemovedFromLibrary();

 private: // data
  LibraryPtr library;
};

/**
 * A tool to manage a `Library`.
 */
class Manager
{
 public: // types
  typedef std::vector<std::string> Paths;

 public: // functions
  explicit Manager(LibraryManipulator manipulator);
  explicit Manager(LibraryPtr library);

  /**
   * Read a `library.xml` and add book in the file to the library.
   *
   * @param path The (utf8) path to the `library.xml`.
   * @param readOnly Set if the libray path could be overwritten latter with
   *                 updated content.
   * @param trustLibrary use book metadata coming from XML.
   * @return True if file has been properly parsed.
   */
  bool readFile(const std::string& path, bool readOnly = true, bool trustLibrary = true);

  /**
   * Sync the contents of the library with one or more `library.xml` files.
   *
   * The metadata of the library files is trusted unconditionally.
   * Any books not present in the input library.xml files are removed
   * from the library.
   *
   * @param paths The (utf8) paths to the `library.xml` files.
   */
  void reload(const Paths& paths);

  /**
   * Load a library content store in the string.
   *
   * @param xml The content corresponding of the library xml
   * @param readOnly Set if the libray path could be overwritten latter with
   *                 updated content.
   * @param libraryPath The library path (used to resolve relative path)
   * @return True if the content has been properly parsed.
   */
  bool readXml(const std::string& xml,
               const bool readOnly = true,
               const std::string& libraryPath = "",
               bool trustLibrary = true);

  /**
   * Load a library content stored in a OPDS stream.
   *
   * @param content The content of the OPDS stream.
   * @param readOnly Set if the library path could be overwritten later with
   *                 updated content.
   * @param libraryPath The library path (used to resolve relative path)
   * @return True if the content has been properly parsed.
   */
  bool readOpds(const std::string& content, const std::string& urlHost);


  /**
   * Load a bookmark file.
   *
   * @param path The path of the file to read.
   * @return True if the content has been properly parsed.
   */
  bool readBookmarkFile(const std::string& path);

  /**
   * Add a book to the library.
   *
   * @param pathToOpen The path to the zim file to add.
   * @param pathToSave The path to store in the library in place of pathToOpen.
   * @param url        The url of the book to store in the library.
   * @param checMetaData Tell if we check metadata before adding book to the
   *                     library.
   * @return The id of the book if the book has been added to the library.
   *         Else, an empty string.
   */
  std::string addBookFromPathAndGetId(const std::string& pathToOpen,
                                 const std::string& pathToSave = "",
                                 const std::string& url = "",
                                 const bool checkMetaData = false);

  /**
   * Add a book to the library.
   *
   * @param pathToOpen The path to the zim file to add.
   * @param pathToSave The path to store in the library in place of pathToOpen.
   * @param url        The url of the book to store in the library.
   * @param checMetaData Tell if we check metadata before adding book to the
   *                     library.
   * @return True if the book has been added to the library.
   */

  bool addBookFromPath(const std::string& pathToOpen,
                       const std::string& pathToSave = "",
                       const std::string& url = "",
                       const bool checkMetaData = false);

  /**
   * Add all books from the directory tree into the library.
   * 
   * @param path          The path of the directory to scan.
   * @param verboseFlag   Verbose logs flag.
   */
  void addBooksFromDirectory(const std::string& path,
                             const bool verboseFlag = false);

  std::string writableLibraryPath;

  bool m_hasSearchResult = false;
  uint64_t m_totalBooks = 0;
  uint64_t m_startIndex = 0;
  uint64_t m_itemsPerPage = 0;

 protected:
  kiwix::LibraryManipulator manipulator;

  bool readBookFromPath(const std::string& path, Book* book);
  bool parseXmlDom(const pugi::xml_document& doc,
                   bool readOnly,
                   const std::string& libraryPath,
                   bool trustLibrary);
  bool parseOpdsDom(const pugi::xml_document& doc,
                    const std::string& urlHost);

};
}

#endif
