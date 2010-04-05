/*
 * Copyright (C) 2007 Tommi Maekitalo
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * is provided AS IS, WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, and
 * NON-INFRINGEMENT.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 *
 */

#ifndef ZIM_SEARCH_H
#define ZIM_SEARCH_H

#include <zim/article.h>
#include <vector>
#include <map>

namespace zim
{
  class SearchResult
  {
      Article article;
      mutable double priority;
      struct WordAttr
      {
        unsigned count;
        unsigned addweight;
        WordAttr() : count(0), addweight(1) { }
      };

      typedef std::map<std::string, WordAttr> WordListType; // map word => count and addweight
      typedef std::map<size_type, std::string> PosListType;  // map position => word
      WordListType wordList;
      PosListType posList;

    public:
      SearchResult() : priority(0) { }
      explicit SearchResult(const Article& article_, unsigned priority_ = 0)
        : article(article_),
          priority(priority_)
          { }
      const Article& getArticle() const  { return article; }
      double getPriority() const;
      void foundWord(const std::string& word, size_type pos, unsigned addweight);
      unsigned getCountWords() const  { return wordList.size(); }
      unsigned getCountPositions() const  { return posList.size(); }
  };

  class Search
  {
    public:
      class Results : public std::vector<SearchResult>
      {
          std::string expr;

        public:
          void setExpression(const std::string& e)
            { expr = e; }
          const std::string& getExpression() const
            { return expr; }
      };

    private:
      static double weightOcc;
      static double weightOccOff;
      static double weightPlus;
      static double weightDist;
      static double weightPos;
      static double weightPosRel;
      static double weightDistinctWords;
      static unsigned searchLimit;

      File indexfile;
      File articlefile;

    public:
      Search()
          { }

      explicit Search(const File& zimfile)
        : indexfile(zimfile),
          articlefile(zimfile)
          { }
      Search(const File& articlefile_, const File& indexfile_)
        : indexfile(indexfile_),
          articlefile(articlefile_)
          { }

      void search(Results& results, const std::string& expr);
      void find(Results& results, char ns, const std::string& praefix, unsigned limit = searchLimit);
      void find(Results& results, char ns, const std::string& begin, const std::string& end, unsigned limit = searchLimit);

      static double getWeightOcc()                 { return weightOcc; }
      static double getWeightOccOff()              { return weightOccOff; }
      static double getWeightPlus()                { return weightPlus; }
      static double getWeightDist()                { return weightDist; }
      static double getWeightPos()                 { return weightPos; }
      static double getWeightPosRel()              { return weightPosRel; }
      static double getWeightDistinctWords()       { return weightDistinctWords; }
      static unsigned getSearchLimit()             { return searchLimit; }

      static void setWeightOcc(double v)           { weightOcc = v; }
      static void setWeightOccOff(double v)        { weightOccOff = v; }
      static void setWeightPlus(double v)          { weightPlus = v; }
      static void setWeightDist(double v)          { weightDist = v; }
      static void setWeightPos(double v)           { weightPos = v; }
      static void setWeightPosRel(double v)        { weightPosRel = v; }
      static void setWeightDistinctWords(double v) { weightDistinctWords = v; }
      static void setSearchLimit(unsigned v)       { searchLimit = v; }
  };
}

#endif // ZIM_SEARCH_H
