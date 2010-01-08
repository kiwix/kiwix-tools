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


#ifndef ZIM_UNLZMASTREAM_H
#define ZIM_UNLZMASTREAM_H

#include <iostream>
#include <stdexcept>
#include <lzma.h>

namespace zim
{
  class UnlzmaError : public std::runtime_error
  {
      lzma_ret ret;

    public:
      UnlzmaError(lzma_ret ret_, const std::string& msg)
        : std::runtime_error(msg),
          ret(ret_)
          { }

      lzma_ret getRetcode() const  { return ret; }
  };

  class UnlzmaStreamBuf : public std::streambuf
  {
      lzma_stream stream;
      char_type* iobuffer;
      unsigned bufsize;
      std::streambuf* sinksource;

      char_type* ibuffer()            { return iobuffer; }
      std::streamsize ibuffer_size()  { return bufsize >> 1; }
      char_type* obuffer()            { return iobuffer + ibuffer_size(); }
      std::streamsize obuffer_size()  { return bufsize >> 1; }

    public:
      explicit UnlzmaStreamBuf(std::streambuf* sinksource_, unsigned bufsize = 8192);
      ~UnlzmaStreamBuf();

      /// see std::streambuf
      int_type overflow(int_type c);
      /// see std::streambuf
      int_type underflow();
      /// see std::streambuf
      int sync();

      void setSinksource(std::streambuf* sinksource_)   { sinksource = sinksource_; }
  };

  class UnlzmaStream : public std::iostream
  {
      UnlzmaStreamBuf streambuf;

    public:
      explicit UnlzmaStream(std::streambuf* sinksource, unsigned bufsize = 8192)
        : std::iostream(0),
          streambuf(sinksource, bufsize)
        { init(&streambuf); }
      explicit UnlzmaStream(std::ios& sinksource, unsigned bufsize = 8192)
        : std::iostream(0),
          streambuf(sinksource.rdbuf(), bufsize)
        { init(&streambuf); }

      void setSinksource(std::streambuf* sinksource)   { streambuf.setSinksource(sinksource); }
      void setSinksource(std::ios& sinksource)         { streambuf.setSinksource(sinksource.rdbuf()); }
      void setSink(std::ostream& sink)                 { streambuf.setSinksource(sink.rdbuf()); }
      void setSource(std::istream& source)             { streambuf.setSinksource(source.rdbuf()); }
  };
}

#endif // ZIM_UNLZMASTREAM_H

