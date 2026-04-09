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

#include <tools/regexTools.h>

#include <unicode/regex.h>
#include <unicode/ucnv.h>

#include <memory>
#include <map>
#include <stdexcept>
#include <mutex>

std::map<std::string, std::shared_ptr<icu::RegexPattern>> regexCache;
static std::mutex regexLock;

std::unique_ptr<icu::RegexMatcher> buildMatcher(const std::string& regex, icu::UnicodeString& content)
{
  std::shared_ptr<icu::RegexPattern> pattern;
  /* Regex is in cache */
  try {
    pattern = regexCache.at(regex);
  } catch (std::out_of_range&) {
    // Redo the search with a lock to avoid race condition.
    std::lock_guard<std::mutex> l(regexLock);
    try {
      pattern = regexCache.at(regex);
    } catch (std::out_of_range&) {
      UErrorCode status = U_ZERO_ERROR;
      UParseError pe;
      icu::UnicodeString uregex(regex.c_str());
      pattern.reset(icu::RegexPattern::compile(uregex, UREGEX_CASE_INSENSITIVE, pe, status));
      regexCache[regex] = pattern;
    }
  }
  UErrorCode status = U_ZERO_ERROR;
  return std::unique_ptr<icu::RegexMatcher>(pattern->matcher(content, status));
}

bool matchRegex(const std::string& content, const std::string& regex)
{
  ucnv_setDefaultName("UTF-8");
  icu::UnicodeString ucontent(content.c_str());
  auto matcher = buildMatcher(regex, ucontent);
  return matcher->find();
}

std::string replaceRegex(const std::string& content,
                         const std::string& replacement,
                         const std::string& regex)
{
  ucnv_setDefaultName("UTF-8");
  icu::UnicodeString ureplacement(replacement.c_str());
  icu::UnicodeString ucontent(content.c_str());
  auto matcher = buildMatcher(regex, ucontent);
  UErrorCode status = U_ZERO_ERROR;
  auto uresult = matcher->replaceAll(ureplacement, status);
  std::string tmp;
  uresult.toUTF8String(tmp);
  return tmp;
}
