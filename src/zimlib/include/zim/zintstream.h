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

namespace zim
{
  class IZIntStream
  {
      std::istream& stream;

    public:
      explicit IZIntStream(std::istream& stream_)
        : stream(stream_)
        { }

      IZIntStream& get(size_type &value);
      operator void*() const  { return stream; }
  };

  class OZIntStream
  {
      std::ostream& stream;

    public:
      explicit OZIntStream(std::ostream& stream_)
        : stream(stream_)
        { }

      OZIntStream& put(size_type value);
      operator void*() const  { return stream; }
  };

}
#endif  //  ZIM_ZINTSTREAM_H
