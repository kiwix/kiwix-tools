/*
 * Copyright 2014 Emmanuel Engelhart <kelson@kiwix.org>
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

// Implement function declared in tools.h and tools/otherTools.h
#include "tools.h"
#include "tools/otherTools.h"


#include <algorithm>
#include <iomanip>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#include "tools/stringTools.h"
#include "server/i18n_utils.h"
#include "libkiwix-resources.h"

#include <map>
#include <sstream>
#include <pugixml.hpp>

#include <zim/uuid.h>
#include <zim/suggestion_iterator.h>


static std::map<std::string, std::string> codeisomapping {
{ "aa", "aar" },
{ "af", "afr" },
{ "ak", "aka" },
{ "am", "amh" },
{ "ar", "ara" },
{ "as", "asm" },
{ "az", "aze" },
{ "ba", "bak" },
{ "be", "bel" },
{ "bg", "bul" },
{ "bm", "bam" },
{ "bn", "ben" },
{ "bo", "bod" },
{ "br", "bre" },
{ "bs", "bos" },
{ "ca", "cat" },
{ "ce", "che" },
{ "co", "cos" },
{ "cs", "ces" },
{ "cu", "chu" },
{ "cv", "chv" },
{ "cy", "cym" },
{ "da", "dan" },
{ "de", "deu" },
{ "dv", "div" },
{ "dz", "dzo" },
{ "ee", "ewe" },
{ "el", "ell" },
{ "en", "eng" },
{ "es", "spa" },
{ "et", "est" },
{ "eu", "eus" },
{ "fa", "fas" },
{ "ff", "ful" },
{ "fi", "fin" },
{ "fo", "fao" },
{ "fr", "fra" },
{ "fy", "fry" },
{ "ga", "gle" },
{ "gd", "gla" },
{ "gl", "glg" },
{ "gn", "grn" },
{ "gu", "guj" },
{ "gv", "glv" },
{ "ha", "hau" },
{ "he", "heb" },
{ "hi", "hin" },
{ "hr", "hrv" },
{ "hu", "hun" },
{ "hy", "hye" },
{ "ia", "ina" },
{ "id", "ind" },
{ "ig", "ibo" },
{ "is", "isl" },
{ "it", "ita" },
{ "iu", "iku" },
{ "ja", "jpn" },
{ "jv", "jav" },
{ "ka", "kat" },
{ "ki", "kik" },
{ "kk", "kaz" },
{ "kl", "kal" },
{ "km", "khm" },
{ "kn", "kan" },
{ "ko", "kor" },
{ "ks", "kas" },
{ "ku", "kur" },
{ "kw", "cor" },
{ "ky", "kir" },
{ "lb", "ltz" },
{ "lg", "lug" },
{ "ln", "lin" },
{ "lo", "lao" },
{ "lt", "lit" },
{ "lv", "lav" },
{ "mg", "mlg" },
{ "mi", "mri" },
{ "mi", "mri" },
{ "mk", "mkd" },
{ "ml", "mal" },
{ "mn", "mon" },
{ "mr", "mar" },
{ "ms", "msa" },
{ "mt", "mlt" },
{ "my", "mya" },
{ "nb", "nob" },
{ "ne", "nep" },
{ "nl", "nld" },
{ "nn", "nno" },
{ "no", "nor" },
{ "ny", "nya" },
{ "oc", "oci" },
{ "om", "orm" },
{ "or", "ori" },
{ "os", "oss" },
{ "pa", "pan" },
{ "pl", "pol" },
{ "ps", "pus" },
{ "pt", "por" },
{ "qu", "que" },
{ "rm", "roh" },
{ "rn", "run" },
{ "ro", "ron" },
{ "ru", "rus" },
{ "rw", "kin" },
{ "sa", "san" },
{ "sd", "snd" },
{ "se", "sme" },
{ "sg", "sag" },
{ "si", "sin" },
{ "sk", "slk" },
{ "sl", "slv" },
{ "sn", "sna" },
{ "so", "som" },
{ "sq", "sqi" },
{ "sr", "srp" },
{ "ss", "ssw" },
{ "st", "sot" },
{ "sv", "swe" },
{ "sw", "swa" },
{ "ta", "tam" },
{ "te", "tel" },
{ "tg", "tgk" },
{ "th", "tha" },
{ "ti", "tir" },
{ "tk", "tuk" },
{ "tl", "tgl" },
{ "tn", "tsn" },
{ "to", "ton" },
{ "tr", "tur" },
{ "ts", "tso" },
{ "tt", "tat" },
{ "ug", "uig" },
{ "uk", "ukr" },
{ "ur", "urd" },
{ "uz", "uzb" },
{ "ve", "ven" },
{ "vi", "vie" },
{ "wa", "wln" },
{ "wo", "wol" },
{ "xh", "xho" },
{ "yo", "yor" },
{ "zh", "zho" },
{ "zu", "zul" }
};

void kiwix::sleep(unsigned int milliseconds)
{
#ifdef _WIN32
  Sleep(milliseconds);
#else
  usleep(1000 * milliseconds);
#endif
}


struct XmlStringWriter: pugi::xml_writer
{
  std::string result;
  virtual void write(const void* data, size_t size){
    result.append(static_cast<const char*>(data), size);
  }
};

std::string kiwix::nodeToString(const pugi::xml_node& node)
{
  XmlStringWriter writer;
  node.print(writer, "  ");
  return writer.result;
}

std::string kiwix::converta2toa3(const std::string& a2code){
  return codeisomapping.at(a2code);
}

std::vector<std::string> kiwix::convertTags(const std::string& tags_str)
{
  auto tags = kiwix::split(tags_str, ";");
  std::vector<std::string> tagsList;
  bool picSeen(false), vidSeen(false), detSeen(false), indexSeen(false);
  for (auto tag: tags) {
    picSeen |= (tag == "nopic" || startsWith(tag, "_pictures:"));
    vidSeen |= (tag == "novid" || startsWith(tag, "_videos:"));
    detSeen |= (tag == "nodet" || startsWith(tag, "_details:"));
    indexSeen |= kiwix::startsWith(tag, "_ftindex");
    if (tag == "nopic") {
      tagsList.push_back("_pictures:no");
    } else if (tag == "novid") {
      tagsList.push_back("_videos:no");
    } else if (tag == "nodet") {
      tagsList.push_back("_details:no");
    } else if (tag == "_ftindex") {
      tagsList.push_back("_ftindex:yes");
    } else {
      tagsList.push_back(tag);
    }
  }
  if (!indexSeen) {
    tagsList.push_back("_ftindex:no");
  }
  if (!picSeen) {
    tagsList.push_back("_pictures:yes");
  }
  if (!vidSeen) {
    tagsList.push_back("_videos:yes");
  }
  if (!detSeen) {
    tagsList.push_back("_details:yes");
  }
  return tagsList;
}

std::string kiwix::getTagValueFromTagList(
  const std::vector<std::string>& tagList, const std::string& tagName)
{
  for (auto tag: tagList) {
    if (tag[0] == '_') {
      auto delimPos = tag.find(':');
      if (delimPos == std::string::npos) {
        // No delimiter... what to do ?
        continue;
      }
      auto cTagName = tag.substr(1, delimPos-1);
      auto cTagValue = tag.substr(delimPos+1);
      if (cTagName == tagName) {
        return cTagValue;
      }
    }
  }
  std::stringstream ss;
  ss << tagName << " cannot be found";
  throw std::out_of_range(ss.str());
}

bool kiwix::convertStrToBool(const std::string& value)
{
  if (value == "yes") {
    return true;
  } else if (value == "no") {
    return false;
  }

  std::stringstream ss;
  ss << "Tag value '" << value << "' cannot be converted to bool.";
  throw std::domain_error(ss.str());
}

std::string kiwix::gen_date_str()
{
  auto now = std::time(0);
  auto tm = std::localtime(&now);

  std::stringstream is;
  is << std::setw(2) << std::setfill('0')
     << 1900+tm->tm_year << "-"
     << std::setw(2) << std::setfill('0') << tm->tm_mon+1 << "-"
     << std::setw(2) << std::setfill('0') << tm->tm_mday << "T"
     << std::setw(2) << std::setfill('0') << tm->tm_hour << ":"
     << std::setw(2) << std::setfill('0') << tm->tm_min << ":"
     << std::setw(2) << std::setfill('0') << tm->tm_sec << "Z";
  return is.str();
}

std::string kiwix::gen_uuid(const std::string& s)
{
  return kiwix::to_string(zim::Uuid::generate(s));
}

kainjow::mustache::data kiwix::onlyAsNonEmptyMustacheValue(const std::string& s)
{
  return s.empty()
       ? kainjow::mustache::data(false)
       : kainjow::mustache::data(s);
}

std::string kiwix::render_template(const std::string& template_str, kainjow::mustache::data data)
{
  kainjow::mustache::mustache tmpl(template_str);
  std::stringstream ss;
  tmpl.render(data, [&ss](const std::string& str) { ss << str; });
  return ss.str();
}

// The escapeQuote parameter of escapeForJSON() defaults to true.
// This constant makes the calls to escapeForJSON() where the quote symbol
// should not be escaped (as it is later replaced with the HTML character entity
// &quot;) more readable.
static const bool DONT_ESCAPE_QUOTE = false;

std::string kiwix::escapeForJSON(const std::string& s, bool escapeQuote)
{
  std::ostringstream oss;
  for (char c : s) {
    if ( c == '\\' ) {
      oss << "\\\\";
    } else if ( unsigned(c) < 0x20U ) {
      switch ( c ) {
      case '\n': oss << "\\n"; break;
      case '\r': oss << "\\r"; break;
      case '\t': oss << "\\t"; break;
      default:   oss << "\\u" << std::setw(4) << std::setfill('0') << unsigned(c);
      }
    } else if ( c == '"' && escapeQuote ) {
      oss << "\\\"";
    } else {
      oss << c;
    }
  }
  return oss.str();
}

namespace
{

std::string makeFulltextSearchSuggestion(const std::string& lang,
                                         const std::string& queryString)
{
  return kiwix::i18n::expandParameterizedString(lang, "suggest-full-text-search",
               {
                  {"SEARCH_TERMS", queryString}
               }
         );
}

} // unnamed namespace

kiwix::Suggestions::Suggestions()
  : m_data(kainjow::mustache::data::type::list)
{
}

void kiwix::Suggestions::add(const zim::SuggestionItem& suggestion)
{
  kainjow::mustache::data result;

  const std::string label = suggestion.hasSnippet()
                          ? suggestion.getSnippet()
                          : suggestion.getTitle();

  result.set("label", escapeForJSON(label, DONT_ESCAPE_QUOTE));
  result.set("value", escapeForJSON(suggestion.getTitle(), DONT_ESCAPE_QUOTE));
  result.set("kind", "path");
  result.set("path", escapeForJSON(suggestion.getPath(), DONT_ESCAPE_QUOTE));
  result.set("first", m_data.is_empty_list());
  m_data.push_back(result);
}

void kiwix::Suggestions::addFTSearchSuggestion(const std::string& uiLang,
                                               const std::string& queryString)
{
  kainjow::mustache::data result;
  const std::string label = makeFulltextSearchSuggestion(uiLang, queryString);
  result.set("label", escapeForJSON(label, DONT_ESCAPE_QUOTE));
  result.set("value", escapeForJSON(queryString + " ", DONT_ESCAPE_QUOTE));
  result.set("kind", "pattern");
  result.set("first", m_data.is_empty_list());
  m_data.push_back(result);
}

std::string kiwix::Suggestions::getJSON() const
{
  kainjow::mustache::data data;
  data.set("suggestions", m_data);

  return render_template(RESOURCE::templates::suggestion_json, data);
}
