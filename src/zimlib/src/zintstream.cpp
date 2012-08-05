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

#include <zim/zintstream.h>
#include <stdint.h>
#include "log.h"

log_define("zim.zintstream")

namespace zim
{
  size_type ZIntStream::get()
  {
    char ch;
    if (!_istream->get(ch))
      return *this;

    if (ch == '\xff')
    {
      log_error("invalid bytestream in int decompressor");
      _istream->setstate(std::ios::failbit);
    }
      
    size_type uuvalue = static_cast<size_type>(static_cast<unsigned char>(ch));
    uint64_t ubound = 0x80;
    size_type add = 0;
    unsigned short s = 7;
    unsigned short N = 0;
    size_type mask = 0x7F;
    while (ch & 0x80)
    {
      ++N;
      ch <<= 1;
      --s;
      add += ubound;
      ubound <<= 7;
      mask >>= 1;
    }

    uuvalue &= mask;

    while (N-- && _istream->get(ch))
    {
      uuvalue |= static_cast<size_type>(static_cast<unsigned char>(ch)) << s;
      s += 8;
    }

    if (_istream)
    {
      uuvalue += add;
    }
    else
    {
      log_error("incomplete bytestream in int decompressor");
      _istream->setstate(std::ios::failbit);
    }

    return uuvalue;
  }

  ZIntStream& ZIntStream::put(size_type value)
  {
    size_type nmask = 0;
    size_type mask = 0x7F;
    uint64_t ubound = 0x80;
    unsigned short N = 0;

    while (value >= ubound)
    {
      value -= ubound;
      ubound <<= 7;
      nmask = (nmask >> 1) | 0x80;
      mask = mask >> 1;
      ++N;
    }

    _ostream->put(static_cast<char>(nmask | (value & mask)));
    value >>= 7 - N;
    while (N--)
    {
      _ostream->put(static_cast<char>(value & 0xFF));
      value >>= 8;
    }

    return *this;
  }

}
