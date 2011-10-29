/*
 * Copyright 2011 Emmanuel Engelhart <kelson@kiwix.org>
 * Copyright 2011 Renaud Gaudin <reg@kiwix.org>
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

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <kiwix/manager.h>
#include <pathTools.h>
#include <componentTools.h>
#include <regexTools.h>
#include "contentManager.h"

ContentManager::ContentManager(int x) : manager() {}

bool ContentManager::OpenLibraryFromFile(string path, bool readOnly) {
    try {
        return this->manager.readFile(path, readOnly);
    } catch (exception &e) {
        cerr << e.what() << endl;
        return false;
    }
    return true;
}

bool ContentManager::OpenLibraryFromText(string &xml, bool readOnly) {

    bool returnValue = true;
    const char *cXml;
    cXml = xml.c_str();

    try {
        return this->manager.readXml(cXml, readOnly);
    } catch (exception &e) {
        cerr << e.what() << endl;
        return false;
    }

    return false;
}

bool ContentManager::WriteLibrary() {

    try {
        return this->manager.writeFile(this->manager.writableLibraryPath);
    } catch (exception &e) {
        cerr << e.what() << endl;
        return false;
    }

    return false;
}

bool ContentManager::WriteLibraryToFile(string &path) {

    try {
        return this->manager.writeFile(path.c_str());
    } catch (exception &e) {
        cerr << e.what() << endl;
        return false;
    }

    return false;
}

bool ContentManager::AddBookFromPath(string &path) {

    try {
        return this->manager.addBookFromPath(path.c_str(), path);
    } catch (exception &e) {
        cerr << e.what() << endl;
        return false;
    }

    return false;
}

bool ContentManager::RemoveBookById(string &id) {

    try {
        return this->manager.removeBookById(id.c_str());
    } catch (exception &e) {
        cerr << e.what() << endl;
        return false;
    }
    return false;
}

bool ContentManager::SetCurrentBookId(string &id) {

    try {
        return this->manager.setCurrentBookId(id.c_str());
    } catch (exception &e) {
        cerr << e.what() << endl;
        return false;
    }

    return false;
    }

string ContentManager::GetCurrentBookId() {
    string none = "";
    try {
        return this->manager.getCurrentBookId();
    } catch (exception &e) {
        cerr << e.what() << endl;
        return none;
    }

    return none;
}

/*bool ContentManager::GetBookById(const char* &id, 
					  char* &path, 
					  char* &title,
					  char* &indexPath, 
					  char* &indexType, 
					  char* &description,
					  char* &articleCount, 
					  char* &mediaCount, 
					  char* &size,
					  char* &creator,
					  char* &date,
					  char* &language, 
					  char* &favicon, 
					  char* &url, bool *retVal) {
  *retVal = PR_FALSE;
  const char *cid;
  NS_CStringGetData(id, &cid);

  try {
    kiwix::Book book;

    if (this->manager.getBookById(cid, book)) {
      path = nsDependentCString(book.pathAbsolute.data(), book.pathAbsolute.size());
      title = nsDependentCString(book.title.data(), book.title.size());
      indexPath = nsDependentCString(book.indexPathAbsolute.data(), book.indexPathAbsolute.size());
      articleCount = nsDependentCString(book.articleCount.data(), book.articleCount.size());
      mediaCount = nsDependentCString(book.mediaCount.data(), book.mediaCount.size());
      size = nsDependentCString(book.size.data(), book.size.size());
      creator = nsDependentCString(book.creator.data(), book.creator.size());
      date = nsDependentCString(book.date.data(), book.date.size());
      language = nsDependentCString(book.language.data(), book.language.size());
      url = nsDependentCString(book.url.data(), book.url.size());
      
      string faviconUrl = "";
      if (!book.faviconMimeType.empty()) {
	faviconUrl = "url(data:" + book.faviconMimeType + ";base64," + book.favicon + ")";
      }
      favicon = nsDependentCString(faviconUrl.data(), faviconUrl.size());

      string indexTypeString = "";
      if (book.indexType == kiwix::XAPIAN) {
	indexTypeString = "xapian";
      } else if (book.indexType == kiwix::CLUCENE) {
	indexTypeString = "clucene";
      }
      indexType = nsDependentCString(indexTypeString.data(), indexTypeString.size());

      description = nsDependentCString(book.description.data(), book.description.size());

      *retVal = PR_TRUE;
    }
  } catch (exception &e) {
    cerr << e.what() << endl;
  }
  
  return NS_OK;
}*/

bool ContentManager::UpdateBookLastOpenDateById(string &id) {
    try {
        return this->manager.updateBookLastOpenDateById(id.c_str());
    } catch (exception &e) {
        cerr << e.what() << endl;
        return false;
    }

    return false;
}

unsigned int ContentManager::GetBookCount(const bool localBooks, const bool remoteBooks) {
    int count = 0;

    try {
        return this->manager.getBookCount(localBooks, remoteBooks);
    } catch (exception &e) {
        cerr << e.what() << endl;
        return count;
    }

    return count;
}

/*bool ContentManager::ListBooks(const char* &mode, const char* &sortBy, PRUint32 maxSize, 
					const char* &language, const char* &publisher, const char* &search, bool *retVal) {
  *retVal = PR_FALSE;
  const char *cmode; NS_CStringGetData(mode, &cmode);
  const char *csortBy; NS_CStringGetData(sortBy, &csortBy);
  const char *clanguage; NS_CStringGetData(language, &clanguage);
  const char *cpublisher; NS_CStringGetData(publisher, &cpublisher);
  const char *csearch; NS_CStringGetData(search, &csearch);

  try {

    // Set the mode enum
    kiwix::supportedListMode listMode;
    if (std::string(cmode) == "lastOpen") {
      listMode = kiwix::LASTOPEN;
    } else if ( std::string(cmode) == "remote") {
      listMode = kiwix::REMOTE;
    } else {
      listMode = kiwix::LOCAL;
    }

    // Set the sortBy enum
    kiwix::supportedListSortBy listSortBy;
    if (std::string(csortBy) == "publisher") {
      listSortBy = kiwix::PUBLISHER;
    } else if ( std::string(csortBy) == "date") {
      listSortBy = kiwix::DATE;
    } else if ( std::string(csortBy) == "size") {
      listSortBy = kiwix::SIZE;
    } else {
      listSortBy = kiwix::TITLE;
    }

    if (this->manager.listBooks(listMode, listSortBy, maxSize, clanguage, cpublisher, csearch)) {
      *retVal = PR_TRUE;
    }
  } catch (exception &e) {
    cerr << e.what() << endl;
  }
  

  return NS_OK;
}*/

const char* ContentManager::GetListNextBookId() {

    string id;
    try {
        if (!this->manager.bookIdList.empty()) {
        id = *(this->manager.bookIdList.begin());
        this->manager.bookIdList.erase(this->manager.bookIdList.begin());
        }
    } catch (exception &e) {
        cerr << e.what() << endl;
    }
    return id.c_str();
}

bool ContentManager::SetBookIndex(string &id, string &path, string &indexType) {
    try {
        kiwix::supportedIndexType iType;
        if (indexType == "clucene") {
            iType = kiwix::CLUCENE;
        } else {
            iType = kiwix::XAPIAN;
        }

        return this->manager.setBookIndex(id.c_str(), path.c_str(), iType);
    } catch (exception &e) {
        cerr << e.what() << endl;
        return false;
    }

    return false;
}

bool ContentManager::SetBookPath(string &id, string &path) {
    try {
        return this->manager.setBookPath(id.c_str(), path.c_str());
    } catch (exception &e) {
        cerr << e.what() << endl;
        return false;
    }
    return false;
}

string ContentManager::GetBooksLanguages() {
    string languagesStr = "";

    vector<string> booksLanguages = this->manager.getBooksLanguages();
    vector<string>::iterator itr;
    for ( itr = booksLanguages.begin(); itr != booksLanguages.end(); ++itr ) {
        languagesStr += *itr + ";";
    }
    return languagesStr;
}

string ContentManager::GetBooksPublishers() {
    string publishersStr = "";

    vector<string> booksPublishers = this->manager.getBooksPublishers();
    vector<string>::iterator itr;
    for ( itr = booksPublishers.begin(); itr != booksPublishers.end(); ++itr ) {
        publishersStr += *itr + ";";
    }
    return publishersStr;
}
