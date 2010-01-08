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

#include <zim/indexarticle.h>
#include <zim/zintstream.h>
#include <sstream>
#include <stdexcept>
#include "log.h"
#include "ptrstream.h"

log_define("zim.indexarticle")

namespace zim
{
  bool IndexArticle::noOffset = false;

  void IndexArticle::readEntries()
  {
    if (!good() || categoriesRead)
      return;

    log_debug("read entries for article " << getUrl());

    if (getParameter().empty())
      readEntriesB();
    else
      readEntriesZ();

    categoriesRead = true;
  }

  void IndexArticle::readEntriesZ()
  {
    std::istringstream s(getParameter());
    zim::ZIntStream extra(s);

    unsigned flagfield;  // field with one bit (bits 0-3) for each cateogry
    extra.get(flagfield);

    log_debug("flags: h" << std::hex << flagfield);

    unsigned offset = 0;
    for (unsigned c = 0; c <= 3; ++c)
    {
      bool catNotEmpty = (flagfield & 1);
      flagfield >>= 1;

      if (catNotEmpty)
      {
        log_debug("read category " << c);

        unsigned len;
        Entry entry;
        bool s = extra.get(len) && extra.get(entry.index);
        if (s && getNamespace() == 'X')
          s = extra.get(entry.pos);
        else
          entry.pos = 0;

        unsigned pos = entry.pos;

        if (!s)
          throw std::runtime_error("invalid index entry");

        log_debug("first index " << entry.index << " pos " << entry.pos);
        entries[c].push_back(entry);

        log_debug("read data from offset " << offset << " len " << len);
        zim::Blob b = getData();
        ptrstream data(const_cast<char*>(b.data() + offset), const_cast<char*>(b.data() + offset + len));
        ZIntStream zdata(data);

        unsigned index;
        unsigned indexOffset = 0;
        while (zdata.get(index))
        {
          entry.index = indexOffset + index;

          if (!noOffset)
            indexOffset += index;

          if (getNamespace() == 'X')
          {
            unsigned p;
            if (!zdata.get(p))
              throw std::runtime_error("invalid index entry");
            pos += p;
            entry.pos = p;
          }
          else
            entry.pos = 0;

          log_debug("index " << entry.index << " pos " << entry.pos);

          entries[c].push_back(entry);
        }

        offset += len;
      }
    }

  }

  namespace
  {
    class Eof { };

    zim::size_type getSizeValue(std::istream& in)
    {
      zim::size_type ret;
      in.read(reinterpret_cast<char*>(&ret), sizeof(zim::size_type));
      if (!in)
        throw Eof();
      ret = fromLittleEndian<zim::size_type>(&ret);
      return ret;
    }
  }

  void IndexArticle::readEntriesB()
  {
    try
    {
      zim::size_type categoryCount[4];
      zim::Blob b = getData();
      ptrstream data(const_cast<char*>(b.data()), const_cast<char*>(b.end()));
      for (unsigned c = 0; c < 4; ++c)
        categoryCount[c] = getSizeValue(data);

      for (unsigned c = 0; c < 4; ++c)
      {
        log_debug("read " << categoryCount[c] << " entries for category " << c);
        for (unsigned n = 0; n < categoryCount[c]; ++n)
        {
          Entry entry;
          entry.index = getSizeValue(data);
          if (getNamespace() == 'X')
            entry.pos = getNamespace() ? getSizeValue(data) : 0;
          entries[c].push_back(entry);
        }
      }
    }
    catch (const Eof&)
    {
      log_error("end of file when reading index entries for article " << getTitle());
      return;
    }
  }

}
