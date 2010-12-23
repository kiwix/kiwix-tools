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
 *      FnObjDump.cpp
 *
 * $CTPP$
 */

#include "CDT.hpp"
#include "FnObjDump.hpp"

#include <math.h>
#include <stdio.h>

namespace CTPP // C++ Template Engine
{

//
// Constructor
//
FnObjDump::FnObjDump(): pCDT(NULL)
{
	;;
}

//
// Pre-execution handler setup
//
INT_32 FnObjDump::PreExecuteSetup(OutputCollector          & oCollector,
                                  CDT                      & oCDT,
                                  const ReducedStaticText  & oSyscalls,
                                  const ReducedStaticData  & oStaticData,
                                  const ReducedStaticText  & oStaticText,
                                  Logger                   & oLogger)
{
	pCDT = &oCDT;

return 0;
}

//
// Handler
//
INT_32 FnObjDump::Handler(CDT            * aArguments,
                          const UINT_32    iArgNum,
                          CDT            & oCDTRetVal,
                          Logger         & oLogger)
{
	if (iArgNum == 0)
	{
		oCDTRetVal = "Global data object:\n";
		oCDTRetVal.Append(pCDT -> Dump());
	}
	else
	{
		oCDTRetVal = "";
		// Temp
		CHAR_8 szBuffer[CTPP_ESCAPE_BUFFER_LEN + 1];

		INT_32 iI = iArgNum - 1;
		for (; iI >= 0; --iI)
		{
			snprintf(szBuffer, CTPP_ESCAPE_BUFFER_LEN, "Argument %d:\n", iArgNum - iI);
			oCDTRetVal.Append(szBuffer);
			oCDTRetVal.Append(aArguments[iI].Dump());
		}
	}
return 0;
}

//
// Get function name
//
CCHAR_P FnObjDump::GetName() const { return "obj_dump"; }

//
// A destructor
//
FnObjDump::~FnObjDump() throw() { ;; }

} // namespace CTPP
// End.
