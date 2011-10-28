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

/*bool ContentManager::RemoveBookById(const char* &id, bool *retVal) {
  *retVal = PR_FALSE;
  const char *cid;
  NS_CStringGetData(id, &cid);
  
  try {
    if (this->manager.removeBookById(cid)) {
      *retVal = PR_TRUE;
    }
  } catch (exception &e) {
    cerr << e.what() << endl;
  }
  
  return NS_OK;
}

bool ContentManager::SetCurrentBookId(const char* &id, bool *retVal) {
  *retVal = PR_FALSE;
  const char *cid;
  NS_CStringGetData(id, &cid);
  
  try {
    if (this->manager.setCurrentBookId(cid)) {
      *retVal = PR_TRUE;
    }
  } catch (exception &e) {
    cerr << e.what() << endl;
  }
  
  return NS_OK;
}

bool ContentManager::GetCurrentBookId(char* &id, bool *retVal) {
  *retVal = PR_FALSE;
  
  try {
    string current = this->manager.getCurrentBookId();
    id = nsDependentCString(current.data(), current.size());
    *retVal = PR_TRUE;
  } catch (exception &e) {
    cerr << e.what() << endl;
  }
  
  return NS_OK;
}

bool ContentManager::GetBookById(const char* &id, 
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
}

bool ContentManager::UpdateBookLastOpenDateById(const char* &id, bool *retVal) {
  *retVal = PR_FALSE;
  const char *cid;
  NS_CStringGetData(id, &cid);
  
  try {
    if (this->manager.updateBookLastOpenDateById(cid)) {
      *retVal = PR_TRUE;
    }
  } catch (exception &e) {
    cerr << e.what() << endl;
  }
  
  return NS_OK;
}

bool ContentManager::GetBookCount(const bool localBooks, const bool remoteBooks, PRUint32 *count, bool *retVal) {
  *retVal = PR_TRUE;
  *count = 0;

  try {
    *count = this->manager.getBookCount(localBooks, remoteBooks);
  } catch (exception &e) {
    cerr << e.what() << endl;
  }

  return NS_OK;
}

bool ContentManager::ListBooks(const char* &mode, const char* &sortBy, PRUint32 maxSize, 
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
}

bool ContentManager::GetListNextBookId(char* &id, bool *retVal) {
  *retVal = PR_FALSE;
  
  try {
    if (!this->manager.bookIdList.empty()) {
      string idString = *(this->manager.bookIdList.begin());
      id = nsDependentCString(idString.data(), idString.size());
      this->manager.bookIdList.erase(this->manager.bookIdList.begin());
      *retVal = PR_TRUE;
    }
  } catch (exception &e) {
    cerr << e.what() << endl;
  }
  
  return NS_OK;
}

bool ContentManager::SetBookIndex(const char* &id, const string &path, const char* &indexType, bool *retVal) {
  *retVal = PR_FALSE;

  const char *cid;
  NS_CStringGetData(id, &cid);
  const char *cindexType;
  NS_CStringGetData(indexType, &cindexType);
  const char *pathToSave = strdup(nsStringToUTF8(path));

  try {
    kiwix::supportedIndexType iType;
    if (std::string(cindexType) == "clucene") {
      iType = kiwix::CLUCENE;
    } else {
      iType = kiwix::XAPIAN;
    }

    if (this->manager.setBookIndex(cid, pathToSave, iType)) {
      *retVal = PR_TRUE;
    }
  } catch (exception &e) {
    cerr << e.what() << endl;
  }
  
  free((void*)pathToSave);

  return NS_OK;
}

bool ContentManager::SetBookPath(const char* &id, const string &path, bool *retVal) {
  *retVal = PR_FALSE;

  const char *cid;
  NS_CStringGetData(id, &cid);
  const char *pathToSave = strdup(nsStringToUTF8(path));

  try {
    if (this->manager.setBookPath(cid, pathToSave)) {
      *retVal = PR_TRUE;
    }
  } catch (exception &e) {
    cerr << e.what() << endl;
  }
  
  free((void*)pathToSave);

  return NS_OK;
}

bool ContentManager::GetBooksLanguages(char* &languages, bool *retVal) {
  *retVal = PR_TRUE;
  string languagesStr = "";
  
  vector<string> booksLanguages = this->manager.getBooksLanguages();
  vector<string>::iterator itr;
  for ( itr = booksLanguages.begin(); itr != booksLanguages.end(); ++itr ) {
    languagesStr += *itr + ";";
  }

  languages = nsDependentCString(languagesStr.data(), languagesStr.size());
  return NS_OK;
}

bool ContentManager::GetBooksPublishers(char* &publishers, bool *retVal) {
  *retVal = PR_TRUE;
  string publishersStr = "";
  
  vector<string> booksPublishers = this->manager.getBooksPublishers();
  vector<string>::iterator itr;
  for ( itr = booksPublishers.begin(); itr != booksPublishers.end(); ++itr ) {
    publishersStr += *itr + ";";
  }

  publishers = nsDependentCString(publishersStr.data(), publishersStr.size());
  return NS_OK;
}*/

