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
 *      FnNumeral.cpp
 *
 * $CTPP$
 */

#include "CDT.hpp"
#include "CTPP2Logger.hpp"
#include "FnNumeral.hpp"

namespace CTPP // C++ Template Engine
{

//
// Constructor
//
FnNumeral::FnNumeral()
{
	;;
}

//
// Handler
//
INT_32 FnNumeral::Handler(CDT            * aArguments,
                          const UINT_32    iArgNum,
                          CDT            & oCDTRetVal,
                          Logger         & oLogger)
{
	// At least two arguments need
	if (iArgNum < 2)
	{
		oLogger.Emerg("Usage: NUMERAL(data1, data2, ..., dataN); at least 2 arguments need");
		return -1;
	}

	// Number
	INT_64 iNumber = aArguments[iArgNum - 1].GetInt();

	const STLW::string sLang = aArguments[iArgNum - 2].GetString();
	if (sLang == "ru")
	{
		if (iArgNum != 5)
		{

			oCDTRetVal = "";
			oLogger.Error("For Russian language need 5 arguments: NUMERAL(number, language, first, second-fourth, fifth-20th)");
			return -1;
		}

		INT_32 iN = iNumber % 100;
		if ((iN >= 5 && iN <= 20) || iN == 0)
		{
			oCDTRetVal = aArguments[0];
			return 0;
		}

		iN %= 10;
		switch(iN)
		{
			case 0:
				oCDTRetVal = aArguments[0];
				break;
			case 1:
				oCDTRetVal = aArguments[2];
				break;
			case 2:
			case 3:
			case 4:
				oCDTRetVal = aArguments[1];
				break;
			default:
				oCDTRetVal = aArguments[0];
		}
	}
	else if (sLang == "en")
	{
		if (iArgNum != 4)
		{
			oCDTRetVal = "";
			oLogger.Error("For English language need 4 arguments: NUMERAL(number, language, first, other)");
			return -1;
		}

		if (iNumber == 1) { oCDTRetVal = aArguments[1]; }
		else              { oCDTRetVal = aArguments[0]; }
	}
	else
	{
		oLogger.Error("Language %s not supported yet", sLang.c_str());
		return -1;
	}


return 0;
}

//
// Get function name
//
CCHAR_P FnNumeral::GetName() const { return "numeral"; }

//
// A destructor
//
FnNumeral::~FnNumeral() throw() { ;; }

} // namespace CTPP
// End.
