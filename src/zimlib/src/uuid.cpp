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

#include <zim/uuid.h>
#include <iostream>
#include <time.h>
#include <zim/zim.h> // necessary to have the new types
#include "log.h"
#ifdef WITH_CXXTOOLS
#include <cxxtools/md5stream.h>
#endif

#ifdef _WIN32

#  include <time.h>
#  include <windows.h>
int gettimeofday(struct timeval* tp, void* tzp) {
    DWORD t;
    t = timeGetTime();
    tp->tv_sec = t / 1000;
    tp->tv_usec = t % 1000;
    return 0;
}

#define getpid GetCurrentProcessId

#else
#  include <sys/time.h>
#endif

log_define("zim.uuid")

namespace zim
{
  namespace
  {
    char hex[] = "0123456789abcdef";
    inline char hi(char v)
    { return hex[(v >> 4) & 0xf]; }

    inline char lo(char v)
    { return hex[v & 0xf]; }
  }

  Uuid Uuid::generate()
  {
    Uuid ret;

    struct timeval tv;
    gettimeofday(&tv, 0);

#ifdef WITH_CXXTOOLS

    cxxtools::Md5stream m;

    clock_t c = clock();

    m << c << tv.tv_sec << tv.tv_usec;

    m.getDigest(reinterpret_cast<unsigned char*>(&ret.data[0]));

#else

    union {
      void* p;
      int32_t n;
    } u;
    u.p = &ret;

    *reinterpret_cast<int32_t*>(ret.data) = u.n;
    *reinterpret_cast<int32_t*>(ret.data + 4) = static_cast<int32_t>(tv.tv_sec);
    *reinterpret_cast<int32_t*>(ret.data + 8) = static_cast<int32_t>(tv.tv_usec);
    *reinterpret_cast<int32_t*>(ret.data + 12) = static_cast<int32_t>(getpid());

#endif

    log_debug("generated uuid: " << ret.data);

    return ret;
  }

  std::ostream& operator<< (std::ostream& out, const Uuid& uuid)
  {
    for (unsigned n = 0; n < 4; ++n)
      out << hi(uuid.data[n]) << lo(uuid.data[n]);
    out << '-';
    for (unsigned n = 4; n < 6; ++n)
      out << hi(uuid.data[n]) << lo(uuid.data[n]);
    out << '-';
    for (unsigned n = 6; n < 8; ++n)
      out << hi(uuid.data[n]) << lo(uuid.data[n]);
    out << '-';
    for (unsigned n = 6; n < 8; ++n)
      out << hi(uuid.data[n]) << lo(uuid.data[n]);
    out << '-';
    for (unsigned n = 8; n < 16; ++n)
      out << hi(uuid.data[n]) << lo(uuid.data[n]);
    return out;
  }

}
