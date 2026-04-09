/*
 * Copyright 2024 Veloman Yunkan <veloman.yunkan@gmail.com>
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

#ifndef KIWIX_I18N
#define KIWIX_I18N

#include <map>
#include <string>

namespace kiwix
{

std::string getTranslatedString(const std::string& lang, const std::string& key);

namespace i18n
{

typedef std::map<std::string, std::string> Parameters;

std::string expandParameterizedString(const std::string& lang,
                                      const std::string& key,
                                      const Parameters& params);

class GetTranslatedString
{
public:
  explicit GetTranslatedString(const std::string& lang) : m_lang(lang) {}

  std::string operator()(const std::string& key) const
  {
    return getTranslatedString(m_lang, key);
  }

  std::string operator()(const std::string& key, const Parameters& params) const
  {
    return expandParameterizedString(m_lang, key, params);
  }

private:
  const std::string m_lang;
};

} // namespace i18n

class ParameterizedMessage
{
public: // types
  typedef i18n::Parameters Parameters;

public: // functions
  ParameterizedMessage(const std::string& msgId, const Parameters& params)
    : msgId(msgId)
    , params(params)
  {}

  std::string getText(const std::string& lang) const;

  const std::string& getMsgId()  const { return msgId; }
  const Parameters&  getParams() const { return params; }

private: // data
  const std::string msgId;
  const Parameters  params;
};

inline ParameterizedMessage nonParameterizedMessage(const std::string& msgId)
{
  const ParameterizedMessage::Parameters noParams;
  return ParameterizedMessage(msgId, noParams);
}

std::string translateBookCategory(const std::string& lang, const std::string& category);

} // namespace kiwix

#endif // KIWIX_I18N
