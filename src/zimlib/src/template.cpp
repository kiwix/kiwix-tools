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

#include <zim/template.h>

namespace zim
{
  void TemplateParser::state_data(char ch)
  {
    data += ch;

    if (ch == '<')
    {
      state = &TemplateParser::state_lt;
      save = data.size() - 1;
    }
  }

  void TemplateParser::state_lt(char ch)
  {
    data += ch;

    if (ch == '%')
      state = &TemplateParser::state_token0;
    else
      state = &TemplateParser::state_data;
  }

  void TemplateParser::state_token0(char ch)
  {
    data += ch;

    if (ch == '/')
      state = &TemplateParser::state_link0;
    else
    {
      token = data.size() - 1;
      state = &TemplateParser::state_token;
    }
  }

  void TemplateParser::state_token(char ch)
  {
    data += ch;

    if (ch == '%')
      state = &TemplateParser::state_token_end;
  }

  void TemplateParser::state_token_end(char ch)
  {
    if (ch == '>')
    {
      if (event)
      {
        event->onData(data.substr(0, save));
        event->onToken(data.substr(token, data.size() - token - 1));
        data.clear();
      }

      state = &TemplateParser::state_data;
    }
    else
    {
      data += ch;
      state = &TemplateParser::state_data;
    }
  }

  void TemplateParser::state_link0(char ch)
  {
    data += ch;

    ns = ch;
    state = &TemplateParser::state_link;
  }

  void TemplateParser::state_link(char ch)
  {
    data += ch;

    if (ch == '/')
    {
      token = data.size();
      state = &TemplateParser::state_title;
    }
    else
      state = &TemplateParser::state_data;
  }

  void TemplateParser::state_title(char ch)
  {
    data += ch;

    if (ch == '%')
    {
      token_e = data.size() - 1;
      state = &TemplateParser::state_title_end;
    }
  }

  void TemplateParser::state_title_end(char ch)
  {
    data += ch;

    if (ch == '>')
    {
      if (event)
      {
        event->onData(data.substr(0, save));
        event->onLink(ns, data.substr(token, token_e - token));
      }

      data.clear();
      state = &TemplateParser::state_data;
    }
  }

  void TemplateParser::flush()
  {
    if (event)
      event->onData(data);
    data.clear();
    state = &TemplateParser::state_data;
  }
}
