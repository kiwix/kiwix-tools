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

#include "manager.h"

#include "tools.h"
#include "tools/pathTools.h"

#include <pugixml.hpp>
#include <filesystem>
#include <iostream>
#include <set>
#include <queue>
#include <cctype>
#include <algorithm>

namespace fs = std::filesystem;

namespace kiwix
{

////////////////////////////////////////////////////////////////////////////////
// LibraryManipulator
////////////////////////////////////////////////////////////////////////////////

LibraryManipulator::LibraryManipulator(LibraryPtr library)
  : library(library)
{}

LibraryManipulator::~LibraryManipulator()
{}

bool LibraryManipulator::addBookToLibrary(const Book& book)
{
  const auto ret = library->addBook(book);
  if ( ret ) {
    bookWasAddedToLibrary(book);
  }
  return ret;
}

void LibraryManipulator::addBookmarkToLibrary(const Bookmark& bookmark)
{
  library->addBookmark(bookmark);
  bookmarkWasAddedToLibrary(bookmark);
}

uint32_t LibraryManipulator::removeBooksNotUpdatedSince(Library::Revision rev)
{
  const auto n = library->removeBooksNotUpdatedSince(rev);
  if ( n != 0 ) {
    booksWereRemovedFromLibrary();
  }
  return n;
}

void LibraryManipulator::bookWasAddedToLibrary(const Book& book)
{
}

void LibraryManipulator::bookmarkWasAddedToLibrary(const Bookmark& bookmark)
{
}

void LibraryManipulator::booksWereRemovedFromLibrary()
{
}

////////////////////////////////////////////////////////////////////////////////
// Manager
////////////////////////////////////////////////////////////////////////////////

/* Constructor */
Manager::Manager(LibraryManipulator manipulator):
  writableLibraryPath(""),
  manipulator(manipulator)
{
}

Manager::Manager(LibraryPtr library) :
  writableLibraryPath(""),
  manipulator(LibraryManipulator(library))
{
}

bool Manager::parseXmlDom(const pugi::xml_document& doc,
                          bool readOnly,
                          const std::string& libraryPath,
                          bool trustLibrary)
{
  pugi::xml_node libraryNode = doc.child("library");

  std::string libraryVersion = libraryNode.attribute("version").value();

  for (pugi::xml_node bookNode = libraryNode.child("book"); bookNode;
       bookNode = bookNode.next_sibling("book")) {
    kiwix::Book book;

    book.setReadOnly(readOnly);
    book.updateFromXml(bookNode,
                       removeLastPathElement(libraryPath));

    if (!trustLibrary && !book.getPath().empty()) {
      this->readBookFromPath(book.getPath(), &book);
    }
    manipulator.addBookToLibrary(book);
  }

  return true;
}

bool Manager::readXml(const std::string& xml,
                      bool readOnly,
                      const std::string& libraryPath,
                      bool trustLibrary)
{
  pugi::xml_document doc;
  pugi::xml_parse_result result
      = doc.load_buffer((void*)xml.data(), xml.size());

  if (result) {
    this->parseXmlDom(doc, readOnly, libraryPath, trustLibrary);
  }

  return true;
}



bool Manager::parseOpdsDom(const pugi::xml_document& doc, const std::string& urlHost)
{
  pugi::xml_node libraryNode = doc.child("feed");

  try {
    m_totalBooks = strtoull(libraryNode.child("totalResults").child_value(), 0, 0);
    m_startIndex = strtoull(libraryNode.child("startIndex").child_value(), 0, 0);
    m_itemsPerPage = strtoull(libraryNode.child("itemsPerPage").child_value(), 0, 0);
    m_hasSearchResult = true;
  } catch(...) {
    m_hasSearchResult = false;
  }

  for (pugi::xml_node entryNode = libraryNode.child("entry"); entryNode;
       entryNode = entryNode.next_sibling("entry")) {
    kiwix::Book book;

    book.setReadOnly(false);
    book.updateFromOpds(entryNode, urlHost);

    /* Update the book properties with the new importer */
    manipulator.addBookToLibrary(book);
  }

  return true;
}



bool Manager::readOpds(const std::string& content, const std::string& urlHost)
{
  pugi::xml_document doc;
  pugi::xml_parse_result result
      = doc.load_buffer((void*)content.data(), content.size());

  if (result) {
    this->parseOpdsDom(doc, urlHost);
    return true;
  }

  return false;
}

bool Manager::readFile(
  const std::string& path,
  bool readOnly,
  bool trustLibrary)
{
  bool retVal = true;
  pugi::xml_document doc;

#ifdef _WIN32
  pugi::xml_parse_result result = doc.load_file(Utf8ToWide(path).c_str());
#else
  pugi::xml_parse_result result = doc.load_file(path.c_str());
#endif

  if (result) {
    this->parseXmlDom(doc, readOnly, path, trustLibrary);
  } else {
    retVal = false;
  }

  /* This has to be set (although if the file does not exists) to be
   * able to know where to save the library if new content are
   * available */
  if (!readOnly) {
    this->writableLibraryPath = path;
  }

  return retVal;
}


/* Add a book to the library. Return empty string if failed, book id otherwise
 */
std::string Manager::addBookFromPathAndGetId(const std::string& pathToOpen,
                                             const std::string& pathToSave,
                                             const std::string& url,
                                             const bool checkMetaData)
{
  kiwix::Book book;

  if (this->readBookFromPath(pathToOpen, &book)) {
    if (!pathToSave.empty() && pathToSave != pathToOpen) {
      book.setPath(isRelativePath(pathToSave)
                ? computeAbsolutePath(
                      removeLastPathElement(writableLibraryPath),
                      pathToSave)
                : pathToSave);
    }

    if (!checkMetaData
        || (!book.getTitle().empty() && !book.getLanguages().empty()
            && !book.getDate().empty())) {
      book.setUrl(url);
      manipulator.addBookToLibrary(book);
      return book.getId();
    }
  }

  return "";
}

/* Wrapper over Manager::addBookFromPath which return a bool instead of a string
 */
bool Manager::addBookFromPath(const std::string& pathToOpen,
                              const std::string& pathToSave,
                              const std::string& url,
                              const bool checkMetaData)
{
  return !(
      this->addBookFromPathAndGetId(pathToOpen, pathToSave, url, checkMetaData)
          .empty());
}

void Manager::addBooksFromDirectory(const std::string& path,
                                    const bool verboseFlag)
{
  std::set<std::string> iteratedDirs;
  std::queue<std::string> dirQueue;
  dirQueue.push(fs::absolute(path).u8string());
  int totalBooksAdded = 0;
  if (verboseFlag)
    std::cout << "Adding books from the directory tree: " << dirQueue.front() << std::endl;

  while (!dirQueue.empty()) {
    const auto currentPath = dirQueue.front();
    dirQueue.pop();
    if (verboseFlag)
      std::cout << "Visiting directory: " << currentPath << std::endl;
    for (const auto& dirEntry : fs::directory_iterator(currentPath)) {
      auto resolvedPath = dirEntry.path();
      if (fs::is_symlink(dirEntry)) {
        try {
          resolvedPath = fs::canonical(dirEntry.path());
        } catch (const std::exception& e) {
          std::cerr << "Could not resolve symlink " << resolvedPath.u8string() << " to a valid path. Skipping..." << std::endl;
          continue;
        }
      }
      const std::string pathString = resolvedPath.u8string();
      std::string resolvedPathExtension = resolvedPath.extension().u8string();
      std::transform(resolvedPathExtension.begin(), resolvedPathExtension.end(), resolvedPathExtension.begin(),
          [](unsigned char c){ return std::tolower(c); });
      if (fs::is_directory(resolvedPath)) {
        if (iteratedDirs.find(pathString) == iteratedDirs.end())
          dirQueue.push(pathString);
        else if (verboseFlag)
          std::cout << "Already iterated over " << pathString << ". Skipping..." << std::endl;
      } else if (resolvedPathExtension == ".zim" || resolvedPathExtension == ".zimaa") {
        if (!this->addBookFromPath(pathString, pathString, "", false)) {
          std::cerr << "Could not add " << pathString << " into the library." << std::endl;
        } else if (verboseFlag) {
          std::cout << "Added " << pathString << " into the library." << std::endl;
          totalBooksAdded++;
        }
      } else if (verboseFlag) {
        std::cout << "Skipped " << pathString << " - unsupported file type or permission denied." << std::endl;
      }
    }
    iteratedDirs.insert(currentPath);
  }

  if (verboseFlag)
    std::cout << "Traversal completed. Total books added: " << totalBooksAdded << std::endl;
}

bool Manager::readBookFromPath(const std::string& path, kiwix::Book* book)
{
  std::string tmp_path = path;
  if (isRelativePath(path)) {
    tmp_path = computeAbsolutePath(getCurrentDirectory(), path);
  }
  try {
    zim::Archive archive(tmp_path);
    book->update(archive);
    book->setPathValid(true);
  } catch (const std::exception& e) {
    book->setPathValid(false);
    return false;
  }

  return true;
}

bool Manager::readBookmarkFile(const std::string& path)
{
  pugi::xml_document doc;
  pugi::xml_parse_result result = doc.load_file(path.c_str());

  if (!result) {
    return false;
  }

  pugi::xml_node libraryNode = doc.child("bookmarks");

  for (pugi::xml_node node = libraryNode.child("bookmark"); node;
       node = node.next_sibling("bookmark")) {
    kiwix::Bookmark bookmark;

    bookmark.updateFromXml(node);

    manipulator.addBookmarkToLibrary(bookmark);
  }

  return true;
}

void Manager::reload(const Paths& paths)
{
  const auto libRevision = manipulator.getLibrary()->getRevision();
  for (std::string path : paths) {
    if (!path.empty()) {
      if ( kiwix::isRelativePath(path) )
        path = kiwix::computeAbsolutePath(kiwix::getCurrentDirectory(), path);

      if (!readFile(path, false, true)) {
        throw std::runtime_error("Failed to load the XML library file '" + path + "'.");
      }
    }
  }

  manipulator.removeBooksNotUpdatedSince(libRevision);
}

}
