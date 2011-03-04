/*
 * Copyright (C) 2003 Tommi Maekitalo
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * As a special exception, you may use this file as part of a free
 * software library without restriction. Specifically, if other files
 * instantiate templates or use macros or inline functions from this
 * file, or you compile this file and link it with other files to
 * produce an executable, this file does not by itself cause the
 * resulting executable to be covered by the GNU General Public
 * License. This exception does not however invalidate any other
 * reasons why the executable file might be covered by the GNU Library
 * General Public License.
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
 *
 * copied from cxxtools
 */

#ifndef ZIM_MD5STREAM_H
#define ZIM_MD5STREAM_H

#include "md5.h"
#include <iostream>
#include <iterator>
#include <algorithm>

namespace zim
{

class Md5streambuf : public std::streambuf
{
  public:
    Md5streambuf();

    void getDigest(unsigned char digest[16]);

  private:
    static const unsigned int bufsize = 64;
    char buffer[bufsize];
    zim_MD5_CTX context;
    unsigned char digest[16];

    std::streambuf::int_type overflow(std::streambuf::int_type ch);
    std::streambuf::int_type underflow();
    int sync();
};

/**
 This is a easy and safe interface to MD5-calculation.

 To get a MD5-sum of data, instantiate a md5stream, copy your data
 into it and read the digest.

 After calling getDigest or getHexDigest, the class can be reused
 for another md5-calculation. The algorithm is automatically
 reinitialized when the first character is received.

 example:
 \code
  int main(int argc, char* argv[])
  {
    Md5stream s;
    for (int i = 1; i < argc; ++i)
    {
      std::ifstream in(argv[i]);
      if (in)
      {
        s << in.rdbuf();
        std::cout << s.getHexDigest() << "  " << argv[i] << std::endl;
      }
    }
  }
 \endcode
 */
class Md5stream : public std::ostream
{
  public:
    typedef std::ostreambuf_iterator<char> iterator;

  private:
    Md5streambuf streambuf;
    char hexdigest[33];

  public:
    /// initializes md5-calculation
    Md5stream()
      : std::ostream(0)
    {
      init(&streambuf);
    }

    /// ends md5-calculation and returns 16 bytes digest
    void getDigest(unsigned char digest[16])
    { streambuf.getDigest(digest); }
    /// ends md5-calculation and digest as 32 bytes hex
    const char* getHexDigest();

    /// returns output-iterator to Md5stream
    iterator begin()
      { return iterator(&streambuf); }
};

template <typename iterator_type>
std::string md5(iterator_type from, iterator_type to)
{
  Md5stream s;
  std::copy(from, to, std::ostream_iterator<char>(s));
  return s.getHexDigest();
}

template <typename data_type>
std::string md5(const data_type& data)
{
  Md5stream s;
  s << data;
  return s.getHexDigest();
}

}

#endif  // ZIM_MD5STREAM_H
