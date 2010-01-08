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

#ifndef ZIM_LZMASTREAM_H
#define ZIM_LZMASTREAM_H

#include <iostream>
#include <stdexcept>
#include <lzma.h>
#include <vector>

namespace zim
{
  class LzmaError : public std::runtime_error
  {
      lzma_ret ret;

    public:
      LzmaError(lzma_ret ret_, const std::string& msg)
        : std::runtime_error(msg),
          ret(ret_)
          { }

      lzma_ret getRetcode() const  { return ret; }
  };

  class LzmaStreamBuf : public std::streambuf
  {
      lzma_stream stream;
      std::vector<char_type> obuffer;
      std::streambuf* sink;

    public:
      LzmaStreamBuf(std::streambuf* sink_,
        uint32_t preset = 3 | LZMA_PRESET_EXTREME,
        lzma_check check = LZMA_CHECK_CRC32   /* LZMA_CHECK_NONE */,
        unsigned bufsize = 8192);
      ~LzmaStreamBuf();

      /// see std::streambuf
      int_type overflow(int_type c);
      /// see std::streambuf
      int_type underflow();
      /// see std::streambuf
      int sync();
      /// end stream
      int end();

      void setSink(std::streambuf* sink_)   { sink = sink_; }
  };

  class LzmaStream : public std::ostream
  {
      LzmaStreamBuf streambuf;

    public:
      explicit LzmaStream(std::streambuf* sink,
        uint32_t preset = 3 | LZMA_PRESET_EXTREME,
        lzma_check check = LZMA_CHECK_CRC32   /* LZMA_CHECK_NONE */,
        unsigned bufsize = 8192)
        : std::ostream(0),
          streambuf(sink, preset, check, bufsize)
        { init(&streambuf); }
      explicit LzmaStream(std::ostream& sink,
        uint32_t preset = 3 | LZMA_PRESET_EXTREME,
        lzma_check check = LZMA_CHECK_CRC32   /* LZMA_CHECK_NONE */,
        unsigned bufsize = 8192)
        : std::ostream(0),
          streambuf(sink.rdbuf(), preset, check, bufsize)
        { init(&streambuf); }

      void end();
      void setSink(std::streambuf* sink)   { streambuf.setSink(sink); }
      void setSink(std::ostream& sink)     { streambuf.setSink(sink.rdbuf()); }
  };
}

#endif // ZIM_LZMASTREAM_H
