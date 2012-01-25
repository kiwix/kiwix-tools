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

#include "zimAccessor.h"

ZimAccessor::ZimAccessor(int x) : reader(NULL) {}

/* Instanciate the ZIM file handler */
bool ZimAccessor::LoadFile(string path) {
  try {
    this->reader = new kiwix::Reader(path);
    return true;
  } catch (exception &e) {
    cerr << e.what() << endl;
  }

  return false;
}

/* Unload the ZIM file */
bool ZimAccessor::Unload() {
  if (this->reader != NULL) {
    delete this->reader;
    this->reader = NULL;
    return true;
  }
  
  return false;
}

/* Reset the cursor for GetNextArticle() */
bool ZimAccessor::Reset() {
  try {
    this->reader->reset();
    return true;
  } catch (exception &e) {
    cerr << e.what() << endl;
  }
  
  return false;
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
  }
  
  return false;
}

/* Return the UID of the ZIM file */
bool ZimAccessor::GetId(string &id) {
  try {
    if (this->reader != NULL) {
      id = this->reader->getId();
      return true;
    }
  } catch (exception &e) {
    cerr << e.what() << endl;
  }
  
  return false;
}

/* Return a random article URL */
bool ZimAccessor::GetRandomPageUrl(string &url) {
  try {
    if (this->reader != NULL) {
      url = this->reader->getRandomPageUrl();
      return true;
    }
  } catch (exception &e) {
    cerr << e.what() << endl;
  }

  return false;
}

/* Return a page url fronm title */
bool ZimAccessor::GetPageUrlFromTitle(const string &title, string &url) {
  try {
    if (this->reader != NULL) {
      if (this->reader->getPageUrlFromTitle(title, url)) {
	return true;
      }
    }
  } catch (exception &e) {
    cerr << e.what() << endl;
  }

  return false;
}

/* Return the welcome page URL */
bool ZimAccessor::GetMainPageUrl(string &url) {
  try {
    if (this->reader != NULL) {
      url = this->reader->getMainPageUrl();
      return true;
    }
  } catch (exception &e) {
    cerr << e.what() << endl;
  }
  
  return false;
}

/* Get a metatag value */
bool ZimAccessor::GetMetatag(const string &name, string &value) {
  try {
    if (this->reader != NULL) {
      if (this->reader->getMetatag(name, value)) {
	return true;
      }
    }
  } catch (exception &e) {
    cerr << e.what() << endl;
  }
  
  return false;
}

/* Get a content from a zim file */
bool ZimAccessor::GetContent(string url, string &content, unsigned int &contentLength, string &contentType) {
  
  /* default value */
  content = "";
  contentType = "";
  contentLength = 0;
  
  try {
    if (this->reader != NULL) {
      if (this->reader->getContentByUrl(url, content, contentLength, contentType)) {
	return true;
      }
    }
  } catch (exception &e) {
    cerr << e.what() << endl;
    }
  
  return false;
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
  }

  return false;
}

/* Get next suggestion */
bool ZimAccessor::GetNextSuggestion(string &title) {
  try {
    if (this->reader != NULL) {
      if (this->reader->getNextSuggestion(title)) {
	return true;
      }
    }
  } catch (exception &e) {
    cerr << e.what() << endl;
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
  }
  
  return true;
}
