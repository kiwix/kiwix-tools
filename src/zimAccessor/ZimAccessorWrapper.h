
#pragma once

#ifdef __cplusplus
extern "C" { 
#endif

#include <ctype.h>

typedef void * HZIMCLASS;

HZIMCLASS ZimAccessor_Create( void );

void ZimAccessor_Destroy( HZIMCLASS h );

int ZimAccessor_LoadFile( HZIMCLASS h, char* zimPath );
const char* ZimAccessor_GetMainPageUrl( HZIMCLASS h );
const char* ZimAccessor_GetContent( HZIMCLASS h, char* url);

#ifdef __cplusplus 
};
#endif
