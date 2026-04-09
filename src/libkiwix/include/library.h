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

#ifndef KIWIX_LIBRARY_H
#define KIWIX_LIBRARY_H

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <mutex>
#include <zim/archive.h>
#include <zim/search.h>

#include "book.h"
#include "bookmark.h"
#include "common.h"

#define KIWIX_LIBRARY_VERSION "20110515"

namespace Xapian {
class WritableDatabase;
};

namespace kiwix
{

class OPDSDumper;
class Library;

enum supportedListSortBy { UNSORTED, TITLE, SIZE, DATE, CREATOR, PUBLISHER };
enum supportedListMode {
  ALL = 0,
  LOCAL = 1,
  REMOTE = 1 << 1,
  NOLOCAL = 1 << 2,
  NOREMOTE = 1 << 3,
  VALID = 1 << 4,
  NOVALID = 1 << 5
};

enum MigrationMode {
  /** When migrating bookmarks, do not allow to migrate to an older book than the currently pointed one
   * (or date stored in the bookmark if book is invalid)
   *
   * If no newer books are found, no upgrade is made.
   */
  UPGRADE_ONLY = 0,

  /** Try hard to do a migration. This mostly does:
   * - Try to find a newer book.
   * - If book is invalid: find a best book, potentially older.
   * Older book will never be returned if current book is a valid one.
   */
  ALLOW_DOWNGRADE = 1,
};

class Filter {
  public: // types
    using Tags = std::vector<std::string>;

  private: // data
    uint64_t activeFilters;
    Tags _acceptTags;
    Tags _rejectTags;
    std::string _category;
    std::string _lang;
    std::string _publisher;
    std::string _creator;
    size_t _maxSize;
    std::string _query;
    bool _queryIsPartial;
    std::string _name;
    std::string _flavour;

  public: // functions
    Filter();
    ~Filter() = default;

    /**
     *  Set the filter to check local.
     *
     *  A local book is a book with a path.
     *  If accept is true, only local book are accepted.
     *  If accept is false, only non local book are accepted.
     */
    Filter& local(bool accept);

    /**
     *  Set the filter to check remote.
     *
     *  A remote book is a book with a url.
     *  If accept is true, only remote book are accepted.
     *  If accept is false, only non remote book are accepted.
     */
    Filter& remote(bool accept);

    /**
     *  Set the filter to check validity.
     *
     *  A valid book is a book with a path pointing to a existing zim file.
     *  If accept is true, only valid book are accepted.
     *  If accept is false, only non valid book are accepted.
     */
    Filter& valid(bool accept);

    /**
     * Set the filter to only accept book with corresponding tag.
     */
    Filter& acceptTags(const Tags& tags);
    Filter& rejectTags(const Tags& tags);

    /**
     *  Set the filter to only accept books in the specified category.
     *
     *  Multiple categories can be specified as a comma-separated list (in
     *  which case a book in any of those categories will match).
     */
    Filter& category(std::string category);

    /**
     *  Set the filter to only accept books in the specified language.
     *
     *  Multiple languages can be specified as a comma-separated list (in
     *  which case a book in any of those languages will match).
     */
    Filter& lang(std::string lang);

    Filter& publisher(std::string publisher);
    Filter& creator(std::string creator);
    Filter& maxSize(size_t size);
    Filter& query(std::string query, bool partial=true);
    Filter& name(std::string name);
    Filter& flavour(std::string flavour);
    Filter& clearLang();
    Filter& clearCategory();

    bool hasQuery() const;
    const std::string& getQuery() const { return _query; }
    bool queryIsPartial() const { return _queryIsPartial; }

    bool hasName() const;
    const std::string& getName() const { return _name; }

    bool hasCategory() const;
    const std::string& getCategory() const { return _category; }

    bool hasLang() const;
    const std::string& getLang() const { return _lang; }

    bool hasPublisher() const;
    const std::string& getPublisher() const { return _publisher; }

    bool hasCreator() const;
    const std::string& getCreator() const { return _creator; }

    bool hasFlavour() const;
    const std::string& getFlavour() const { return _flavour; }

    const Tags& getAcceptTags() const { return _acceptTags; }
    const Tags& getRejectTags() const { return _rejectTags; }

private: // functions
    friend class Library;

    bool accept(const Book& book) const;
};


class ZimSearcher : public zim::Searcher
{
  public:
    explicit ZimSearcher(zim::Searcher&& searcher)
      : zim::Searcher(searcher)
    {}

    std::unique_lock<std::mutex> getLock() {
      return std::unique_lock<std::mutex>(m_mutex);
    }
    virtual ~ZimSearcher() = default;
  private:
    std::mutex m_mutex;
};

template<typename, typename>
class ConcurrentCache;

template<typename, typename>
class MultiKeyCache;

using LibraryPtr = std::shared_ptr<Library>;
using ConstLibraryPtr = std::shared_ptr<const Library>;


// Some compiler we use don't have [[nodiscard]] attribute.
// We don't want to declare `create` with it in this case.
#define LIBKIWIX_NODISCARD
#if defined __has_cpp_attribute
  #if __has_cpp_attribute (nodiscard)
    #undef LIBKIWIX_NODISCARD
    #define LIBKIWIX_NODISCARD [[nodiscard]]
  #endif
#endif

/**
 * A Library store several books.
 */
class Library: public std::enable_shared_from_this<Library>
{
 public:
  typedef uint64_t Revision;
  typedef std::vector<std::string> BookIdCollection;
  typedef std::map<std::string, int> AttributeCounts;
  typedef std::set<std::string> BookIdSet;

 private:
  Library();

 public:
  LIBKIWIX_NODISCARD static LibraryPtr create() {
    return LibraryPtr(new Library());
  }
  ~Library();

  /**
   * Library is not a copiable object. However it can be moved.
   */
  Library(const Library& ) = delete;
  Library(Library&& ) = delete;
  void operator=(const Library& ) = delete;
  Library& operator=(Library&& ) = delete;

  /**
   * Add a book to the library.
   *
   * If a book already exist in the library with the same id, update
   * the existing book instead of adding a new one.
   *
   * @param book The book to add.
   * @return True if the book has been added.
   *         False if a book has been updated.
   */
  bool addBook(const Book& book);

  /**
   * A self-explanatory alias for addBook()
   */
  bool addOrUpdateBook(const Book& book) { return addBook(book); }

  /**
   * Add a bookmark to the library.
   *
   * @param bookmark the book to add.
   */
  void addBookmark(const Bookmark& bookmark);

  /**
   * Remove a bookmark
   *
   * @param zimId The zimId of the bookmark.
   * @param url The url of the bookmark.
   * @return True if the bookmark has been removed.
   */
  bool removeBookmark(const std::string& zimId, const std::string& url);

  /**
   * Migrate all invalid bookmarks.
   *
   * All invalid bookmarks (ie pointing to unknown books, no check is made on bookmark pointing to
   * invalid articles of valid book) will be migrated (if possible) to a better book.
   * "Better book", will be determined using method `getBestTargetBookId`.
   *
   * @return A tuple<int, int>: <The number of bookmarks updated>, <Number of invalid bookmarks before migration was performed>.
   */
  std::tuple<int, int> migrateBookmarks(MigrationMode migrationMode = ALLOW_DOWNGRADE);

  /**
   * Migrate all bookmarks associated to a specific book.
   *
   * All bookmarks associated to `sourceBookId` book will be migrated to a better book.
   * "Better book", will be determined using method `getBestTargetBookId`.
   *
   * @param sourceBookId the source bookId of the bookmarks to migrate.
   * @param migrationMode how we will find the best book.
   * @return The number of bookmarks updated.
   */
  int migrateBookmarks(const std::string& sourceBookId, MigrationMode migrationMode = UPGRADE_ONLY);

  /**
   * Migrate bookmarks
   *
   * Migrate all bookmarks pointing to `source` to `destination`.
   *
   * @param sourceBookId the source bookId of the bookmarks to migrate.
   * @param targetBookId the destination bookId to migrate the bookmarks to.
   * @return The number of bookmarks updated.
   */
  int migrateBookmarks(const std::string& sourceBookId, const std::string& targetBookId);
 
  /**
   * Get the best available bookId for a bookmark.
   *
   * Given a bookmark, return the best available bookId.
   * "best available bookId" is determined using heuristitcs based on book name, flavour and date.
   *
   * @param bookmark The bookmark to search the bookId for.
   * @param migrationMode The migration mode to use.
   * @return A bookId. Potentially empty string if no suitable book found.
   */
  std::string getBestTargetBookId(const Bookmark& bookmark, MigrationMode migrationMode) const;

  /**
   * Get the best bookId for a combination of book's name, flavour and date.
   *
   * Given a bookName (mandatory), try to find the best book.
   * If preferedFlavour is given, will try to find a book with the same flavour. If not found, return a book with a different flavour.
   * If minDate is given, return a book newer than minDate. If not found, return a empty bookId.
   *
   * @param bookName The name of the book
   * @param preferedFlavour The prefered flavour.
   * @param minDate the minimal book date acceptable. Must be a string in the format "YYYY-MM-DD".
   * @return A bookId corresponding to the query, or empty string if not found.
   */
  std::string getBestTargetBookId(const std::string& bookName, const std::string& preferedFlavour="", const std::string& minDate="") const;

  // XXX: This is a non-thread-safe operation
  const Book& getBookById(const std::string& id) const;
  // XXX: This is a non-thread-safe operation
  const Book& getBookByPath(const std::string& path) const;

  Book getBookByIdThreadSafe(const std::string& id) const;

  std::shared_ptr<zim::Archive> getArchiveById(const std::string& id);
  std::shared_ptr<ZimSearcher> getSearcherById(const std::string& id) {
    return getSearcherByIds(BookIdSet{id});
  }
  std::shared_ptr<ZimSearcher> getSearcherByIds(const BookIdSet& ids);

  /**
   * Remove a book from the library.
   *
   * @param id the id of the book to remove.
   * @return True if the book were in the lirbrary and has been removed.
   */
  bool removeBookById(const std::string& id);

  /**
   * Write the library to a file.
   *
   * @param path the path of the file to write to.
   * @return True if the library has been correctly saved.
   */
  bool writeToFile(const std::string& path) const;

  /**
   * Write the library bookmarks to a file.
   *
   * @param path the path of the file to write to.
   * @return True if the library has been correctly saved.
   */
  bool writeBookmarksToFile(const std::string& path) const;

  /**
   * Get the number of book in the library.
   *
   * @param localBooks If we must count local books (books with a path).
   * @param remoteBooks If we must count remote books (books with an url)
   * @return The number of books.
   */
  unsigned int getBookCount(const bool localBooks, const bool remoteBooks) const;

  /**
   * Get all languagues of the books in the library.
   *
   * @return A list of languages.
   */
  std::vector<std::string> getBooksLanguages() const;

  /**
   * Get all languagues of the books in the library with counts.
   *
   * @return A list of languages with the count of books in each language.
   */
  AttributeCounts getBooksLanguagesWithCounts() const;

  /**
   * Get all categories of the books in the library.
   *
   * @return A list of categories.
   */
  std::vector<std::string> getBooksCategories() const;

  /**
   * Get all book creators of the books in the library.
   *
   * @return A list of book creators.
   */
  std::vector<std::string> getBooksCreators() const;

  /**
   * Get all book publishers of the books in the library.
   *
   * @return A list of book publishers.
   */
  std::vector<std::string> getBooksPublishers() const;

  /**
   * Get all bookmarks.
   *
   * @return A list of bookmarks
   */
  const std::vector<kiwix::Bookmark> getBookmarks(bool onlyValidBookmarks = true) const;

  /**
   * Get all book ids of the books in the library.
   *
   * @return A list of book ids.
   */
  BookIdCollection getBooksIds() const;

  /**
   * Filter the library and return the id of the keep elements.
   *
   * @param filter The filter to use.
   * @return The list of bookIds corresponding to the filter.
   */
  BookIdCollection filter(const Filter& filter) const;


  /**
   * Sort (in place) bookIds using the given comparator.
   *
   * @param bookIds the list of book Ids to sort
   * @param comparator how to sort the books
   * @return The sorted list of books
   */
  void sort(BookIdCollection& bookIds, supportedListSortBy sortBy, bool ascending) const;

  /**
   * Return the current revision of the library.
   *
   * The revision of the library is updated (incremented by one) by
   * the addBook() and removeBookById() operations.
   *
   * @return Current revision of the library.
   */
  Revision getRevision() const;

  /**
   * Remove books that have not been updated since the specified revision.
   *
   * @param  rev the library revision to use
   * @return Count of books that were removed by this operation.
   */
  uint32_t removeBooksNotUpdatedSince(Revision rev);

  friend class OPDSDumper;
  friend class libXMLDumper;

private: // types
  typedef const std::string& (Book::*BookStrPropMemFn)() const;
  struct Entry : Book
  {
    Library::Revision lastUpdatedRevision = 0;
  };

private: // functions
  AttributeCounts getBookAttributeCounts(BookStrPropMemFn p) const;
  std::vector<std::string> getBookPropValueSet(BookStrPropMemFn p) const;
  BookIdCollection filterViaBookDB(const Filter& filter) const;
  std::string getBestFromBookCollection(BookIdCollection books, const Bookmark& bookmark, MigrationMode migrationMode) const;
  unsigned int getBookCount_not_protected(const bool localBooks, const bool remoteBooks) const;
  void updateBookDB(const Book& book);
  void dropCache(const std::string& bookId);

private: //data
  mutable std::recursive_mutex m_mutex;
  Library::Revision m_revision;
  std::map<std::string, Entry> m_books;
  using ArchiveCache = ConcurrentCache<std::string, std::shared_ptr<zim::Archive>>;
  std::unique_ptr<ArchiveCache> mp_archiveCache;
  using SearcherCache = MultiKeyCache<std::string, std::shared_ptr<ZimSearcher>>;
  std::unique_ptr<SearcherCache> mp_searcherCache;
  std::vector<kiwix::Bookmark> m_bookmarks;
  std::unique_ptr<Xapian::WritableDatabase> m_bookDB;
};

// We don't need it anymore and we don't want to polute any other potential usage
// of `LIBKIWIX_NODISCARD` token.
#undef LIBKIWIX_NODISCARD


}

#endif
