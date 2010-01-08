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
#include <zim/endian.h>
#include <stdlib.h>
#include <sstream>

#include "log.h"

#include "config.h"

#ifdef ENABLE_ZLIB
#include <zim/deflatestream.h>
#include <zim/inflatestream.h>
#endif

#ifdef ENABLE_BZIP2
#include <zim/bzip2stream.h>
#include <zim/bunzip2stream.h>
#endif

#ifdef ENABLE_LZMA
#include <zim/lzmastream.h>
#include <zim/unlzmastream.h>
#endif

log_define("zim.cluster")

#define log_debug1(e)

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
    log_debug1("read");

    // read first offset, which specifies, how many offsets we need to read
    size_type offset;
    in.read(reinterpret_cast<char*>(&offset), sizeof(offset));
    if (in.fail())
      return;

    offset = fromLittleEndian(&offset);

    size_type n = offset / 4;
    size_type a = offset;

    log_debug1("first offset is " << offset << " n=" << n << " a=" << a);

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
        log_debug1("fail at " << n);
        return;
      }
      offset = fromLittleEndian(&offset);
      log_debug1("offset=" << offset << '(' << offset-a << ')');
      offsets.push_back(offset - a);
    }

    // last offset points past the end of the cluster, so we know now, how may bytes to read
    if (offsets.size() > 1)
    {
      n = offsets.back() - offsets.front();
      data.resize(n);
      log_debug1("read " << n << " bytes of data");
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
    log_debug1("addBlob(ptr, " << blob.size() << ')');
    data.insert(data.end(), blob.data(), blob.end());
    offsets.push_back(data.size());
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
    log_trace("read cluster");

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
#ifdef ENABLE_ZLIB
          log_debug("uncompress data (zlib)");
          zim::InflateStream is(in);
          is.exceptions(std::ios::failbit | std::ios::badbit);
          clusterImpl.read(is);
#else
          throw std::runtime_error("zlib not enabled in this library");
#endif
          break;
        }

      case zimcompBzip2:
        {
#ifdef ENABLE_BZIP2
          log_debug("uncompress data (bzip2)");
          zim::Bunzip2Stream is(in);
          is.exceptions(std::ios::failbit | std::ios::badbit);
          clusterImpl.read(is);
#else
          throw std::runtime_error("bzip2 not enabled in this library");
#endif
          break;
        }

      case zimcompLzma:
        {
#ifdef ENABLE_LZMA
          log_debug("uncompress data (lzma)");
          zim::UnlzmaStream is(in);
          is.exceptions(std::ios::failbit | std::ios::badbit);
          clusterImpl.read(is);
#else
          throw std::runtime_error("lzma not enabled in this library");
#endif
          break;
        }

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
    log_trace("write cluster");

    out.put(static_cast<char>(clusterImpl.getCompression()));

    switch(clusterImpl.getCompression())
    {
      case zimcompDefault:
      case zimcompNone:
        clusterImpl.write(out);
        break;

      case zimcompZip:
        {
#ifdef ENABLE_ZLIB
          log_debug("compress data (zlib)");
          zim::DeflateStream os(out);
          os.exceptions(std::ios::failbit | std::ios::badbit);
          clusterImpl.write(os);
          os.flush();
#else
          throw std::runtime_error("zlib not enabled in this library");
#endif
          break;
        }

      case zimcompBzip2:
        {
#ifdef ENABLE_BZIP2
          log_debug("compress data (bzip2)");
          zim::Bzip2Stream os(out);
          os.exceptions(std::ios::failbit | std::ios::badbit);
          clusterImpl.write(os);
          os.end();
#else
          throw std::runtime_error("bzip2 not enabled in this library");
#endif
          break;
        }

      case zimcompLzma:
        {
#ifdef ENABLE_LZMA
          uint32_t lzmaPreset = 3 | LZMA_PRESET_EXTREME;
          /**
           * read lzma preset from environment
           * ZIM_LZMA_PRESET is a number followed optionally by a
           * suffix 'e'. The number gives the preset and the suffix tells,
           * if LZMA_PRESET_EXTREME should be set.
           * e.g.:
           *   ZIM_LZMA_LEVEL=9   => 9
           *   ZIM_LZMA_LEVEL=3e  => 3 + extreme
           */
          const char* e = ::getenv("ZIM_LZMA_LEVEL");
          if (e)
          {
            char flag = '\0';
            std::istringstream s(e);
            s >> lzmaPreset >> flag;
            if (flag == 'e')
              lzmaPreset |= LZMA_PRESET_EXTREME;
          }

          log_debug("compress data (lzma, " << std::hex << lzmaPreset << ")");
          zim::LzmaStream os(out, lzmaPreset);
          os.exceptions(std::ios::failbit | std::ios::badbit);
          clusterImpl.write(os);
          os.end();
#else
          throw std::runtime_error("lzma not enabled in this library");
#endif
          break;
        }

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
