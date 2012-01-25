#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <pathTools.h>
#include <kiwix/reader.h>

class ZimAccessor {
 public:
  ZimAccessor(int);
  bool LoadFile(string path);
  bool GetArticleCount(unsigned int &count);
  bool Reset();
  bool Unload();
  bool GetId(string &id);
  bool GetPageUrlFromTitle(const string &title, string &url);
  bool GetMainPageUrl(string &url);
  bool GetRandomPageUrl(string &url);
  bool GetMetatag(const string &tag, string &value);
  bool GetContent(string url, string &content, unsigned int &contentLength, string &contentType);
  bool SearchSuggestions(const string &prefix, unsigned int suggestionsCount);
  bool GetNextSuggestion(string &title);
  bool CanCheckIntegrity();
  bool IsCorrupted();
  kiwix::Reader *reader;
};
