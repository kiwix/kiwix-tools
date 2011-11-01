
#pragma once

#ifdef __cplusplus
extern "C" { 
#endif

#include <ctype.h>

typedef void * HCONTCLASS;

typedef struct {
    const char* path;
    const char* title;
    const char* indexPath;
    const char* indexType;
    const char* description;
    const char* articleCount;
    const char* mediaCount;
    const char* size;
    const char* creator;
    const char* date;
    const char* language;
    const char* favicon;
    const char* url;
} CMBook;

HCONTCLASS ContentManager_Create( void );

void ContentManager_Destroy( HCONTCLASS h );

int ContentManager_OpenLibraryFromFile( HCONTCLASS h, char* path, int readOnly );
int ContentManager_OpenLibraryFromText( HCONTCLASS h, char* xml, int readOnly );
int ContentManager_WriteLibrary( HCONTCLASS h );
int ContentManager_WriteLibraryToFile( HCONTCLASS h, char* path );
int ContentManager_AddBookFromPath( HCONTCLASS h, char* path );
int ContentManager_RemoveBookById( HCONTCLASS h, char* id );
int ContentManager_SetCurrentBookId( HCONTCLASS h, char* id );
const char* ContentManager_GetCurrentBookId( HCONTCLASS h );
CMBook ContentManager_GetBookById( HCONTCLASS h, char* id );
int ContentManager_UpdateBookLastOpenDateById( HCONTCLASS h, char* id );
unsigned int ContentManager_GetBookCount(HCONTCLASS h, int remote, int local );
const char* ContentManager_GetListNextBookId( HCONTCLASS h );
int ContentManager_SetBookIndex( HCONTCLASS h, char* id, char* path, char* type );
int ContentManager_SetBookPath( HCONTCLASS h, char* id, char* path );
const char* ContentManager_GetBooksLanguages( HCONTCLASS h );
const char* ContentManager_GetBooksPublishers( HCONTCLASS h );
int ContentManager_ListBooks( HCONTCLASS h, char* mode, char* sortBy, unsigned int maxSize, char* language, char* publisher, char* search );

#ifdef __cplusplus 
};
#endif
