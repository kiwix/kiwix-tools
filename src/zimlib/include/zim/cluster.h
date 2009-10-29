/*
 * Copyright (C) 2009 Tommi Maekitalo
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

#ifndef ZIM_CLUSTER_H
#define ZIM_CLUSTER_H

#include <zim/zim.h>
#include <zim/refcounted.h>
#include <zim/smartptr.h>
#include <iosfwd>
#include <vector>

namespace zim
{
  class Blob;
  class Cluster;

  class ClusterImpl : public RefCounted
  {
      friend std::istream& operator>> (std::istream& in, ClusterImpl& blobImpl);
      friend std::ostream& operator<< (std::ostream& out, const ClusterImpl& blobImpl);

      typedef std::vector<size_type> Offsets;
      typedef std::vector<char> Data;

      CompressionType compression;
      Offsets offsets;
      Data data;

      void read(std::istream& in);
      void write(std::ostream& out) const;

    public:
      ClusterImpl();

      void setCompression(CompressionType c)  { compression = c; }
      CompressionType getCompression() const  { return compression; }
      bool isCompressed() const               { return compression == zimcompZip || compression == zimcompBzip2 || compression == zimcompLzma; }

      size_type getCount() const              { return offsets.size() - 1; }
      const char* getData(unsigned n) const   { return &data[ offsets[n] ]; }
      size_type getSize(unsigned n) const     { return offsets[n+1] - offsets[n]; }
      size_type getSize() const               { return offsets.size() * sizeof(size_type) + data.size(); }
      Blob getBlob(size_type n) const;
      void clear();

      void addBlob(const Blob& blob);
      void addBlob(const char* data, unsigned size);
  };

  class Cluster
  {
      friend std::istream& operator>> (std::istream& in, Cluster& blob);
      friend std::ostream& operator<< (std::ostream& out, const Cluster& blob);

      SmartPtr<ClusterImpl> impl;

      ClusterImpl* getImpl();

    public:
      Cluster();

      void setCompression(CompressionType c)  { getImpl()->setCompression(c); }
      CompressionType getCompression() const  { return impl ? impl->getCompression() : zimcompNone; }
      bool isCompressed() const
        { return impl && (impl->getCompression() == zimcompZip
                       || impl->getCompression() == zimcompBzip2
                       || impl->getCompression() == zimcompLzma); }

      const char* getBlobPtr(size_type n) const     { return impl->getData(n); }
      size_type getBlobSize(size_type n) const      { return impl->getSize(n); }
      Blob getBlob(size_type n) const;

      size_type count() const   { return impl ? impl->getCount() : 0; }
      size_type size() const    { return impl ? impl->getSize() : 0; }
      void clear()              { impl = 0; }

      void addBlob(const char* data, unsigned size) { getImpl()->addBlob(data, size); }
      void addBlob(const Blob& blob)                { getImpl()->addBlob(blob); }

      operator bool() const   { return impl; }
  };

  std::istream& operator>> (std::istream& in, ClusterImpl& blobImpl);
  std::istream& operator>> (std::istream& in, Cluster& blob);
  std::ostream& operator<< (std::ostream& out, const ClusterImpl& blobImpl);
  std::ostream& operator<< (std::ostream& out, const Cluster& blob);

}

#endif // ZIM_CLUSTER_H
