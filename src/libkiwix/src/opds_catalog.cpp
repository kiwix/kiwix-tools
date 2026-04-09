/*
 * Copyright 2021 Veloman Yunkan <veloman.yunkan@gmail.com>
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

#include "opds_catalog.h"
#include "tools/stringTools.h"

#include <sstream>

namespace kiwix
{

namespace
{
const char opdsSearchEndpoint[] = "/catalog/v2/entries";

enum Separator { AMP };

std::ostringstream& operator<<(std::ostringstream& oss, Separator sep)
{
  if ( oss.tellp() > 0 )
    oss << "&";
  return oss;
}

std::string buildSearchString(const Filter& f)
{
  std::ostringstream oss;
  if ( f.hasQuery() )
    oss << AMP << "q=" << urlEncode(f.getQuery());

  if ( f.hasCategory() )
    oss << AMP << "category=" << urlEncode(f.getCategory());

  if ( f.hasLang() )
    oss << AMP << "lang=" << urlEncode(f.getLang());

  if ( f.hasName() )
    oss << AMP << "name=" << urlEncode(f.getName());

  if ( !f.getAcceptTags().empty() )
    oss << AMP << "tag=" << urlEncode(join(f.getAcceptTags(), ";"));

  return oss.str();
}

} // unnamed namespace

std::string getSearchUrl(const Filter& f)
{
  const std::string searchString = buildSearchString(f);

  if ( searchString.empty() )
    return opdsSearchEndpoint;
  else
    return opdsSearchEndpoint + ("?" + searchString);
}

} // namespace kiwix
