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
#include <ZimAccessorWrapper.h>


int main(int argc, char* argv[])
{
    if (argc < 2) {
        printf("Usage: %s zimPath\n", argv[0]);
        printf("No ZIM file path provided.\n");
        exit(0);
    }
    char* zimPath = argv[1];
    printf("Kiwix C Tester.\nLoading %s...\n", zimPath);

    HZIMCLASS zimA = NULL;
    zimA = ZimAccessor_Create();
    if (zimA == NULL) {
        printf("Error creating ZimAccessor class\n");
        exit(1);
    }
    ZimAccessor_LoadFile( zimA, zimPath );
    printf("Done.\n");
    const char* hp;
    hp = ZimAccessor_GetMainPageUrl( zimA );
    printf("homepage: %s\n", hp);

    char * url = "A/Lugha.html";
    const char* content;
    content = ZimAccessor_GetContent( zimA, url );

    printf("%s", content);

    ZimAccessor_Destroy( zimA );
    zimA = NULL;

    return 0;
}
