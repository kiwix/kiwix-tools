/*-
 * Copyright (c) 2004 - 2010 CTPP Team
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 4. Neither the name of the CTPP Team nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *      CTPP2VMDebugInfo.cpp
 *
 * $CTPP$
 */
#include "CTPP2VMDebugInfo.hpp"

#include <stdio.h>

namespace CTPP // C++ Template Engine
{

/**
   SSSSSSSS SSSSSSSS SSSSSSSS - String description, 24 bits
                              LLLLLLLL LLLLLLLL LLLL - Line, 20 bits
                                                    PPPP PPPPPPPP PPPPPPPP - Position in line, 20 bits
   00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
   F   F    F   F    F   F    F   F    F   F    F   F    F   F    F   F
*/

//
// Constructor
//
VMDebugInfo::VMDebugInfo(const CCharIterator  & oCCharIterator,
                         const UINT_32          iIStringDescr): iStringDescr(iIStringDescr),
                                                                iLine(oCCharIterator.GetLine()),
                                                                iPos(oCCharIterator.GetLinePos())
{
	;;
}

//
// Constructor
//
VMDebugInfo::VMDebugInfo(const UINT_32  iIStringDescr,
                         const UINT_32  iILine,
                         const UINT_32  iIPos): iStringDescr(iIStringDescr),
                                                iLine(iILine),
                                                iPos(iIPos)
{
	;;
}


//
// Constructor
//
VMDebugInfo::VMDebugInfo(const UINT_64  iEncoded): iStringDescr((iEncoded >> 40) & 0x00FFFFFF),
                                                   iLine((iEncoded >> 20)        & 0x000FFFFF),
                                                   iPos(iEncoded                 & 0x000FFFFF)
{
	;;
}

//
// Get encoded debug information
//
UINT_64 VMDebugInfo::GetInfo() const
{
	// Stupid typecast
	UINT_64 iTMP = iStringDescr;

	return ((iTMP   & 0x00FFFFFF) << 40) +
	       ((iLine  & 0x000FFFFF) << 20) +
	       ((iPos   & 0x000FFFFF));
}

//
// Get string ID
//
UINT_32 VMDebugInfo::GetDescrId() const { return iStringDescr; }

//
// Get line
//
UINT_32 VMDebugInfo::GetLine() const    { return iLine;        }

//
// Get line position
//
UINT_32 VMDebugInfo::GetLinePos() const { return iPos;         }

} // namespace CTPP
// End.
