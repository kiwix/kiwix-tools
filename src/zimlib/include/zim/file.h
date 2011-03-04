/*
 * Copyright (C) 2006,2009 Tommi Maekitalo
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

#ifndef ZIM_FILE_H
#define ZIM_FILE_H

#include <string>
#include <iterator>
#include <zim/zim.h>
#include <zim/fileimpl.h>
#include <zim/blob.h>
#include <zim/smartptr.h>

namespace zim
{
  class Article;

  class File
  {
      SmartPtr<FileImpl> impl;

    public:
      File()
        { }
      explicit File(const std::string& fname)
        : impl(new FileImpl(fname.c_str()))
        { }

      const std::string& getFilename() const   { return impl->getFilename(); }
      const Fileheader& getFileheader() const  { return impl->getFileheader(); }

      Dirent getDirent(size_type idx)          { return impl->getDirent(idx); }
      Dirent getDirentByTitle(size_type idx)   { return impl->getDirentByTitle(idx); }
      size_type getCountArticles() const       { return impl->getCountArticles(); }

      Article getArticle(size_type idx) const;
      Article getArticle(char ns, const std::string& url);
      Article getArticleByUrl(const std::string& url);
      Article getArticleByTitle(size_type idx);
      Article getArticleByTitle(char ns, const std::string& title);

      Cluster getCluster(size_type idx) const  { return impl->getCluster(idx); }
      size_type getCountClusters() const       { return impl->getCountClusters(); }
      offset_type getClusterOffset(size_type idx) const    { return impl->getClusterOffset(idx); }

      Blob getBlob(size_type clusterIdx, size_type blobIdx)
        { return getCluster(clusterIdx).getBlob(blobIdx); }

      size_type getNamespaceBeginOffset(char ch)
        { return impl->getNamespaceBeginOffset(ch); }
      size_type getNamespaceEndOffset(char ch)
        { return impl->getNamespaceEndOffset(ch); }
      size_type getNamespaceCount(char ns)
        { return getNamespaceEndOffset(ns) - getNamespaceBeginOffset(ns); }

      std::string getNamespaces()
        { return impl->getNamespaces(); }
      bool hasNamespace(char ch);

      class const_iterator;

      const_iterator begin();
      const_iterator beginByTitle();
      const_iterator end();
      std::pair<bool, const_iterator> findx(char ns, const std::string& url);
      std::pair<bool, const_iterator> findx(const std::string& url);
      std::pair<bool, const_iterator> findxByTitle(char ns, const std::string& title);
      const_iterator findByTitle(char ns, const std::string& title);
      const_iterator find(char ns, const std::string& url);
      const_iterator find(const std::string& url);

      bool good() const    { return impl.getPointer() != 0; }

      const std::string& getMimeType(uint16_t idx) const   { return impl->getMimeType(idx); }

      std::string getChecksum()   { return impl->getChecksum(); }
      bool verify()               { return impl->verify(); }
  };

  std::string urldecode(const std::string& url);

}

#endif // ZIM_FILE_H

