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

#ifndef ZIM_BZIP2STREAM_H
#define ZIM_BZIP2STREAM_H

#include <iostream>
#include <stdexcept>
#include <bzlib.h>
#include <vector>
#include <zim/bzip2.h>

namespace zim
{
  class Bzip2CompressError : public Bzip2Error
  {
    public:
      Bzip2CompressError(int zRet_, const std::string& msg)
        : Bzip2Error(zRet_, msg)
          { }
  };

  class Bzip2StreamBuf : public std::streambuf
  {
      bz_stream stream;
      std::vector<char_type> obuffer;
      std::streambuf* sink;

    public:
      explicit Bzip2StreamBuf(std::streambuf* sink_, int blockSize100k = 9,
        int workFactor = 30, unsigned bufsize = 8192);
      ~Bzip2StreamBuf();

      /// see std::streambuf
      int_type overflow(int_type c);
      /// see std::streambuf
      int_type underflow();
      /// see std::streambuf
      int sync();

      /// end bzip2-stream
      int end();
      void setSink(std::streambuf* sink_)   { sink = sink_; }
  };

  class Bzip2Stream : public std::ostream
  {
      Bzip2StreamBuf streambuf;

    public:
      explicit Bzip2Stream(std::streambuf* sink, int blockSize100k = 9,
        int workFactor = 30, unsigned bufsize = 8192)
        : std::ostream(0),
          streambuf(sink, blockSize100k, workFactor, bufsize)
        { init(&streambuf); }
      explicit Bzip2Stream(std::ostream& sink, int blockSize100k = 9,
        int workFactor = 30, unsigned bufsize = 8192)
        : std::ostream(0),
          streambuf(sink.rdbuf(), blockSize100k, workFactor, bufsize)
        { init(&streambuf); }

      void end();
      void setSink(std::streambuf* sink)   { streambuf.setSink(sink); }
      void setSink(std::ostream& sink)     { streambuf.setSink(sink.rdbuf()); }
  };
}

#endif // ZIM_BZIP2STREAM_H

