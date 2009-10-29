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

#include <zim/articlesearch.h>

namespace zim
{
  ArticleSearch::Results ArticleSearch::search(const std::string& expr)
  {
    Results ret;

  // TODO: implement title-cache
#if 0
    if (titles.empty())
    {
      for (File::const_iterator it = articleFile.begin(); it != articleFile.end(); ++it)
      {
        if (article.isMainArticle()
          && article.getLibraryMimeType() == zim::Dirent::zimMimeTextHtml
          && article.getNamespace() == 'A')
        {
          titles.push_back(article.getTitle());
        }
      }
    }
#endif

    for (File::const_iterator it = articleFile.begin(); it != articleFile.end(); ++it)
    {
      std::string title = it->getTitle().toUtf8();
      if (title.find(expr) != std::string::npos)
        ret.push_back(*it);
    }
    return ret;
  }
}
