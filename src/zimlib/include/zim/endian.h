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

#ifndef ENDIAN_H
#define ENDIAN_H

#include <algorithm>
#include <iostream>
#include <zim/zim.h>

namespace zim
{

/// Returns true, if machine is big-endian (high byte first).
/// e.g. PowerPC
inline bool isBigEndian()
{
  const int i = 1;
  return *reinterpret_cast<const int8_t*>(&i) == 0;
}

/// Returns true, if machine is little-endian (low byte first).
/// e.g. x86
inline bool isLittleEndian()
{
  const int i = 1;
  return *reinterpret_cast<const int8_t*>(&i) == 1;
}

////////////////////////////////////////////////////////////////////////
template <typename T>
void toLittleEndian(const T& d, char* dst, bool bigEndian = isBigEndian())
{
  if (bigEndian)
  {
    std::reverse_copy(
      reinterpret_cast<const char*>(&d),
      reinterpret_cast<const char*>(&d) + sizeof(T),
      dst);
  }
  else
  {
    std::copy(
      reinterpret_cast<const char*>(&d),
      reinterpret_cast<const char*>(&d) + sizeof(T),
      dst);
  }
}

template <typename T>
T fromLittleEndian(const T* ptr, bool bigEndian = isBigEndian())
{
  if (bigEndian)
  {
    T ret;
    std::reverse_copy(reinterpret_cast<const int8_t*>(ptr),
                      reinterpret_cast<const int8_t*>(ptr) + sizeof(T),
                      reinterpret_cast<int8_t*>(&ret));
    return ret;
  }
  else
  {
    return *ptr;
  }
}

////////////////////////////////////////////////////////////////////////
template <typename T>
void toBigEndian(const T& d, char* dst, bool bigEndian = isBigEndian())
{
  if (bigEndian)
  {
    std::copy(
      reinterpret_cast<const char*>(&d),
      reinterpret_cast<const char*>(&d) + sizeof(T),
      dst);
  }
  else
  {
    std::reverse_copy(
      reinterpret_cast<const char*>(&d),
      reinterpret_cast<const char*>(&d) + sizeof(T),
      dst);
  }
}

template <typename T>
T fromBigEndian(const T* ptr, bool bigEndian = isBigEndian())
{
  if (bigEndian)
  {
    return *ptr;
  }
  else
  {
    T ret;
    std::reverse_copy(reinterpret_cast<const int8_t*>(ptr),
                      reinterpret_cast<const int8_t*>(ptr) + sizeof(T),
                      reinterpret_cast<int8_t*>(&ret));
    return ret;
  }
}

}

#endif // ENDIAN_H

