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
 *      FnRandom.cpp
 *
 * $CTPP$
 */

#include "CDT.hpp"
#include "CTPP2Logger.hpp"
#include "FnRandom.hpp"

#include <stdlib.h>

#ifdef _MSC_VER
	#define random()        rand()
	#define INT_64(x)       (INT_64)(x)
	#define srandomdev()    srand( (unsigned)time(NULL) );
#endif

namespace CTPP // C++ Template Engine
{

//
// Constructor
//
FnRandom::FnRandom()
{
#if defined(__FreeBSD__) || defined(_MSC_VER)
	srandomdev();
#else
	srandom(time(NULL));
#endif
}

//
// Handler
//
INT_32 FnRandom::Handler(CDT            * aArguments,
                         const UINT_32    iArgNum,
                         CDT            & oCDTRetVal,
                         Logger         & oLogger)
{
	// 0 .. MAX(unsigned long)
	if (iArgNum == 0)
	{
		oCDTRetVal = UINT_64(random());
		return 0;
	}
	// RAND(x) -> 0 .. RAND(x)
	else if (iArgNum == 1)
	{
		const CDT & oFirstArg = aArguments[0];
		// Integer value
		if (oFirstArg.GetType() == CDT::INT_VAL)
		{
			oCDTRetVal = INT_64(random() % oFirstArg.GetInt());
			return 0;
		}
		// Floating point value
		else if (oFirstArg.GetType() == CDT::REAL_VAL)
		{
			oCDTRetVal = W_FLOAT(oFirstArg.GetFloat() * (random() % UINT_64(RAND_MAX)) / UINT_64(RAND_MAX));
			return 0;
		}

		// Invalid data type, just return 0
		oCDTRetVal = 0;
		return 0;
	}
	// RAND(x, y) -> x .. y
	else if (iArgNum == 2)
	{
		const CDT & oFirstArg  = aArguments[1];
		const CDT & oSecondArg = aArguments[0];

		// x and y is integer values
		if (oFirstArg.GetType()  == CDT::INT_VAL &&
		    oSecondArg.GetType() == CDT::INT_VAL)
		{
			const UINT_64 iFirst    = oFirstArg.GetInt();
			const UINT_64 iInterval = oSecondArg.GetInt() - iFirst;

			oCDTRetVal = INT_64(random() % iInterval + iFirst);
			return 0;
		}
		// x or y (or both) are floating point value
		else if ((oFirstArg.GetType() == CDT::REAL_VAL && oSecondArg.GetType() == CDT::INT_VAL)  ||
		         (oFirstArg.GetType() == CDT::INT_VAL  && oSecondArg.GetType() == CDT::REAL_VAL) ||
		         (oFirstArg.GetType() == CDT::REAL_VAL && oSecondArg.GetType() == CDT::REAL_VAL))
		{
			const W_FLOAT dFirst    = oFirstArg.GetFloat();
			const W_FLOAT dInterval = oSecondArg.GetFloat() - dFirst;

			oCDTRetVal = W_FLOAT(dInterval * (random() % UINT_64(RAND_MAX)) / UINT_64(RAND_MAX) + dFirst);
			return 0;
		}

		// Invalid data type, just return 0
		oCDTRetVal = 0;
		return 0;
	}

	oLogger.Emerg("Usage: RANDOM() or RANDOM(value) or RANDOM(val_start, val_end)");
return -1;
}

//
// Get function name
//
CCHAR_P FnRandom::GetName() const { return "random"; }

//
// A destructor
//
FnRandom::~FnRandom() throw() { ;; }

} // namespace CTPP
// End.
