/*
 * Copyright 2020 Veloman Yunkan <veloman.yunkan@gmail.com>
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


#include "etag.h"

#include "tools/stringTools.h"

#include <algorithm>
#include <sstream>

namespace kiwix {

namespace {

// Characters in the options part of the ETag could in principle be picked up
// from the latin alphabet in natural order (the character corresponding to
// ETag::Option opt would be 'a'+opt; that would somewhat simplify the code in
// this file). However it is better to have some mnemonics in the option names,
// hence below variable: all_options[opt] corresponds to the character going
// into the ETag for ETag::Option opt.
// IMPORTANT: The characters in all_options must come in sorted order (so that
// IMPORTANT: isValidOptionsString() works correctly).
const char all_options[] = "Zz";

static_assert(ETag::OPTION_COUNT == sizeof(all_options) - 1, "");

bool isValidETagBody(const std::string& s)
{
  return !s.empty() && s.find_first_of("\"/") == std::string::npos;
}

bool isSubsequenceOf(const std::string& s, const std::string& sortedString)
{
  std::string::size_type i = 0;
  for ( const char c : s )
  {
    const std::string::size_type j = sortedString.find(c, i);
    if ( j == std::string::npos )
      return false;
    i = j+1;
  }
  return true;
}

bool isValidOptionsString(const std::string& s)
{
  return isSubsequenceOf(s, all_options);
}

} // namespace


void ETag::set_option(Option opt)
{
  if ( ! get_option(opt) )
  {
    m_options.push_back(all_options[opt]);
    std::sort(m_options.begin(), m_options.end());
  }
}

bool ETag::get_option(Option opt) const
{
  return m_options.find(all_options[opt]) != std::string::npos;
}

std::string ETag::get_etag() const
{
  if ( m_body.empty() )
    return std::string();

  return "\"" + m_body + "/" + m_options + "\"";
}

ETag::ETag(const std::string& body, const std::string& options)
{
  if ( isValidETagBody(body) && isValidOptionsString(options) )
  {
    m_body = body;
    m_options = options;
  }
}

ETag ETag::parse(std::string s)
{
  if ( kiwix::startsWith("W/", s) )
    s = s.substr(2);

  if ( s.front() != '"' || s.back() != '"' )
    return ETag();

  s = s.substr(1, s.size()-2);

  const std::string::size_type i = s.find('/');
  if ( i == std::string::npos )
    return ETag();

  return ETag(s.substr(0, i), s.substr(i+1));
}

ETag ETag::match(const std::string& etags, const std::string& body)
{
  std::istringstream ss(etags);
  std::string etag_str;
  while ( ss >> etag_str )
  {
    if ( etag_str.back() == ',' )
      etag_str.pop_back();

    const ETag etag = parse(etag_str);
    if ( etag && etag.m_body == body )
      return etag;
  }

  return ETag();
}

} // namespace kiwix
