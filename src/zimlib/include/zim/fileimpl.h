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

#ifndef ZIM_FILEIMPL_H
#define ZIM_FILEIMPL_H

#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <zim/refcounted.h>
#include <zim/zim.h>
#include <zim/qunicode.h>
#include <zim/fileheader.h>
#include <zim/cache.h>
#include <zim/dirent.h>
#include <zim/cluster.h>

namespace zim
{
  class FileImpl : public RefCounted
  {
      std::ifstream zimFile;
      Fileheader header;
      std::string filename;

      typedef std::vector<offset_type> OffsetsType;
      OffsetsType indexOffsets;
      OffsetsType clusterOffsets;

      Cache<size_type, Dirent> direntCache;
      Cache<offset_type, Cluster> clusterCache;
      typedef std::map<char, size_type> NamespaceCache;
      NamespaceCache namespaceBeginCache;
      NamespaceCache namespaceEndCache;

      std::string namespaces;
      time_t mtime;

    public:
      explicit FileImpl(const char* fname);

      time_t getMTime() const   { return mtime; }

      const std::string& getFilename() const   { return filename; }
      const Fileheader& getFileheader() const  { return header; }

      Dirent getDirent(size_type idx);
      size_type getCountArticles() const       { return indexOffsets.size(); }

      Cluster getCluster(size_type idx);
      size_type getCountClusters() const       { return clusterOffsets.size(); }
      offset_type getClusterOffset(size_type idx) const    { return clusterOffsets[idx]; }

      size_type getNamespaceBeginOffset(char ch);
      size_type getNamespaceEndOffset(char ch);
      size_type getNamespaceCount(char ns)
        { return getNamespaceEndOffset(ns) - getNamespaceBeginOffset(ns); }

      std::string getNamespaces();
      bool hasNamespace(char ch);

  };

}

#endif // ZIM_FILEIMPL_H

