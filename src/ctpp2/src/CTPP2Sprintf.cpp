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
 *      CTPP2Sprintf.cpp
 *
 * $CTPP$
 */
#include "CTPP2Sprintf.hpp"
#include "CTPP2DTOA.hpp"
#include "CTPP2StringBuffer.hpp"

namespace CTPP // C++ Template Engine
{

#define F_LEFT_ALIGN  0x00000001
#define F_FORCE_SIGN  0x00000002
#define F_SIGN_SPACE  0x00000004
#define F_HASH_SIGN   0x00000008

enum eFmtLengths   { F_NONE      = 0,
                     F_SHORT     = 1,
                     F_LONG      = 2,
                     F_LONG_LONG = 3};

enum eFmtSpecifier { F_ESCAPED  = 0,
                     F_CHAR     = 1,
                     F_DECIMAL  = 2,
                     F_SCI      = 3,
                     F_FLOAT_F  = 4,
                     F_FLOAT_G  = 5,
                     F_OCTAL    = 6,
                     F_HEX      = 7,
                     F_STRING   = 8,
                     F_POINTER  = 9,
                     F_UNSIGNED = 10};

enum eFmtSpecifierCase { FM_UPPER = 0,
                         FM_LOWER = 1};

enum eParserState { C_INITIAL_STATE = 0,
                    C_START_FOUND   = 1};

static CCHAR_P szDigitsUc = "0123456789ABCDEFX";
static CCHAR_P szDigitsLc = "0123456789abcdefx";
#define C_INT_BUFFER_LEN 128


template<typename T>INT_32 DoFormat(T         iData,
                                    INT_32    iRadix,
                                    CCHAR_P   szDigits,
                                    CHAR_P    szBuffer,
                                    bool    & bIsNegative)
{
	// Remove warning!
	if (iData == 0)
	{
		bIsNegative = false;
		szBuffer[C_INT_BUFFER_LEN - 1] = '0';
		return 1;
	}
	else if (!(iData > 0))
	{
		bIsNegative = true;
		iData *= -1;
	}

	INT_32 iPos    = 0;
	INT_32 iOffset = C_INT_BUFFER_LEN;
	while(iData > 0)
	{
		--iOffset;
		++iPos;
		INT_32 iDigit = iData % iRadix;
		iData /= iRadix;
		szBuffer[iOffset] = szDigits[iDigit];
	}

return iPos;
}

//
// Character
//
static void FmtChar(StringBuffer   & oBuffer,
                    const CDT      & oCurrentArgument,
                    const UINT_32    iFmtFlags,
                    const INT_32     iWidth,
                    CHAR_8           chPadSymbol)
{
	const CDT::eValType oValType = oCurrentArgument.GetType();
	if (oValType == CDT::UNDEF && iWidth > 0)
	{
		oBuffer.Append(iWidth, chPadSymbol);
		return;
	}

	UCHAR_8 ucTMP = ' ';
	if (oValType == CDT::INT_VAL || oValType == CDT::REAL_VAL)
	{
		ucTMP = oCurrentArgument.GetInt();
	}
	else if (oValType == CDT::STRING_VAL)
	{
		const STLW::string sTMP = oCurrentArgument.GetString();
		if (sTMP.empty() && iWidth > 0)
		{
			oBuffer.Append(iWidth, chPadSymbol);
			return;
		}
		ucTMP = sTMP[0];
	}
	else if (oValType == CDT::POINTER_VAL) { ucTMP = 'P'; }
	else if (oValType == CDT::ARRAY_VAL)   { ucTMP = 'A'; }
	else if (oValType == CDT::HASH_VAL)    { ucTMP = 'H'; }

	if (iFmtFlags & F_LEFT_ALIGN)
	{
		oBuffer.Append(1, ucTMP);
		if (iWidth > 1) { oBuffer.Append(iWidth - 1, chPadSymbol); }
	}
	else
	{
		if (iWidth > 1) { oBuffer.Append(iWidth - 1, chPadSymbol); }
		oBuffer.Append(1, ucTMP);
	}
}

//
// String
//
static void FmtString(StringBuffer   & oBuffer,
                      const CDT      & oCurrentArgument,
                      const UINT_32    iFmtFlags,
                      const INT_32     iWidth,
                      const INT_32     iMaxChars,
                      CHAR_8           chPadSymbol)
{
	const STLW::string sTMP    = oCurrentArgument.GetString();

	INT_32 iFormatSize = sTMP.size();
	if (iFormatSize > iMaxChars && iMaxChars > 0) { iFormatSize = iMaxChars; }

	if (iFmtFlags & F_LEFT_ALIGN)
	{
		oBuffer.Append(sTMP.data(), iFormatSize);
		if (iWidth > iFormatSize) { oBuffer.Append(iWidth - iFormatSize, chPadSymbol); }
	}
	else
	{
		if (iWidth > iFormatSize) { oBuffer.Append(iWidth - iFormatSize, chPadSymbol); }
		oBuffer.Append(sTMP.data(), iFormatSize);
	}
}

//
// Integer
//
static void FmtInt(StringBuffer        & oBuffer,
                   const CDT           & oCurrentArgument,
                   const UINT_32         iFmtFlags,
                   const eFmtLengths   & oFmtLengths,
                   CCHAR_P               szDigits,
                   const eFmtSpecifier & oFmtSpecifier,
                   const INT_32          iRadix,
                   INT_32                iWidth,
                   INT_32                iPrecision,
                   CHAR_8                chPadSymbol)
{
	if (iWidth < 0)     { iWidth = 0; }
	if (iPrecision < 0) { iPrecision = 0; }

	CHAR_8 szBuffer[C_INT_BUFFER_LEN + 1];
	szBuffer[C_INT_BUFFER_LEN] = 0;
	INT_32 iPos      = 0;
	bool bIsNegative = false;
	// Signed
	if (oFmtSpecifier == F_DECIMAL)
	{
		switch (oFmtLengths)
		{
			case F_NONE:
			case F_LONG:
				iPos = DoFormat<INT_32>(oCurrentArgument.GetInt(), iRadix, szDigits, szBuffer, bIsNegative);
				break;
			case F_SHORT:
				iPos = DoFormat<INT_16>(oCurrentArgument.GetInt(), iRadix, szDigits, szBuffer, bIsNegative);
				break;
			case F_LONG_LONG:
				iPos = DoFormat<INT_64>(oCurrentArgument.GetInt(), iRadix, szDigits, szBuffer, bIsNegative);
				break;
		}
	}
	// Unsigned, hex, octal
	else
	{
		switch (oFmtLengths)
		{
			case F_NONE:
			case F_LONG:
				iPos = DoFormat<UINT_32>(oCurrentArgument.GetInt(), iRadix, szDigits, szBuffer, bIsNegative);
				break;
			case F_SHORT:
				iPos = DoFormat<UINT_16>(oCurrentArgument.GetInt(), iRadix, szDigits, szBuffer, bIsNegative);
				break;
			case F_LONG_LONG:
				iPos = DoFormat<UINT_64>(oCurrentArgument.GetInt(), iRadix, szDigits, szBuffer, bIsNegative);
				break;
		}
	}

	UINT_32 iBufferPos = C_INT_BUFFER_LEN - iPos;
	// Left-aligned output
	if (iFmtFlags & F_LEFT_ALIGN || chPadSymbol == '0')
	{
		// Sign
		if      (bIsNegative)              { oBuffer.Append(1, '-'); --iWidth; }
		else if (iFmtFlags & F_FORCE_SIGN) { oBuffer.Append(1, '+'); --iWidth; }
		else if (iFmtFlags & F_SIGN_SPACE) { oBuffer.Append(1, ' '); --iWidth; }

		// Hash
		if (iFmtFlags & F_HASH_SIGN)
		{
			if      (oFmtSpecifier == F_OCTAL)
			{
				oBuffer.Append(1, '0');
			}
			else if (oFmtSpecifier == F_HEX || oFmtSpecifier == F_POINTER)
			{
				oBuffer.Append(1, '0');
				oBuffer.Append(1, szDigits[16]);
			}
		}

		// Zeroes, if need
		if (iPrecision > iPos)
		{
			iWidth -= (iPrecision - iPos);
			oBuffer.Append(iPrecision - iPos, '0');
		}

		if (chPadSymbol == '0')
		{
			// Spaces, if need
			if (iWidth > iPos) { oBuffer.Append(iWidth - iPos, '0'); }
			// Value
			oBuffer.Append(szBuffer + iBufferPos, iPos);
		}
		else
		{
			// Value
			oBuffer.Append(szBuffer + iBufferPos, iPos);
			// Spaces, if need
			if (iWidth > iPos) { oBuffer.Append(iWidth - iPos, ' '); }
		}
		return;
	}

	// Right-aligned
	INT_32 iFormattedLength = iPos;

	// Precision
	if (iPrecision > iPos)
	{
		iFormattedLength += (iPrecision - iPos);
	}

	// Sign?
	if (bIsNegative || iFmtFlags & (F_FORCE_SIGN | F_SIGN_SPACE)) { ++iFormattedLength; }

	// Hash
	if (iFmtFlags & F_HASH_SIGN)
	{
		if      (oFmtSpecifier == F_OCTAL)   { ++iFormattedLength; }
		else if (oFmtSpecifier == F_HEX ||
		         oFmtSpecifier == F_POINTER) { iFormattedLength += 2; }
	}

	// Spaces, if need
	if (iWidth > iFormattedLength) { oBuffer.Append(iWidth - iFormattedLength, ' '); }

	// Sign
	if      (bIsNegative)              { oBuffer.Append(1, '-'); --iWidth; }
	else if (iFmtFlags & F_FORCE_SIGN) { oBuffer.Append(1, '+'); --iWidth; }
	else if (iFmtFlags & F_SIGN_SPACE) { oBuffer.Append(1, ' '); --iWidth; }

	// Hash
	if (iFmtFlags & F_HASH_SIGN)
	{
		if      (oFmtSpecifier == F_OCTAL)
		{
			oBuffer.Append(1, '0');
		}
		else if (oFmtSpecifier == F_HEX || oFmtSpecifier == F_POINTER)
		{
			oBuffer.Append(1, '0');
			oBuffer.Append(1, szDigits[16]);
		}
	}

	if (iPrecision > iPos) { oBuffer.Append(iPrecision - iPos, '0'); }
	// Value
	oBuffer.Append(szBuffer + iBufferPos, iPos);
}

//
// Scientific
//
static void FmtSci(StringBuffer       & oBuffer,
                   const CDT          & oCurrentArgument,
                   const UINT_32        iFmtFlags,
                   const eFmtLengths  & oFmtLengths,
                   const CHAR_8         chExponentSymbol,
                   INT_32               iWidth,
                   INT_32               iPrecision,
                   CHAR_8               chPadSymbol)
{
	using namespace CTPP;

	/*
	 * eE  The double argument is rounded and converted in the style
	 *     [-]d.ddde+-dd where there is one digit before the decimal-point
	 *     character and the number of digits after it is equal to the pre-
	 *     cision; if the precision is missing, it is taken as 6; if the
	 *     precision is zero, no decimal-point character appears.  An E con-
	 *     version uses the letter `E' (rather than `e') to introduce the
	 *     exponent.  The exponent always contains at least two digits; if
	 *     the value is zero, the exponent is 00.
	 */

	if (iPrecision == -1) { iPrecision = 6; }
	if (iWidth == -1)     { iWidth     = 6; }

	const INT_32   iMode     = 2;
	INT_32         iExponent = 0;
	INT_32         iSign     = 0;
	CHAR_P         szEnd     = NULL;

	Bigint *freelist[Kmax+1];
	for (UINT_32 iPos = 0; iPos <= Kmax; ++iPos) { freelist[iPos] = NULL; }

	W_FLOAT dData = oCurrentArgument.GetFloat();
	AllocatedBlock * aBlocks = NULL;

	CHAR_P szBuffer = ctpp_dtoa(&aBlocks, freelist, dData, iMode, iPrecision, &iExponent, &iSign, &szEnd);
	bool bIsNegative = iSign < 0;
	--iExponent;

	// Format Exponent
	CHAR_8 szExponentBuffer[C_INT_BUFFER_LEN + 1];
	szExponentBuffer[C_INT_BUFFER_LEN] = 0;
	bool bExponentIsNegative = false;
	// Signed
	INT_32 iExponentPos = DoFormat<INT_32>(iExponent, 10, szDigitsUc, szExponentBuffer, bExponentIsNegative);

	// Atleast 2 digits in exponent
	if (iExponentPos == 1)
	{
		iExponentPos = 2;
		szExponentBuffer[C_INT_BUFFER_LEN - 2] = '0';
	}

	++iExponentPos;
	// Add exponent sign
	szExponentBuffer[C_INT_BUFFER_LEN - iExponentPos] = bExponentIsNegative ? '-' : '+';
	// Add exponent char
	++iExponentPos;
	szExponentBuffer[C_INT_BUFFER_LEN - iExponentPos] = chExponentSymbol;

	INT_32 iPos = szEnd - szBuffer;

	// Data length
	INT_32 iFormattedLength = iExponentPos + 1;
	//;// = iPos + iExponentPos + 1;
	// Precision
	if (iPrecision > iPos)
	{
		//iFormattedLength += (iPrecision - iPos);
		iFormattedLength += iPrecision;
	}
	else
	{
		iPos = iPrecision;
		iFormattedLength += iPos;
	}

	// Sign?
	if (bIsNegative || iFmtFlags & (F_FORCE_SIGN | F_SIGN_SPACE)) { ++iFormattedLength; }

	// Right-aligned
	if ((iFmtFlags & F_LEFT_ALIGN) != F_LEFT_ALIGN)
	{
		// Spaces, if need
		if (iWidth > iFormattedLength) { oBuffer.Append(iWidth - iFormattedLength, ' '); }
	}

	// Sign
	if      (bIsNegative)              { oBuffer.Append(1, '-'); }
	else if (iFmtFlags & F_FORCE_SIGN) { oBuffer.Append(1, '+'); }
	else if (iFmtFlags & F_SIGN_SPACE) { oBuffer.Append(1, ' '); }

	// Value
	oBuffer.Append(szBuffer, 1);
	oBuffer.Append(1, '.');
	oBuffer.Append(szBuffer + 1, iPos - 1);

	// Free memory
	freedtoa(&aBlocks);

	// Zeroes, if need
	if (iPrecision > iPos) { oBuffer.Append(iPrecision - iPos, '0'); }
	// Exponent
	oBuffer.Append(szExponentBuffer + C_INT_BUFFER_LEN - iExponentPos, iExponentPos);

	// Left-aligned
	if ((iFmtFlags & F_LEFT_ALIGN) == F_LEFT_ALIGN)
	{
		// Spaces, if need
		if (iWidth > iFormattedLength) { oBuffer.Append(iWidth - iFormattedLength, ' '); }
	}
}

//
// Floating point
//
static void FmtFloat(StringBuffer            & oBuffer,
                     const CDT               & oCurrentArgument,
                     const UINT_32             iFmtFlags,
                     const eFmtLengths       & oFmtLengths,
                     const CHAR_8              chExponentSymbol,
                     const eFmtSpecifier     & oFmtSpecifier,
                     INT_32                    iWidth,
                     INT_32                    iPrecision,
                     CHAR_8                    chPadSymbol)
{
	using namespace CTPP;

	/*
	 * fF  The double argument is rounded and converted to decimal notation
	 *     in the style [-]ddd.ddd, where the number of digits after the
	 *     decimal-point character is equal to the precision specification.
	 *     If the precision is missing, it is taken as 6; if the precision
	 *     is explicitly zero, no decimal-point character appears.  If a
	 *     decimal point appears, at least one digit appears before it.
	 *
	 * gG  The double argument is converted in style f or e (or F or E for G
	 *     conversions).  The precision specifies the number of significant
	 *     digits.  If the precision is missing, 6 digits are given; if the
	 *     precision is zero, it is treated as 1.  Style e is used if the
	 *     exponent from its conversion is less than -4 or greater than or
	 *     equal to the precision.  Trailing zeros are removed from the
	 *     fractional part of the result; a decimal point appears only if it
	 *     is followed by at least one digit.
	 */
//	if (iWidth == -1) { iWidth = 6; }

	INT_32 iMode;
	INT_32 iExponent = 0;
	INT_32 iSign     = 0;
	CHAR_P szEnd     = NULL;
	INT_32 iFormatPrecision;
	if (oFmtSpecifier == F_FLOAT_F)
	{
		iMode = 3;
		if (iPrecision == -1) { iPrecision = 6; }

		iFormatPrecision = iPrecision;
	}
	// Only one decision left
	else /* if (oFmtSpecifier == F_FLOAT_G) */
	{
		iMode = 2;
		if      (iPrecision == -1) { iPrecision = 6; }
		else if (iPrecision ==  0) { iPrecision = 1; }

		iFormatPrecision = iPrecision + 1;
	}

	Bigint *freelist[Kmax+1];
	for (UINT_32 iPos = 0; iPos <= Kmax; ++iPos) { freelist[iPos] = NULL; }

	W_FLOAT dData = oCurrentArgument.GetFloat();
	AllocatedBlock * aBlocks = NULL;

	CHAR_P szBuffer = ctpp_dtoa(&aBlocks, freelist, dData, iMode, iFormatPrecision, &iExponent, &iSign, &szEnd);
	bool bIsNegative = iSign < 0;
	--iExponent;

	INT_32 iPos = szEnd - szBuffer;

	// Data length
	INT_32 iFormattedLength;
	if (oFmtSpecifier == F_FLOAT_F)
	{
		if (iExponent < 0)
		{
			iFormattedLength = iPrecision + 2;
		}
		else
		{
			iFormattedLength = iPrecision + iExponent + 1;
		}

	}
	else /* if (oFmtSpecifier == F_FLOAT_G) */
	{
		/*
		 *  Style e is used if the
		 *  exponent from its conversion is less than -4 or greater than or
		 *  equal to the precision.
		 */
		if (iExponent < -4 || iExponent >= iPrecision)
		{
			// Free memory
			freedtoa(&aBlocks);

			if (iWidth == -1) { iWidth = 6; }
			FmtSci(oBuffer, oCurrentArgument, iFmtFlags, oFmtLengths, chExponentSymbol, iWidth, iPrecision, chPadSymbol);
			return;
		}

		if (iPos > iPrecision) { iPos = iPrecision; }
		else                   { iPrecision = iPos; }

		iFormattedLength = iPrecision;
		if (iExponent < 0) { iFormattedLength += 1 - iExponent; }
	}

	// Sign?
	if (bIsNegative || iFmtFlags & (F_FORCE_SIGN | F_SIGN_SPACE)) { ++iFormattedLength; }

	// Right-aligned
	if ((iFmtFlags & F_LEFT_ALIGN) != F_LEFT_ALIGN)
	{
		// Spaces, if need
		if (iWidth > iFormattedLength) { oBuffer.Append(iWidth - iFormattedLength, ' '); }
	}

	// Sign
	if      (bIsNegative)              { oBuffer.Append(1, '-'); }
	else if (iFmtFlags & F_FORCE_SIGN) { oBuffer.Append(1, '+'); }
	else if (iFmtFlags & F_SIGN_SPACE) { oBuffer.Append(1, ' '); }

	if (oFmtSpecifier == F_FLOAT_F)
	{
		// Value
		if (iExponent < 0)
		{
			const INT_32 iChars = iPrecision + iExponent + 1;

			oBuffer.Append(1, '0');
			oBuffer.Append(1, '.');
			oBuffer.Append(-iExponent - 1, '0');

			if (iChars > iPos)
			{
				oBuffer.Append(szBuffer, iPos);
				oBuffer.Append(iChars - iPos, '0');
			}
			else
			{
				oBuffer.Append(szBuffer, iChars);
			}
		}
		else
		{
			// Value
			oBuffer.Append(szBuffer, iExponent + 1);
			if (iPrecision > 0)
			{
				iPos -= iExponent;
				oBuffer.Append(1, '.');
				oBuffer.Append(szBuffer + iExponent + 1, iPos - 1);

				if (iPrecision + 1 > iPos)
				{
					oBuffer.Append(iPrecision - iPos + 1, '0');
				}
			}
		}
	}
	else /* if (oFmtSpecifier == F_FLOAT_G) */
	{
		// Value
		if (iExponent < 0)
		{
			const INT_32 iChars = (iPrecision < iPos) ? iPrecision : iPos;

			oBuffer.Append(1, '0');
			oBuffer.Append(1, '.');
			oBuffer.Append(- iExponent - 1, '0');
			oBuffer.Append(szBuffer, iChars);
		}
		else
		{
			oBuffer.Append(szBuffer, iExponent + 1);
			if (iPos > iExponent + 1)
			{
				oBuffer.Append(1, '.');
				oBuffer.Append(szBuffer + iExponent + 1, iPrecision - iExponent - 1);
			}
		}
	}

	// Free memory
	freedtoa(&aBlocks);

	// Left-aligned
	if ((iFmtFlags & F_LEFT_ALIGN) == F_LEFT_ALIGN)
	{
		// Spaces, if need
		if (iWidth > iFormattedLength) { oBuffer.Append(iWidth - iFormattedLength, ' '); }
	}
}

//
//
//
INT_32 FormatString(const STLW::string & sFormatString, STLW::string & sResult, const CDT & oArgs)
{
	using namespace CTPP;
	StringBuffer oBuffer(sResult);

	CCHAR_P sPos = sFormatString.data();
	CCHAR_P sEnd = sFormatString.data() + sFormatString.size();
	CCHAR_P sEndSave = sPos;

	UINT_32 iPos = 0;
	for(;;)
	{
		INT_32   iWidth            = -1;
		INT_32   iPrecision        = -1;
		CHAR_8   chPadSymbol       = ' ';
		UINT_32  iFmtFlags         = 0;
		eFmtLengths   oFmtLengths  = F_NONE;
		eParserState  oParserState = C_INITIAL_STATE;
		// Find "%" at start of token
		while(sPos != sEnd)
		{
			if (*sPos == '%')
			{
				oParserState = C_START_FOUND;
				break;
			}
			++sPos;
		}
		oBuffer.Append(sEndSave, sPos);

		if (oParserState == C_START_FOUND)
		{
			++sPos;
			// Check end of string
			if (sPos == sEnd) { return -1; }

			bool bEndCycle = false;
			while (!bEndCycle)
			{
				// Flags
				switch(*sPos)
				{
					// '-' Left-justify within the given field width; Right justification is the default (see width sub-specifier).
					case '-':
						iFmtFlags |= F_LEFT_ALIGN;
						++sPos;
						break;

					// '+' Forces to preceed the result with a plus or minus sign (+ or -) even for positive numbers.
					//     By default, only negative numbers are preceded with a - sign.
					case '+':
						iFmtFlags |= F_FORCE_SIGN;
						iFmtFlags &= ~F_SIGN_SPACE;
						++sPos;
						break;

					// ' ' (space) If no sign is going to be written, a blank space is inserted before the value.
					case ' ':
						iFmtFlags |= F_SIGN_SPACE;
						iFmtFlags &= ~F_FORCE_SIGN;
						++sPos;
						break;

					// '#' Used with o, x or X specifiers the value is preceeded with 0, 0x or 0X respectively for values different than zero.
					//     Used with e, E and f, it forces the written output to contain a decimal point even if no digits would follow.
					//     By default, if no digits follow, no decimal point is written.
					//     Used with g or G the result is the same as with e or E but trailing zeros are not removed.
					case '#':
						iFmtFlags |= F_HASH_SIGN;
						++sPos;
						break;

					// '0' Left-pads the number with zeroes (0) instead of spaces, where padding is specified (see width sub-specifier).
					case '0':
						chPadSymbol = '0';
						++sPos;
						break;

					default:
						bEndCycle = true;
						break;
				}

				// Check end of string
				if (sPos == sEnd) { return -1; }
			}

			/* Width
			 * number  Minimum number of characters to be printed. If the value to be printed is shorter than this number,
			 *         the result is padded with blank spaces. The value is not truncated even if the result is larger.
			 */
			if (*sPos > '0' && *sPos <= '9')
			{
				iWidth = 0;
				while (sPos != sEnd && (*sPos >= '0' && *sPos <= '9'))
				{
					iWidth = iWidth * 10 + *sPos - '0';
					++sPos;
				}
			}
			/*
			 * '*'     The width is not specified in the format string, but as an additional integer value argument
			 *         preceding the argument that has to be formatted.
			 */
			else if (*sPos == '*')
			{
				iWidth = oArgs.GetCDT(iPos).GetInt();
				++iPos;
				++sPos;
			}

			// Check end of string
			if (sPos == sEnd) { return -1; }

			// .precision
			if (*sPos == '.')
			{
				++sPos;
				if (sPos == sEnd) { return -1; }

				iPrecision = 0;
				if (*sPos >= '0' && *sPos <= '9')
				{
					while (sPos != sEnd && (*sPos >= '0' && *sPos <= '9'))
					{
						iPrecision = iPrecision * 10 + *sPos - '0';
						++sPos;
					}
				}
				else if (*sPos == '*')
				{
					iPrecision = oArgs.GetCDT(iPos).GetInt();
					++iPos;
					++sPos;
				}
			}

			// length
			switch(*sPos)
			{
				// h The argument is interpreted as a short int or unsigned short int (only applies to integer specifiers: i, d, o, u, x and X).
				case 'h':
					oFmtLengths = F_SHORT;
					++sPos;
					break;
				// l The argument is interpreted as a long int or unsigned long int for integer specifiers (i, d, o, u, x and X), and as a wide character or wide character string for specifiers c and s.
				case 'l':
				// L The argument is interpreted as a long double (only applies to floating point specifiers: e, E, f, g and G).
				case 'L':
					{
						oFmtLengths = F_LONG;
						++sPos;
						if (sPos == sEnd) { return -1; }
						if (*sPos == 'l' || *sPos == 'L')
						{
							oFmtLengths = F_LONG_LONG;
							++sPos;
						}
					}
					break;
			}

			// Check end of string
			if (sPos == sEnd) { return -1; }

			// Specifiers
			//  A % followed by another % character will write % to the string.
			if (*sPos == '%') { oBuffer.Append(sPos, 1); }
			else
			{
				const CDT oCurrentArgument = oArgs.GetCDT(iPos);
				++iPos;

				switch(*sPos)
				{
					//  Character 'a'
					case 'c':
						FmtChar(oBuffer, oCurrentArgument, iFmtFlags, iWidth, chPadSymbol);
						break;
					// 'd' or 'i' Signed decimal integer '392'
					case 'd':
					case 'i':
						FmtInt(oBuffer, oCurrentArgument, iFmtFlags, oFmtLengths, szDigitsLc, F_DECIMAL, 10, iWidth, iPrecision, chPadSymbol);
						break;
					// Scientific notation (mantise/exponent) using e character '3.9265e+2'
					case 'e':
						FmtSci(oBuffer, oCurrentArgument, iFmtFlags, oFmtLengths, 'e', iWidth, iPrecision, chPadSymbol);
						break;
					// Scientific notation (mantise/exponent) using E character '3.9265E+2'
					case 'E':
						FmtSci(oBuffer, oCurrentArgument, iFmtFlags, oFmtLengths, 'E', iWidth, iPrecision, chPadSymbol);
						break;
					// Decimal floating point '392.65'
					case 'f':
						FmtFloat(oBuffer, oCurrentArgument, iFmtFlags, oFmtLengths, 'e', F_FLOAT_F, iWidth, iPrecision, chPadSymbol);
						break;
					// Use the shorter of %e or %f '392.65'
					case 'g':
						FmtFloat(oBuffer, oCurrentArgument, iFmtFlags, oFmtLengths, 'e', F_FLOAT_G, iWidth, iPrecision, chPadSymbol);
						break;
					case 'F':
						FmtFloat(oBuffer, oCurrentArgument, iFmtFlags, oFmtLengths, 'E', F_FLOAT_F, iWidth, iPrecision, chPadSymbol);
						break;
					case 'G':
						FmtFloat(oBuffer, oCurrentArgument, iFmtFlags, oFmtLengths, 'E', F_FLOAT_G, iWidth, iPrecision, chPadSymbol);
						break;
					// Signed octal '610'
					case 'o':
						FmtInt(oBuffer, oCurrentArgument, iFmtFlags, oFmtLengths, szDigitsLc, F_OCTAL, 8, iWidth, iPrecision, chPadSymbol);
						break;
					// String of characters 'sample'
					case 's':
						FmtString(oBuffer, oCurrentArgument, iFmtFlags, iWidth, iPrecision, chPadSymbol);
						break;
					// Unsigned decimal integer '7235'
					case 'u':
						FmtInt(oBuffer, oCurrentArgument, iFmtFlags, oFmtLengths, szDigitsLc, F_UNSIGNED, 10, iWidth, iPrecision, chPadSymbol);
						break;
					// Unsigned hexadecimal integer '7fa'
					case 'x':
						FmtInt(oBuffer, oCurrentArgument, iFmtFlags, oFmtLengths, szDigitsLc, F_HEX, 16, iWidth, iPrecision, chPadSymbol);
						break;
					// Unsigned hexadecimal integer (capital letters) 7FA
					case 'X':
						FmtInt(oBuffer, oCurrentArgument, iFmtFlags, oFmtLengths, szDigitsUc, F_HEX, 16, iWidth, iPrecision, chPadSymbol);
						break;
					// Pointer address 1a2b3c4e
					case 'p':
						FmtInt(oBuffer, oCurrentArgument, iFmtFlags, oFmtLengths, szDigitsLc, F_POINTER, 16, iWidth, iPrecision, chPadSymbol);
						break;
					// Invalid format; return error code
					default:
						return -1;
				}
				++sPos;
			}

			sEndSave = sPos;
		}

		if (sPos == sEnd) { break; }
	}
	oBuffer.Flush();

return 0;
}

} // namespace CTPP
// End.
