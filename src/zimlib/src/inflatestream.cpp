/* inflatestream.cpp
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


#include "zim/inflatestream.h"
#include "log.h"
#include <sstream>

log_define("zim.inflatestream")

namespace zim
{
  namespace
  {
    int checkError(int ret, z_stream& stream)
    {
      if (ret != Z_OK && ret != Z_STREAM_END)
      {
        log_error("InflateError " << ret << ": \"" << (stream.msg ? stream.msg : "") << '"');
        std::ostringstream msg;
        msg << "inflate-error " << ret;
        if (stream.msg)
          msg << ": " << stream.msg;
        throw InflateError(ret, msg.str());
      }
      return ret;
    }
  }

  InflateStreamBuf::InflateStreamBuf(std::streambuf* sinksource_, unsigned bufsize_)
    : iobuffer(new char_type[bufsize_]),
      bufsize(bufsize_),
      sinksource(sinksource_)
  {
    stream.zalloc = Z_NULL;
    stream.zfree = Z_NULL;
    stream.opaque = 0;
    stream.total_out = 0;
    stream.total_in = 0;
    stream.next_in = Z_NULL;
    stream.avail_in = 0;

    checkError(::inflateInit(&stream), stream);
  }

  InflateStreamBuf::~InflateStreamBuf()
  {
    ::inflateEnd(&stream);
    delete[] iobuffer;
  }

  InflateStreamBuf::int_type InflateStreamBuf::overflow(int_type c)
  {
    log_debug("InflateStreamBuf::overflow");

    if (pptr())
    {
      // initialize input-stream for
      stream.next_in = (Bytef*)obuffer();
      stream.avail_in = pptr() - pbase();

      int ret;
      do
      {
        // initialize ibuffer
        stream.next_out = (Bytef*)ibuffer();
        stream.avail_out = ibuffer_size();

        log_debug("pre:avail_out=" << stream.avail_out << " avail_in=" << stream.avail_in);
        ret = ::inflate(&stream, Z_SYNC_FLUSH);
        checkError(ret, stream);
        log_debug("post:avail_out=" << stream.avail_out << " avail_in=" << stream.avail_in << " ret=" << ret);

        // copy zbuffer to sinksource
        std::streamsize count = ibuffer_size() - stream.avail_out;
        std::streamsize n = sinksource->sputn(reinterpret_cast<char*>(ibuffer()), count);
        if (n < count)
          return traits_type::eof();
      } while (ret != Z_STREAM_END && stream.avail_in > 0);
    }

    // reset outbuffer
    setp(obuffer(), obuffer() + obuffer_size());
    if (c != traits_type::eof())
      sputc(traits_type::to_char_type(c));

    return 0;
  }

  InflateStreamBuf::int_type InflateStreamBuf::underflow()
  {
    // read from sinksource and decompress into obuffer

    stream.next_out = (Bytef*)obuffer();
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
          log_debug("in_avail=" << sinksource->in_avail());
          stream.avail_in = sinksource->sgetn(ibuffer(), std::min(sinksource->in_avail(), ibuffer_size()));
        }
        else
        {
          // no data available
          stream.avail_in = sinksource->sgetn(ibuffer(), ibuffer_size());
          log_debug(stream.avail_in << " bytes read from source");
          if (stream.avail_in == 0)
            return traits_type::eof();
        }

        stream.next_in = (Bytef*)ibuffer();
      }

      // we decompress it now into obuffer

      // at least one character received from source - pass to decompressor

      log_debug("pre:avail_out=" << stream.avail_out << " avail_in=" << stream.avail_in);
      int ret = ::inflate(&stream, Z_SYNC_FLUSH);
      log_debug("post:avail_out=" << stream.avail_out << " avail_in=" << stream.avail_in << " ret=" << ret);

      checkError(ret, stream);

      setg(obuffer(), obuffer(), obuffer() + obuffer_size() - stream.avail_out);

    } while (gptr() == egptr());

    return sgetc();
  }

  int InflateStreamBuf::sync()
  {
    if (pptr() && overflow(traits_type::eof()) == traits_type::eof())
      return -1;
    return 0;
  }
}
