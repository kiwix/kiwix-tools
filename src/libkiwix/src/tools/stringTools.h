/*
 * Copyright 2011-2012 Emmanuel Engelhart <kelson@kiwix.org>
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

#ifndef KIWIX_STRINGTOOLS_H
#define KIWIX_STRINGTOOLS_H

#include <unicode/unistr.h>
#include <unicode/locid.h>

#include <string>
#include <vector>
#include <sstream>
#include <stdexcept>

namespace kiwix
{
std::string beautifyInteger(uint64_t number);
void printStringInHexadecimal(const char* s);
void printStringInHexadecimal(icu::UnicodeString s);
void stringReplacement(std::string& str,
                       const std::string& oldStr,
                       const std::string& newStr);
std::string encodeDiples(const std::string& str);

std::string removeAccents(const std::string& text);
void loadICUExternalTables();

class ICULanguageInfo
{
public:
  explicit ICULanguageInfo(const std::string& langCode);

  std::string iso3Code() const;
  std::string selfName() const;

private:
  const icu::Locale locale;
};

std::string escapeForJSON(const std::string& s, bool escapeQuote = true);

/* urlEncode() is the equivalent of JS encodeURIComponent(), with the only
 * difference that the slash (/) symbol is NOT encoded. */
std::string urlEncode(const std::string& value);
std::string urlDecode(const std::string& value, bool component = false);

std::string trim(const std::string& s);

std::string join(const std::vector<std::string>& list, const std::string& sep);

std::string ucAll(const std::string& word);
std::string lcAll(const std::string& word);
std::string ucFirst(const std::string& word);
std::string lcFirst(const std::string& word);

/* This function is broken, related Github issue 
 * https://github.com/kiwix/libkiwix/issues/1188 */ 
std::string toTitle(const std::string& word);

std::string normalize(const std::string& word);
template<typename T>
std::string to_string(T value)
{
    std::ostringstream oss;
    oss << value;
    return oss.str();
}

template<typename T>
T extractFromString(const std::string& str) {
    std::istringstream iss(str);
    T ret;
    iss >> ret;
    if(iss.fail() || !iss.eof()) {
        throw std::invalid_argument("no conversion");
    }
    return ret;
}

template<>
std::string extractFromString(const std::string& str);

bool startsWith(const std::string& base, const std::string& start);

std::string stripSuffix(const std::string& str, const std::string& suffix);

std::vector<std::string> getTitleVariants(const std::string& title);
} //namespace kiwix
#endif
