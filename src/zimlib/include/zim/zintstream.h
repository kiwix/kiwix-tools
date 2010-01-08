/*
 * Copyright (C) 2007 Tommi Maekitalo
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

#ifndef ZIM_ZINTSTREAM_H
#define ZIM_ZINTSTREAM_H

#include <string>
#include <iostream>
#include <zim/zim.h>

/*
  ZInt implements a int compressor and decompressor. The algorithm compresses
  small values into fewer bytes.
  
  The idea is to add information about used bytes in the first byte. The number
  of additional bytes used is specified by the number of set bits counted from
  the most significant bit. So the numbers 0-127 are encoded as is, since they
  fit into the 7 low order bits and the high order bit specifies, that no
  additional bytes are used. The number starting from 128 up to 16383 need more
  than 7 bits, so we need to set the highest order bit to 1 and the next bit to
  0, leaving 6 bits of actual data, which is used as the low order bits of the
  number.

  Since the numbers 0-127 are already encoded in one byte, the 127 is
  substracted from the actual number, so a 2 byte zero is actually a 128.

  The same logic continues on the 3rd, 4th, ... byte. Up to 7 additional bytes
  are used, so the first byte must contain at least one 0.

  binary                          range
  ------------------------------- --------------------------------------------------
  0xxx xxxx                       0 - 127
  10xx xxxx xxxx xxxx             128 - (2^14+128-1 = 16511)
  110x xxxx xxxx xxxx xxxx xxxx   16512 - (2^21+16512-1 = 2113663)
  1110 xxxx xxxx xxxx xxxx xxxx xxxx xxxx
                                  2113664 - (2^28+2113664-1 = 270549119)
  ...

*/

namespace zim
{
  class ZIntStream
  {
      std::istream* _istream;
      std::ostream* _ostream;

    public:
      /// prepare ZIntStream for compression or decompression
      explicit ZIntStream(std::iostream& iostream)
        : _istream(&iostream),
          _ostream(&iostream)
          { }

      /// prepare ZIntStream for decompression
      explicit ZIntStream(std::istream& istream)
        : _istream(&istream),
          _ostream(0)
          { }

      /// prepare ZIntStream for compression
      explicit ZIntStream(std::ostream& ostream)
        : _istream(0),
          _ostream(&ostream)
          { }

      /// decompresses one value from input stream and returns it
      size_type get();

      ZIntStream& get(size_type &value)
        { value = get(); return *this; }

      /// compresses one value to output stream
      ZIntStream& put(size_type value);

      operator bool() const
        { return (_istream == 0 || *_istream)
              && (_ostream == 0 || *_ostream); }
  };

}
#endif  //  ZIM_ZINTSTREAM_H
