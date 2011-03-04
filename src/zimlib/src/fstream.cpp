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
#include <sys/types.h>
#include <sys/stat.h>

#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif

#ifndef O_LARGEFILE 
#define O_LARGEFILE 0
#endif

#ifndef O_BINARY
#define O_BINARY 0
#endif

log_define("zim.fstream")

namespace zim
{
  class FileNotFound : public std::runtime_error
  {
    public:
      FileNotFound()
        : std::runtime_error("file not found")
        { }
  };

////////////////////////////////////////////////////////////
// OpenfileInfo
//
streambuf::OpenfileInfo::OpenfileInfo(const std::string& fname_)
  : fname(fname_),
#ifdef HAVE_OPEN64
    fd(::open64(fname.c_str(), O_RDONLY | O_LARGEFILE | O_BINARY))
#else
    fd(::open(fname.c_str(), O_RDONLY | O_LARGEFILE | O_BINARY))
#endif
{
  if (fd < 0)
    throw FileNotFound();
}

streambuf::OpenfileInfo::~OpenfileInfo()
{
  ::close(fd);
}

////////////////////////////////////////////////////////////
// FileInfo
//
streambuf::FileInfo::FileInfo(const std::string& fname_, int fd)
  : fname(fname_)
{
#if defined(_WIN32)
  __int64 ret = ::_lseeki64(fd, 0, SEEK_END);
#elif defined(HAVE_LSEEK64)
  off64_t ret = ::lseek64(fd, 0, SEEK_END);
#else
  off_t ret = ::lseek(fd, 0, SEEK_END);
#endif
  if (ret < 0)
  {
    std::ostringstream msg;
    msg << "error " << errno << " seeking to end in file " << fname << ": " << strerror(errno);
    throw std::runtime_error(msg.str());
  }

  fsize = static_cast<zim::offset_type>(ret);
}

std::streambuf::int_type streambuf::overflow(std::streambuf::int_type ch)
{
  return traits_type::eof();
}

std::streambuf::int_type streambuf::underflow()
{
  log_debug("underflow; bufsize=" << buffer.size());

  int n;
  do
  {
    n = ::read(currentFile->fd, &buffer[0], buffer.size());
    if (n < 0)
    {
      std::ostringstream msg;
      msg << "error " << errno << " reading from file: " << strerror(errno);
      throw std::runtime_error(msg.str());
    }
    else if (n == 0)
    {
      FilesType::iterator it;
      for (it = files.begin(); it != files.end(); ++it)
      {
        if ((*it)->fname == currentFile->fname)
        {
          ++it;
          break;
        }
      }

      if (it == files.end())
        return traits_type::eof();

      setCurrentFile((*it)->fname, 0);
    }
  } while (n == 0);

  char* p = &buffer[0];
  setg(p, p, p + n);
  return traits_type::to_int_type(*gptr());
}

int streambuf::sync()
{
  return traits_type::eof();
}

namespace
{
  void parseFilelist(const std::string& list, std::vector<std::string>& out)
  {
    enum {
      state_0,
      state_t,
      state_e
    } state = state_0;

    for (std::string::const_iterator it = list.begin(); it != list.end(); ++it)
    {
      switch (state)
      {
        case state_0:
          out.push_back(std::string(1, *it));
          state = state_t;
          break;

        case state_t:
          if (*it == ':')
            out.push_back(std::string(1, *it));
          else if (*it == '\\')
            state = state_e;
          else
            out.back() += *it;
          break;

        case state_e:
          out.back() += *it;
          state = state_t;
          break;
      }
    }
  }
}

streambuf::streambuf(const std::string& fname, unsigned bufsize, unsigned noOpenFiles)
  : buffer(bufsize),
    openFilesCache(noOpenFiles)
{
  log_debug("streambuf for " << fname << " with " << bufsize << " bytes");

  try
  {
    currentFile = new OpenfileInfo(fname);
    files.push_back(new FileInfo(fname, currentFile->fd));
    openFilesCache.put(fname, currentFile);
  }
  catch (const FileNotFound&)
  {
    int errnoSave = errno;
    try
    {
      for (char ch0 = 'a'; ch0 <= 'z'; ++ch0)
      {
        std::string fname0 = fname + ch0;
        for (char ch1 = 'a'; ch1 <= 'z'; ++ch1)
        {
          std::string fname1 = fname0 + ch1;

          currentFile = new OpenfileInfo(fname1);
          files.push_back(new FileInfo(fname1, currentFile->fd));

          openFilesCache.put(fname1, currentFile);
        }
      }
    }
    catch (const FileNotFound&)
    {
      if (files.empty())
      {
        std::ostringstream msg;
        msg << "error " << errnoSave << " opening file \"" << fname << "\": " << strerror(errnoSave);
        throw std::runtime_error(msg.str());
      }
    }
  }

  setCurrentFile((*files.begin())->fname, 0);
}

void streambuf::setCurrentFile(const std::string& fname, zim::offset_type off)
{
  std::pair<bool, OpenfileInfoPtr> f = openFilesCache.getx(fname);
  if (f.first)
  {
    currentFile = f.second;
  }
  else
  {
    // file not found in cache
    currentFile = new OpenfileInfo(fname);
    openFilesCache.put(fname, currentFile);
  }

  if (f.first || off != 0) // found in cache or seek requested
  {
#if defined(_WIN32)
    offset_type ret = ::_lseeki64(currentFile->fd, off, SEEK_SET);
#elif defined(HAVE_LSEEK64)
    off64_t ret = ::lseek64(currentFile->fd, off, SEEK_SET);
#else
    off_t ret = ::lseek(currentFile->fd, off, SEEK_SET);
#endif
    if (ret < 0)
    {
      std::ostringstream msg;
      msg << "error " << errno << " seeking to "<< off << " in file " << fname << ": " << strerror(errno);
      throw std::runtime_error(msg.str());
    }
  }
}

void streambuf::seekg(zim::offset_type off)
{
  setg(0, 0, 0);
  currentPos = off;

  zim::offset_type o = off;
  FilesType::iterator it;
  for (it = files.begin(); it != files.end() && (*it)->fsize < o; ++it)
    o -= (*it)->fsize;

  if (it == files.end())
  {
    std::ostringstream msg;
    msg << "error seeking to "<< off;
    throw std::runtime_error(msg.str());
  }

  setCurrentFile((*it)->fname, o);
}

zim::offset_type streambuf::fsize() const
{
  zim::offset_type o = 0;
  for (FilesType::const_iterator it = files.begin(); it != files.end(); ++it)
    o += (*it)->fsize;
  return o;
}

}
