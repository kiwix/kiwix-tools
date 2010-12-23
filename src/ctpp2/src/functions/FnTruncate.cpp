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
 *      FnTruncate.cpp
 *
 * $CTPP$
 */

#include "CDT.hpp"
#include "CTPP2Logger.hpp"
#include "FnTruncate.hpp"

namespace CTPP // C++ Template Engine
{

//
// Constructor
//
FnTruncate::FnTruncate()
{
	;;
}

//
// Handler
//
INT_32 FnTruncate::Handler(CDT            * aArguments,
                           const UINT_32    iArgNum,
                           CDT            & oCDTRetVal,
                           Logger         & oLogger)
{
	if (iArgNum == 2)
	{
		const UINT_32       iMaxLen = UINT_32(aArguments[0].GetInt());
		const STLW::string   sData   = aArguments[1].GetString();

		if (sData.size() > iMaxLen) { oCDTRetVal = STLW::string(sData, 0, iMaxLen); }
		else                        { oCDTRetVal = sData;                          }

		return 0;
	}
	else if (iArgNum == 3)
	{
		const UINT_32  iMaxLen = UINT_32(aArguments[1].GetInt());
		STLW::string    sData   = aArguments[2].GetString();

		if (sData.size() > iMaxLen)
		{
			sData = STLW::string(sData, 0, iMaxLen);
			sData.append(aArguments[0].GetString());
		}

		oCDTRetVal = sData;
		return 0;
	}

	oLogger.Emerg("Usage: TRUNCATE(data, offset) or TRUNCATE(data, offset, add_on)");
return -1;
}

//
// Get function name
//
CCHAR_P FnTruncate::GetName() const { return "truncate"; }

//
// A destructor
//
FnTruncate::~FnTruncate() throw() { ;; }

} // namespace CTPP
// End.
