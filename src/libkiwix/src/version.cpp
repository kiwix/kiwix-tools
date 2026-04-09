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

#include <iostream>
#include <sstream>

#include <version.h>
#include <zim/zim.h>
#include <kiwix_config.h>
#include <unicode/uversion.h>
#include <pugixml.hpp>
#include <curl/curl.h>
#include <microhttpd.h>
#include <xapian.h>
#include <mustache.hpp>
#include <zlib.h>

namespace kiwix
{
  LibVersions getVersions() {
    LibVersions versions = {
      { "libkiwix",      LIBKIWIX_VERSION    },
      { "libzim",        LIBZIM_VERSION      },
      { "libxapian",     XAPIAN_VERSION      },
      { "libcurl",       LIBCURL_VERSION     },
      { "libmicrohttpd", MHD_get_version()   },
      { "libz",          ZLIB_VERSION        }
    };

    // U_ICU_VERSION does not include the patch level if 0
    std::ostringstream libicu_version;
    libicu_version << U_ICU_VERSION_MAJOR_NUM << "." << U_ICU_VERSION_MINOR_NUM << "." << U_ICU_VERSION_PATCHLEVEL_NUM;
    versions.push_back({ "libicu", libicu_version.str() });

    // No human readable version string for pugixml
    const unsigned pugixml_major = (PUGIXML_VERSION - PUGIXML_VERSION % 1000) / 1000;
    const unsigned pugixml_minor = (PUGIXML_VERSION - pugixml_major * 1000 - PUGIXML_VERSION % 10) / 10;
    const unsigned pugixml_patch = PUGIXML_VERSION - pugixml_major * 1000 - pugixml_minor * 10;
    std::ostringstream libpugixml_version;
    libpugixml_version << pugixml_major << "." << pugixml_minor << "." << pugixml_patch;
    versions.push_back({ "libpugixml", libpugixml_version.str() });

    // Needs version 5.0 of Mustache
#if defined(KAINJOW_MUSTACHE_VERSION_MAJOR)
    std::ostringstream libmustache_version;
    libmustache_version << KAINJOW_MUSTACHE_VERSION_MAJOR << "." <<
      KAINJOW_MUSTACHE_VERSION_MINOR << "." << KAINJOW_MUSTACHE_VERSION_PATCH;
    versions.push_back({ "libmustache", libmustache_version.str() });
#endif

    return versions;
  }

  void printVersions(std::ostream& out) {
    LibVersions versions = getVersions();
    for (const auto& iter : versions) {
      out << (iter != versions.front() ? "+ " : "")
                << iter.first << " " << iter.second << std::endl;
    }
  }
} //namespace kiwix
