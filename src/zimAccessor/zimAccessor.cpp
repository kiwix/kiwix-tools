/*
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
#include <kiwix/reader.h>
#include "zimAccessor.h"

ZimAccessor::ZimAccessor(int x) : reader(NULL) {}

bool ZimAccessor::LoadFile(string path) {
    try {
        this->reader = new kiwix::Reader(path);
    } catch (exception &e) {
        cerr << e.what() << endl;
        return false;
    }
    return true;
}

/* Reset the cursor for GetNextArticle() */
bool ZimAccessor::Reset() {
    try {
        this->reader->reset();
    } catch (exception &e) {
        cerr << e.what() << endl;
        return false;
    }
    return true;
}

/* Get the count of articles which can be indexed/displayed */
bool ZimAccessor::GetArticleCount(unsigned int &count) {
    try {
        if (this->reader != NULL) {
            count = this->reader->getArticleCount();
            return true;
        }
    } catch (exception &e) {
        cerr << e.what() << endl;
        return false;
    }
    return false;
}

/* Return the UID of the ZIM file */
bool ZimAccessor::GetId(string &id) {
    try {
        if (this->reader != NULL) {
          id = this->reader->getId().c_str();
          return true;
        }
    } catch (exception &e) {
        cerr << e.what() << endl;
    }

  return false;
}

/* Return a page url fronm title */
bool ZimAccessor::GetPageUrlFromTitle(const string &title, string &url) {
    string urlstr;
    //const char *ctitle;
    //NS_CStringGetData(title, &ctitle);

    try {
        if (this->reader != NULL) {
            if (this->reader->getPageUrlFromTitle(title.c_str(), urlstr)) {
                url = urlstr.c_str();
                return true;
            }
        }
    } catch (exception &e) {
        cerr << e.what() << endl;
        return false;
    }
    return false;
}

/* Return the welcome page URL */
bool ZimAccessor::GetMainPageUrl(string &url) {
    try {
        if (this->reader != NULL) {
            string urlstr = this->reader->getMainPageUrl();

            if (urlstr.empty()) {
            urlstr = this->reader->getFirstPageUrl(); 
            }

            url = urlstr.c_str();
            return true;
        }
    } catch (exception &e) {
        cerr << e.what() << endl;
        return false;
    }

    return false;
}

/* Get a metatag value */
bool ZimAccessor::GetMetatag(const string &name, string &value) {
    string valueStr;

    try {
        if (this->reader != NULL) {
            if (this->reader->getMetatag(name.c_str(), valueStr)) {
                value = valueStr.data(); 
                return true;
            }
        }
    } catch (exception &e) {
        cerr << e.what() << endl;
        return false;
    }

    return false;
}

/* Get a content from a zim file */
bool ZimAccessor::GetContent(string url, string &content, unsigned int &contentLength, string &contentType) {

    /* strings */
    string contentStr;
    string contentTypeStr;
    unsigned int contentLengthInt;

    /* default value */
    content = "";
    contentLength = 0;

    try {
        if (this->reader != NULL) {
            if (this->reader->getContentByUrl(url, contentStr, contentLength, contentTypeStr)) {
                contentType = contentTypeStr.data();
                content = contentStr.data();
                return true;
            }
        }
    } catch (exception &e) {
        cerr << e.what() << endl;
        return false;
    }

    return true;
}

/* Search titles by prefix*/
bool ZimAccessor::SearchSuggestions(const string &prefix, unsigned int suggestionsCount) {
    try {
        if (this->reader != NULL) {
            if (this->reader->searchSuggestions(prefix.c_str(), suggestionsCount)) {
                return true;
            }
        }
    } catch (exception &e) {
        cerr << e.what() << endl;
        return false;
    }

    return false;
}

/* Get next suggestion */
bool ZimAccessor::GetNextSuggestion(string &title) {
    string titleStr;

    try {
        if (this->reader != NULL) {
            if (this->reader->getNextSuggestion(titleStr)) {
                title = titleStr.c_str();
                return true;
            }
        }
    } catch (exception &e) {
        cerr << e.what() << endl;
        return false;
    }

    return false;
}

/* Can I check the integrity - for old ZIM file without checksum */
bool ZimAccessor::CanCheckIntegrity() {
    try {
        if (this->reader != NULL) {
            return this->reader->canCheckIntegrity();
        }
    } catch (exception &e) {
        cerr << e.what() << endl;
        return false;
    }

    return false;
}

/* Return true if corrupted, false otherwise */
bool ZimAccessor::IsCorrupted() {
    try {
        if (this->reader != NULL) {
            return this->reader->isCorrupted();
        }
    } catch (exception &e) {
        cerr << e.what() << endl;
        return true;
    }

    return true;
}
