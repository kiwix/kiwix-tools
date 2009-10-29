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
#include "log.h"

log_define("zim.zintstream")

namespace zim
{
  IZIntStream& IZIntStream::get(unsigned &value)
  {
    char ch;
    if (!stream.get(ch))
      return *this;

    unsigned ret = static_cast<unsigned>(static_cast<unsigned char>(ch));
    unsigned numb = ret & 0x3;
    ret >>= 2;
    unsigned s = 6;
    while (numb && stream.get(ch))
    {
      ret += static_cast<unsigned>(
               static_cast<unsigned char>(ch)) + 1 << s;
      s += 8;
      --numb;
    }

    if (numb)
    {
      log_error("incomplete bytestream");
      stream.setstate(std::ios::failbit);
    }
    else
      value = ret;

    return *this;
  }

  OZIntStream& OZIntStream::put(size_type value)
  {
    char data[4];
    unsigned count;
    if (value < 64)
    {
      count = 1;
      data[0] = (value << 2);
      log_debug(value << " => " << std::hex << static_cast<unsigned>(static_cast<unsigned char>(data[0])));
    }
    else if (value < 16384 + 64)
    {
      value -= 64;
      count = 2;
      data[0] = value << 2 | 1;
      data[1] = value >> 6;
      log_debug(value << " => " << std::hex << static_cast<unsigned>(static_cast<unsigned char>(data[0]))
                                << std::hex << static_cast<unsigned>(static_cast<unsigned char>(data[1])));
    }
    else if (value < 4194304 + 16384 + 64)
    {
      value -= 16384 + 64;
      count = 3;
      data[0] = value << 2 | 2;
      data[1] = value >> 6;
      data[2] = value >> 14;
      log_debug(value << " => " << std::hex << static_cast<unsigned>(static_cast<unsigned char>(data[0]))
                                << std::hex << static_cast<unsigned>(static_cast<unsigned char>(data[1]))
                                << std::hex << static_cast<unsigned>(static_cast<unsigned char>(data[2])));
    }
    else
    {
      value -= 4194304 + 16384 + 64;
      count = 4;
      data[0] = value << 2 | 3;
      data[1] = value >> 6;
      data[2] = value >> 14;
      data[3] = value >> 22;
      log_debug(value << " => " << std::hex << static_cast<unsigned>(static_cast<unsigned char>(data[0]))
                                << std::hex << static_cast<unsigned>(static_cast<unsigned char>(data[1]))
                                << std::hex << static_cast<unsigned>(static_cast<unsigned char>(data[2]))
                                << std::hex << static_cast<unsigned>(static_cast<unsigned char>(data[4])));
    }

    stream.write(reinterpret_cast<char*>(&data[0]), count);

    return *this;
  }

}
