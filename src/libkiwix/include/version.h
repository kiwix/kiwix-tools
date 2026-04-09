/*
 * Copyright 2021 Emmanuel Engelhart <kelson@kiwix.org>
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

#ifndef KIWIX_VERSION_H
#define KIWIX_VERSION_H

#include <string>
#include <vector>
#include <iostream>

namespace kiwix
{
  typedef std::vector<std::pair<std::string, std::string>> LibVersions;
  LibVersions getVersions();
  void printVersions(std::ostream& out = std::cout);
}

#endif // KIWIX_VERSION_H
