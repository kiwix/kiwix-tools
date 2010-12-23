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
 *      FnHrefParam.cpp
 *
 * $CTPP$
 */

#include "CDT.hpp"
#include "CTPP2Logger.hpp"
#include "CTPP2Util.hpp"
#include "FnHrefParam.hpp"

namespace CTPP // C++ Template Engine
{

//
// Constructor
//
FnHrefParam::FnHrefParam()
{
	;;
}

//
// Handler
//
INT_32 FnHrefParam::Handler(CDT            * aArguments,
                            const UINT_32    iArgNum,
                            CDT            & oCDTRetVal,
                            Logger         & oLogger)
{
	if (iArgNum != 2)
	{
		oLogger.Emerg("Usage: HREF_PARAM(name, value)");
		return -1;
	}

	oCDTRetVal = URLEscape(aArguments[1].GetString());
	oCDTRetVal.Append("=");
	oCDTRetVal.Append(URLEscape(aArguments[0].GetString()));
	oCDTRetVal.Append("&amp;");

return 0;
}

//
// Get function name
//
CCHAR_P FnHrefParam::GetName() const { return "href_param"; }

//
// A destructor
//
FnHrefParam::~FnHrefParam() throw() { ;; }

} // namespace CTPP
// End.
