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

#include <stdint.h>

#ifdef _WIN32
typedef unsigned __int64 uint64_t;
typedef signed __int8 int8_t;
typedef signed __int32 int32_t;
#endif

namespace zim
{
  typedef uint32_t size_type;
  typedef uint64_t offset_type;

  enum CompressionType
  {
    zimcompDefault,
    zimcompNone,
    zimcompZip,
    zimcompBzip2,
    zimcompLzma
  };

  enum MimeType
  {
    zimMimeNone = -1,
    zimMimeTextHtml,
    zimMimeTextPlain,
    zimMimeImageJpeg,
    zimMimeImagePng,
    zimMimeImageTiff,
    zimMimeTextCss,
    zimMimeImageGif,
    zimMimeIndex,
    zimMimeApplicationJavaScript,
    zimMimeImageIcon,
    zimMimeTextXml,
    zimMimeTextHtmlTemplate
  };

}

#endif // ZIM_ZIM_H

