/*
 * Copyright (C) 2009 Tommi Maekitalo
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

#include <sstream>
#include <stdlib.h>

namespace zim
{
  unsigned envValue(const char* env, unsigned def)
  {
    const char* v = ::getenv(env);
    if (v)
    {
      std::istringstream s(v);
      s >> def;
    }
    return def;
  }

  unsigned envMemSize(const char* env, unsigned def)
  {
    const char* v = ::getenv(env);
    if (v)
    {
      char unit = '\0';
      std::istringstream s(v);
      s >> def >> unit;

      switch (unit)
      {
        case 'k':
        case 'K': def *= 1024; break;
        case 'm':
        case 'M': def *= 1024 * 1024; break;
        case 'g':
        case 'G': def *= 1024 * 1024 * 1024; break;
      }
    }
    return def;
  }
}

