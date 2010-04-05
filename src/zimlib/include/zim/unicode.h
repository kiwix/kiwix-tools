/*
 * Copyright (C) 2008 Tommi Maekitalo
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

#include <locale>
#include <zim/zim.h>

namespace zim
{
    uint32_t tolower(uint32_t ucs);

    uint32_t toupper(uint32_t ucs);

    std::ctype_base::mask ctypeMask(uint32_t ch);

    inline bool isalpha(uint32_t ch)
    {
        return ctypeMask(ch) & std::ctype_base::alpha;
    }


    inline bool isalnum(uint32_t ch)
    {
        return ctypeMask(ch) & std::ctype_base::alnum;
    }


    inline bool ispunct(uint32_t ch)
    {
        return ctypeMask(ch) & std::ctype_base::punct;
    }


    inline bool iscntrl(uint32_t ch)
    {
        return ctypeMask(ch) & std::ctype_base::cntrl;
    }


    inline bool isdigit(uint32_t ch)
    {
        return ctypeMask(ch) & std::ctype_base::digit;
    }


    inline bool isxdigit(uint32_t ch)
    {
        return ctypeMask(ch) & std::ctype_base::xdigit;
    }

    inline bool isgraph(uint32_t ch)
    {
        return ctypeMask(ch) & std::ctype_base::graph;
    }


    inline bool islower(uint32_t ch)
    {
        return ctypeMask(ch) & std::ctype_base::lower;
    }


    inline bool isupper(uint32_t ch)
    {
        return ctypeMask(ch) & std::ctype_base::upper;
    }


    inline bool isprint(uint32_t ch)
    {
        return ctypeMask(ch) & std::ctype_base::print;
    }


    inline bool isspace(uint32_t ch)
    {
        return ctypeMask(ch) & std::ctype_base::space;
    }

}
