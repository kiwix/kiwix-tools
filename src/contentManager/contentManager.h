
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
        /*bool RemoveBookById(string);
        bool GetBookById(string, string, string, 
	              string, string, string,
	              string, string, string,
	              string, string, string, 
	              string, string);
        bool SetCurrentBookId(string);
        bool GetCurrentBookId(string);
        bool GetListNextBookId(string);
        bool SetBookIndex(string, string, string);
        bool SetBookPath(string, string);
        bool UpdateBookLastOpenDateById(string);
        bool ListBooks(string, string, unsigned int, string, string, string);
        bool GetBookCount(bool, bool, unsigned int);
        bool GetBooksLanguages(string);
        bool GetBooksPublishers(string);*/
    protected:
        kiwix::Manager manager;

};
