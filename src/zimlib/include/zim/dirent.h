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

#ifndef ZIM_DIRENT_H
#define ZIM_DIRENT_H

#include <string>
#include <zim/zim.h>
#include <zim/qunicode.h>

namespace zim
{
  class Dirent
  {
      bool redirect;
      MimeType mimeType;

      size_type clusterNumber;  // only used when redirect is false
      size_type blobNumber;    // only used when redirect is false

      size_type redirectIndex;  // only used when redirect is true

      char ns;
      QUnicodeString title;
      std::string parameter;

    public:
      Dirent()  {}

      bool isRedirect() const                 { return redirect; }
      MimeType getMimeType() const            { return mimeType; }

      size_type getClusterNumber() const      { return isRedirect() ? 0 : clusterNumber; }
      size_type getBlobNumber() const         { return isRedirect() ? 0 : blobNumber; }
      void setCluster(size_type clusterNumber_, size_type blobNumber_)
        { clusterNumber = clusterNumber_; blobNumber = blobNumber_; }

      size_type getRedirectIndex() const      { return isRedirect() ? redirectIndex : 0; }

      char getNamespace() const               { return ns; }
      const QUnicodeString& getTitle() const  { return title; }
      const std::string& getParameter() const { return parameter; }

      uint16_t getExtraLen() const
      {
        uint16_t s = title.getValue().size();
        if (!parameter.empty())
          s += (parameter.size() + 1);
        return s;
      }

      unsigned getDirentSize() const
      {
        return (isRedirect() ? 10 : 14) + getExtraLen();
      }

      void setTitle(char ns_, const QUnicodeString& title_)
      {
        ns = ns_;
        title = title_;
      }

      void setParameter(const std::string& parameter_)
      {
        parameter = parameter_;
      }

      void setRedirect(size_type idx)
      {
        redirect = true;
        redirectIndex = idx;
        mimeType = zimMimeNone;
        clusterNumber = 0;
        blobNumber = 0;
      }

      void setArticle(MimeType mimeType_, size_type clusterNumber_, size_type blobNumber_)
      {
        redirect = false;
        mimeType = mimeType_;
        clusterNumber = clusterNumber_;
        blobNumber = blobNumber_;
      }

      QUnicodeString getUrl() const;
  };

  std::ostream& operator<< (std::ostream& out, const Dirent& fh);
  std::istream& operator>> (std::istream& in, Dirent& fh);

}

#endif // ZIM_DIRENT_H

