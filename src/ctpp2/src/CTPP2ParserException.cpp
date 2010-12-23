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
 *      CTPP2ParserException.hpp
 *
 * $CTPP$
 */
#include "CTPP2ParserException.hpp"

#include <stdlib.h>
#include <string.h>

namespace CTPP // C++ Template Engine
{

// //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Class CTPPParserException
//

//
// Get string describing the general cause of error
//
CCHAR_P CTPPParserException::what() const throw() { return "Parsing exception"; }

//
// A destructor
//
CTPPParserException::~CTPPParserException() throw() { ;; }

// //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Class CTPPParserSyntaxError
//

//
// Constructor
//
CTPPParserSyntaxError::CTPPParserSyntaxError(CCHAR_P        szIReason,
                                             const UINT_32  iILine,
                                             const UINT_32  iIPos): szReason(strdup(szIReason)),
                                                                    iLine(iILine),
                                                                    iPos(iIPos) { ;; }

//
// Get string describing the general cause of error
//
CCHAR_P CTPPParserSyntaxError::what() const throw() { return szReason; }

//
// Get line where error ocured
//
UINT_32 CTPPParserSyntaxError::GetLine() const { return iLine; }

//
// Get column where error ocured
//
UINT_32 CTPPParserSyntaxError::GetLinePos() const { return iPos; }

//
// A destructor
//
CTPPParserSyntaxError::~CTPPParserSyntaxError() throw() { free(szReason); }

// //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Class CTPPParserOperatorsMismatch
//

//
// Constructor
//
CTPPParserOperatorsMismatch::CTPPParserOperatorsMismatch(CCHAR_P        szIExpected,
                                                         CCHAR_P        szIFound,
                                                         const UINT_32  iILine,
                                                         const UINT_32  iIPos): szExpected(strdup(szIExpected)),
                                                                                szFound(strdup(szIFound)),
                                                                                iLine(iILine),
                                                                                iPos(iIPos) { ;; }

//
// Get string describing the general cause of error
//
CCHAR_P CTPPParserOperatorsMismatch::what() const throw() { return "Found wrong operator"; }

//
// Get name of exprcted operator
//
CCHAR_P CTPPParserOperatorsMismatch::Expected() const { return szExpected; }

//
// Get name of found operator
//
CCHAR_P CTPPParserOperatorsMismatch::Found() const { return szFound; }

//
// Get line where error ocured
//
UINT_32 CTPPParserOperatorsMismatch::GetLine() const { return iLine; }

//
// Get column where error ocured
//
UINT_32 CTPPParserOperatorsMismatch::GetLinePos() const { return iPos; }

//
// A destructor
//
CTPPParserOperatorsMismatch::~CTPPParserOperatorsMismatch() throw()
{
	free(szExpected);
	free(szFound);
}

} // namespace CTPP
// End.
