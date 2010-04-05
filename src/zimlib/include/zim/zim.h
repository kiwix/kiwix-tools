/*
 * Copyright (C) 2006 Tommi Maekitalo
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

#ifndef ZIM_ZIM_H
#define ZIM_ZIM_H

#include <limits.h>

namespace zim
{
#if USHRT_MAX == 0xffff

    typedef unsigned short uint16_t;

#elif UINT_MAX == 0xffff

    typedef unsigned int uint16_t;

#elif ULONG_MAX == 0xffff

    typedef unsigned long uint16_t;

#else

}
#include <stdint.h>
namespace zim
{

#endif

#if USHRT_MAX == 0xffffffffUL

    typedef unsigned short size_type;

#elif UINT_MAX == 0xffffffffUL

    typedef unsigned int size_type;

#elif ULONG_MAX == 0xffffffffUL

    typedef unsigned long size_type;

#else

}
#include <stdint.h>
namespace zim
{
    typedef uint32_t size_type;

#endif

#if UINT_MAX == 18446744073709551615ULL

    typedef unsigned int offset_type;

#elif ULONG_MAX == 18446744073709551615ULL

    typedef unsigned long offset_type;

#elif ULLONG_MAX == 18446744073709551615ULL

    typedef unsigned long long offset_type;

#else

}
#include <stdint.h>
namespace zim
{
    typedef uint64_t offset_type;

#endif

  enum CompressionType
  {
    zimcompDefault,
    zimcompNone,
    zimcompZip,
    zimcompBzip2,
    zimcompLzma
  };

  static const char MimeHtmlTemplate[] = "text/x-zim-htmltemplate";
}

#endif // ZIM_ZIM_H

