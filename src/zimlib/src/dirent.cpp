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

#include <zim/dirent.h>
#include <zim/zim.h>
#include <zim/endian.h>
#include "log.h"
#include <algorithm>

log_define("zim.dirent")

namespace zim
{
  //////////////////////////////////////////////////////////////////////
  // Dirent
  //

  std::ostream& operator<< (std::ostream& out, const Dirent& dirent)
  {
    union
    {
      char d[16];
      long a;
    } header;
    toLittleEndian(dirent.getMimeType(), header.d);
    header.d[2] = static_cast<char>(dirent.getParameter().size());
    header.d[3] = dirent.getNamespace();

    log_debug("title=" << dirent.getTitle() << " title.size()=" << dirent.getTitle().size());

    toLittleEndian(dirent.getVersion(), header.d + 4);

    if (dirent.isRedirect())
    {
      toLittleEndian(dirent.getRedirectIndex(), header.d + 8);
      out.write(header.d, 12);
    }
    else
    {
      toLittleEndian(dirent.getClusterNumber(), header.d + 8);
      toLittleEndian(dirent.getBlobNumber(), header.d + 12);
      out.write(header.d, 16);
    }

    out << dirent.getUrl() << '\0';

    std::string t = dirent.getTitle();
    if (t != dirent.getUrl())
      out << t;
    out << '\0' << dirent.getParameter();

    return out;
  }

  std::istream& operator>> (std::istream& in, Dirent& dirent)
  {
    union
    {
      long a;
      char d[16];
    } header;

    in.read(header.d, 12);
    if (in.fail())
    {
      log_warn("error reading dirent header");
      return in;
    }

    if (in.gcount() != 12)
    {
      log_warn("error reading dirent header (2)");
      in.setstate(std::ios::failbit);
      return in;
    }

    uint16_t mimeType = fromLittleEndian(reinterpret_cast<const uint16_t*>(header.d));
    bool redirect = (mimeType == std::numeric_limits<uint16_t>::max());
    char ns = header.d[3];
    size_type version = fromLittleEndian(reinterpret_cast<const size_type*>(header.d + 4));
    dirent.setVersion(version);

    if (redirect)
    {
      size_type redirectIndex = fromLittleEndian(reinterpret_cast<const size_type*>(header.d + 8));

      log_debug("redirectIndex=" << redirectIndex);

      dirent.setRedirect(redirectIndex);
    }
    else
    {
      log_debug("read article entry");

      in.read(header.d + 12, 4);
      if (in.fail())
      {
        log_warn("error reading article dirent header");
        return in;
      }

      if (in.gcount() != 4)
      {
        log_warn("error reading article dirent header (2)");
        in.setstate(std::ios::failbit);
        return in;
      }

      size_type clusterNumber = fromLittleEndian(reinterpret_cast<const size_type*>(header.d + 8));
      size_type blobNumber = fromLittleEndian(reinterpret_cast<const size_type*>(header.d + 12));

      log_debug("mimeType=" << mimeType << " clusterNumber=" << clusterNumber << " blobNumber=" << blobNumber);

      dirent.setArticle(mimeType, clusterNumber, blobNumber);
    }
    
    char ch;
    std::string url;
    std::string title;
    std::string parameter;

    log_debug("read url, title and parameters");

    while (in.get(ch) && ch != '\0')
      url += ch;

    while (in.get(ch) && ch != '\0')
      title += ch;

    uint8_t extraLen = static_cast<uint8_t>(header.d[2]);
    while (extraLen-- > 0 && in.get(ch))
      parameter += ch;

    dirent.setUrl(ns, url);
    dirent.setTitle(title);
    dirent.setParameter(parameter);

    return in;
  }

  std::string Dirent::getLongUrl() const
  {
    log_trace("Dirent::getLongUrl()");
    log_debug("namespace=" << getNamespace() << " title=" << getTitle());

    return std::string(1, getNamespace()) + '/' + getUrl();
  }

}
