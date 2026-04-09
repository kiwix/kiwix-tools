/*
 * Copyright 2023 Nikhil Tanwar <2002nikhiltanwar@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU  General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

#ifndef KIWIX_HTML_DUMPER_H
#define KIWIX_HTML_DUMPER_H

#include <string>

#include "library_dumper.h"

namespace kiwix
{

/**
 * A class to dump Library in HTML format.
 */
class HTMLDumper : public LibraryDumper
{
 public:
  HTMLDumper(const Library* library, const NameMapper* NameMapper);
  ~HTMLDumper();


  /**
   * Dump library in HTML
   *
   * @return HTML content
   */
  std::string dumpPlainHTML(kiwix::Filter filter) const;
};

}

#endif // KIWIX_HTML_DUMPER_H
