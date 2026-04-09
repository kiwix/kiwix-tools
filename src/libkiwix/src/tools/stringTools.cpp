/*
 * Copyright 2011 Emmanuel Engelhart <kelson@kiwix.org>
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

// Implement function declared in tools.h and tools/stringTools.h
#include "tools.h"
#include "tools/stringTools.h"

#include "tools/pathTools.h"
#include <unicode/normlzr.h>
#include <unicode/rep.h>
#include <unicode/translit.h>
#include <unicode/ucnv.h>
#include <unicode/uniset.h>
#include <unicode/ustring.h>

#include <algorithm>
#include <cctype>
#include <iostream>
#include <iomanip>
#include <regex>

namespace
{

bool isAsciiOnly(const std::string& s)
{
  for ( const unsigned char c : s ) {
    if ( c >= 128 )
      return false;
  }
  return true;
}

std::string asciiToLower(std::string s)
{
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) {
        return ('A' <= c && c <= 'Z') ? c - ('Z' - 'z') : c;
        });
    return s;
}

} // unnamed namespace

/* tell ICU where to find its dat file (tables) */
void kiwix::loadICUExternalTables()
{
#ifdef __APPLE__
  std::string executablePath = getExecutablePath();
  std::string executableDirectory = removeLastPathElement(executablePath);
  std::string datPath
      = computeAbsolutePath(executableDirectory, "icudt58l.dat");
  try {
    u_setDataDirectory(datPath.c_str());
  } catch (std::exception& e) {
    std::cerr << e.what() << std::endl;
  }
#endif
}

kiwix::ICULanguageInfo::ICULanguageInfo(const std::string& langCode)
  : locale(langCode.c_str())
{}

std::string kiwix::ICULanguageInfo::iso3Code() const
{
  return locale.getISO3Language();
}

std::string kiwix::ICULanguageInfo::selfName() const
{
  icu::UnicodeString langSelfNameICUString;
  locale.getDisplayLanguage(locale, langSelfNameICUString);
  std::string langSelfName;
  langSelfNameICUString.toUTF8String(langSelfName);
  return langSelfName;
}

std::string kiwix::removeAccents(const std::string& text)
{
  if ( isAsciiOnly(text) )
    return asciiToLower(text);

  loadICUExternalTables();
  ucnv_setDefaultName("UTF-8");
  UErrorCode status = U_ZERO_ERROR;
  std::unique_ptr<icu::Transliterator> removeAccentsTrans(icu::Transliterator::createInstance(
      "Lower; NFD; [:M:] remove; NFC", UTRANS_FORWARD, status));
  icu::UnicodeString ustring(text.c_str());
  removeAccentsTrans->transliterate(ustring);
  std::string unaccentedText;
  ustring.toUTF8String(unaccentedText);
  return unaccentedText;
}

/* Prepare integer for display */
std::string kiwix::beautifyInteger(uint64_t number)
{
  std::stringstream numberStream;
  numberStream << number;
  std::string numberString = numberStream.str();

  signed int offset = numberString.size() - 3;
  while (offset > 0) {
    numberString.insert(offset, ",");
    offset -= 3;
  }

  return numberString;
}

std::string kiwix::beautifyFileSize(uint64_t number)
{
  std::stringstream ss;
  ss << std::fixed << std::setprecision(2);
  if (number>>30)
    ss << (number/(1024.0*1024*1024)) << " GB";
  else if (number>>20)
    ss << (number/(1024.0*1024)) << " MB";
  else if (number>>10)
    ss << (number/1024.0) << " KB";
  else
    ss << number << " B";
  return ss.str();
}

void kiwix::printStringInHexadecimal(icu::UnicodeString s)
{
  std::cout << std::showbase << std::hex;
  for (int i = 0; i < s.length(); i++) {
    char c = (char)((s.getTerminatedBuffer())[i]);
    if (c & 0x80) {
      std::cout << (c & 0xffff) << " ";
    } else {
      std::cout << c << " ";
    }
  }
  std::cout << std::endl;
}

void kiwix::printStringInHexadecimal(const char* s)
{
  std::cout << std::showbase << std::hex;
  for (char const* pc = s; *pc; ++pc) {
    if (*pc & 0x80) {
      std::cout << (*pc & 0xffff);
    } else {
      std::cout << *pc;
    }
    std::cout << ' ';
  }
  std::cout << std::endl;
}

void kiwix::stringReplacement(std::string& str,
                              const std::string& oldStr,
                              const std::string& newStr)
{
  size_t pos = 0;
  while ((pos = str.find(oldStr, pos)) != std::string::npos) {
    str.replace(pos, oldStr.length(), newStr);
    pos += newStr.length();
  }
}

/* Encode string to avoid XSS attacks */
std::string kiwix::encodeDiples(const std::string& str)
{
  std::string result = str;
  kiwix::stringReplacement(result, "<", "&lt;");
  kiwix::stringReplacement(result, ">", "&gt;");
  return result;
}

namespace
{

bool isReservedUrlChar(char c)
{
  switch (c) {
  case ';':
  case ',':
  case '?':
  case ':':
  case '@':
  case '&':
  case '=':
  case '+':
  case '$':
  case '#':
    return true;
  default:
    return false;
  }
}

bool isHarmlessUriChar(char c)
{
  if (c >= 'a' && c <= 'z')
    return true;
  if (c >= 'A' && c <= 'Z')
    return true;
  if (c >= '0' && c <= '9')
    return true;

  switch (c) {
  case '-':
  case '_':
  case '.':
  case '!':
  case '~':
  case '*':
  case '\'':
  case '(':
  case ')':
  case '/':
    return true;
  }
  return false;
}

int hexToInt(char c) {
  switch (c) {
  case '0': return 0;
  case '1': return 1;
  case '2': return 2;
  case '3': return 3;
  case '4': return 4;
  case '5': return 5;
  case '6': return 6;
  case '7': return 7;
  case '8': return 8;
  case '9': return 9;
  case 'A': case 'a': return 10;
  case 'B': case 'b': return 11;
  case 'C': case 'c': return 12;
  case 'D': case 'd': return 13;
  case 'E': case 'e': return 14;
  case 'F': case 'f': return 15;
  default: return -1;
  }
}

} // unnamed namespace

std::string kiwix::urlEncode(const std::string& value)
{
  std::ostringstream os;
  os << std::hex << std::uppercase;
  for (const char c : value) {
    if (isHarmlessUriChar(c)) {
      os << c;
    } else {
      const unsigned int charVal = static_cast<unsigned char>(c);
      os << '%' << std::setw(2) << std::setfill('0') << charVal;
    }
  }
  return os.str();
}

std::string kiwix::urlDecode(const std::string& value, bool component)
{
  std::ostringstream os;
  for (std::string::const_iterator it = value.begin();
       it != value.end();
       it++) {

    // If there aren't enough characters left for this to be a
    // valid escape code, just use the character and move on
    if (value.end() - it < 3) {
      os << *it;
      continue;
    }

    if (*it == '%') {
      char hi = *(++it);
      char lo = *(++it);
      int iHi = hexToInt(hi);
      int iLo = hexToInt(lo);
      if (iHi < 0 || iLo < 0) {
        // Invalid escape sequence
        os << '%' << hi << lo;
        continue;
      }
      char c = (char)(iHi << 4 | iLo);
      if (!component && isReservedUrlChar(c)) {
        os << '%' << hi << lo;
      } else {
        os << c;
      }
    } else {
      os << *it;
    }
  }

  return os.str();
}

/* Split string in a token array */
std::vector<std::string> kiwix::split(const std::string& str,
                                      const std::string& delims,
                                      bool dropEmpty,
                                      bool keepDelim)
{
  std::string::size_type lastPos = 0;
  std::string::size_type pos = 0;
  std::vector<std::string> tokens;
  while( (pos = str.find_first_of(delims, lastPos)) < str.length() )
  {
    auto token = str.substr(lastPos, pos - lastPos);
    if (!dropEmpty || !token.empty()) {
      tokens.push_back(token);
    }
    if (keepDelim) {
      tokens.push_back(str.substr(pos, 1));
    }
    lastPos = pos + 1;
  }

  auto token = str.substr(lastPos);
  if (!dropEmpty || !token.empty()) {
    tokens.push_back(token);
  }
  return tokens;
}

std::string kiwix::join(const std::vector<std::string>& list, const std::string& sep)
{
  std::stringstream ss;
  bool first = true;
  for (auto& s:list) {
    if (!first) {
      ss << sep;
    }
    first = false;
    ss << s;
  }
  return ss.str();
}


std::string kiwix::ucFirst(const std::string& word)
{
  if (word.empty()) {
    return "";
  }

  std::string result;

  icu::UnicodeString unicodeWord(word.c_str());
  auto unicodeFirstLetter = icu::UnicodeString(unicodeWord, 0, 1).toUpper();
  unicodeWord.replace(0, 1, unicodeFirstLetter);
  unicodeWord.toUTF8String(result);

  return result;
}

std::string kiwix::ucAll(const std::string& word)
{
  if (word.empty()) {
    return "";
  }

  std::string result;

  icu::UnicodeString unicodeWord(word.c_str());
  unicodeWord.toUpper().toUTF8String(result);

  return result;
}

std::string kiwix::lcFirst(const std::string& word)
{
  if (word.empty()) {
    return "";
  }

  std::string result;

  icu::UnicodeString unicodeWord(word.c_str());
  auto unicodeFirstLetter = icu::UnicodeString(unicodeWord, 0, 1).toLower();
  unicodeWord.replace(0, 1, unicodeFirstLetter);
  unicodeWord.toUTF8String(result);

  return result;
}

std::string kiwix::lcAll(const std::string& word)
{
  if (word.empty()) {
    return "";
  }

  std::string result;

  icu::UnicodeString unicodeWord(word.c_str());
  unicodeWord.toLower().toUTF8String(result);

  return result;
}

std::string kiwix::toTitle(const std::string& word)
{
  if (word.empty()) {
    return "";
  }

  std::string result;

  icu::UnicodeString unicodeWord(word.c_str());
  unicodeWord = unicodeWord.toTitle(0);
  unicodeWord.toUTF8String(result);

  return result;
}

std::string kiwix::normalize(const std::string& word)
{
  return kiwix::lcAll(word);
}


bool kiwix::startsWith(const std::string& base, const std::string& start)
{
   return start.length() <= base.length()
        && std::equal(start.begin(), start.end(), base.begin());
}

std::string kiwix::stripSuffix(const std::string& str, const std::string& suffix)
{
  if (str.size() > suffix.size()) {
    const auto subStr = str.substr(str.size() - suffix.size(), str.size());
    if (subStr == suffix) {
      return str.substr(0, str.size() - suffix.size());
    }
  }
  return str;
}

std::vector<std::string> kiwix::getTitleVariants(const std::string& title) {
  std::vector<std::string> variants;
  variants.push_back(title);
  variants.push_back(kiwix::ucFirst(title));
  variants.push_back(kiwix::lcFirst(title));
  variants.push_back(kiwix::toTitle(title));
  return variants;
}

template<>
std::string kiwix::extractFromString(const std::string& str) {
  return str;
}

std::string kiwix::getSlugifiedFileName(const std::string& filename)
{
#ifdef _WIN32
  const std::regex reservedCharsReg(R"([<>:"/\\|?*])");
#else
  const std::regex reservedCharsReg("/");
#endif
  return std::regex_replace(filename, reservedCharsReg, "_");
}

std::string kiwix::trim(const std::string& s)
{
  auto is_space = [](unsigned char c) { return std::isspace(c); };
  auto start = std::find_if_not(s.begin(), s.end(), is_space);
  auto end = std::find_if_not(s.rbegin(), s.rend(), is_space).base();
  return (start < end) ? std::string(start, end) : std::string();
}
