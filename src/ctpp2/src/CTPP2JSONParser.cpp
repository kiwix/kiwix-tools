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
 *      CTPP2JSONParser.cpp
 *
 * $CTPP$
 */

#include "CTPP2JSONParser.hpp"

#include "CTPP2ParserException.hpp"

#include <stdio.h>

#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif

#ifdef HAVE_STRING_H
#include <string.h>
#endif

namespace CTPP // C++ Template Engine
{

//
// Constructor
//
CTPP2JSONParser::CTPP2JSONParser(CDT & oICDT): oCDT(oICDT)
{
	;;
}

//
// CTPP2 string "blah-blah \" clah-clah " | 'blah-blah \' clah-clah '
//
CCharIterator CTPP2JSONParser::IsString(CCharIterator szData, CCharIterator szEnd)
{
	sTMPBuf.erase();

	INT_32 bEscaped = 0;
	CHAR_8 chEOS    = *szData;
	// Check start of string
	if (chEOS != '"' && chEOS != '\'') { return NULL; }

	++szData;
	if (szData == szEnd) { throw CTPPParserSyntaxError("expected terminating character but end of JSON object found", szData.GetLine(), szData.GetLinePos()); }

	while (szData != szEnd)
	{
		// Escaper
		if (*szData == '\\' && bEscaped == 0)
		{
			++szData;
			bEscaped = 1;
		}
		// Other cases
		else
		{
			// End-of-String ?
			if (*szData == chEOS && bEscaped == 0) { return ++szData; }
			if (bEscaped == 1)
			{
				switch (*szData)
				{
					// man ascii
					case 'a': sTMPBuf += '\a'; ++szData; break;
					case 'b': sTMPBuf += '\b'; ++szData; break;
					case 't': sTMPBuf += '\t'; ++szData; break;
					case 'n': sTMPBuf += '\n'; ++szData; break;
					case 'v': sTMPBuf += '\v'; ++szData; break;
					case 'f': sTMPBuf += '\f'; ++szData; break;
					case 'r': sTMPBuf += '\r'; ++szData; break;
					// Unicode + 4 digits
					case 'u':
					{
						++szData;
						UINT_32 iTMP = 0;
						for (INT_32 iI = 3; iI >= 0; --iI)
						{
							if (szData == szEnd) { throw CTPPParserSyntaxError("invalid unicode escape sequence", szData.GetLine(), szData.GetLinePos()); }

							UCHAR_8 iCh = *szData;

							if (iCh >= '0' && iCh <= '9') { iTMP += ((iCh - '0') << (iI * 4)); }
							else
							{
								iCh |= 0x20;
								if (*szData >= 'a' && *szData <= 'f') { iTMP += ((iCh - 'a' + 10) << (iI * 4)); }
								else                                  { throw CTPPParserSyntaxError("invalid unicode escape sequence", szData.GetLine(), szData.GetLinePos()); }
							}

							++szData;
						}
						UCHAR_8 iCh = ((iTMP & 0x0000FF00) >> 8);
						if (iCh != 0) { sTMPBuf += iCh; }

						iCh = (iTMP & 0x000000FF);
						sTMPBuf += iCh;
					}
					break;
					default:
						sTMPBuf += *szData;
						++szData;
				}
			}
			else
			{
				sTMPBuf += *szData;
				++szData;
			}

			bEscaped = 0;
		}
	}

	// Expected ' or "
	if (szData == szEnd) { throw CTPPParserSyntaxError("expected terminating character but end of JSON object found", szData.GetLine(), szData.GetLinePos()); }

// This should *never* happened
return NULL;
}

//
// JSON old fashion string blah-blah.clah-clah
//
CCharIterator CTPP2JSONParser::IsStringCompatOldVersion(CCharIterator szData, CCharIterator szEnd)
{
	sTMPBuf.erase();
	if (szData == szEnd) { return NULL; }
	while (szData != szEnd)
	{
		if (!((*szData >= 'A' && *szData <= 'Z') ||
		      (*szData >= 'a' && *szData <= 'z'))) { return szData; }

		sTMPBuf += *szData;
		szData++;
	}

return NULL;
}

//
// JSON variable [a-zA-Z_][a-zA-Z0-9_]
//
CCharIterator CTPP2JSONParser::IsVar(CCharIterator szData, CCharIterator szEnd)
{
	if (szData != szEnd &&
	    !((*szData >= 'a' && *szData <= 'z') ||
	      (*szData >= 'A' && *szData <= 'Z') || *szData == '_')
	    )
	{
		return NULL;
	}
	++szData;

	while (szData != szEnd)
	{
		if (!((*szData >= 'a' && *szData <= 'z') ||
		      (*szData >= 'A' && *szData <= 'Z') ||
		      (*szData >= '0' && *szData <= '9') ||
		       *szData == '_')
		    )
		{
			return szData;
		}
		++szData;
	}
return NULL;
}

//
// JSON number [-]0-9[. [0-9] ] [E][+- 0-9]
//
CCharIterator CTPP2JSONParser::IsNum(CCharIterator szData, CCharIterator szEnd)
{
	CCharIterator szStart = szData;

	// First character: digit or sign
	if (!((*szData >= '0' && *szData <= '9') || *szData == '-' || *szData == '+')) { return NULL; }
	++szData;

	// Digits
	while (szData != szEnd)
	{
		if (!(*szData >= '0' && *szData <= '9')) { break; }
		++ szData;
	}

	// Optional '.'. If not found, this is an integer value
	if (szData == szEnd || *szData != '.')
	{
		INT_32 iDigitSize = szEnd() - szStart();
		CHAR_P szDigit = new CHAR_8[iDigitSize + 1];
		memcpy(szDigit, szStart(), iDigitSize);
		szDigit[iDigitSize] = '\0';

		long long iLL = 0;
		sscanf(szDigit, "%lli", &iLL);

		iIntData = iLL;
		iParsedNumberType = 0;

		delete [] szDigit;
		return szData;
	}
	++szData;

	// Digits after point
	while (szData != szEnd)
	{
		if (!(*szData >= '0' && *szData <= '9')) { break; }
		++ szData;
	}

	// Scientific?
	if (szData == szEnd || (*szData != 'E' && *szData != 'e'))
	{
		INT_32 iDigitSize = szEnd() - szStart();
		CHAR_P szDigit = new CHAR_8[iDigitSize + 1];
		memcpy(szDigit, szStart(), iDigitSize);
		szDigit[iDigitSize] = '\0';

		double iFF = 0;
		sscanf(szDigit, "%lg", &iFF);

		dFloatData = iFF;
		iParsedNumberType = 1;

		delete [] szDigit;
		return szData;
	}
	else
	{
		INT_32 iFoundDigits = 0;
		++ szData;
		// Error?
		if (szData == szEnd) { throw CTPPParserSyntaxError("exponent has no digits", szData.GetLine(), szData.GetLinePos()); }
		// Exponent sign
		if (*szData == '-' || *szData == '+') { ++ szData; }
		// Error?
		if (szData == szEnd) { throw CTPPParserSyntaxError("exponent has no digits", szData.GetLine(), szData.GetLinePos()); }
		// Exponent value
 		while (szData != szEnd)
		{
			if (!(*szData >= '0' && *szData <= '9')) { break; }
			iFoundDigits = 1;
			++ szData;
		}
		if (iFoundDigits == 0) { throw CTPPParserSyntaxError("exponent has no digits", szData.GetLine(), szData.GetLinePos()); }

		INT_32 iDigitSize = szEnd() - szStart();
		CHAR_P szDigit = new CHAR_8[iDigitSize + 1];
		memcpy(szDigit, szStart(), iDigitSize);
		szDigit[iDigitSize] = '\0';

		double iFF = 0;
		sscanf(szDigit, "%le", &iFF);

		dFloatData = iFF;
		iParsedNumberType = 1;

		delete [] szDigit;
		return szData;
	}

return NULL;
}

//
// JSON white space
//
CCharIterator CTPP2JSONParser::IsWhiteSpace(CCharIterator szData, CCharIterator szEnd, const UINT_32 & iSkipFirst)
{
	// Special case
	if (iSkipFirst == 1)
	{
		if (szData != szEnd && (*szData != ' ' && *szData != '\t' && *szData != '\r' && *szData != '\n')) { return NULL; }
		++szData;
	}

	for (;;)
	{
		// Skip spaces
		while (szData != szEnd)
		{
			if (*szData != ' ' && *szData != '\t' && *szData != '\r' && *szData != '\n') { break; }
			++szData;
		}

		// Check comments: "//" or "/* ... */"
		if (szData == szEnd || *szData != '/') { return szData; }

		CCharIterator szTMP = szData;

		++szData;
		if (szData == szEnd) { return szTMP; }

		// Long comment
		if (*szData == '*')
		{
			++szData;
			// Skip data
			while (szData != szEnd)
			{
				if (*szData == '*')
				{
					++szData;
					if      (szData == szEnd) { return szData;   }
					else if (*szData == '/')  { ++szData; break; }
				}
				++szData;
			}
		}
		// One-line comment
		else if (*szData == '/')
		{
			++szData;
			// Skip data
			while (szData != szEnd)
			{
				if (*szData == '\n') { break; }
				++szData;
			}
		}
		// Not a comment
		else { return szTMP; }
	}

return szData;
}

//
// JSON Object
//
CCharIterator CTPP2JSONParser::IsObject(CCharIterator szData, CCharIterator szEnd, CDT & oCurrentCDT)
{
	if (*szData() != '{') { return NULL; }
	++szData;

	oCurrentCDT   = CDT(CDT::HASH_VAL);

	// Parse array
	CCharIterator  sTMP     = szData;
	bool           bIsFirst = true;
	for(;;)
	{
		// Skip white space
		szData = IsWhiteSpace(szData, szEnd, 0);
		// Unexpected EOD
		if (szData == szEnd)
		{
			// "foo" : [ EOD
			if (bIsFirst) { throw CTPPParserSyntaxError("expected key after '{', but end of JSON object found", szData.GetLine(), szData.GetLinePos()); }

			// "foo" : [ 1, EOD
			throw CTPPParserSyntaxError("expected key after ',', but end of JSON object found", szData.GetLine(), szData.GetLinePos());
		}

		// Key
		STLW::string sKey;
		// Value
		CDT         oCDTValue;

		// Check key
		sTMP = IsKey(szData, szEnd, sKey);
		// Error?
		if (sTMP == NULL)
		{
			if (!bIsFirst) { throw CTPPParserSyntaxError("expected key after ','", szData.GetLine(), szData.GetLinePos()); }
			// Skip white space
			sTMP = IsWhiteSpace(szData, szEnd, 0);
			// Check empty hash
			if (*sTMP == '}') { ++sTMP; break; }

			throw CTPPParserSyntaxError("expected key or '}'", szData.GetLine(), szData.GetLinePos());
		}

		// Skip white space
		sTMP = IsWhiteSpace(sTMP, szEnd, 0);
		if (sTMP == szEnd) { throw CTPPParserSyntaxError("expected ':', but end of JSON object found", sTMP.GetLine(), sTMP.GetLinePos()); }

		// Delimiter
		if (*sTMP != ':') { throw CTPPParserSyntaxError("expected ':' after key", sTMP.GetLine(), sTMP.GetLinePos()); }
		++sTMP;

		// Skip white space
		sTMP = IsWhiteSpace(sTMP, szEnd, 0);
		if (sTMP == szEnd) { throw CTPPParserSyntaxError("expected value after ':', but end of JSON object found", sTMP.GetLine(), sTMP.GetLinePos()); }

		szData = sTMP;
		// Value
		sTMP = IsValue(szData, szEnd, oCDTValue);
		// Error?
		if (sTMP == NULL) { throw CTPPParserSyntaxError("expected value after ':'", szData.GetLine(), szData.GetLinePos()); }

		// Skip white space
		sTMP = IsWhiteSpace(sTMP, szEnd, 0);
		// Unexpected EOF
		if (sTMP == szEnd) { throw CTPPParserSyntaxError("expected ',' or '}', but end of JSON object found", szData.GetLine(), szData.GetLinePos()); }

		// Store object
		oCurrentCDT[sKey] = oCDTValue;

		// End of object
		if (*sTMP == '}') { ++sTMP; break; }
		// Next key : value pair?
		if (*sTMP != ',') { throw CTPPParserSyntaxError("',' expected", sTMP.GetLine(), sTMP.GetLinePos()); }

		bIsFirst = false;
		++sTMP;
		szData = sTMP;
	}

return sTMP;
}

//
// JSON Key
//
CCharIterator CTPP2JSONParser::IsKey(CCharIterator szData, CCharIterator szEnd, STLW::string & sKey)
{
	// Check key
	CCharIterator sTMP = IsString(szData, szEnd);
	// String key found, ok
	if (sTMP != NULL)
	{
		sKey.assign(sTMPBuf);
		return sTMP;
	}

	// Check number
	sTMP = szData;
	szData = IsNum(szData, szEnd);
	if (szData != NULL)
	{
		sKey.assign(sTMP(), szData() - sTMP());
		return szData;
	}

return NULL;
}

//
// JSON Array
//
CCharIterator CTPP2JSONParser::IsArray(CCharIterator szData, CCharIterator szEnd, CDT & oCurrentCDT)
{
	if (*szData() != '[') { return NULL; }
	++szData;

	oCurrentCDT   = CDT(CDT::ARRAY_VAL);

	// Parse array
	CCharIterator  sTMP     = szData;
	UINT_32        iArrayIndex = 0;
	bool           bIsFirst    = true;
	for(;;)
	{
		// Skip white space
		szData = IsWhiteSpace(szData, szEnd, 0);
		// Unexpected EOD
		if (szData == szEnd)
		{
			// "foo" : [ EOD
			if (bIsFirst) { throw CTPPParserSyntaxError("expected value or ']' after '[', but end of JSON object found", szData.GetLine(), szData.GetLinePos()); }

			// "foo" : [ 1, EOD
			throw CTPPParserSyntaxError("expected value after ',', but end of JSON object found", szData.GetLine(), szData.GetLinePos());
		}

		// Value
		CDT oCDTValue;
		// Value
		sTMP = IsValue(szData, szEnd, oCDTValue);
		// Error?
		if (sTMP == NULL)
		{
			// Check number of iteration
			if (!bIsFirst) { throw CTPPParserSyntaxError("expected value after ','", szData.GetLine(), szData.GetLinePos()); }

			// Skip white space
			sTMP = IsWhiteSpace(szData, szEnd, 0);
			// Check empty array
			if (*sTMP == ']') { ++sTMP; break; }

			throw CTPPParserSyntaxError("expected value or ']'", szData.GetLine(), szData.GetLinePos());
		}

		// Skip white space
		sTMP = IsWhiteSpace(sTMP, szEnd, 0);
		// Unexpected EOF
		if (sTMP == szEnd)
		{
			// "foo" : [ EOD
			// "foo" : [ 1, EOD
			throw CTPPParserSyntaxError("expected ',' or ']', but end of JSON object found", sTMP.GetLine(), sTMP.GetLinePos());
		}

		// Store object
		oCurrentCDT[iArrayIndex] = oCDTValue;
		// End of array?
		if (*sTMP == ']') { ++sTMP; break; }
		// Next element?
		if (*sTMP != ',') { throw CTPPParserSyntaxError("',' or ']' expected", sTMP.GetLine(), sTMP.GetLinePos()); }

		bIsFirst = false;
		++sTMP;
		++iArrayIndex;

		szData = sTMP;
	}

return sTMP;
}

//
// JSON Value
//
CCharIterator CTPP2JSONParser::IsValue(CCharIterator szData, CCharIterator szEnd, CDT & oCurrentCDT)
{
	// Check object
	CCharIterator sTMP = IsObject(szData, szEnd, oCurrentCDT);
	if (sTMP != NULL) { return sTMP; }
	else
	{
		// Check array
		sTMP = IsArray(szData, szEnd, oCurrentCDT);
		if (sTMP != NULL) { return sTMP; }
		else
		{
			// Check number
			sTMP = IsNum(szData, szEnd);
			if (sTMP != NULL)
			{
				if (iParsedNumberType == 0) { oCurrentCDT = iIntData;   }
				else                        { oCurrentCDT = dFloatData; }

				return sTMP;
			}
			else
			{
				// Check string
				sTMP = IsString(szData, szEnd);
				if (sTMP != NULL)
				{
					oCurrentCDT = sTMPBuf;
					return sTMP;
				}
				else
				{
					// Check special cases
					sTMP = IsStringCompatOldVersion(szData, szEnd);
					if (sTMP != NULL)
					{
						if      (strcasecmp("null",  sTMPBuf.c_str()) == 0) { oCurrentCDT = CDT(CDT::UNDEF); }
						else if (strcasecmp("false", sTMPBuf.c_str()) == 0) { oCurrentCDT = 0; }
						else if (strcasecmp("true",  sTMPBuf.c_str()) == 0) { oCurrentCDT = 1; }
						// Unexpected data found
						else { return NULL; }

						return sTMP;
					}
				}
			}
		}
	}

return NULL;
}

//
// Parse JSON object
//
INT_32 CTPP2JSONParser::Parse(CCharIterator szData, CCharIterator szEnd)
{
	// Skip white space
	szData = IsWhiteSpace(szData, szEnd, 0);
	// Unexpected EOD?
	if (szData == szEnd) { throw CTPPParserSyntaxError("empty JSON object", szData.GetLine(), szData.GetLinePos()); }

	// Check object/array/value/special case (bukk, true, false)
	CCharIterator sTMP = IsValue(szData, szEnd, oCDT);
	if (sTMP == NULL)
	{
		throw CTPPParserSyntaxError("not an JSON object", szData.GetLine(), szData.GetLinePos());
	}

	// Skip white space
	sTMP = IsWhiteSpace(sTMP, szEnd, 0);
	if (sTMP != szEnd)
	{
		throw CTPPParserSyntaxError("syntax error", sTMP.GetLine(), sTMP.GetLinePos());
	}

return 0;
}

//
// A destructor
//
CTPP2JSONParser::~CTPP2JSONParser() throw()
{
	;;
}

} // namespace CTPP
// End.
