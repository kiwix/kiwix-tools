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
 *      FnCast.cpp
 *
 * $CTPP$
 */

#include "CDT.hpp"
#include "CTPP2Logger.hpp"
#include "FnCast.hpp"

#include <stdio.h>

namespace CTPP // C++ Template Engine
{

//
// Constructor
//
FnCast::FnCast()
{
	;;
}

//
// Handler
//
INT_32 FnCast::Handler(CDT            * aArguments,
                       const UINT_32    iArgNum,
                       CDT            & oCDTRetVal,
                       Logger         & oLogger)
{
	// 2 arguments need
	if (iArgNum != 2)
	{
		oLogger.Emerg("Usage: CAST(data, '[sSoOdDiIhHfF]')");
		return -1;
	}

	// Arg 1: destination type
	const STLW::string & sWhat = aArguments[1].GetString();
	if (sWhat.length() == 0)
	{
		oLogger.Error("Last argument should be 's', 'S', 'o', 'O', 'd', 'D', 'i', 'I', 'h', 'H', 'f' or 'F', but is `%s`", sWhat.c_str());
		return -1;
	}

	CDT & oTMP = aArguments[0];
	switch(sWhat[0])
	{
		// S[tring]
		case 'S':
		case 's':
			oCDTRetVal = oTMP.GetString();
			return 0;

		// O[ct[al]]
		case 'o':
		case 'O':
			if      (oTMP.GetType() <= CDT::REAL_VAL)   { oCDTRetVal = oTMP.GetInt(); return 0; }
			else if (oTMP.GetType() == CDT::STRING_VAL)
			{
				unsigned long long iLL = 0;
				sscanf(oTMP.GetString().c_str(), "%llo", &iLL);
				oCDTRetVal = INT_64(iLL);
				return 0;
			}
			oLogger.Error("Invalid source type of first argument. Should be INT, REAL or STRING, but is `%s`", oTMP.PrintableType());
			return -1;

		// D[ec[imal]]
		case 'd':
		case 'D':
			if      (oTMP.GetType() <= CDT::REAL_VAL)   { oCDTRetVal = oTMP.GetInt(); return 0; }
			else if (oTMP.GetType() == CDT::STRING_VAL)
			{
				long long iLL = 0;
				sscanf(oTMP.GetString().c_str(), "%lld", &iLL);
				oCDTRetVal = INT_64(iLL);
				return 0;
			}
			oLogger.Error("Invalid source type of first argument. Should be INT, REAL or STRING, but is `%s`", oTMP.PrintableType());
			return -1;

		// I[nt[eger]]
		case 'i':
		case 'I':
			if      (oTMP.GetType() <= CDT::REAL_VAL)   { oCDTRetVal = oTMP.GetInt(); return 0; }
			else if (oTMP.GetType() == CDT::STRING_VAL)
			{
				long long iLL = 0;
				sscanf(oTMP.GetString().c_str(), "%lli", &iLL);
				oCDTRetVal = INT_64(iLL);
				return 0;
			}
			oLogger.Error("Invalid source type of first argument. Should be INT, REAL or STRING, but is `%s`", oTMP.PrintableType());
			return -1;

		// H[ex[adecimal]]
		case 'H':
		case 'h':
			if      (oTMP.GetType() <= CDT::REAL_VAL)   { oCDTRetVal = oTMP.GetInt(); return 0; }
			else if (oTMP.GetType() == CDT::STRING_VAL)
			{
				unsigned long long iLL = 0;
				sscanf(oTMP.GetString().c_str(), "%llx", &iLL);
				oCDTRetVal = INT_64(iLL);
				return 0;
			}
			oLogger.Error("Invalid source type of first argument. Should be INT, REAL or STRING, but is `%s`", oTMP.PrintableType());
			return -1;

		// F[loat]
		case 'F':
		case 'f':
			if      (oTMP.GetType() <= CDT::REAL_VAL)   { oCDTRetVal = oTMP.GetFloat(); return 0; }
			else if (oTMP.GetType() == CDT::STRING_VAL)
			{
				double dVal = 0;
				sscanf(oTMP.GetString().c_str(), "%lg", &dVal);
				oCDTRetVal = dVal;
				return 0;
			}
			oLogger.Error("Invalid source type of first argument. Should be INT, REAL or STRING, but is `%s`", oTMP.PrintableType());
			return -1;

		// Error
		default:
			oLogger.Error("Last argument should be 's', 'S', 'o', 'O', 'd', 'D', 'i', 'I', 'h', 'H', 'f' or 'F', but is `%s`", sWhat.c_str());
			return -1;
	}
}

//
// Get function name
//
CCHAR_P FnCast::GetName() const { return "cast"; }

//
// A destructor
//
FnCast::~FnCast() throw() { ;; }

} // namespace CTPP
// End.
