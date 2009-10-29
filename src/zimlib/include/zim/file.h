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

      Dirent getDirent(size_type idx);
      size_type getCountArticles() const       { return impl->getCountArticles(); }

      Article getArticle(size_type idx) const;
      Article getArticle(char ns, const QUnicodeString& title, bool collate = false);

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
      const_iterator end();
      std::pair<bool, const_iterator> findx(char ns, const QUnicodeString& title, bool collate = false);
      const_iterator find(char ns, const QUnicodeString& title, bool collate = false);

      bool good() const    { return impl.getPointer() != 0; }
      time_t getMTime() const   { return impl->getMTime(); }
  };

}

#endif // ZIM_FILE_H

