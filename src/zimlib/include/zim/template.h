/*
 * Copyright (C) 2009 Tommi Maekitalo
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * is provided AS IS, WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, and
 * NON-INFRINGEMENT.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 *
 */

#ifndef ZIM_TEMPLATE_H
#define ZIM_TEMPLATE_H

#include <string>

namespace zim
{
  class TemplateParser
  {
    public:
      class Event
      {
        public:
          virtual void onData(const std::string& data) = 0;
          virtual void onToken(const std::string& token) = 0;
          virtual void onLink(char ns, const std::string& url) = 0;
      };

    private:
      Event* event;

      std::string data;
      std::string::size_type save;
      std::string::size_type token;
      std::string::size_type token_e;
      char ns;
      typedef void (TemplateParser::*state_type)(char);

      state_type state;

      void state_data(char ch);
      void state_lt(char ch);
      void state_token0(char ch);
      void state_token(char ch);
      void state_token_end(char ch);
      void state_link0(char ch);
      void state_link(char ch);
      void state_title(char ch);
      void state_title_end(char ch);

    public:
      explicit TemplateParser(Event* ev)
        : event(ev),
          state(&TemplateParser::state_data)
        { }

      void parse(char ch)
      {
        (this->*state)(ch);
      }

      void parse(const std::string& s)
      {
        for (std::string::const_iterator ch = s.begin(); ch != s.end(); ++ch)
          parse(*ch);
      }

      void flush();

  };

}

#endif // ZIM_TEMPLATE_H
