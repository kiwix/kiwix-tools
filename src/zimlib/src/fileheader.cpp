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
  const size_type Fileheader::zimVersion = 4;
  const size_type Fileheader::size = 56;

  std::ostream& operator<< (std::ostream& out, const Fileheader& fh)
  {
    char header[56];
    toLittleEndian(Fileheader::zimMagic, header);
    toLittleEndian(Fileheader::zimVersion, header + 4);
    std::copy(fh.getUuid().data, fh.getUuid().data + sizeof(Uuid), header + 8);
    toLittleEndian(fh.getArticleCount(), header + 24);
    toLittleEndian(fh.getIndexPtrPos(), header + 28);
    toLittleEndian(fh.getClusterCount(), header + 36);
    toLittleEndian(fh.getClusterPtrPos(), header + 40);
    toLittleEndian(fh.getMainPage(), header + 48);
    toLittleEndian(fh.getLayoutPage(), header + 52);

    out.write(header, 56);

    return out;
  }

  std::istream& operator>> (std::istream& in, Fileheader& fh)
  {
    char header[56];
    in.read(header, 56);
    if (in.fail())
      return in;
    if (in.gcount() != 56)
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

    size_type version = fromLittleEndian(reinterpret_cast<const size_type*>(header + 4));
    if (version != Fileheader::zimVersion)
    {
      log_error("invalid zimfile version " << version << " found - "
          << Fileheader::zimVersion << " expected");
      in.setstate(std::ios::failbit);
      return in;
    }

    Uuid uuid;
    std::copy(header + 8, header + 24, uuid.data);
    size_type articleCount = fromLittleEndian(reinterpret_cast<const size_type*>(header + 24));
    offset_type indexPtrPos = fromLittleEndian(reinterpret_cast<const offset_type*>(header + 28));
    size_type blobCount = fromLittleEndian(reinterpret_cast<const size_type*>(header + 36));
    offset_type blobPtrPos = fromLittleEndian(reinterpret_cast<const offset_type*>(header + 40));
    size_type mainPage = fromLittleEndian(reinterpret_cast<const size_type*>(header + 48));
    size_type layoutPage = fromLittleEndian(reinterpret_cast<const size_type*>(header + 52));

    fh.setUuid(uuid);
    fh.setArticleCount(articleCount);
    fh.setIndexPtrPos(indexPtrPos);
    fh.setClusterCount(blobCount);
    fh.setClusterPtrPos(blobPtrPos);
    fh.setMainPage(mainPage);
    fh.setLayoutPage(layoutPage);

    return in;
  }

}
