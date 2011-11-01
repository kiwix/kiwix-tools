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
#include "ContentManagerWrapper.h"
#include "contentManager.h"

/* creates instance of ContentManager */
HCONTCLASS ContentManager_Create( void ) {
    ContentManager * contentManager = new ContentManager(0); 

    // Return its pointer (opaque)
    return (HCONTCLASS)contentManager;
}

/* Delete instance of ZimAccessor */
void ContentManager_Destroy( HCONTCLASS h ) {
    assert(h != NULL);

    // Convert from handle to ContentManager pointer
    ContentManager * contentManager = (ContentManager *)h;

    delete contentManager;
}

int ContentManager_OpenLibraryFromFile( HCONTCLASS h, char* path, int readOnly) {
    assert(h != NULL);

    ContentManager * contentManager = (ContentManager *)h;

    return contentManager->OpenLibraryFromFile( string(path), readOnly );
}

int ContentManager_OpenLibraryFromText( HCONTCLASS h, char* xml, int readOnly) {
    assert(h != NULL);

    ContentManager * contentManager = (ContentManager *)h;

    string xml_str = string(xml);
    return contentManager->OpenLibraryFromText( xml_str, readOnly );
}

int ContentManager_WriteLibrary( HCONTCLASS h) {
    assert(h != NULL);

    ContentManager * contentManager = (ContentManager *)h;

    return contentManager->WriteLibrary();
}

int ContentManager_WriteLibraryToFile( HCONTCLASS h, char* path) {
    assert(h != NULL);

    ContentManager * contentManager = (ContentManager *)h;

    string path_str = string(path);
    return contentManager->WriteLibraryToFile( path_str );
}

int ContentManager_AddBookFromPath( HCONTCLASS h, char* path) {
    assert(h != NULL);

    ContentManager * contentManager = (ContentManager *)h;

    string path_str = string(path);
    return contentManager->AddBookFromPath( path_str );
}

int ContentManager_RemoveBookById( HCONTCLASS h, char* id) {
    assert(h != NULL);

    ContentManager * contentManager = (ContentManager *)h;

    string id_str = string(id);
    return contentManager->RemoveBookById( id_str );
}

int ContentManager_SetCurrentBookId( HCONTCLASS h, char* id) {
    assert(h != NULL);

    ContentManager * contentManager = (ContentManager *)h;

    string id_str = string(id);
    return contentManager->SetCurrentBookId( id_str );
}

const char* ContentManager_GetCurrentBookId( HCONTCLASS h ) {
    assert(h != NULL);

    ContentManager * contentManager = (ContentManager *)h;

    string id_str = contentManager->GetCurrentBookId();
    return id_str.c_str();
}

CMBook ContentManager_GetBookById( HCONTCLASS h, char* id ) {
    assert(h != NULL);

    ContentManager * contentManager = (ContentManager *)h;

    CMBook book;
    string path, title, indexPath, indexType, description, articleCount, mediaCount, size, creator, date, language, favicon, url;
    string id_str = string(id);

    contentManager->GetBookById(id_str, path, title, indexPath, indexType, description, articleCount, mediaCount, size, creator, date, language, favicon, url);
    book.path = path.c_str();
    book.title = title.c_str();
    book.indexPath = indexPath.c_str();
    book.indexType = indexType.c_str();
    book.description = description.c_str();
    book.articleCount = articleCount.c_str();
    book.mediaCount = mediaCount.c_str();
    book.size = size.c_str();
    book.creator = creator.c_str();
    book.date = date.c_str();
    book.language = language.c_str();
    book.favicon = favicon.c_str();
    book.url = url.c_str();
    return book;
}

int ContentManager_UpdateBookLastOpenDateById( HCONTCLASS h, char* id) {
    assert(h != NULL);

    ContentManager * contentManager = (ContentManager *)h;

    string id_str = string(id);
    return contentManager->UpdateBookLastOpenDateById( id_str );
}

unsigned int ContentManager_GetBookCount( HCONTCLASS h, int remote, int local) {
    assert(h != NULL);

    ContentManager * contentManager = (ContentManager *)h;

    return contentManager->GetBookCount( remote, local );
}

const char* ContentManager_GetListNextBookId( HCONTCLASS h) {
    assert(h != NULL);

    ContentManager * contentManager = (ContentManager *)h;

    return contentManager->GetListNextBookId();
}

int ContentManager_SetBookIndex( HCONTCLASS h, char* id, char* path, char* type) {
    assert(h != NULL);

    ContentManager * contentManager = (ContentManager *)h;

    string id_str = string(id);
    string path_str = string(path);
    string type_str = string(type);
    return contentManager->SetBookIndex( id_str, path_str, type_str );
}

int ContentManager_SetBookPath( HCONTCLASS h, char* id, char* path) {
    assert(h != NULL);

    ContentManager * contentManager = (ContentManager *)h;

    string id_str = string(id);
    string path_str = string(path);
    return contentManager->SetBookPath( id_str, path_str );
}

const char* ContentManager_GetBooksLanguages( HCONTCLASS h ) {
    assert(h != NULL);

    ContentManager * contentManager = (ContentManager *)h;

    string languages = contentManager->GetBooksLanguages();
    return languages.c_str();
}

const char* ContentManager_GetBooksPublishers( HCONTCLASS h ) {
    assert(h != NULL);

    ContentManager * contentManager = (ContentManager *)h;

    string publishers = contentManager->GetBooksPublishers();
    return publishers.c_str();
}

int ContentManager_ListBooks( HCONTCLASS h, char* mode, char* sortBy, unsigned int maxSize, char* language, char* publisher, char* search ) {
    assert(h != NULL);

    ContentManager * contentManager = (ContentManager *)h;

    string mode_str = string(mode);
    string sort_str = string(sortBy);
    string lang_str = string(language);
    string pub_str = string(publisher);
    string search_str = string(search);
    return contentManager->ListBooks( mode_str, sort_str, maxSize, lang_str, pub_str, search_str );
}
