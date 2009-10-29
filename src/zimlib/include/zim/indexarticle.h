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

#ifndef ZIM_INDEXARTICLE_H
#define ZIM_INDEXARTICLE_H

#include <zim/article.h>
#include <vector>

namespace zim
{
  class IndexArticle : public Article
  {
    public:
      struct Entry
      {
        unsigned index;
        unsigned pos;
      };

      typedef std::vector<Entry> EntriesType;

    private:
      EntriesType entries[4];
      bool categoriesRead;
      void readEntries();
      void readEntriesZ();  // directmedia style zint-compression
      void readEntriesB();  // article compressed style

      static bool noOffset;

    public:
      IndexArticle(const Article& article)
        : Article(article),
          categoriesRead(false)
        { }

      unsigned getCategoryCount(unsigned cat)
        { readEntries(); return entries[cat].size(); }
      const EntriesType& getCategory(unsigned cat)
        { readEntries(); return entries[cat]; }
      unsigned getTotalCount()
      {
        readEntries();
        unsigned c = 0;
        for (unsigned cat = 0; cat < 4; ++cat)
          c += entries[cat].size();
        return c;
      }

      static void setNoOffset(bool sw = true)   { noOffset = sw; }
      static bool getNoOffset()                 { return noOffset; }
  };
}

#endif // ZIM_INDEXARTICLE_H
