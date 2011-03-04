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

#ifndef ZIM_FSTREAM_H
#define ZIM_FSTREAM_H

#include <iostream>
#include <vector>
#include <zim/zim.h>
#include <zim/smartptr.h>
#include <zim/cache.h>
#include <zim/refcounted.h>

namespace zim
{
  class streambuf : public std::streambuf
  {
      struct FileInfo : public RefCounted
      {
        std::string fname;
        zim::offset_type fsize;

        FileInfo()  { }
        FileInfo(const std::string& fname_, int fd);
      };

      struct OpenfileInfo : public RefCounted
      {
        std::string fname;
        int fd;

        explicit OpenfileInfo(const std::string& fname);
        ~OpenfileInfo();
      };

      typedef SmartPtr<FileInfo> FileInfoPtr;
      typedef std::vector<FileInfoPtr> FilesType;

      typedef SmartPtr<OpenfileInfo> OpenfileInfoPtr;
      typedef Cache<std::string, OpenfileInfoPtr> OpenFilesCacheType;

      std::vector<char> buffer;

      FilesType files;
      OpenFilesCacheType openFilesCache;
      OpenfileInfoPtr currentFile;
      zim::offset_type currentPos;

      std::streambuf::int_type overflow(std::streambuf::int_type ch);
      std::streambuf::int_type underflow();
      int sync();

      void setCurrentFile(const std::string& fname, zim::offset_type off);

    public:
      streambuf(const std::string& fname, unsigned bufsize, unsigned openFilesCache);

      void seekg(zim::offset_type off);
      void setBufsize(unsigned s)
      { buffer.resize(s); }
      zim::offset_type fsize() const;
  };

  class ifstream : public std::istream
  {
      streambuf myStreambuf;

    public:
      explicit ifstream(const std::string& fname, unsigned bufsize = 8192, unsigned openFilesCache = 5)
        : std::istream(0),
          myStreambuf(fname, bufsize, openFilesCache)
      {
        init(&myStreambuf);
      }

      void seekg(zim::offset_type off) { myStreambuf.seekg(off); }
      void setBufsize(unsigned s) { myStreambuf.setBufsize(s); }
      zim::offset_type fsize() const  { return myStreambuf.fsize(); }
  };

}

#endif // ZIM_FSTREAM_H
