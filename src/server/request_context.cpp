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

RequestContext::RequestContext(struct MHD_Connection* connection,
                               std::string rootLocation,
                               const std::string& _url,
                               const std::string& method,
                               const std::string& version) :
  connection(connection),
  full_url(_url),
  url(_url),
  valid_url(true),
  version(version),
  acceptEncodingDeflate(false),
  accept_range(false),
  range_pair(0, -1)
{
  if (method == "GET") {
    this->method = RequestMethod::GET;
  } else if (method == "HEAD") {
    this->method = RequestMethod::HEAD;
  } else if (method == "POST") {
    this->method = RequestMethod::POST;
  } else if (method == "PUT") {
    this->method = RequestMethod::PUT;
  } else if (method == "DELETE") {
    this->method = RequestMethod::DELETE_;
  } else if (method == "CONNECT") {
    this->method = RequestMethod::CONNECT;
  } else if (method == "OPTIONS") {
    this->method = RequestMethod::OPTIONS;
  } else if (method == "TRACE") {
    this->method = RequestMethod::TRACE;
  } else if (method == "PATCH") {
    this->method = RequestMethod::PATCH;
  } else {
    this->method = RequestMethod::OTHER;
  }

  MHD_get_connection_values(connection, MHD_HEADER_KIND, &RequestContext::fill_header, this);
  MHD_get_connection_values(connection, MHD_GET_ARGUMENT_KIND, &RequestContext::fill_argument, this);

  valid_url = true;
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

  /*Check if range is requested. */
  try {
    auto range = get_header(MHD_HTTP_HEADER_RANGE);
    int start = 0;
    int end = -1;
    std::istringstream iss(range);
    char c;

    iss >> start >> c;
    if (iss.good() && c=='-') {
      iss >> end;
      if (iss.fail()) {
        // Something went wrong will extracting.
        end = -1;
      }
      if (iss.eof()) {
        accept_range = true;
        range_pair = std::pair<int, int>(start, end);
      }
    }
  } catch (const std::out_of_range&) {}


}

RequestContext::~RequestContext()
{}


int RequestContext::fill_header(void *__this, enum MHD_ValueKind kind,
                                 const char *key, const char *value)
{
  RequestContext *_this = static_cast<RequestContext*>(__this);
  _this->headers[key] = value;
  return MHD_YES;
}

int RequestContext::fill_argument(void *__this, enum MHD_ValueKind kind,
                                   const char *key, const char* value)
{
  RequestContext *_this = static_cast<RequestContext*>(__this);
  _this->arguments[key] = value == nullptr ? "" : value;
  return MHD_YES;
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
  printf("has_range : %d\n", accept_range);
  printf(".............\n");
}


RequestMethod RequestContext::get_method() {
  return method;
}

std::string RequestContext::get_url() {
  return url;
}

std::string RequestContext::get_url_part(int number) {
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

std::string RequestContext::get_full_url() {
  return full_url;
}

bool RequestContext::is_valid_url() {
  return valid_url;
}

bool RequestContext::has_range() {
  return accept_range;
}

std::pair<int, int> RequestContext::get_range() {
  return range_pair;
}

template<>
std::string RequestContext::get_argument(const std::string& name) {
  return arguments.at(name);
}

std::string RequestContext::get_header(const std::string& name) {
  return headers.at(name);
}
