
#pragma once

#ifdef __cplusplus
extern "C" { 
#endif

#include <ctype.h>

typedef void * HZIMCLASS;

HZIMCLASS ZimAccessor_Create( void );

void ZimAccessor_Destroy( HZIMCLASS h );

int ZimAccessor_LoadFile( HZIMCLASS h, char* zimPath );
int ZimAccessor_Reset( HZIMCLASS h );
unsigned int ZimAccessor_GetArticleCount( HZIMCLASS h );
const char* ZimAccessor_GetId( HZIMCLASS h );
const char* ZimAccessor_GetPageUrlFromTitle( HZIMCLASS h, char* url);
const char* ZimAccessor_GetMainPageUrl( HZIMCLASS h );
const char* ZimAccessor_GetMetatag( HZIMCLASS h, char* name);
const char* ZimAccessor_GetContent( HZIMCLASS h, char* url);
unsigned int ZimAccessor_SearchSuggestions( HZIMCLASS h, char* prefix);
const char* ZimAccessor_GetNextSuggestion( HZIMCLASS h );
int ZimAccessor_CanCheckIntegrity( HZIMCLASS h );
int ZimAccessor_IsCorrupted( HZIMCLASS h );

#ifdef __cplusplus 
};
#endif
