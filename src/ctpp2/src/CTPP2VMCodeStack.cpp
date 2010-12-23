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
 *      CTPP2VMCodeStack.cpp
 *
 * $CTPP$
 */

#include "CTPP2VMCodeStack.hpp"
#include "CTPP2VMStackException.hpp"

namespace CTPP // C++ Template Engine
{

//
// Constructor
//
VMCodeStack::VMCodeStack(const INT_32  iIMaxStackSize): iMaxStackSize(iIMaxStackSize),
                                                        iStackPointer(iIMaxStackSize)
{
	aStack = new UINT_32[iMaxStackSize];
}

//
//  Push address into stack
//
void VMCodeStack::PushAddress(const UINT_32  iAddress)
{
	if (iStackPointer == 0) { throw StackOverflow(0); }

	--iStackPointer;

	aStack[iStackPointer] = iAddress;
}

//
// Remove top stack element
//
UINT_32 VMCodeStack::PopAddress()
{
	if (iStackPointer == iMaxStackSize) { throw StackUnderflow(0); }

return aStack[iStackPointer++];
}

//
// Reset stack of arguments to default state
//
void VMCodeStack::Reset() { iStackPointer = iMaxStackSize; }

//
// A destructor
//
VMCodeStack::~VMCodeStack() throw()
{
#ifdef _DEBUG
fprintf(stderr, "Code Stack(%d): SP %d\n", iMaxStackSize, iStackPointer);
#endif

	delete [] aStack;
}

} // namespace CTPP
// End.
