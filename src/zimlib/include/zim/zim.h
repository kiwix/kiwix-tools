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
// define 8 bit integer types
//
  typedef unsigned char uint8_t;
  typedef char int8_t;

// define 16 bit integer types
//
#if USHRT_MAX == 0xffff

  typedef unsigned short uint16_t;
  typedef short int16_t;

#elif UINT_MAX == 0xffff

  typedef unsigned int uint16_t;
  typedef int int16_t;

#elif ULONG_MAX == 0xffff

  typedef unsigned long uint16_t;
  typedef long int16_t;

#else

}
#include <stdint.h>
namespace zim
{

#endif

// define 32 bit integer types
//
#if USHRT_MAX == 0xffffffffUL

  typedef unsigned short uint32_t;
  typedef short int32_t;

#elif UINT_MAX == 0xffffffffUL

  typedef unsigned int uint32_t;
  typedef int int32_t;

#elif ULONG_MAX == 0xffffffffUL

  typedef unsigned long uint32_t;
  typedef long int32_t;

#else

}
#include <stdint.h>
namespace zim
{

#endif

// define 64 bit integer types
//
#if UINT_MAX == 18446744073709551615ULL

  typedef unsigned int uint64_t;
  typedef int int64_t;

#elif ULONG_MAX == 18446744073709551615ULL

  typedef unsigned long uint64_t;
  typedef long int64_t;

#elif ULLONG_MAX == 18446744073709551615ULL

  typedef unsigned long long uint64_t;
  typedef long long int64_t;

#else

}
#include <stdint.h>
namespace zim
{
#endif

  typedef uint32_t size_type;
  
  #ifdef _WIN32
  typedef __int64 offset_type;
  #else
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

