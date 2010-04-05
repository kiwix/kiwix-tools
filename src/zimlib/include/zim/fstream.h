/*
 * Copyright (C) 2010 Tommi Maekitalo
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

#ifndef ZIM_FSTREAM_H
#define ZIM_FSTREAM_H

#include <iostream>
#include <vector>
#include <zim/zim.h>

namespace zim
{
  class streambuf : public std::streambuf
  {
      std::vector<char> buffer;
      int fd;

      std::streambuf::int_type overflow(std::streambuf::int_type ch);
      std::streambuf::int_type underflow();
      int sync();

    public:
      typedef zim::offset_type offset_type;

      streambuf(const char* fname, unsigned bufsize);
      ~streambuf();

      void seekg(offset_type off);
      void setBufsize(unsigned s)
      { buffer.resize(s); }
  };

  class ifstream : public std::iostream
  {
      streambuf myStreambuf;

    public:
      typedef streambuf::offset_type offset_type;

      explicit ifstream(const char* fname, unsigned bufsize = 8192)
        : std::iostream(&myStreambuf),
          myStreambuf(fname, bufsize)
      { }

      void seekg(offset_type off) { myStreambuf.seekg(off); }
      void setBufsize(unsigned s) { myStreambuf.setBufsize(s); }
  };

}

#endif // ZIM_FSTREAM_H
