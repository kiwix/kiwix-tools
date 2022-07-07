/*
 * Copyright 2009-2016 Emmanuel Engelhart <kelson@kiwix.org>
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

#ifndef _KIWIX_TOOLS_VERSION_H_
#define _KIWIX_TOOLS_VERSION_H_

#ifndef KIWIX_TOOLS_VERSION
  #define KIWIX_TOOLS_VERSION "undefined"
#endif

#include <kiwix/version.h>
#include <zim/version.h>
#include <iostream>

void version()
{
  std::cout << "kiwix-tools " << KIWIX_TOOLS_VERSION << std::endl << std::endl;
  kiwix::printVersions();
  std::cout << std::endl;
  zim::printVersions();
}

#endif //_KIWIX_TOOLs_VERSION_H_
