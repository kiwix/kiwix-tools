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

#include <zim/lzmastream.h>
#include <zim/zim.h>
#include "log.h"
#include <cstring>
#include <sstream>

log_define("zim.lzma.compress")

namespace zim
{
  namespace
  {
    lzma_ret checkError(lzma_ret ret)
    {
      if (ret != LZMA_OK && ret != LZMA_STREAM_END)
      {
        std::ostringstream msg;
        msg << "lzma-error " << ret;
        switch (ret)
        {
            case LZMA_OK: msg << ": LZMA_OK"; break;
            case LZMA_STREAM_END: msg << ": LZMA_STREAM_END"; break;
            case LZMA_NO_CHECK: msg << ": LZMA_NO_CHECK"; break;
            case LZMA_UNSUPPORTED_CHECK: msg << ": LZMA_UNSUPPORTED_CHECK"; break;
            case LZMA_GET_CHECK: msg << ": LZMA_GET_CHECK"; break;
            case LZMA_MEM_ERROR: msg << ": LZMA_MEM_ERROR"; break;
            case LZMA_MEMLIMIT_ERROR: msg << ": LZMA_MEMLIMIT_ERROR"; break;
            case LZMA_FORMAT_ERROR: msg << ": LZMA_FORMAT_ERROR"; break;
            case LZMA_OPTIONS_ERROR: msg << ": LZMA_OPTIONS_ERROR"; break;
            case LZMA_DATA_ERROR: msg << ": LZMA_DATA_ERROR"; break;
            case LZMA_BUF_ERROR: msg << ": LZMA_BUF_ERROR"; break;
            case LZMA_PROG_ERROR: msg << ": LZMA_PROG_ERROR"; break;
        }
        log_error(msg.str());
        throw LzmaError(ret, msg.str());
      }
      return ret;
    }
  }

  LzmaStreamBuf::LzmaStreamBuf(std::streambuf* sink_, uint32_t preset, lzma_check check, unsigned bufsize_)
    : obuffer(bufsize_),
      sink(sink_)
  {
    std::memset(reinterpret_cast<void*>(&stream), 0, sizeof(stream));

    checkError(
      ::lzma_easy_encoder(&stream, preset, check));

    setp(&obuffer[0], &obuffer[0] + obuffer.size());
  }

  LzmaStreamBuf::~LzmaStreamBuf()
  {
    ::lzma_end(&stream);
  }

  LzmaStreamBuf::int_type LzmaStreamBuf::overflow(int_type c)
  {
    // initialize input-stream
    stream.next_in = reinterpret_cast<const uint8_t*>(&obuffer[0]);
    stream.avail_in = pptr() - &obuffer[0];

    // initialize zbuffer for compressed data
    char zbuffer[8192];
    stream.next_out = reinterpret_cast<uint8_t*>(zbuffer);
    stream.avail_out = sizeof(zbuffer);

    // compress
    checkError(::lzma_code(&stream, LZMA_RUN));

    // copy zbuffer to sink / consume deflated data
    std::streamsize count = sizeof(zbuffer) - stream.avail_out;
    if (count > 0)
    {
      std::streamsize n = sink->sputn(zbuffer, count);
      if (n < count)
        return traits_type::eof();
    }

    // move remaining characters to start of obuffer
    if (stream.avail_in > 0)
      memmove(&obuffer[0], stream.next_in, stream.avail_in);

    // reset outbuffer
    setp(&obuffer[0] + stream.avail_in, &obuffer[0] + obuffer.size());
    if (c != traits_type::eof())
      sputc(traits_type::to_char_type(c));

    return 0;
  }

  LzmaStreamBuf::int_type LzmaStreamBuf::underflow()
  {
    return traits_type::eof();
  }

  int LzmaStreamBuf::sync()
  {
    // initialize input-stream for
    stream.next_in = reinterpret_cast<const uint8_t*>(&obuffer[0]);
    stream.avail_in = pptr() - &obuffer[0];
    char zbuffer[8192];
    while (stream.avail_in > 0)
    {
      // initialize zbuffer
      stream.next_out = (uint8_t*)zbuffer;
      stream.avail_out = sizeof(zbuffer);

      checkError(::lzma_code(&stream, LZMA_FINISH));

      // copy zbuffer to sink
      std::streamsize count = sizeof(zbuffer) - stream.avail_out;
      if (count > 0)
      {
        std::streamsize n = sink->sputn(zbuffer, count);
        if (n < count)
          return -1;
      }
    };

    // reset outbuffer
    setp(&obuffer[0], &obuffer[0] + obuffer.size());
    return 0;
  }

  int LzmaStreamBuf::end()
  {
    char zbuffer[8192];
    // initialize input-stream for
    stream.next_in = reinterpret_cast<const uint8_t*>(&obuffer[0]);
    stream.avail_in = pptr() - &obuffer[0];
    lzma_ret ret;
    do
    {
      // initialize zbuffer
      stream.next_out = (uint8_t*)zbuffer;
      stream.avail_out = sizeof(zbuffer);

      ret = checkError(::lzma_code(&stream, LZMA_FINISH));

      // copy zbuffer to sink
      std::streamsize count = sizeof(zbuffer) - stream.avail_out;
      if (count > 0)
      {
        std::streamsize n = sink->sputn(zbuffer, count);
        if (n < count)
          throw LzmaError(static_cast<lzma_ret>(0), "failed to send compressed data to sink in lzmastream");
      }
    } while (ret != LZMA_STREAM_END);

    // reset outbuffer
    setp(&obuffer[0], &obuffer[0] + obuffer.size());
    return 0;
  }

  void LzmaStream::end()
  {
    if (streambuf.end() != 0)
      setstate(failbit);
  }

}
