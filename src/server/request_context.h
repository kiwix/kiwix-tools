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


#ifndef REQUEST_CONTEXT_H
#define REQUEST_CONTEXT_H

#include <string>
#include <sstream>
#include <map>
#include <stdexcept>

extern "C" {
#include <microhttpd.h>
}

enum class RequestMethod {
    GET,
    HEAD,
    POST,
    PUT,
    DELETE_,
    CONNECT,
    OPTIONS,
    TRACE,
    PATCH,
    OTHER
};

class KeyError : public std::runtime_error {};
class IndexError: public std::runtime_error {};


class RequestContext {
  public:
    RequestContext(struct MHD_Connection* connection,
                   std::string rootLocation,
                   const std::string& url,
                   const std::string& method,
                   const std::string& version);
    ~RequestContext();

    void print_debug_info();

    bool is_valid_url();

    std::string get_header(const std::string& name);
    template<typename T=std::string>
    T get_argument(const std::string& name) {
        std::istringstream stream(arguments.at(name));
        T v;
        stream >> v;
        return v;
    }


    RequestMethod get_method();
    std::string get_url();
    std::string get_url_part(int part);
    std::string get_full_url();

    bool has_range();
    std::pair<int, int> get_range();

    bool can_compress() { return acceptEncodingDeflate; }

    // [TODO] Move this to the response builder
    int httpResponseCode;

  private:
    std::string full_url;
    std::string url;
    bool valid_url;
    RequestMethod method;
    std::string version;

    bool acceptEncodingDeflate;

    bool accept_range;
    std::pair<int, int> range_pair;
    std::map<std::string, std::string> headers;
    std::map<std::string, std::string> arguments;

    static int fill_header(void *, enum MHD_ValueKind, const char*, const char*);
    static int fill_argument(void *, enum MHD_ValueKind, const char*, const char*);
};

template<> std::string RequestContext::get_argument(const std::string& name);


#endif //REQUEST_CONTEXT_H
