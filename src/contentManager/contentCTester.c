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
#include "ContentManagerWrapper.h"


int main(int argc, char* argv[])
{
    if (argc < 2) {
        printf("Usage: %s zimPath\n", argv[0]);
        printf("No ZIM file path provided.\n");
        exit(0);
    }
    char* zimPath = argv[1];
    printf("Content Manager C Tester.\nLoading %s...\n", zimPath);
    
    HCONTCLASS contM = NULL;
    contM = ContentManager_Create();
    if (contM == NULL) {
        printf("Error creating ContentManager class\n");
        exit(1);
    }

    int readOnly = 1;
    if (ContentManager_OpenLibraryFromFile( contM, zimPath, readOnly ))
        printf("Successfully opened library from %s\n", zimPath);
    else
        printf("Unable to open library from %s\n", zimPath);

    char * xmltext = "<library current='dummy'><book id='dummy' path='blabla.zim' indexPath='blabla.idx' indexType='xapian'/></library>";
    if (ContentManager_OpenLibraryFromText( contM, xmltext, readOnly ))
        printf("Successfully opened library from text.\n");
    else
        printf("Unable to open library from text.\n");

    if (ContentManager_WriteLibrary( contM ))
        printf("Successfully wrote library.\n");
    else
        printf("Unable to write library\n");

    char* libPath = "/tmp/totoc";
    if (ContentManager_WriteLibraryToFile( contM, libPath ))
        printf("Successfully wrote library to %s\n", libPath);
    else
        printf("Unable to write library to %s\n", libPath);

    if (ContentManager_AddBookFromPath( contM, zimPath ))
        printf("Successfully added book from %s\n", zimPath);
    else
        printf("Unable to add book from %s\n", zimPath);

    char* bookId = "57b0b7c3-25a5-431e-431e-dce1771ee052963f";
    if (ContentManager_SetCurrentBookId( contM, bookId ))
        printf("Successfully set book %s\n", bookId);
    else
        printf("Unable to set book %s\n", bookId);

    const char* currBookId = ContentManager_GetCurrentBookId( contM );
    printf("Current Book: %s\n", currBookId);

    if (ContentManager_UpdateBookLastOpenDateById( contM, bookId ))
        printf("Successfully updated last open date for book %s\n", bookId);
    else
        printf("Unable to update last open date for book %s\n", bookId);

    unsigned int bookCount = 0;
    bookCount = ContentManager_GetBookCount( contM, 1, 1 );
    printf("Book count: %d\n", bookCount);

    char* indexPath = "/home/reg/wksw.index";
    char* indexType = "xapian";
    if (ContentManager_SetBookIndex( contM, bookId, indexPath, indexType ))
        printf("Successfully added index for book %s\n", bookId);
    else
        printf("Unable to add index for book %s\n", bookId);

    if (ContentManager_SetBookPath( contM, bookId, zimPath ))
        printf("Successfully set path for book %s\n", bookId);
    else
        printf("Unable to set path for book %s\n", bookId);

    const char* langs;
    langs = ContentManager_GetBooksLanguages( contM );
    printf("Languages: %s\n", langs);

    const char* pubs;
    langs = ContentManager_GetBooksPublishers( contM );
    printf("Publishers: %s\n", langs);

    char* mode = "lastOpen";
    char* lsortBy = "size";
    unsigned int lmaxSize = 0;
    char* llanguage = "";
    char* lpublisher = "";
    char* lsearch = "";
    if (ContentManager_ListBooks( contM, mode, lsortBy, lmaxSize, llanguage, lpublisher, lsearch ))
        printf("Successfully listed books\n");
    else
        printf("Unable to list books\n");

    const char* nextBookId;
    nextBookId = ContentManager_GetListNextBookId( contM );
    printf("Next book id: %s\n", nextBookId);

    CMBook book = ContentManager_GetBookById( contM, bookId );
    printf("Book: %s\n\tarticles: %s\n\tlangs: %s\n", book.title, book.articleCount, book.language);

    if (ContentManager_RemoveBookById( contM, bookId ))
        printf("Successfully removed book %s\n", bookId);
    else
        printf("Unable to remove book %s\n", bookId);

    ContentManager_WriteLibraryToFile( contM, libPath );

    ContentManager_Destroy( contM );
    contM = NULL;

    return 0;
}
