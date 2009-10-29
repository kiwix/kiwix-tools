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

#include <zim/bzip2stream.h>
#include "log.h"
#include <sstream>
#include <string.h>

log_define("zim.bzip2.compress")

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
        throw Bzip2CompressError(ret, msg.str());
      }
      return ret;
    }
  }

  Bzip2StreamBuf::Bzip2StreamBuf(std::streambuf* sink_, int blockSize100k,
        int workFactor, unsigned bufsize_)
    : obuffer(bufsize_),
      sink(sink_)
  {
    memset(&stream, 0, sizeof(bz_stream));

    checkError(::BZ2_bzCompressInit(&stream, blockSize100k, 0, workFactor),
        stream);
    setp(&obuffer[0], &obuffer[0] + obuffer.size());
  }

  Bzip2StreamBuf::~Bzip2StreamBuf()
  {
    log_debug("bzCompressEnd");
    ::BZ2_bzCompressEnd(&stream);
  }

  Bzip2StreamBuf::int_type Bzip2StreamBuf::overflow(int_type c)
  {
    log_debug("Bzip2StreamBuf::overflow");

    // initialize input-stream
    stream.next_in = &obuffer[0];
    stream.avail_in = pptr() - &obuffer[0];

    // initialize zbuffer for deflated data
    char zbuffer[8192];
    stream.next_out = zbuffer;
    stream.avail_out = sizeof(zbuffer);

    // deflate
    log_debug("pre:avail_out=" << stream.avail_out << " avail_in=" << stream.avail_in << " BZ_RUN");
    int ret = checkError(::BZ2_bzCompress(&stream, BZ_RUN), stream);
    log_debug("post:avail_out=" << stream.avail_out << " avail_in=" << stream.avail_in << " ret=" << ret << " total_out_lo32=" << stream.total_out_lo32);

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

  Bzip2StreamBuf::int_type Bzip2StreamBuf::underflow()
  {
    return traits_type::eof();
  }

  int Bzip2StreamBuf::sync()
  {
    log_debug("Bzip2StreamBuf::sync");

    // initialize input-stream for
    stream.next_in = &obuffer[0];
    stream.avail_in = pptr() - pbase();
    char zbuffer[8192];
    int ret;
    do
    {
      // initialize zbuffer
      stream.next_out = zbuffer;
      stream.avail_out = sizeof(zbuffer);

      log_debug("pre:avail_out=" << stream.avail_out << " avail_in=" << stream.avail_in << " BZ_FLUSH");
      ret = checkError(::BZ2_bzCompress(&stream, BZ_FLUSH), stream);
      log_debug("post:avail_out=" << stream.avail_out << " avail_in=" << stream.avail_in << " ret=" << ret << " total_out_lo32=" << stream.total_out_lo32);

      // copy zbuffer to sink
      std::streamsize count = sizeof(zbuffer) - stream.avail_out;
      if (count > 0)
      {
        std::streamsize n = sink->sputn(zbuffer, count);
        if (n < count)
          return -1;
      }

    } while (ret != BZ_RUN_OK);

    // reset outbuffer
    setp(&obuffer[0], &obuffer[0] + obuffer.size());
    return 0;
  }

  int Bzip2StreamBuf::end()
  {
    log_debug("Bzip2StreamBuf::end");

    char zbuffer[8192];
    // initialize input-stream for
    stream.next_in = &obuffer[0];
    stream.avail_in = pptr() - pbase();
    int ret;
    do
    {
      // initialize zbuffer
      stream.next_out = zbuffer;
      stream.avail_out = sizeof(zbuffer);

      log_debug("pre:avail_out=" << stream.avail_out << " avail_in=" << stream.avail_in << " BZ_FINISH");
      ret = checkError(::BZ2_bzCompress(&stream, BZ_FINISH), stream);
      log_debug("post:avail_out=" << stream.avail_out << " avail_in=" << stream.avail_in << " ret=" << ret << " total_out_lo32=" << stream.total_out_lo32);

      // copy zbuffer to sink
      std::streamsize count = sizeof(zbuffer) - stream.avail_out;
      if (count > 0)
      {
        std::streamsize n = sink->sputn(zbuffer, count);
        if (n < count)
          return -1;
      }
    } while (ret != BZ_STREAM_END);

    // reset outbuffer
    setp(&obuffer[0], &obuffer[0] + obuffer.size());
    return 0;
  }

  void Bzip2Stream::end()
  {
    if (streambuf.end() != 0)
      setstate(failbit);
  }
}
