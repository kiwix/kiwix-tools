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


#ifndef KIWIXLIB_SERVER_BYTE_RANGE_H
#define KIWIXLIB_SERVER_BYTE_RANGE_H

#include <cstdint>
#include <string>

namespace kiwix {

class ByteRange
{
  public: // types
    // ByteRange is parsed in a request, then it must be resolved (taking
    // into account the actual size of the requested resource) before
    // being applied in the response.
    // The Kind enum represents possible states in such a lifecycle.
    enum Kind {
      // The request is not a range request (no Range header)
      NONE,

      // The value of the Range header is not a valid continuous
      // range. Note that a valid (according to RFC7233) sequence of multiple
      // byte ranges is considered invalid in the current implementation
      // (i.e. only single-range partial requests are supported).
      INVALID,

      // This byte-range has been successfully parsed from the request
      PARSED,

      // This is a response to a regular (non-range) request
      RESOLVED_FULL_CONTENT,

      // The range request is invalid or unsatisfiable
      RESOLVED_UNSATISFIABLE,

      // This is a response to a (satisfiable) range request
      RESOLVED_PARTIAL_CONTENT,
    };

  public: // functions
    // Constructs a ByteRange object of NONE kind
    ByteRange();

    // Constructs a ByteRange object of the given kind (except NONE)
    ByteRange(Kind kind, int64_t first, int64_t last);

    // Constructs a ByteRange object of PARSED kind corresponding to a
    // range request of the form "Range: bytes=-suffix_length"
    explicit ByteRange(int64_t suffix_length);

    Kind kind() const { return kind_; }
    int64_t first() const;
    int64_t last() const;
    int64_t length() const;

    static ByteRange parse(const std::string& rangeStr);
    ByteRange resolve(int64_t contentSize) const;

  private: // data
    Kind kind_;
    int64_t first_;
    int64_t last_;
};

} // namespace kiwix

#endif //KIWIXLIB_SERVER_BYTE_RANGE_H
