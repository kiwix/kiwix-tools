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

bool ContentManager::GetBookById(string &id,
					  string &path,
					  string &title,
					  string &indexPath,
					  string &indexType,
					  string &description,
					  string &articleCount,
					  string &mediaCount,
					  string &size,
					  string &creator,
					  string &date,
					  string &language,
					  string &favicon,
					  string &url) {

    try {
        kiwix::Book book;

        if (this->manager.getBookById(id.c_str(), book)) {
            path = book.pathAbsolute.data();
            title = book.title.data();
            indexPath = book.indexPathAbsolute.data();
            articleCount = book.articleCount.data();
            mediaCount = book.mediaCount.data();
            size = book.size.data();
            creator = book.creator.data();
            date = book.date.data();
            language = book.language.data();
            url = book.url.data();

            string faviconUrl = "";
            if (!book.faviconMimeType.empty()) {
                faviconUrl = "url(data:" + book.faviconMimeType + ";base64," + book.favicon + ")";
            }
            favicon = faviconUrl.data();

            string indexTypeString = "";
            if (book.indexType == kiwix::XAPIAN) {
                indexTypeString = "xapian";
            }
            indexType = indexTypeString.data();

            description = book.description.data();

            return true;
        }
    } catch (exception &e) {
        cerr << e.what() << endl;
        return false;
    }
    return false;
}

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

bool ContentManager::ListBooks(string &mode, string &sortBy, unsigned int maxSize,
					string &language, string &publisher, string &search) {
    try {

        // Set the mode enum
        kiwix::supportedListMode listMode;
        if (mode == "lastOpen") {
            listMode = kiwix::LASTOPEN;
        } else if ( mode == "remote") {
            listMode = kiwix::REMOTE;
        } else {
            listMode = kiwix::LOCAL;
        }

        // Set the sortBy enum
        kiwix::supportedListSortBy listSortBy;
        if (sortBy == "publisher") {
            listSortBy = kiwix::PUBLISHER;
        } else if ( sortBy == "date") {
            listSortBy = kiwix::DATE;
        } else if ( sortBy == "size") {
            listSortBy = kiwix::SIZE;
        } else {
            listSortBy = kiwix::TITLE;
        }

        return this->manager.listBooks(listMode, listSortBy, maxSize, language.c_str(), publisher.c_str(), search.c_str());
    } catch (exception &e) {
        cerr << e.what() << endl;
        return false;
    }
    return false;
}

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
            iType = kiwix::XAPIAN;

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
