/* deflatestream.cpp
 * Copyright (C) 2003-2005 Tommi Maekitalo
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


#include "zim/deflatestream.h"
#include "log.h"
#include <sstream>
#include <string.h>

log_define("zim.deflatestream")

namespace zim
{
  namespace
  {
    int checkError(int ret, z_stream& stream)
    {
      if (ret != Z_OK && ret != Z_STREAM_END)
      {
        log_error("DeflateError " << ret << ": \"" << (stream.msg ? stream.msg : "") << '"');
        std::ostringstream msg;
        msg << "deflate-error " << ret;
        if (stream.msg)
          msg << ": " << stream.msg;
        throw DeflateError(ret, msg.str());
      }
      return ret;
    }
  }

  DeflateStreamBuf::DeflateStreamBuf(std::streambuf* sink_, int level, unsigned bufsize_)
    : obuffer(bufsize_),
      sink(sink_)
  {
    memset(&stream, 0, sizeof(z_stream));
    stream.zalloc = Z_NULL;
    stream.zfree = Z_NULL;
    stream.opaque = 0;
    stream.total_out = 0;
    stream.total_in = 0;
    stream.next_in = Z_NULL;
    stream.next_out = Z_NULL;
    stream.avail_in = 0;
    stream.avail_out = 0;

    checkError(::deflateInit(&stream, level), stream);
    setp(&obuffer[0], &obuffer[0] + obuffer.size());
  }

  DeflateStreamBuf::~DeflateStreamBuf()
  {
    ::deflateEnd(&stream);
  }

  DeflateStreamBuf::int_type DeflateStreamBuf::overflow(int_type c)
  {
    // initialize input-stream
    stream.next_in = reinterpret_cast<Bytef*>(&obuffer[0]);
    stream.avail_in = pptr() - &obuffer[0];

    // initialize zbuffer for deflated data
    char zbuffer[8192];
    stream.next_out = reinterpret_cast<Bytef*>(zbuffer);
    stream.avail_out = sizeof(zbuffer);

    // deflate
    checkError(::deflate(&stream, Z_NO_FLUSH), stream);

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

  DeflateStreamBuf::int_type DeflateStreamBuf::underflow()
  {
    return traits_type::eof();
  }

  int DeflateStreamBuf::sync()
  {
    // initialize input-stream for
    stream.next_in = reinterpret_cast<Bytef*>(&obuffer[0]);
    stream.avail_in = pptr() - &obuffer[0];
    char zbuffer[8192];
    while (stream.avail_in > 0)
    {
      // initialize zbuffer
      stream.next_out = (Bytef*)zbuffer;
      stream.avail_out = sizeof(zbuffer);

      checkError(::deflate(&stream, Z_SYNC_FLUSH), stream);

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

  int DeflateStreamBuf::end()
  {
    char zbuffer[8192];
    // initialize input-stream for
    stream.next_in = reinterpret_cast<Bytef*>(&obuffer[0]);
    stream.avail_in = pptr() - &obuffer[0];
    while (true)
    {
      // initialize zbuffer
      stream.next_out = (Bytef*)zbuffer;
      stream.avail_out = sizeof(zbuffer);

      int ret = checkError(::deflate(&stream, Z_FINISH), stream);

      // copy zbuffer to sink
      std::streamsize count = sizeof(zbuffer) - stream.avail_out;
      if (count > 0)
      {
        std::streamsize n = sink->sputn(zbuffer, count);
        if (n < count)
          throw DeflateError(0, "failed to send compressed data to sink in deflatestream");
      }
      if (ret == Z_STREAM_END)
        break;
    };

    // reset outbuffer
    setp(&obuffer[0], &obuffer[0] + obuffer.size());
    return 0;
  }

  void DeflateStream::end()
  {
    if (streambuf.end() != 0)
      setstate(failbit);
  }
}
