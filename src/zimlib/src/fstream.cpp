/*
 * Copyright (C) 2010 Tommi Maekitalo
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

#include <zim/fstream.h>
#include "log.h"
#include "config.h"
#include <sstream>
#include <stdexcept>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

log_define("zim.fstream")

namespace zim
{
std::streambuf::int_type streambuf::overflow(std::streambuf::int_type ch)
{
  return traits_type::eof();
}

std::streambuf::int_type streambuf::underflow()
{
  log_debug("underflow; bufsize=" << buffer.size());

  int n = ::read(fd, &buffer[0], buffer.size());
  if (n < 0)
  {
    std::ostringstream msg;
    msg << "error " << errno << " reading from file: " << strerror(errno);
    throw std::runtime_error(msg.str());
  }
  else if (n == 0)
    return traits_type::eof();

  char* p = &buffer[0];
  setg(p, p, p + n);
  return traits_type::to_int_type(*gptr());
}

int streambuf::sync()
{
  return traits_type::eof();
}

streambuf::streambuf(const char* fname, unsigned bufsize)
  : buffer(bufsize),
#ifdef HAVE_OPEN64
    fd(::open64(fname, 0))
#else
    fd(::open(fname, 0))
#endif
{
  log_debug("streambuf for " << fname << " with " << bufsize << " bytes");

  if (fd < 0)
  {
    std::ostringstream msg;
    msg << "error " << errno << " opening file \"" << fname << "\": " << strerror(errno);
    throw std::runtime_error(msg.str());
  }
}

streambuf::~streambuf()
{
  ::close(fd);
}

void streambuf::seekg(offset_type off)
{
  setg(0, 0, 0);
#ifdef HAVE_LSEEK64
  off64_t ret = ::lseek64(fd, off, SEEK_SET);
#else
  off_t ret = ::lseek(fd, off, SEEK_SET);
#endif
  if (ret < 0)
  {
    std::ostringstream msg;
    msg << "error " << errno << " seeking to "<< off << " in file: " << strerror(errno);
    throw std::runtime_error(msg.str());
  }
}

}
