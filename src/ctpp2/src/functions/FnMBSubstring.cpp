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
 *      FnMBSubstring.cpp
 *
 * $CTPP$
 */

#include "CDT.hpp"
#include "CTPP2Logger.hpp"
#include "CTPP2Util.hpp"
#include "FnMBSubstring.hpp"

namespace CTPP // C++ Template Engine
{

//
// Constructor
//
FnMBSubstring::FnMBSubstring()
{
	;;
}

//
// Handler
//
INT_32 FnMBSubstring::Handler(CDT            * aArguments,
                              const UINT_32    iArgNum,
                              CDT            & oCDTRetVal,
                              Logger         & oLogger)
{
	// Check number of parameters
	if (iArgNum == 0) { oCDTRetVal = ""; return 0; }

	// substr('foobar', 2) -> 'obar'
	if (iArgNum == 2)
	{
		const UINT_32       iOffset = UINT_32(aArguments[0].GetInt());
		const STLW::string  sResult = aArguments[1].GetString();

		CCHAR_P  szStart  = sResult.data();
		CCHAR_P  szEnd    = sResult.data() + sResult.size();
		INT_32   iPos     = 0;
		UINT_32  iCharPos = 0;
		for(;;)
		{
			INT_32 iCharLen = utf_charlen(szStart + iPos, szEnd);

			if (iCharLen == -3) { break; }

			// Check character length
			if (iCharLen < 0) { iCharLen = 1; }
			// Skip errors
			else              { ++iCharPos;   }
			iPos += iCharLen;

			if (iCharPos >= iOffset) { break; }
		}

		if (iCharPos < iOffset) { oCDTRetVal = ""; }
		else                    { oCDTRetVal = sResult.substr(iPos); }

		return 0;
	}
	// substr('foobar', 2, 3) -> 'oba'
	if (iArgNum == 3)
	{
		const UINT_32       iBytes  = UINT_32(aArguments[0].GetInt());
		const UINT_32       iOffset = UINT_32(aArguments[1].GetInt());
		const STLW::string  sResult = aArguments[2].GetString();

		CCHAR_P  szStart  = sResult.data();
		CCHAR_P  szEnd    = sResult.data() + sResult.size();
		INT_32   iPos     = 0;
		UINT_32  iCharOffset = 0;
		UINT_32  iCharPos = 0;
		for(;;)
		{
			INT_32 iCharLen = utf_charlen(szStart + iPos, szEnd);

			if (iCharLen == -3) { break; }

			// Check character length
			if (iCharLen < 0) { iCharLen = 1; }
			// Skip errors
			else              { ++iCharPos;   }
			iPos += iCharLen;

			if (iCharPos == iOffset)          { iCharOffset = iPos; }

			if (iCharPos == iOffset + iBytes) { break; }
		}

		if (sResult.size() < iCharOffset) { oCDTRetVal = ""; }
		else                              { oCDTRetVal = sResult.substr(iCharOffset, iPos - iCharOffset); }

		return 0;
	}
	// substr('foobar', 2, 3, '1234567') -> 'fo1234567r'
	if (iArgNum == 4)
	{
		STLW::string        sReplacement = aArguments[0].GetString();
		const UINT_32       iBytes       = UINT_32(aArguments[1].GetInt());
		const UINT_32       iOffset      = UINT_32(aArguments[2].GetInt());
		const STLW::string  sTMP         = aArguments[3].GetString();

		CCHAR_P  szStart  = sTMP.data();
		CCHAR_P  szEnd    = sTMP.data() + sTMP.size();
		UINT_32  iPos     = 0;
		UINT_32  iCharOffset = 0;
		UINT_32  iCharPos = 0;
		for(;;)
		{
			INT_32 iCharLen = utf_charlen(szStart + iPos, szEnd);

			if (iCharLen == -3) { break; }

			// Check character length
			if (iCharLen < 0) { iCharLen = 1; }
			// Skip errors
			else              { ++iCharPos;   }
			iPos += iCharLen;

			if (iCharPos == iOffset)          { iCharOffset = iPos; }

			if (iCharPos == iOffset + iBytes) { break; }
		}
		if (sTMP.size() < iCharOffset) { oCDTRetVal = ""; return 0; }

		STLW::string  sResult(sTMP, 0, iCharOffset);
		sResult.append(sReplacement);

		if (iPos == sTMP.size()) { oCDTRetVal = sResult; return 0; }

		sResult.append(sTMP, iPos, STLW::string::npos);
		oCDTRetVal = sResult;
		return 0;
	}

	oLogger.Emerg("Usage: MB_SUBSTR(data, offset) or MB_SUBSTR(data, offset, length) or MB_SUBSTR(data, offset, length, replacement)");
return -1;
}

//
// Get function name
//
CCHAR_P FnMBSubstring::GetName() const { return "mb_substr"; }

//
// A destructor
//
FnMBSubstring::~FnMBSubstring() throw() { ;; }

} // namespace CTPP
// End.
