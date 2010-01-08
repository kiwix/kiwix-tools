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

#include <zim/bunzip2stream.h>
#include "log.h"
#include <sstream>
#include <cstring>
#include <algorithm>

log_define("zim.bzip2.uncompress")

namespace zim
{
  namespace
  {
    int checkError(int ret, bz_stream& stream)
    {
      if (ret != BZ_OK && ret != BZ_RUN_OK && ret != BZ_FLUSH_OK && ret != BZ_FINISH_OK
        && ret != BZ_STREAM_END)
      {
        std::ostringstream msg;
        msg << "bzip2-error " << ret << ": " << Bzip2Error::getErrorString(ret);
        log_error(msg.str());
        throw Bzip2UncompressError(ret, msg.str());
      }
      return ret;
    }

    // because in windows std::min is not defined, we define our own min-function
    template <typename T>
    bool mymin(const T& a, const T& b)
    { return a <= b ? a : b; }
  }

  Bunzip2StreamBuf::Bunzip2StreamBuf(std::streambuf* sinksource_, bool small, unsigned bufsize_)
    : iobuffer(new char_type[bufsize_]),
      bufsize(bufsize_),
      sinksource(sinksource_)
  {
    std::memset(&stream, 0, sizeof(bz_stream));

    checkError(::BZ2_bzDecompressInit(&stream, 0, static_cast<int>(small)), stream);
  }

  Bunzip2StreamBuf::~Bunzip2StreamBuf()
  {
    ::BZ2_bzDecompressEnd(&stream);
    delete[] iobuffer;
  }

  Bunzip2StreamBuf::int_type Bunzip2StreamBuf::overflow(int_type c)
  {
    if (pptr())
    {
      // initialize input-stream for
      stream.next_in = obuffer();
      stream.avail_in = pptr() - pbase();

      int ret;
      do
      {
        // initialize ibuffer
        stream.next_out = ibuffer();
        stream.avail_out = ibuffer_size();

        ret = ::BZ2_bzDecompress(&stream);
        checkError(ret, stream);

        // copy ibuffer to sinksource
        std::streamsize count = ibuffer_size() - stream.avail_out;
        std::streamsize n = sinksource->sputn(reinterpret_cast<char*>(ibuffer()), count);
        if (n < count)
          return traits_type::eof();
      } while (ret != BZ_STREAM_END && stream.avail_in > 0);
    }

    // reset outbuffer
    setp(obuffer(), obuffer() + obuffer_size());
    if (c != traits_type::eof())
      sputc(traits_type::to_char_type(c));

    return 0;
  }

  Bunzip2StreamBuf::int_type Bunzip2StreamBuf::underflow()
  {
    // read from sinksource and decompress into obuffer

    stream.next_out = obuffer();
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
          stream.avail_in = sinksource->sgetn(ibuffer(), mymin(sinksource->in_avail(), ibuffer_size()));
        }
        else
        {
          // no data available
          stream.avail_in = sinksource->sgetn(ibuffer(), ibuffer_size());
          if (stream.avail_in == 0)
            return traits_type::eof();
        }

        stream.next_in = ibuffer();
      }

      // we decompress it now into obuffer

      // at least one character received from source - pass to decompressor

      int ret = ::BZ2_bzDecompress(&stream);

      checkError(ret, stream);

      setg(obuffer(), obuffer(), obuffer() + obuffer_size() - stream.avail_out);

    } while (gptr() == egptr());

    return sgetc();
  }

  int Bunzip2StreamBuf::sync()
  {
    if (pptr() && overflow(traits_type::eof()) == traits_type::eof())
      return -1;
    return 0;
  }
}
