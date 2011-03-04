/*
 * Copyright (C) 2008 Tommi Maekitalo
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

#include <zim/fileheader.h>
#include <iostream>
#include <algorithm>
#include "log.h"

log_define("zim.file.header")

namespace zim
{
  const size_type Fileheader::zimMagic = 0x044d495a; // ="ZIM^d"
  const size_type Fileheader::zimVersion = 5;
  const size_type Fileheader::size = 80;

  std::ostream& operator<< (std::ostream& out, const Fileheader& fh)
  {
    char header[Fileheader::size];
    toLittleEndian(Fileheader::zimMagic, header);
    toLittleEndian(Fileheader::zimVersion, header + 4);
    std::copy(fh.getUuid().data, fh.getUuid().data + sizeof(Uuid), header + 8);
    toLittleEndian(fh.getArticleCount(), header + 24);
    toLittleEndian(fh.getClusterCount(), header + 28);
    toLittleEndian(fh.getUrlPtrPos(), header + 32);
    toLittleEndian(fh.getTitleIdxPos(), header + 40);
    toLittleEndian(fh.getClusterPtrPos(), header + 48);
    toLittleEndian(fh.getMimeListPos(), header + 56);
    toLittleEndian(fh.getMainPage(), header + 64);
    toLittleEndian(fh.getLayoutPage(), header + 68);
    toLittleEndian(fh.getChecksumPos(), header + 72);

    out.write(header, Fileheader::size);

    return out;
  }

  std::istream& operator>> (std::istream& in, Fileheader& fh)
  {
    char header[Fileheader::size];
    in.read(header, Fileheader::size);
    if (in.fail())
      return in;
    if (static_cast<size_type>(in.gcount()) != Fileheader::size)
    {
      in.setstate(std::ios::failbit);
      return in;
    }

    size_type magicNumber = fromLittleEndian(reinterpret_cast<const size_type*>(header));
    if (magicNumber != Fileheader::zimMagic)
    {
      log_error("invalid magic number " << magicNumber << " found - "
          << Fileheader::zimMagic << " expected");
      in.setstate(std::ios::failbit);
      return in;
    }

    uint16_t version = fromLittleEndian(reinterpret_cast<const uint16_t*>(header + 4));
    if (version != static_cast<size_type>(Fileheader::zimVersion))
    {
      log_error("invalid zimfile version " << version << " found - "
          << Fileheader::zimVersion << " expected");
      in.setstate(std::ios::failbit);
      return in;
    }

    Uuid uuid;
    std::copy(header + 8, header + 24, uuid.data);
    size_type articleCount = fromLittleEndian(reinterpret_cast<const size_type*>(header + 24));
    size_type clusterCount = fromLittleEndian(reinterpret_cast<const size_type*>(header + 28));
    offset_type urlPtrPos = fromLittleEndian(reinterpret_cast<const offset_type*>(header + 32));
    offset_type titleIdxPos = fromLittleEndian(reinterpret_cast<const offset_type*>(header + 40));
    offset_type clusterPtrPos = fromLittleEndian(reinterpret_cast<const offset_type*>(header + 48));
    offset_type mimeListPos = fromLittleEndian(reinterpret_cast<const offset_type*>(header + 56));
    size_type mainPage = fromLittleEndian(reinterpret_cast<const size_type*>(header + 64));
    size_type layoutPage = fromLittleEndian(reinterpret_cast<const size_type*>(header + 68));
    offset_type checksumPos = fromLittleEndian(reinterpret_cast<const offset_type*>(header + 72));

    fh.setUuid(uuid);
    fh.setArticleCount(articleCount);
    fh.setClusterCount(clusterCount);
    fh.setUrlPtrPos(urlPtrPos);
    fh.setTitleIdxPos(titleIdxPos);
    fh.setClusterPtrPos(clusterPtrPos);
    fh.setMimeListPos(mimeListPos);
    fh.setMainPage(mainPage);
    fh.setLayoutPage(layoutPage);
    fh.setChecksumPos(checksumPos);

    return in;
  }

}
