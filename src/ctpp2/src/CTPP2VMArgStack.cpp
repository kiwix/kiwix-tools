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
 *      CZTPP2VMArgStack.cpp
 *
 * $CTPP$
 */

#include "CTPP2VMArgStack.hpp"
#include "CTPP2VMStackException.hpp"

namespace CTPP // C++ Template Engine
{

//
// Constructor
//
VMArgStack::VMArgStack(const INT_32  iIMaxStackSize): iMaxStackSize(iIMaxStackSize),
                                                      iStackPointer(iIMaxStackSize)
//                                                    ,iBasePointer(iIMaxStackSize)
{
	aStack = new CDT[iMaxStackSize];
}

//
// Get stack usage factor
//
INT_32 VMArgStack::GetSize() const { return iStackPointer; }

//
// Push element into stack
//
INT_32 VMArgStack::PushElement(const CDT & oCDT)
{
	if (iStackPointer == 0) { throw StackOverflow(0); }

	--iStackPointer;

	aStack[iStackPointer] = oCDT;

return iStackPointer;
}

//
// Remove top stack element
//
INT_32 VMArgStack::PopElement()
{
	if (iStackPointer == iMaxStackSize) { throw StackUnderflow(0); }

	++iStackPointer;

return iStackPointer;
}

//
// Get element from specified position
//
CDT & VMArgStack::GetElement(const INT_32  iPos)
{
	if (iPos >= iMaxStackSize) { throw StackUnderflow(0); }

	if (iPos <  iStackPointer)  { throw StackOverflow(0); }

return aStack[iPos];
}

//
// Clear stack on specified depth
//
INT_32 VMArgStack::ClearStack(const INT_32  iDepth)
{
	INT_32 iNewSP = iStackPointer + iDepth;

	if (iNewSP > iMaxStackSize) { throw StackUnderflow(0); }

	iStackPointer = iNewSP;

return iNewSP;
}

//
// Reset stack of arguments to default state
//
void VMArgStack::Reset() { iStackPointer = iMaxStackSize; }

//
// A destructor
//
VMArgStack::~VMArgStack() throw()
{
#ifdef _DEBUG
fprintf(stderr, "Arg Stack(%d): SP %d\n", iMaxStackSize, iStackPointer);
#endif
	delete [] aStack;
}

} // namespace CTPP
// End.
