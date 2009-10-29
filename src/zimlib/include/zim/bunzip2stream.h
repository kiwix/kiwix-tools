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


#ifndef ZIM_BUNZIP2STREAM_H
#define ZIM_BUNZIP2STREAM_H

#include <iostream>
#include <stdexcept>
#include <bzlib.h>
#include <zim/bzip2.h>

namespace zim
{
  class Bzip2UncompressError : public Bzip2Error
  {
    public:
      Bzip2UncompressError(int zRet_, const std::string& msg)
        : Bzip2Error(zRet_, msg)
          { }
  };

  class Bunzip2StreamBuf : public std::streambuf
  {
      bz_stream stream;
      char_type* iobuffer;
      unsigned bufsize;
      std::streambuf* sinksource;

      char_type* ibuffer()            { return iobuffer; }
      std::streamsize ibuffer_size()  { return bufsize >> 1; }
      char_type* obuffer()            { return iobuffer + ibuffer_size(); }
      std::streamsize obuffer_size()  { return bufsize >> 1; }

    public:
      explicit Bunzip2StreamBuf(std::streambuf* sinksource_, bool small = false, unsigned bufsize = 8192);
      ~Bunzip2StreamBuf();

      /// see std::streambuf
      int_type overflow(int_type c);
      /// see std::streambuf
      int_type underflow();
      /// see std::streambuf
      int sync();

      void setSinksource(std::streambuf* sinksource_)   { sinksource = sinksource_; }
  };

  class Bunzip2Stream : public std::iostream
  {
      Bunzip2StreamBuf streambuf;

    public:
      explicit Bunzip2Stream(std::streambuf* sinksource, bool small = false, unsigned bufsize = 8192)
        : std::iostream(0),
          streambuf(sinksource, small, bufsize)
        { init(&streambuf); }
      explicit Bunzip2Stream(std::ios& sinksource, bool small = false, unsigned bufsize = 8192)
        : std::iostream(0),
          streambuf(sinksource.rdbuf(), small, bufsize)
        { init(&streambuf); }

      void setSinksource(std::streambuf* sinksource)   { streambuf.setSinksource(sinksource); }
      void setSinksource(std::ios& sinksource)         { streambuf.setSinksource(sinksource.rdbuf()); }
      void setSink(std::ostream& sink)                 { streambuf.setSinksource(sink.rdbuf()); }
      void setSource(std::istream& source)             { streambuf.setSinksource(source.rdbuf()); }
  };
}

#endif // ZIM_BUNZIP2STREAM_H

