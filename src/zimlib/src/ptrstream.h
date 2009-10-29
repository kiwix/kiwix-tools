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

#ifndef ZIM_PTRSTREAM_H
#define ZIM_PTRSTREAM_H

#include <iostream>

namespace zim
{
  class ptrstreambuf : public std::streambuf
  {
    public:
      ptrstreambuf(char* start, char* end)
      {
        setp(start, end);
        setg(start, start, end);
      }

      /// see std::streambuf
      int_type overflow(int_type c);
      /// see std::streambuf
      int_type underflow();
      /// see std::streambuf
      int sync();
  };

  class ptrstream : public std::iostream
  {
      ptrstreambuf streambuf;

    public:
      ptrstream(char* start, char* end)
        : std::iostream(0),
          streambuf(start, end)
      { init(&streambuf); }
  };
}

#endif
