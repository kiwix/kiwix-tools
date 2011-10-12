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

/* Return the welcome page URL */
const char* ZimAccessor_GetMainPageUrl( HZIMCLASS h ) {
    assert(h != NULL);

    ZimAccessor * zimAccessor = (ZimAccessor *)h;

    string hp = "";
    zimAccessor->GetMainPageUrl( hp );
    return hp.c_str();
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
