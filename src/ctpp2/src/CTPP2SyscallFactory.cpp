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
 *      CTPP2SyscallFactory.cpp
 *
 * $CTPP$
 */
#include "CTPP2SyscallFactory.hpp"

namespace CTPP // C++ Template Engine
{

//
// Constructor
//
SyscallFactory::SyscallFactory(const UINT_32  iIMaxHandlers): iMaxHandlers(iIMaxHandlers), iCurrHandlers(0)
{
	aHandlers = new SyscallHandler * [iMaxHandlers];
	for (UINT_32 iI = 0; iI < iMaxHandlers; ++iI) { aHandlers[iI] = NULL; }
}

//
// Get Handler by ID
//
SyscallHandler * SyscallFactory::GetHandlerById(const UINT_32  iHandlerId) const
{
	if (iHandlerId < iCurrHandlers) { return aHandlers[iHandlerId]; }

return NULL;
}

//
// Get Handler by name
//
SyscallHandler * SyscallFactory::GetHandlerByName(CCHAR_P szHandlerName) const
{
	STLW::map<STLW::string, UINT_32, HandlerRefsSort>::const_iterator itmHandlerRefs = mHandlerRefs.find(szHandlerName);

	if (itmHandlerRefs == mHandlerRefs.end()) { return NULL; }

return aHandlers[itmHandlerRefs -> second];
}

//
// Register handler
//
INT_32 SyscallFactory::RegisterHandler(SyscallHandler * pHandler)
{
	if (iCurrHandlers == iMaxHandlers || pHandler == NULL) { return -1; }

	aHandlers[iCurrHandlers] = pHandler;

	mHandlerRefs.insert(STLW::pair<STLW::string, UINT_32>(pHandler -> GetName(), iCurrHandlers));

return iCurrHandlers++;
}

//
// Remove handler from factory
//
INT_32 SyscallFactory::RemoveHandler(CCHAR_P szHandlerName)
{
	STLW::map<STLW::string, UINT_32, HandlerRefsSort>::iterator itmHandlerRefs = mHandlerRefs.find(szHandlerName);

	if (itmHandlerRefs == mHandlerRefs.end()) { return -1; }

	aHandlers[itmHandlerRefs -> second] = NULL;

	mHandlerRefs.erase(itmHandlerRefs);

return 0;
}
//
// A destructor
//
SyscallFactory::~SyscallFactory() throw()
{
	delete [] aHandlers;
}

} // namespace CTPP
// End.
