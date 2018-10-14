/*
 * Copyright 2009-2016 Emmanuel Engelhart <kelson@kiwix.org>
 * Copyright 2017 Matthieu Gautier<mgautier@kymeria.fr>
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


#include "request_context.h"
#include <string.h>
#include <stdexcept>
#include <sstream>
#include <cstdio>

RequestMethod parse_method(const std::string& method) {
  if (method == "GET") {
    return RequestMethod::GET;
  } else if (method == "HEAD") {
    return RequestMethod::HEAD;
  } else if (method == "POST") {
    return RequestMethod::POST;
  } else if (method == "PUT") {
    return RequestMethod::PUT;
  } else if (method == "DELETE") {
    return RequestMethod::DELETE_;
  } else if (method == "CONNECT") {
    return RequestMethod::CONNECT;
  } else if (method == "OPTIONS") {
    return RequestMethod::OPTIONS;
  } else if (method == "TRACE") {
    return RequestMethod::TRACE;
  } else if (method == "PATCH") {
    return RequestMethod::PATCH;
  } else {
    return RequestMethod::OTHER;
  }
}

RequestContext::RequestContext(struct MHD_Connection* connection,
                               std::string rootLocation,
                               const std::string& _url,
                               const std::string& _method,
                               const std::string& version) :
  full_url(_url),
  url(_url),
  valid_url(true),
  method(parse_method(_method)),
  version(version),
  acceptEncodingDeflate(false),
  headers(get_headers(connection)),
  arguments(get_arguments(connection)),
  range(calculate_range())
{
  if (rootLocation.empty()) {
    // nothing special to handle.
    url = full_url;
  } else {
    if (full_url.size() > rootLocation.size() &&
        full_url.substr(0, rootLocation.size()+1) == rootLocation + "/") {
      url = full_url.substr(rootLocation.size());
    } else {
      valid_url = false;
    }
  }

  try {
    acceptEncodingDeflate =
        (get_header(MHD_HTTP_HEADER_ACCEPT_ENCODING).find("deflate") != std::string::npos);
  } catch (const std::out_of_range&) {}
}

RequestContext::~RequestContext()
{}

int insert_into_map(void *_map, enum MHD_ValueKind kind, const char *key, const char *value) {
  std::map<std::string, std::string>& map = *static_cast<std::map<std::string, std::string>*>(_map);
  map[key] = (value == nullptr) ? "" : value;
  return MHD_YES;
}

std::map<std::string, std::string> RequestContext::get_headers(struct MHD_Connection* connection) {
  std::map<std::string, std::string> result;
  MHD_get_connection_values(connection, MHD_HEADER_KIND, insert_into_map, &result);
  return result;
}

std::map<std::string, std::string> RequestContext::get_arguments(struct MHD_Connection* connection) {
  std::map<std::string, std::string> result;
  MHD_get_connection_values(connection, MHD_GET_ARGUMENT_KIND, insert_into_map, &result);
  return result;
}

void RequestContext::print_debug_info() {
  printf("method    : %s (%d)\n", method==RequestMethod::GET ? "GET" :
                                  method==RequestMethod::POST ? "POST" :
                                  "OTHER", (int)method);
  printf("version   : %s\n", version.c_str());
  printf("headers   :\n");
  for (auto it=headers.begin(); it!=headers.end(); it++) {
    printf(" - %s : '%s'\n", it->first.c_str(), it->second.c_str());
  }
  printf("arguments :\n");
  for (auto it=arguments.begin(); it!=arguments.end(); it++) {
    printf(" - %s : '%s'\n", it->first.c_str(), it->second.c_str());
  }
  printf("Parsed : \n");
  printf("url   : %s\n", url.c_str());
  printf("acceptEncodingDeflate : %d\n", acceptEncodingDeflate);
  printf("has_range : %d\n", range_state() == HAS_RANGE);
  printf(".............\n");
}


RequestMethod RequestContext::get_method() const {
  return method;
}

std::string RequestContext::get_url() const {
  return url;
}

std::string RequestContext::get_url_part(int number) const {
  size_t start = 1;
  while(true) {
    auto found = url.find('/', start);
    if (number == 0) {
      if (found == std::string::npos) {
        return url.substr(start);
      } else {
        return url.substr(start, found-start);
      }
    } else {
      if (found == std::string::npos) {
        throw std::out_of_range("No parts");
      }
      start = found + 1;
      number -= 1;
    }
  }
}

std::string RequestContext::get_full_url() const {
  return full_url;
}

bool RequestContext::is_valid_url() const {
  return valid_url;
}

RequestContext::RangeState RequestContext::range_state() const {
  if (!range) return INVALID;

  if (*range != std::make_pair(0, -1)) return NO_RANGE;
  else return HAS_RANGE;
}


#define INVALID_RANGE nullptr;

namespace {
  // TODO: remove if C++14
  template<typename T, typename... Args>
  std::unique_ptr<T> make_unique(Args&&... args)
  {
      return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
  }

  bool is_correct_unit(const std::string& /* range_unit */) {
    // TODO
    return true;
  }

  RequestContext::Range create_range(int start, int end) {
    return ::make_unique<std::pair<int, int>>(start, end);
  }

  /* truncates <unit>= in Range: '<unit>=0-1234'
   * returns true if truncated successfully or no unit in the range */
  bool truncate_range_unit(std::string* range_value) {
    try {
      auto equal_sign_position = range_value->find("=");
      if (equal_sign_position != std::string::npos) {
        std::string range_unit = range_value->substr(0, equal_sign_position);
        *range_value = range_value->substr(equal_sign_position + 1, std::string::npos);

        if (!::is_correct_unit(range_unit)) return false;
      }
    } catch (const std::out_of_range&) {
      printf("Error when calculating range (truncating <unit>=)");
      return false;
    }

    return true;
  }
}

/* Check if range is requested.
 * https://tools.ietf.org/html/rfc7233#section-3.1 */
RequestContext::Range RequestContext::calculate_range() const {
  int start = 0;
  int end = -1;

  if(!has_header(MHD_HTTP_HEADER_RANGE)) return ::create_range(start, end);

  std::string range_value = get_header(MHD_HTTP_HEADER_RANGE);

  // truncating <unit>= in Range: '<unit>=0-1234'
  if(!::truncate_range_unit(&range_value)) return INVALID_RANGE;

  // Multiple ranges requested, not supported
  if (range_value.find(",") != std::string::npos) return INVALID_RANGE;

  std::istringstream range_value_ss(range_value);
  char c;

  try {
    range_value_ss >> start >> c;
    if (range_value_ss.good() && c=='-') {
      range_value_ss >> end;
      if (range_value_ss.fail()) {
        // Something went wrong with extracting.
        end = -1;
      }
    }
  } catch (const std::out_of_range&) {
    printf("Error when calculating range (parsing int-[int])");
    return INVALID_RANGE;
  }

  return ::create_range(start, end);
}

template<>
std::string RequestContext::get_argument(const std::string& name) const {
  return arguments.at(name);
}
