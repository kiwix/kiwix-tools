/*
 * Copyright (C) 2006 Tommi Maekitalo
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

#ifndef ZIM_FILEITERATOR_H
#define ZIM_FILEITERATOR_H

#include <iterator>
#include <zim/article.h>

namespace zim
{
  class File::const_iterator : public std::iterator<std::bidirectional_iterator_tag, Article>
  {
    public:
      enum Mode {
        UrlIterator,
        ArticleIterator
      };

    private:
      File* file;
      size_type idx;
      mutable Article article;
      Mode mode;

      bool is_end() const  { return file == 0 || idx >= file->getCountArticles(); }

    public:
      explicit const_iterator(File* file_ = 0, size_type idx_ = 0, Mode mode_ = UrlIterator)
        : file(file_),
          idx(idx_),
          mode(mode_)
      { }

      size_type getIndex() const   { return idx; }
      const File& getFile() const  { return *file; }

      bool operator== (const const_iterator& it) const
        { return (is_end() && it.is_end())
              || (file == it.file && idx == it.idx); }
      bool operator!= (const const_iterator& it) const
        { return !operator==(it); }

      const_iterator& operator++()
      {
        ++idx;
        article = Article();
        return *this;
      }

      const_iterator operator++(int)
      {
        const_iterator it = *this;
        operator++();
        return it;
      }

      const_iterator& operator--()
      {
        --idx;
        article = Article();
        return *this;
      }

      const_iterator operator--(int)
      {
        const_iterator it = *this;
        operator--();
        return it;
      }

      const Article& operator*() const
      {
        if (!article.good())
          article = mode == UrlIterator ? file->getArticle(idx)
                                        : file->getArticleByTitle(idx);
        return article;
      }

      pointer operator->() const
      {
        operator*();
        return &article;
      }

  };

}

#endif // ZIM_FILEITERATOR_H

