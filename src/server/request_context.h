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
#include <memory>

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

    bool is_valid_url() const;

    bool has_header(const std::string& name) const {
        return headers.find(name) != headers.end();
    }

    std::string get_header(const std::string& name) const {
        return headers.at(name);
    }

    bool has_argument(const std::string& name) const {
        return arguments.find(name) != arguments.end();
    }

    template<typename T=std::string>
    T get_argument(const std::string& name) const {
        std::istringstream stream(arguments.at(name));
        T v;
        stream >> v;
        return v;
    }

    RequestMethod get_method() const;
    std::string get_url() const;
    std::string get_url_part(int part) const;
    std::string get_full_url() const;

    bool can_compress() const { return acceptEncodingDeflate; }

    // [TODO] Move this to the response builder
    int httpResponseCode;

  private:
    std::string full_url;
    std::string url;
    bool valid_url;
    const RequestMethod method;
    std::string version;

    bool acceptEncodingDeflate;

    const std::map<std::string, std::string> headers;
    const std::map<std::string, std::string> arguments;

  public:
    using Range = std::unique_ptr<std::pair<int, int>>;
    enum RangeState{
        INVALID, HAS_RANGE, NO_RANGE
    };

    RangeState range_state() const;
    const Range range;

  private:
    /* Check if range is requested. */
    Range calculate_range() const;

    static std::map<std::string, std::string> get_headers(struct MHD_Connection* connection);
    static std::map<std::string, std::string> get_arguments(struct MHD_Connection* connection);
};

template<> std::string RequestContext::get_argument(const std::string& name) const;


#endif //REQUEST_CONTEXT_H
