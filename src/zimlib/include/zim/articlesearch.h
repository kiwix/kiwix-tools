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

#ifndef ZIM_ARTICLESEARCH_H
#define ZIM_ARTICLESEARCH_H

#include <vector>
#include <zim/file.h>
#include <zim/fileiterator.h>
#include <zim/article.h>

namespace zim
{
  class ArticleSearch
  {
    public:
      typedef std::vector<Article> Results;

    private:
      File articleFile;
      std::string titles;

    public:
      explicit ArticleSearch(const File& articleFile_)
        : articleFile(articleFile_)
        { }

      Results search(const std::string& expr);
  };
}

#endif //  ZIM_ARTICLESEARCH_H
