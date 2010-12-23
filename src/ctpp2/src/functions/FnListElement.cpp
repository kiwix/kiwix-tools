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
 *      FnListElement.cpp
 *
 * $CTPP$
 */

#include "CDT.hpp"
#include "CTPP2Logger.hpp"
#include "FnListElement.hpp"

namespace CTPP // C++ Template Engine
{

// //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Class FnListElement
//

//
// Constructor
//
FnListElement::FnListElement()
{
	;;
}

//
// Handler
//
INT_32 FnListElement::Handler(CDT            * aArguments,
                              const UINT_32    iArgNum,
                              CDT            & oCDTRetVal,
                              Logger         & oLogger)
{
	if (iArgNum <= 1)
	{
		oLogger.Emerg("Usage: LIST_ELEMENT(data1, data2, ..., element_number); at least 2 arguments need");
		return -1;
	}

	// Element index
	const UINT_32 iLastElement = iArgNum - 1;

	// Element
	const UINT_32 iElement = aArguments[iLastElement].GetInt();

	// Element does not exist?
	if (iElement >= iLastElement) { return -1; }

	// Return element
	oCDTRetVal = aArguments[iLastElement - iElement - 1];

return 0;
}

//
// Get function name
//
CCHAR_P FnListElement::GetName() const { return "list_element"; }

//
// A destructor
//
FnListElement::~FnListElement() throw() { ;; }

} // namespace CTPP
// End.
