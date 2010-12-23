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
 *      CTPP2Error.hpp
 *
 * $CTPP$
 */
#ifndef _CTPP2_ERROR_H__
#define _CTPP2_ERROR_H__ 1

/**
  @file CTPP2Error.hpp
  @brief CTPP2 error description
*/

#include "CTPP2Types.h"

#include "STLString.hpp"

namespace CTPP // C++ Template Engine
{

/**
  @struct CTPPError CTPPError.hpp <CTPPError.cpp>
  @brief CTPP Error description
*/
struct CTPPError
{
	/** Template name                        */
	STLW::string   template_name;
	/** Human-readable error description     */
	STLW::string   error_descr;
	/** Error code                           */
	UINT_32        error_code;
	/** Line, where error occured            */
	UINT_32        line;
	/** Position in line where error occured */
	UINT_32        pos;
	/** Instruction pointer                  */
	UINT_32        ip;

	/**
	  @brief Constructor
	  @param sTemplateName - name of template
	  @param sErrorDescr - Error description
	  @param iErrrorCode - Error code
	  @param iLine - Line in template, where error occured
	  @param iPos - Position in line, where error occured
	  @param iIP - Instrction pointer
	*/
	CTPPError(const STLW::string  & sTemplateName  = "",
		  const STLW::string  & sErrorDescr    = "",
		  const UINT_32         iErrrorCode    = 0,
		  const UINT_32         iLine          = 0,
		  const UINT_32         iPos           = 0,
		  const UINT_32         iIP            = 0);
};

} // namespace CTPP
#endif // _CTPP2_ERROR_H__
// End.
