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
 *      CTPP2JSONParser.hpp
 *
 * $CTPP$
 */
#ifndef _CTPP2_JSON_PARSER_H__
#define _CTPP2_JSON_PARSER_H__ 1

#include "CDT.hpp"
#include "CTPP2CharIterator.hpp"

#include "STLString.hpp"

/**
  @file CTPP2JSONParser.hpp
  @brief JSON Parser
*/

namespace CTPP // C++ Template Engine
{

/**
  @class CTPP2JSONParser CTPP2JSONParser.hpp <CTPP2JSONParser.hpp>
  @brief JSON source code parser
*/
class CTPP2DECL CTPP2JSONParser
{
public:
	/**
	  @brief Constructor
	*/
	CTPP2JSONParser(CDT & oICDT);

	/**
	  @brief Parse JSON data
	  @param szData - start of template
	  @param szEnd - end of template
	  @return 0 if success, -1 if any error occured
	*/
	INT_32 Parse(CCharIterator szData, CCharIterator szEnd);

	/**
	  @brief A destructor
	*/
	~CTPP2JSONParser() throw();
private:
	/** Data collector        */
	CDT           & oCDT;
	/** Temp. buffer          */
	STLW::string    sTMPBuf;
	/** Parsed integer value  */
	INT_64         iIntData;
	/** Parsed floating value */
	W_FLOAT        dFloatData;
	/** Type of parsed value  */
	INT_32         iParsedNumberType;

	/**
	  @brief JSON string "blah-blah \" clah-clah " | 'blah-blah \' clah-clah '
	  @param szData - current stream position
	  @param szEnd - end of stream position
	  @return NULL if sequence not found or pointer to end of sequence
	*/
	CCharIterator IsString(CCharIterator szData, CCharIterator szEnd);

	/**
	  @brief JSON old fashion string blah-blah.clah-clah
	  @param szData - current stream position
	  @param szEnd - end of stream position
	  @return NULL if sequence not found or pointer to end of sequenc
	*/
	CCharIterator IsStringCompatOldVersion(CCharIterator szData, CCharIterator szEnd);

	/**
	  @brief JSON variable a-zA-Z_[a-zA-Z0-9_]
	  @param szData - current stream position
	  @param szEnd - end of stream position
	  @return NULL if sequence not found or pointer to end of sequence
	*/
	CCharIterator IsVar(CCharIterator szData, CCharIterator szEnd);

	/**
	  @brief JSON number [-]0-9[. [0-9] ] [E][+- 0-9]
	  @param szData - current stream position
	  @param szEnd - end of stream position
	  @return NULL if sequence not found or pointer to end of sequence
	*/
	CCharIterator IsNum(CCharIterator szData, CCharIterator szEnd);

	/**
	  @brief JSON white space (Tab | ' ')
	  @param szData - current stream position
	  @param szEnd - end of stream position
	  @param iSkipFirst - if set to 0, does not check existence of first space
	  @return NULL if sequence not found or pointer to end of sequence
	*/
	CCharIterator IsWhiteSpace(CCharIterator szData, CCharIterator szEnd, const UINT_32 & iSkipFirst = 1);

	/**
	  @brief JSON Object
	  @param szData - current stream position
	  @param szEnd - end of stream position
	  @param oCurrentCDT - CDT object
	  @return NULL if sequence not found or pointer to end of sequence
	*/
	CCharIterator IsObject(CCharIterator szData, CCharIterator szEnd, CDT & oCurrentCDT);

	/**
	  @brief JSON Object
	  @param szData - current stream position
	  @param szEnd - end of stream position
	  @param oCurrentCDT - CDT object
	  @return NULL if sequence not found or pointer to end of sequence
	*/
	CCharIterator IsArray(CCharIterator szData, CCharIterator szEnd, CDT & oCurrentCDT);

	/**
	  @brief JSON Key
	  @param szData - current stream position
	  @param szEnd - end of stream position
	  @param sKey - JSON hash key
	  @return NULL if sequence not found or pointer to end of sequence
	*/
	CCharIterator IsKey(CCharIterator szData, CCharIterator szEnd, STLW::string & sKey);

	/**
	  @brief JSON Value
	  @param szData - current stream position
	  @param szEnd - end of stream position
	  @param oCurrentCDT - CDT object
	  @return NULL if sequence not found or pointer to end of sequence
	*/
	CCharIterator IsValue(CCharIterator szData, CCharIterator szEnd, CDT & oCurrentCDT);
};

} // namespace CTPP
#endif // _CTPP2_JSON_PARSER_H__
// End.
