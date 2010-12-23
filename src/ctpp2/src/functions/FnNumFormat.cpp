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
 *      FnVersion.cpp
 *
 * $CTPP$
 */

#include "CDT.hpp"
#include "CTPP2Logger.hpp"
#include "FnNumFormat.hpp"

#include <stdio.h>

namespace CTPP // C++ Template Engine
{

//
// Constructor
//
FnNumFormat::FnNumFormat()
{
	;;
}

//
// Handler
//
INT_32 FnNumFormat::Handler(CDT            * aArguments,
                            const UINT_32    iArgNum,
                            CDT            & oCDTRetVal,
                            Logger         & oLogger)
{
	// Only 2 args allowed
	if (iArgNum != 2)
	{
		oLogger.Emerg("Usage: NUM_FORMAT(data, format)");
		return -1;
	}

	STLW::string sTMP = aArguments[0].GetString();

	INT_64   iData;
	W_FLOAT  dData;
	CDT::eValType eType = aArguments[1].CastToNumber(iData, dData);

	// Integer
	if (eType == CDT::INT_VAL)
	{
		CHAR_8 szRes[C_MAX_SPRINTF_LENGTH + 1];
		CHAR_8 szBuf[C_MAX_SPRINTF_LENGTH + 1];

		szRes[C_MAX_SPRINTF_LENGTH] = '\0';

		INT_32 iNegative = 0;
		if (iData < 0) { iData = -iData; iNegative = 1; }

		INT_32 iCharacters = snprintf(szBuf, C_MAX_SPRINTF_LENGTH, "%lld", (long long)iData);
		INT_32 iResPos = C_MAX_SPRINTF_LENGTH - 1;

		INT_32 iPos      = 0;
		while (iCharacters >= 0)
		{
			szRes[iResPos] = szBuf[iCharacters];
			if (iPos++ % 3 == 0 && iPos != 1) { szRes[--iResPos] = sTMP[0]; }
			--iCharacters;
			--iResPos;
		}

		++iResPos;
		if (iPos % 3 == 1) { ++iResPos; }

		if (iNegative == 1) { szRes[--iResPos] = '-'; }

		oCDTRetVal = &szRes[iResPos];
	}
	// IEEE
	else if (eType == CDT::REAL_VAL)
	{
		CHAR_8 szRes[C_MAX_SPRINTF_LENGTH + 1];
		CHAR_8 szBuf[C_MAX_SPRINTF_LENGTH + 1];

		szRes[C_MAX_SPRINTF_LENGTH] = '\0';

		INT_32 iNegative = 0;
		if (dData < 0) { dData = -dData; iNegative = 1; }

		INT_32 iCharacters = snprintf(szBuf, C_MAX_SPRINTF_LENGTH, "%.*G", CTPP_FLOAT_PRECISION, dData);
		INT_32 iResPos = C_MAX_SPRINTF_LENGTH - 1;

		INT_32 iTMP = iCharacters;
		iCharacters = 0;
		while (iCharacters < iTMP && szBuf[iCharacters++] != '.') { ;; }

		while (iTMP >= iCharacters)
		{
			szRes[iResPos] = szBuf[iTMP];
			--iTMP;
			--iResPos;
		}

		if (szBuf[iTMP] == '.') { iCharacters = iTMP; }

		INT_32 iPos = 0;
		while (iCharacters >= 0)
		{
			szRes[iResPos] = szBuf[iCharacters];
			if (iPos++ % 3 == 0 && iPos != 1) { szRes[--iResPos] = sTMP[0]; }
			--iCharacters;
			--iResPos;
		}

		++iResPos;
		if (iPos % 3 == 1) { ++iResPos; }

		if (iNegative == 1) { szRes[--iResPos] = '-'; }

		oCDTRetVal = &szRes[iResPos];
	}
	// All others
	else
	{
		oCDTRetVal = aArguments[1].GetString();
	}
//	INT_64 iIntData = aArguments[1].GetInt();
//	INT_32 iNegative = 0;
//	if (iIntData < 0) { iIntData = -iIntData; iNegative = 1; }
	
/*
	switch (aArguments[1].GetType())
	{
		// Integer value
		case CDT::INT_VAL:
			{
				CHAR_8 szRes[C_MAX_SPRINTF_LENGTH + 1];
				CHAR_8 szBuf[C_MAX_SPRINTF_LENGTH + 1];

				szRes[C_MAX_SPRINTF_LENGTH] = '\0';

				INT_64 iIntData = aArguments[1].GetInt();

				INT_32 iNegative = 0;
				if (iIntData < 0) { iIntData = -iIntData; iNegative = 1; }

				INT_32 iCharacters = snprintf(szBuf, C_MAX_SPRINTF_LENGTH, "%lld", (long long)iIntData);
				INT_32 iResPos = C_MAX_SPRINTF_LENGTH - 1;

				INT_32 iPos      = 0;
				while (iCharacters >= 0)
				{
					szRes[iResPos] = szBuf[iCharacters];
					if (iPos++ % 3 == 0 && iPos != 1) { szRes[--iResPos] = sTMP[0]; }
					--iCharacters;
					--iResPos;
				}

				++iResPos;
				if (iPos % 3 == 1) { ++iResPos; }

				if (iNegative == 1) { szRes[--iResPos] = '-'; }

				oCDTRetVal = &szRes[iResPos];
			}
			break;

		// Floating point value
		case CDT::REAL_VAL:
			{
				CHAR_8 szRes[C_MAX_SPRINTF_LENGTH + 1];
				CHAR_8 szBuf[C_MAX_SPRINTF_LENGTH + 1];

				szRes[C_MAX_SPRINTF_LENGTH] = '\0';

				W_FLOAT dFloatData = aArguments[1].GetFloat();

				INT_32 iNegative = 0;
				if (dFloatData < 0) { dFloatData = -dFloatData; iNegative = 1; }

				INT_32 iCharacters = snprintf(szBuf, C_MAX_SPRINTF_LENGTH, "%.*G", CTPP_FLOAT_PRECISION, dFloatData);
				INT_32 iResPos = C_MAX_SPRINTF_LENGTH - 1;

				INT_32 iTMP = iCharacters;
				iCharacters = 0;
				while (iCharacters < iTMP && szBuf[iCharacters++] != '.') { ;; }

				while (iTMP >= iCharacters)
				{
					szRes[iResPos] = szBuf[iTMP];
					--iTMP;
					--iResPos;
				}

				if (szBuf[iTMP] == '.') { iCharacters = iTMP; }

				INT_32 iPos = 0;
				while (iCharacters >= 0)
				{
					szRes[iResPos] = szBuf[iCharacters];
					if (iPos++ % 3 == 0 && iPos != 1) { szRes[--iResPos] = sTMP[0]; }
					--iCharacters;
					--iResPos;
				}

				++iResPos;
				if (iPos % 3 == 1) { ++iResPos; }

				if (iNegative == 1) { szRes[--iResPos] = '-'; }

				oCDTRetVal = &szRes[iResPos];
			}
			break;

		// All other types
		default:
			oCDTRetVal = aArguments[1].GetString();
	}
*/
return 0;
}

//
// Get function name
//
CCHAR_P FnNumFormat::GetName() const { return "num_format"; }

//
// A destructor
//
FnNumFormat::~FnNumFormat() throw() { ;; }

} // namespace CTPP
// End.
