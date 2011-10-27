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
#include <zimAccessor.h>


int main(int argc, char* argv[])
{
    ZimAccessor zim_acc(0);
    ZimAccessor* zima = new ZimAccessor(0);

    if (argc < 2) {
        cout << "Usage: " << argv[0] << " zimPath" << endl << endl;
        cout << "No ZIM file path provided." << endl;
        exit(0);
    }
    string zimPath = (string) argv[1];
    cout << "Kiwix Tester." << endl << "Loading " << zimPath << "..." << endl;

    if (zima->LoadFile(zimPath))
        cout << "Successfully loaded ZIM file." << endl;
    else
        cout << "Unable to load ZIM file." << endl;

    unsigned int count = 0;
    if (zima->GetArticleCount(count)) {
        printf("ZIM file contains %d articles.\n", count);
    } else {
        cout << "Unable to count articles in ZIM file." << endl;
    }

    zima->Reset();

    string id;
    zima->GetId(id);
    cout << "Current ID: " << id << endl;

    string title = "Lugha";
    string url;
    zima->GetPageUrlFromTitle(title, url);

    cout << "URL for " << title << ": " << url << endl;

    string homepage;
    zima->GetMainPageUrl(homepage);
    cout << "Homepage is: " << homepage << endl;

    // Untested as it's not used in the code and don't know behavior
    //zima->GetMetatag(const string &name, string &value);

    string content;
    unsigned int contentLength;
    string contentType;
    string req = "A/Lugha.html";
    zima->GetContent(homepage, content, contentLength, contentType);
    cout << content << endl;
    cout << contentType << endl;
    cout << contentLength << endl;

    // Untested as it's not used in the code and don't know behavior
    const string prefix = "Lug";
    unsigned int sugg_count = 1000;
    zima->SearchSuggestions(prefix, sugg_count);
    cout << "Suggestions for " << prefix << ": " << sugg_count << endl;

    string next_sugg;
    zima->GetNextSuggestion(next_sugg);
    cout << "Next Suggestion: " << next_sugg << endl;

    cout << "Can check integrity ? " << zima->CanCheckIntegrity() << endl;

    // commented as it takes a while
    //cout << "Is corrupted ? " << zima->IsCorrupted() << endl;

    delete zima;

    return 0;
}
