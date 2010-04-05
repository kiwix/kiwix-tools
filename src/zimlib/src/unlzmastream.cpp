/*
 * Copyright (C) 2009 Tommi Maekitalo
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */


#include <zim/unlzmastream.h>
#include <zim/zim.h>
#include "log.h"
#include "config.h"
#include <sstream>
#include <cstring>
#include "envvalue.h"

log_define("zim.lzma.uncompress")

namespace zim
{
  namespace
  {
    lzma_ret checkError(lzma_ret ret)
    {
      if (ret != LZMA_OK && ret != LZMA_STREAM_END)
      {
        std::ostringstream msg;
        msg << "inflate-error " << ret;
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
        log_error(msg);
        throw UnlzmaError(ret, msg.str());
      }
      return ret;
    }

  }

  UnlzmaStreamBuf::UnlzmaStreamBuf(std::streambuf* sinksource_, unsigned bufsize_)
    : iobuffer(new char_type[bufsize_]),
      bufsize(bufsize_),
      sinksource(sinksource_)
  {
    std::memset(reinterpret_cast<void*>(&stream), 0, sizeof(stream));

    unsigned memsize = envMemSize("ZIM_LZMA_MEMORY_SIZE", LZMA_MEMORY_SIZE * 1024 * 1024);
    checkError(
      ::lzma_stream_decoder(&stream, memsize, 0));
  }

  UnlzmaStreamBuf::~UnlzmaStreamBuf()
  {
    ::lzma_end(&stream);
    delete[] iobuffer;
  }

  UnlzmaStreamBuf::int_type UnlzmaStreamBuf::overflow(int_type c)
  {
    if (pptr())
    {
      // initialize input-stream for
      stream.next_in = reinterpret_cast<const uint8_t*>(obuffer());
      stream.avail_in = pptr() - pbase();

      lzma_ret ret;
      do
      {
        // initialize ibuffer
        stream.next_out = reinterpret_cast<uint8_t*>(ibuffer());
        stream.avail_out = ibuffer_size();

        ret = ::lzma_code(&stream, LZMA_RUN);
        checkError(ret);

        // copy zbuffer to sinksource
        std::streamsize count = ibuffer_size() - stream.avail_out;
        std::streamsize n = sinksource->sputn(reinterpret_cast<char*>(ibuffer()), count);
        if (n < count)
          return traits_type::eof();
      } while (ret != LZMA_STREAM_END && stream.avail_in > 0);
    }

    // reset outbuffer
    setp(obuffer(), obuffer() + obuffer_size());
    if (c != traits_type::eof())
      sputc(traits_type::to_char_type(c));

    return 0;
  }

  UnlzmaStreamBuf::int_type UnlzmaStreamBuf::underflow()
  {
    // read from sinksource and decompress into obuffer

    stream.next_out = reinterpret_cast<uint8_t*>(obuffer());
    stream.avail_out = obuffer_size();

    do
    {
      // fill ibuffer first if needed
      if (stream.avail_in == 0)
      {
        if (sinksource->in_avail() > 0)
        {
          // there is data already available
          // read compressed data from source into ibuffer
          stream.avail_in = sinksource->sgetn(ibuffer(), std::min(sinksource->in_avail(), ibuffer_size()));
        }
        else
        {
          // no data available
          stream.avail_in = sinksource->sgetn(ibuffer(), ibuffer_size());
          if (stream.avail_in == 0)
            return traits_type::eof();
        }

        stream.next_in = (const uint8_t*)ibuffer();
      }

      // we decompress it now into obuffer

      // at least one character received from source - pass to decompressor

      checkError(::lzma_code(&stream, LZMA_RUN));

      setg(obuffer(), obuffer(), obuffer() + obuffer_size() - stream.avail_out);

    } while (gptr() == egptr());

    return sgetc();
  }

  int UnlzmaStreamBuf::sync()
  {
    if (pptr() && overflow(traits_type::eof()) == traits_type::eof())
      return -1;
    return 0;
  }
}
