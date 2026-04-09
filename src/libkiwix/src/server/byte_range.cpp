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


#include "byte_range.h"

#include "tools/stringTools.h"

#include <cassert>
#include <algorithm>

namespace kiwix {

namespace {

ByteRange parseByteRange(const std::string& rangeStr)
{
  std::istringstream iss(rangeStr);

  int64_t start, end = INT64_MAX;
  if (iss >> start) {
    if ( start < 0 ) {
      if ( iss.eof() )
        return ByteRange(-start);
    } else {
      char c;
      if (iss >> c && c=='-') {
        iss >> end; // if this fails, end is not modified, which is OK
        if (iss.eof() && start <= end)
          return ByteRange(ByteRange::PARSED, start, end);
      }
    }
  }

  return ByteRange(ByteRange::INVALID, 0, INT64_MAX);
}

} // unnamed namespace

ByteRange::ByteRange()
  : kind_(NONE)
  , first_(0)
  , last_(INT64_MAX)
{}

ByteRange::ByteRange(Kind kind, int64_t first, int64_t last)
  : kind_(kind)
  , first_(first)
  , last_(last)
{
  assert(kind != NONE);
  assert(first >= 0);
  assert(last >= first || (first == 0 && last == -1));
}

ByteRange::ByteRange(int64_t suffix_length)
  : kind_(PARSED)
  , first_(-suffix_length)
  , last_(INT64_MAX)
{
  assert(suffix_length > 0);
}

int64_t ByteRange::first() const
{
  assert(kind_ > PARSED);
  return first_;
}

int64_t ByteRange::last() const
{
  assert(kind_ > PARSED);
  return last_;
}

int64_t ByteRange::length() const
{
  assert(kind_ > PARSED);
  return last_ + 1 - first_;
}

ByteRange ByteRange::parse(const std::string& rangeStr)
{
  const std::string byteUnitSpec("bytes=");
  if ( ! kiwix::startsWith(rangeStr, byteUnitSpec) )
    return ByteRange(INVALID, 0, INT64_MAX);

  return parseByteRange(rangeStr.substr(byteUnitSpec.size()));
}

ByteRange ByteRange::resolve(int64_t contentSize) const
{
  if ( kind() == NONE )
    return ByteRange(RESOLVED_FULL_CONTENT, 0, contentSize-1);

  if ( kind() == INVALID )
    return ByteRange(RESOLVED_UNSATISFIABLE, 0, contentSize-1);

  const int64_t resolved_first = first_ < 0
                               ? std::max(int64_t(0), contentSize + first_)
                               : first_;

  const int64_t resolved_last = std::min(contentSize-1, last_);

  if ( resolved_first > resolved_last )
    return ByteRange(RESOLVED_UNSATISFIABLE, 0, contentSize-1);

  return ByteRange(RESOLVED_PARTIAL_CONTENT, resolved_first, resolved_last);
}

} // namespace kiwix
