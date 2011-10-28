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
#include <kiwix/manager.h>
#include <pathTools.h>
#include <componentTools.h>
#include <regexTools.h>
#include "contentManager.h"


int main(int argc, char* argv[])
{
    ContentManager cont_man(0);
    ContentManager* cont = new ContentManager(0);

    if (argc < 2) {
        cout << "Usage: " << argv[0] << " zimPath" << endl << endl;
        cout << "No ZIM file path provided." << endl;
        exit(0);
    }
    string zimPath = (string) argv[1];
    cout << "Kiwix Tester." << endl << "Loading " << zimPath << "..." << endl;

    if (cont->OpenLibraryFromFile(zimPath, true))
        cout << "Successfully opened ZIM file." << endl;
    else
        cout << "Unable to open ZIM file." << endl;

    string xmltext = "<library current='e04bcf41-eae8-b04b-b04b-da2d0c00f4220000'><book id='e04bcf41-eae8-b04b-b04b-da2d0c00f4220000' path='ubuntudoc_fr_01_2009.zim' indexPath='ubuntudoc_fr_01_2009.zim.idx' indexType='xapian'/></library>";
    if (cont->OpenLibraryFromText(xmltext, true))
        cout << "Successfully opened XML library." << endl;
    else
        cout << "Unable to open XML library." << endl;

    if (cont->WriteLibrary())
        cout << "Successfully wrote library." << endl;
    else
        cout << "Unable to write library." << endl;

    if (cont->AddBookFromPath(zimPath))
        cout << "Successfully added book " << zimPath << endl;
    else
        cout << "Unable to add book " << zimPath << endl;

    string lpath = "/tmp/toto";
    if (cont->WriteLibraryToFile(lpath))
        cout << "Successfully wrote library to " << lpath << endl;
    else
        cout << "Unable to write library to " << lpath << endl;

    cont->WriteLibraryToFile(lpath);

    delete cont;

    return 0;
}
