/*
 * Copyright (C) 2008 Tommi Maekitalo
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

#ifndef ZIM_FILEHEADER_H
#define ZIM_FILEHEADER_H

#include <zim/zim.h>
#include <zim/endian.h>
#include <zim/uuid.h>
#include <iosfwd>
#include <limits>

namespace zim
{
  class Fileheader
  {
    public:
      static const size_type zimMagic;
      static const size_type zimVersion;
      static const size_type size;

    private:
      Uuid uuid;
      size_type articleCount;
      offset_type titleIdxPos;
      offset_type urlPtrPos;
      offset_type mimeListPos;
      size_type blobCount;
      offset_type blobPtrPos;
      size_type mainPage;
      size_type layoutPage;
      offset_type checksumPos;

    public:
      Fileheader()
        : articleCount(0),
          titleIdxPos(0),
          urlPtrPos(0),
          blobCount(0),
          blobPtrPos(0),
          mainPage(std::numeric_limits<size_type>::max()),
          layoutPage(std::numeric_limits<size_type>::max()),
          checksumPos(std::numeric_limits<offset_type>::max())
      {}

      const Uuid& getUuid() const                  { return uuid; }
      void setUuid(const Uuid& uuid_)              { uuid = uuid_; }

      size_type getArticleCount() const            { return articleCount; }
      void      setArticleCount(size_type s)       { articleCount = s; }

      offset_type getTitleIdxPos() const           { return titleIdxPos; }
      void        setTitleIdxPos(offset_type p)    { titleIdxPos = p; }

      offset_type getUrlPtrPos() const             { return urlPtrPos; }
      void        setUrlPtrPos(offset_type p)      { urlPtrPos = p; }

      offset_type getMimeListPos() const           { return mimeListPos; }
      void        setMimeListPos(offset_type p)    { mimeListPos = p; }

      size_type   getClusterCount() const          { return blobCount; }
      void        setClusterCount(size_type s)     { blobCount = s; }

      offset_type getClusterPtrPos() const         { return blobPtrPos; }
      void        setClusterPtrPos(offset_type p)  { blobPtrPos = p; }

      bool        hasMainPage() const              { return mainPage != std::numeric_limits<size_type>::max(); }
      size_type   getMainPage() const              { return mainPage; }
      void        setMainPage(size_type s)         { mainPage = s; }

      bool        hasLayoutPage() const            { return layoutPage != std::numeric_limits<size_type>::max(); }
      size_type   getLayoutPage() const            { return layoutPage; }
      void        setLayoutPage(size_type s)       { layoutPage = s; }

      bool        hasChecksum() const              { return getMimeListPos() >= 80; }
      offset_type getChecksumPos() const           { return hasChecksum() ? checksumPos : 0; }
      void        setChecksumPos(offset_type p)    { checksumPos = p; }
  };

  std::ostream& operator<< (std::ostream& out, const Fileheader& fh);
  std::istream& operator>> (std::istream& in, Fileheader& fh);

}

#endif // ZIM_FILEHEADER_H
