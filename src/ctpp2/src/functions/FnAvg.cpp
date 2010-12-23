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
 *      FnAvg.cpp
 *
 * $CTPP$
 */

#include "CDT.hpp"
#include "CTPP2Logger.hpp"
#include "FnAvg.hpp"

#include <math.h>

namespace CTPP // C++ Template Engine
{

//
// Constructor
//
FnAvg::FnAvg()
{
	;;
}

//
// Handler
//
INT_32 FnAvg::Handler(CDT            * aArguments,
                      const UINT_32    iArgNum,
                      CDT            & oCDTRetVal,
                      Logger         & oLogger)
{
	// Need at least 2 args
	if (iArgNum < 2)
	{
		oLogger.Emerg("Usage: AVG(data1, data2, ..., dataN, '[aAgGhHqQ]')");
		return -1;
	}

	// First Arg 1: destination type
	const STLW::string & sWhat = aArguments[iArgNum - 1].GetString();
	if (sWhat.length() == 0)
	{
		oLogger.Error("Last argument should be 'a', 'A', 'g', 'G', 'h', 'H', 'q' or 'Q', but is `%s`", sWhat.c_str());
		return -1;
	}

	// http://en.wikipedia.org/wiki/Average
	switch (sWhat[0])
	{
		// Arithmetic
		case 'A':
		case 'a':
			{
				W_FLOAT  dSum = 0.0;
				for(INT_32 iPos = iArgNum - 2; iPos >= 0 ; --iPos)
				{
					dSum += aArguments[iPos].GetFloat();
				}
				oCDTRetVal = dSum / (iArgNum - 1);
			}
			break;

		// Geometric
		case 'G':
		case 'g':
			{
				W_FLOAT  dSum = 1.0;
				for(INT_32 iPos = iArgNum - 2; iPos >= 0 ; --iPos)
				{
					dSum *= aArguments[iPos].GetFloat();
				}
				oCDTRetVal = pow(dSum, 1.0 / (iArgNum - 1));
			}
			break;

		// Harmonic
		case 'H':
		case 'h':
			{
				W_FLOAT  dSum = 0.0;
				for(INT_32 iPos = iArgNum - 2; iPos >= 0 ; --iPos)
				{
					const W_FLOAT dTMP = aArguments[iPos].GetFloat();
					if (dTMP == 0.0) { dSum = 0; break; }
					dSum += 1.0 / dTMP;
				}
				oCDTRetVal = 1.0 * (iArgNum - 1) / dSum;
			}
			break;

		// Quadratic
		case 'Q':
		case 'q':
			{
				W_FLOAT  dSum = 0.0;
				for(INT_32 iPos = iArgNum - 1; iPos >= 0 ; --iPos)
				{
					const W_FLOAT dTMP = aArguments[iPos].GetFloat();
					dSum += dTMP * dTMP;
				}
				oCDTRetVal = sqrt(dSum / (iArgNum - 1));
			}
			break;

		default:
			oLogger.Error("Last argument should be 'a', 'A', 'g', 'G', 'h', 'H', 'q' or 'Q', but is `%s`", sWhat.c_str());
			return -1;
	}

return 0;
}

//
// Get function name
//
CCHAR_P FnAvg::GetName() const { return "avg"; }

//
// A destructor
//
FnAvg::~FnAvg() throw() { ;; }

} // namespace CTPP
// End.
