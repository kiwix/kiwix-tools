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
 *      CTPP2Exception.cpp
 *
 * $CTPP$
 */
#include "CTPP2Exception.hpp"

namespace CTPP // C++ Template Engine
{

// //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Class CTPPException
//
//

//
// Get human-readable error description
//
CCHAR_P CTPPException::what() const throw() { return "Unknown Error"; }

//
// A destructor
//
CTPPException::~CTPPException() throw() { ;; }

// //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Class CTPPLogicError
//
//

//
// Constrcutor
//
CTPPLogicError::CTPPLogicError(CCHAR_P sIReason) throw(): sReason(strdup(sIReason)) { ;; }

//
// Get human-readable error description
//
CCHAR_P CTPPLogicError::what() const throw() { return sReason; }

//
// A destructor
//
CTPPLogicError::~CTPPLogicError() throw() { free(sReason); }

// //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Class CDTTypeCastException
//
//

//
// Constrcutor
//
CDTTypeCastException::CDTTypeCastException(CCHAR_P sIReason) throw(): sReason(strdup(sIReason)) { ;; }

//
// Get human-readable error description
//
CCHAR_P CDTTypeCastException::what() const throw() { return sReason; }

//
// A destructor
//
CDTTypeCastException::~CDTTypeCastException() throw() { free(sReason); }

// //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Class CDTAccessException
//
//

//
// Get human-readable error description
//
CCHAR_P CDTAccessException::what() const throw() { return "operator []"; }

//
// A destructor
//
CDTAccessException::~CDTAccessException() throw() { ;; }

// //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Class CDTRangeException
//
//

//
// Get human-readable error description
//
CCHAR_P CDTRangeException::what() const throw() { return "pos > size"; }

//
// A destructor
//
CDTRangeException::~CDTRangeException() throw() { ;; }

// //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Class CTPPUnixException
//
//

//
// Constrcutor
//
CTPPUnixException::CTPPUnixException(CCHAR_P sIReason, INT_32 iIErrno) throw(): sReason(strdup(sIReason)), iErrno(iIErrno) { ;; }

//
// Get human-readable error description
//
CCHAR_P CTPPUnixException::what() const throw() { return sReason; }

//
// Get un*x error code
//
INT_32 CTPPUnixException::ErrNo() const throw() { return iErrno; }

//
// A destructor
//
CTPPUnixException::~CTPPUnixException() throw() { free(sReason); }

// //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Class CTPPNullPointerException
//
//

//
// Constrcutor
//
CTPPNullPointerException::CTPPNullPointerException(CCHAR_P sIReason) throw(): sReason(strdup(sIReason)) { ;; }

//
// Get human-readable error description
//
CCHAR_P CTPPNullPointerException::what() const throw() { return sReason; }

//
// A destructor
//
CTPPNullPointerException::~CTPPNullPointerException() throw() { free(sReason); }

// //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Class CTPPCharsetRecodeException
//
//


//
// Constrcutor
//
CTPPCharsetRecodeException::CTPPCharsetRecodeException(CCHAR_P  sISrc,
                                                       CCHAR_P  sIDst) throw(): sSrc(strdup(sISrc)),
                                                                                sDst(strdup(sIDst)) { ;; }

//
// Get human-readable error description
//
CCHAR_P CTPPCharsetRecodeException::what() const throw() { return "Cannot convert from source charset to destination"; }

//
//  Get source charset
//
CCHAR_P CTPPCharsetRecodeException::GetSource() const throw() { return sSrc; }

//
//  Get destination charset
//
CCHAR_P CTPPCharsetRecodeException::GetDestination() const throw() { return sDst; }

//
// A destructor
//
CTPPCharsetRecodeException::~CTPPCharsetRecodeException() throw()
{
	free(sSrc);
	free(sDst);
}

} // namespace CTPP
// End.
