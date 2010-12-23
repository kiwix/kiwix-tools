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
#ifndef _CTPP2_PARSER_EXCEPTION_H__
#define _CTPP2_PARSER_EXCEPTION_H__ 1

#include "CTPP2Exception.hpp"

/**
  @file CTPP2ParserException.hpp
  @brief Collector of generated operation codes
*/

namespace CTPP // C++ Template Engine
{

/**
  @class CTPPParserException CTPP2Parser.hpp <CTPP2Parser.hpp>
  @brief Base CTPP parser exception
*/
class CTPP2DECL CTPPParserException:
  public CTPPException
{
public:
	/**
	  @return A asciz string describing the general cause of error
	*/
	virtual CCHAR_P what() const throw();

	/**
	  @brief Get line where error ocured
	*/
	virtual UINT_32 GetLine() const = 0;

	/**
	  @brief Get column where error ocured
	*/
	virtual UINT_32 GetLinePos() const = 0;

	/**
	  @brief A virtual destructor
	*/
	virtual ~CTPPParserException() throw();
};

/**
  @class CTPPParserSyntaxError CTPP2Parser.hpp <CTPP2Parser.hpp>
  @brief Base CTPP syntax error
*/
class CTPP2DECL CTPPParserSyntaxError:
  public CTPPParserException
{
public:
	/**
	  @brief Constructor
	  @param szIReason - error description
	  @param iILine - line where error ocured
	  @param iIPos - column where error ocured
	*/
	CTPPParserSyntaxError(CCHAR_P        szIReason,
	                      const UINT_32  iILine,
	                      const UINT_32  iIPos);

	/** @return A asciz string describing the general cause of error */
	CCHAR_P what() const throw();

	/**
	  @brief Get line where error ocured
	*/
	UINT_32 GetLine() const;

	/**
	  @brief Get column where error ocured
	*/
	UINT_32 GetLinePos() const;

	/** @brief A virtual destructor */
	~CTPPParserSyntaxError() throw();
private:
	/** Error description */
	CHAR_P             szReason;
	/** Line              */
	const UINT_32      iLine;
	/** Position in line  */
	const UINT_32      iPos;
};

/**
  @class CTPPParserOperatorsMismatch CTPP2Parser.hpp <CTPP2Parser.hpp>
  @brief CTPP operators mismatch; expected one, found other
*/
class CTPP2DECL CTPPParserOperatorsMismatch:
  public CTPPParserException
{
public:
	/**
	  @brief Constructor
	  @param szIExpected - expected opeartor
	  @param szIFound - opeartor that really found in stream
	  @param iILine - line where error ocured
	  @param iIPos - column where error ocured
	*/
	CTPPParserOperatorsMismatch(CCHAR_P        szIExpected,
	                            CCHAR_P        szIFound,
	                            const UINT_32  iILine,
	                            const UINT_32  iIPos);

	/** @return A asciz string describing the general cause of error */
	CCHAR_P what() const throw();

	/** @return Asciz name of expected operator  */
	CCHAR_P Expected() const;

	/** @return Asciz name of found operator */
	CCHAR_P Found() const;

	/**
	  @brief Get line where error ocured
	*/
	UINT_32 GetLine() const;

	/**
	  @brief Get column where error ocured
	*/
	UINT_32 GetLinePos() const;

	/** @brief A virtual destructor */
	~CTPPParserOperatorsMismatch() throw();

private:
	/** Expected operator */
	CHAR_P            szExpected;
	/** Found operator */
	CHAR_P            szFound;

	/** Line              */
	const UINT_32      iLine;
	/** Position in line  */
	const UINT_32      iPos;
};

} // namespace CTPP
#endif // _CTPP2_PARSER_EXCEPTION_H__
// End.
