/*
 * Copyright 2026 Kiwix <contact@kiwix.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
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

#ifndef _KIWIX_TOOLS_UTILS_H_
#define _KIWIX_TOOLS_UTILS_H_

#include <kiwix/tools.h>

#include <iostream>
#include <string>

/*
 * Heuristically detect whether an existing library file is in the legacy
 * library.xml format (i.e. it contains a "<library" element), which is no
 * longer supported. This does not validate the file.
 */
inline bool looksLikeOldLibraryXMLFile(const std::string& path)
{
  if (!kiwix::fileExists(path)) {
    return false;
  }
  const std::string content = kiwix::getFileContent(path);
  return content.find("<library") != std::string::npos;
}

/*
 * If the library file at `path` looks like a legacy library.xml file, print
 * an explanatory error message and return true. Otherwise return false.
 */
inline bool reportIfOldLibraryXMLFile(const std::string& path)
{
  if (!looksLikeOldLibraryXMLFile(path)) {
    return false;
  }
  std::cerr << "The library file '" << path << "' is in the old library.xml "
            << "format, which is no longer supported. Please recreate it in "
            << "OPDS format, e.g. with: kiwix-manage NEW_LIBRARY add ZIMPATH..."
            << std::endl;
  return true;
}

#endif // _KIWIX_TOOLS_UTILS_H_
