
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <kiwix/reader.h>

class ZimAccessor
{
    public:
        ZimAccessor(int);
        bool LoadFile(string);
        bool GetArticleCount(unsigned int&);
        bool Reset();
        bool GetId(string&);
        bool GetPageUrlFromTitle(const string &, string &);
        bool GetMainPageUrl(string &);
        bool GetMetatag(const string &, string &);
        // WARNING: previous API expected an nsIURI instead of string
        bool GetContent(string, string &, unsigned int &, string &);
        bool SearchSuggestions(const string &, unsigned int);
        bool GetNextSuggestion(string &);
        bool CanCheckIntegrity();
        bool IsCorrupted();

    public:
        kiwix::Reader *reader;
};
