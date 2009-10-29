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

#include <zim/cluster.h>
#include <zim/blob.h>
#include <sstream>
#include "log.h"
#include <zim/deflatestream.h>
#include <zim/inflatestream.h>
#include <zim/bzip2stream.h>
#include <zim/bunzip2stream.h>
#include <zim/endian.h>

log_define("zim.cluster")

namespace zim
{
  Cluster::Cluster()
    : impl(0)
    { }

  ClusterImpl* Cluster::getImpl()
  {
    if (impl.getPointer() == 0)
      impl = new ClusterImpl();
    return impl;
  }

  ClusterImpl::ClusterImpl()
    : compression(zimcompDefault)
  {
    offsets.push_back(0);
  }

  void ClusterImpl::read(std::istream& in)
  {
    log_debug("read");

    // read first offset, which specifies, how many offsets we need to read
    size_type offset;
    in.read(reinterpret_cast<char*>(&offset), sizeof(offset));
    if (in.fail())
      return;

    offset = fromLittleEndian(&offset);

    size_type n = offset / 4;
    size_type a = offset;

    log_debug("first offset is " << offset << " n=" << n << " a=" << a);

    // read offsets
    offsets.clear();
    data.clear();
    offsets.reserve(n);
    offsets.push_back(0);
    while (--n)
    {
      in.read(reinterpret_cast<char*>(&offset), sizeof(offset));
      if (in.fail())
      {
        log_debug("fail at " << n);
        return;
      }
      offset = fromLittleEndian(&offset);
      log_debug("offset=" << offset << '(' << offset-a << ')');
      offsets.push_back(offset - a);
    }

    // last offset points past the end of the cluster, so we know now, how may bytes to read
    if (offsets.size() > 1)
    {
      n = offsets.back() - offsets.front();
      data.resize(n);
      log_debug("read " << n << " bytes of data");
      in.read(&(data[0]), n);
    }
  }

  void ClusterImpl::write(std::ostream& out) const
  {
    size_type a = offsets.size() * sizeof(size_type);
    for (Offsets::const_iterator it = offsets.begin(); it != offsets.end(); ++it)
    {
      size_type o = *it;
      o += a;
      o = fromLittleEndian(&o);
      out.write(reinterpret_cast<const char*>(&o), sizeof(size_type));
    }

    out.write(&(data[0]), data.size());
  }

  void ClusterImpl::addBlob(const Blob& blob)
  {
    log_debug("addBlob(ptr, " << blob.size() << ')');
    data.insert(data.end(), blob.data(), blob.end());
    offsets.push_back(data.size());

    for (unsigned n = 0; n < offsets.size(); ++n)
      log_debug("offset[" << n << "]=" << offsets[n]);
  }

  Blob ClusterImpl::getBlob(size_type n) const
  {
    return Blob(const_cast<ClusterImpl*>(this), getData(n), getSize(n));
  }

  void ClusterImpl::clear()
  {
    offsets.clear();
    data.clear();
    offsets.push_back(0);
  }

  void ClusterImpl::addBlob(const char* data, unsigned size)
  {
    addBlob(Blob(data, size));
  }

  Blob Cluster::getBlob(size_type n) const
  {
    return impl->getBlob(n);
  }

  std::istream& operator>> (std::istream& in, ClusterImpl& clusterImpl)
  {
    char c;
    in.get(c);
    clusterImpl.setCompression(static_cast<CompressionType>(c));

    switch (static_cast<CompressionType>(c))
    {
      case zimcompDefault:
      case zimcompNone:
        clusterImpl.read(in);
        break;

      case zimcompZip:
        {
          log_debug("uncompress data (zlib)");
          zim::InflateStream is(in);
          clusterImpl.read(is);
          break;
        }

      case zimcompBzip2:
        {
          log_debug("uncompress data (bzip2)");
          zim::Bunzip2Stream is(in);
          clusterImpl.read(is);
          break;
        }

      case zimcompLzma:
        throw std::runtime_error("lzma decompression is not implemented");

      default:
        log_error("invalid compression flag " << c);
        in.setstate(std::ios::failbit);
        break;
    }

    return in;
  }

  std::istream& operator>> (std::istream& in, Cluster& cluster)
  {
    return in >> *cluster.getImpl();
  }

  std::ostream& operator<< (std::ostream& out, const ClusterImpl& clusterImpl)
  {
    out.put(static_cast<char>(clusterImpl.getCompression()));

    switch(clusterImpl.getCompression())
    {
      case zimcompDefault:
      case zimcompNone:
        clusterImpl.write(out);
        break;

      case zimcompZip:
        {
          log_debug("compress data (zlib)");
          zim::DeflateStream os(out);
          clusterImpl.write(os);
          os.flush();
          break;
        }

      case zimcompBzip2:
        {
          log_debug("compress data (bzip2)");
          zim::Bzip2Stream os(out);
          clusterImpl.write(os);
          os.end();
          break;
        }

      case zimcompLzma:
        throw std::runtime_error("lzma compression is not implemented");

      default:
        std::ostringstream msg;
        msg << "invalid compression flag " << clusterImpl.getCompression();
        log_error(msg.str());
        throw std::runtime_error(msg.str());
    }

    return out;
  }

  std::ostream& operator<< (std::ostream& out, const Cluster& cluster)
  {
    return out << *cluster.impl;
  }
}
