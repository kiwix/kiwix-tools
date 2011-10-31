
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <kiwix/manager.h>
#include <pathTools.h>
#include <componentTools.h>
#include <regexTools.h>

class ContentManager
{
    public:
        ContentManager(int);
        bool OpenLibraryFromFile(string, bool);
        bool OpenLibraryFromText(string&, bool);
        bool WriteLibrary();
        bool WriteLibraryToFile(string&);
        bool AddBookFromPath(string&);
        bool RemoveBookById(string&);
        bool SetCurrentBookId(string&);
        string GetCurrentBookId();
        bool GetBookById(string&, string&, string&,
	              string&, string&, string&,
	              string&, string&, string&,
	              string&, string&, string&,
	              string&, string&);
        bool UpdateBookLastOpenDateById(string&);
        unsigned int GetBookCount(bool, bool);
        const char* GetListNextBookId();
        bool SetBookIndex(string&, string&, string&);
        bool SetBookPath(string&, string&);
        string GetBooksLanguages();
        string GetBooksPublishers();
        bool ListBooks(string&, string&, unsigned int, string&, string&, string&);
    protected:
        kiwix::Manager manager;

};
