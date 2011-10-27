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
#include <assert.h>
#include "ZimAccessorWrapper.h"
#include "zimAccessor.h"

/* creates instance of ZimAccessor */
HZIMCLASS ZimAccessor_Create( void ) {
    ZimAccessor * zimAccessor = new ZimAccessor(0); 

    // Return its pointer (opaque)
    return (HZIMCLASS)zimAccessor;
}

/* Delete instance of ZimAccessor */
void ZimAccessor_Destroy( HZIMCLASS h ) {
    assert(h != NULL);

    // Convert from handle to ZimAccessor pointer
    ZimAccessor * zimAccessor = (ZimAccessor *)h;

    delete zimAccessor;
}

int ZimAccessor_LoadFile( HZIMCLASS h, char* zimPath) {
    assert(h != NULL);

    ZimAccessor * zimAccessor = (ZimAccessor *)h;

    if (zimAccessor->LoadFile( zimPath ))
        return true;
    else
        return false;
}

/* Reset the cursor for GetNextArticle() */
int ZimAccessor_Reset( HZIMCLASS h) {
    assert(h != NULL);

    ZimAccessor * zimAccessor = (ZimAccessor *)h;

    if (zimAccessor->Reset())
        return true;
    else
        return false;
}

/* Get the count of articles which can be indexed/displayed */
unsigned int ZimAccessor_GetArticleCount( HZIMCLASS h) {
    assert(h != NULL);

    ZimAccessor * zimAccessor = (ZimAccessor *)h;

    unsigned int count = 0;
    zimAccessor->GetArticleCount(count);
    return count;
}

/* Return the UID of the ZIM file */
const char* ZimAccessor_GetId( HZIMCLASS h) {
    assert(h != NULL);

    ZimAccessor * zimAccessor = (ZimAccessor *)h;

    string id = "";
    zimAccessor->GetId(id);
    return id.c_str();
}

/* Return a page url fronm title */
const char* ZimAccessor_GetPageUrlFromTitle( HZIMCLASS h, char* title) {
    assert(h != NULL);

    ZimAccessor * zimAccessor = (ZimAccessor *)h;

    string cptitle = string(title);
    string url = "";
    zimAccessor->GetPageUrlFromTitle(cptitle, url);
    return url.c_str();
}

/* Return the welcome page URL */
const char* ZimAccessor_GetMainPageUrl( HZIMCLASS h ) {
    assert(h != NULL);

    ZimAccessor * zimAccessor = (ZimAccessor *)h;

    string hp = "";
    zimAccessor->GetMainPageUrl( hp );
    return hp.c_str();
}

/* Get a metatag value */
const char* ZimAccessor_GetMetatag( HZIMCLASS h, char* name) {
    assert(h != NULL);

    ZimAccessor * zimAccessor = (ZimAccessor *)h;

    string cpname = string(name);
    string value = "";
    zimAccessor->GetMetatag(cpname, value);
    return value.c_str();
}

/* Get a content from a zim file */
const char* ZimAccessor_GetContent(HZIMCLASS h, char* url) {
    assert(h != NULL);

    ZimAccessor * zimAccessor = (ZimAccessor *)h;

    string cpurl = string(url);
    string content = "";
    unsigned int contentLength = 0;
    string contentType = "";
    zimAccessor->GetContent( cpurl, content, contentLength, contentType );
    return content.c_str();
}

/* Search titles by prefix*/
unsigned int ZimAccessor_SearchSuggestions( HZIMCLASS h, char* prefix) {
    assert(h != NULL);

    ZimAccessor * zimAccessor = (ZimAccessor *)h;

    string cpprefix = string(prefix);
    unsigned int suggestionsCount;
    zimAccessor->SearchSuggestions(cpprefix, suggestionsCount);
    return suggestionsCount;
}

/* Get next suggestion */
const char* ZimAccessor_GetNextSuggestion( HZIMCLASS h ) {
    assert(h != NULL);

    ZimAccessor * zimAccessor = (ZimAccessor *)h;

    string title = "";
    zimAccessor->GetNextSuggestion( title );
    return title.c_str();
}

/* Can I check the integrity - for old ZIM file without checksum */
int ZimAccessor_CanCheckIntegrity( HZIMCLASS h ) {
    assert(h != NULL);

    ZimAccessor * zimAccessor = (ZimAccessor *)h;

    return zimAccessor->CanCheckIntegrity();
}

/* Return true if corrupted, false otherwise */
int ZimAccessor_IsCorrupted( HZIMCLASS h ) {
    assert(h != NULL);

    ZimAccessor * zimAccessor = (ZimAccessor *)h;

    return zimAccessor->IsCorrupted();
}

