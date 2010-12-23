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
 *      CTPP2Util.cpp
 *
 * $CTPP$
 */
#include "CTPP2Util.hpp"

#include "CDT.hpp"

namespace CTPP // C++ Template Engine
{

static const UINT_32 aCRC32[] =
{
	0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f, 0xe963a535, 0x9e6495a3,
	0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988, 0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91,
	0x1db71064, 0x6ab020f2, 0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
	0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9, 0xfa0f3d63, 0x8d080df5,
	0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172, 0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b,
	0x35b5a8fa, 0x42b2986c, 0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
	0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423, 0xcfba9599, 0xb8bda50f,
	0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924, 0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d,
	0x76dc4190, 0x01db7106, 0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
	0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d, 0x91646c97, 0xe6635c01,
	0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e, 0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457,
	0x65b0d9c6, 0x12b7e950, 0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
	0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7, 0xa4d1c46d, 0xd3d6f4fb,
	0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0, 0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9,
	0x5005713c, 0x270241aa, 0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
	0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81, 0xb7bd5c3b, 0xc0ba6cad,
	0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a, 0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683,
	0xe3630b12, 0x94643b84, 0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
	0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb, 0x196c3671, 0x6e6b06e7,
	0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc, 0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5,
	0xd6d6a3e8, 0xa1d1937e, 0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
	0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55, 0x316e8eef, 0x4669be79,
	0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236, 0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f,
	0xc5ba3bbe, 0xb2bd0b28, 0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
	0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f, 0x72076785, 0x05005713,
	0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38, 0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21,
	0x86d3d2d4, 0xf1d4e242, 0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
	0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69, 0x616bffd3, 0x166ccf45,
	0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2, 0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db,
	0xaed16a4a, 0xd9d65adc, 0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
	0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693, 0x54de5729, 0x23d967bf,
	0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94, 0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d

};

//
// Calculate crc32 checksum
//
UINT_32 crc32(UCCHAR_P sBuffer, const UINT_32 & iSize)
{
	UINT_32 iCRC = 0;

	for (UINT_32 iI = 0; iI < iSize; ++iI)
	{
		iCRC = aCRC32[(iCRC ^ sBuffer[iI]) & 0xFF] ^ (iCRC >> 8);
	}

return iCRC;
}

//
// Swap bytes for UINT_32 value
//
UINT_32 Swap32(const UINT_32 & iValue)
{
	return ((iValue << 24) & 0xff000000 ) |
	       ((iValue <<  8) & 0x00ff0000 ) |
	       ((iValue >>  8) & 0x0000ff00 ) |
	       ((iValue >> 24) & 0x000000ff );
}

//
// Swap bytes for UINT_64 value
//
UINT_64 Swap64(const UINT_64 & iValue)
{
	union
	{
		UINT_64    ul;
		UINT_32    ui[2];
	} u;

	UINT_32  * pTMP = (UINT_32*)&iValue;

	u.ui[0] = Swap32(pTMP[1]);
	u.ui[1] = Swap32(pTMP[0]);

return u.ul;
}

//
// Escape value
//
STLW::string URLEscape(const STLW::string  & sData)
{
	static const CHAR_8 * szEscape = "0123456789ABCDEF";

	CHAR_8 sBuffer[CTPP_ESCAPE_BUFFER_LEN + 1];

	STLW::string sRetVal = "";

	UINT_32 iBufferPointer = 0;

	// Get string to escape
	STLW::string::const_iterator itsData = sData.begin();
	while (itsData != sData.end())
	{
		UCHAR_8 chTMP = (UCHAR_8)(*itsData);

		if ((chTMP >= 'a' && chTMP <= 'z') ||
		    (chTMP >= 'A' && chTMP <= 'Z') ||
		    (chTMP >= '0' && chTMP <= '9') ||
		     chTMP == '/' || chTMP == '.' || chTMP == '-')
		{
			sBuffer[iBufferPointer++] = *itsData;
		}
		else if (chTMP == ' ') { sBuffer[iBufferPointer++] = '+'; }
		else
		{
			if (iBufferPointer >= (CTPP_ESCAPE_BUFFER_LEN - 3))
			{
				sRetVal.append(sBuffer, iBufferPointer);
				iBufferPointer = 0;
			}
			sBuffer[iBufferPointer++] = '%';
			sBuffer[iBufferPointer++] = szEscape[((chTMP >> 4) & 0x0F)];
			sBuffer[iBufferPointer++] = szEscape[(chTMP & 0x0F)];
		}

		if (iBufferPointer == CTPP_ESCAPE_BUFFER_LEN)
		{
			sRetVal.append(sBuffer, iBufferPointer);
			iBufferPointer = 0;
		}
		++itsData;
	}

	if (iBufferPointer != 0) { sRetVal.append(sBuffer, iBufferPointer); }

return sRetVal;
}

//
// Escape value
//
STLW::string URIEscape(const STLW::string  & sData)
{
	static const CHAR_8 * szEscape = "0123456789ABCDEF";

	CHAR_8 sBuffer[CTPP_ESCAPE_BUFFER_LEN + 1];

	STLW::string sRetVal = "";

	UINT_32 iBufferPointer = 0;

	// Get string to escape
	STLW::string::const_iterator itsData = sData.begin();
	while (itsData != sData.end())
	{
		UCHAR_8 chTMP = (UCHAR_8)(*itsData);

		if ((chTMP >= 'a' && chTMP <= 'z') ||
		    (chTMP >= 'A' && chTMP <= 'Z') ||
		    (chTMP >= '0' && chTMP <= '9') ||
		     chTMP == '/' || chTMP == '.' || chTMP == '-')
		{
			sBuffer[iBufferPointer++] = *itsData;
		}
		else
		{
			if (iBufferPointer >= (CTPP_ESCAPE_BUFFER_LEN - 3))
			{
				sRetVal.append(sBuffer, iBufferPointer);
				iBufferPointer = 0;
			}
			sBuffer[iBufferPointer++] = '%';
			sBuffer[iBufferPointer++] = szEscape[((chTMP >> 4) & 0x0F)];
			sBuffer[iBufferPointer++] = szEscape[(chTMP & 0x0F)];
		}

		if (iBufferPointer == CTPP_ESCAPE_BUFFER_LEN)
		{
			sRetVal.append(sBuffer, iBufferPointer);
			iBufferPointer = 0;
		}
		++itsData;
	}

	if (iBufferPointer != 0) { sRetVal.append(sBuffer, iBufferPointer); }

return sRetVal;
}

//
// Escape value
//
STLW::string HTMLEscape(const STLW::string  & sData)
{
	CHAR_8 sBuffer[CTPP_ESCAPE_BUFFER_LEN + 1];

	STLW::string sRetVal = "";

	UINT_32 iBufferPointer = 0;

	// Get string to escape
	STLW::string::const_iterator itsData = sData.begin();
	while (itsData != sData.end())
	{
		UCHAR_8 chTMP = (UCHAR_8)(*itsData);

		if (chTMP == '"')
		{
			if (iBufferPointer >= (CTPP_ESCAPE_BUFFER_LEN - 6))
			{
				sRetVal.append(sBuffer, iBufferPointer);
				iBufferPointer = 0;
			}

			sBuffer[iBufferPointer++] = '&';
			sBuffer[iBufferPointer++] = 'q';
			sBuffer[iBufferPointer++] = 'u';
			sBuffer[iBufferPointer++] = 'o';
			sBuffer[iBufferPointer++] = 't';
			sBuffer[iBufferPointer++] = ';';
		}
		else if (chTMP == '\'')
		{
			if (iBufferPointer >= (CTPP_ESCAPE_BUFFER_LEN - 5))
			{
				sRetVal.append(sBuffer, iBufferPointer);
				iBufferPointer = 0;
			}

			sBuffer[iBufferPointer++] = '&';
			sBuffer[iBufferPointer++] = '#';
			sBuffer[iBufferPointer++] = '3';
			sBuffer[iBufferPointer++] = '9';
			sBuffer[iBufferPointer++] = ';';
		}
		else if (chTMP == '<')
		{
			if (iBufferPointer >= (CTPP_ESCAPE_BUFFER_LEN - 4))
			{
				sRetVal.append(sBuffer, iBufferPointer);
				iBufferPointer = 0;
			}

			sBuffer[iBufferPointer++] = '&';
			sBuffer[iBufferPointer++] = 'l';
			sBuffer[iBufferPointer++] = 't';
			sBuffer[iBufferPointer++] = ';';
		}
		else if (chTMP == '>')
		{
			if (iBufferPointer >= CTPP_ESCAPE_BUFFER_LEN - 4)
			{
				sRetVal.append(sBuffer, iBufferPointer);
				iBufferPointer = 0;
			}

			sBuffer[iBufferPointer++] = '&';
			sBuffer[iBufferPointer++] = 'g';
			sBuffer[iBufferPointer++] = 't';
			sBuffer[iBufferPointer++] = ';';
		}
		else if (chTMP == '&')
		{
			if (iBufferPointer >= (CTPP_ESCAPE_BUFFER_LEN - 5))
			{
				sRetVal.append(sBuffer, iBufferPointer);
				iBufferPointer = 0;
			}

			sBuffer[iBufferPointer++] = '&';
			sBuffer[iBufferPointer++] = 'a';
			sBuffer[iBufferPointer++] = 'm';
			sBuffer[iBufferPointer++] = 'p';
			sBuffer[iBufferPointer++] = ';';
		}
		else
		{
			sBuffer[iBufferPointer++] = *itsData;
		}

		if (iBufferPointer == CTPP_ESCAPE_BUFFER_LEN)
		{
			sRetVal.append(sBuffer, iBufferPointer);
			iBufferPointer = 0;
		}
		++itsData;
	}

	if (iBufferPointer != 0) { sRetVal.append(sBuffer, iBufferPointer); }

return sRetVal;
}

//
// Escape value
//
STLW::string XMLEscape(const STLW::string  & sData)
{
	CHAR_8 sBuffer[CTPP_ESCAPE_BUFFER_LEN + 1];

	STLW::string sRetVal = "";

	UINT_32 iBufferPointer = 0;

	// Get string to escape
	STLW::string::const_iterator itsData = sData.begin();
	while (itsData != sData.end())
	{
		UCHAR_8 chTMP = (UCHAR_8)(*itsData);

		if (chTMP == '"')
		{
			if (iBufferPointer >= (CTPP_ESCAPE_BUFFER_LEN - 6))
			{
				sRetVal.append(sBuffer, iBufferPointer);
				iBufferPointer = 0;
			}

			sBuffer[iBufferPointer++] = '&';
			sBuffer[iBufferPointer++] = 'q';
			sBuffer[iBufferPointer++] = 'u';
			sBuffer[iBufferPointer++] = 'o';
			sBuffer[iBufferPointer++] = 't';
			sBuffer[iBufferPointer++] = ';';
		}
		else if (chTMP == '\'')
		{
			if (iBufferPointer >= (CTPP_ESCAPE_BUFFER_LEN - 6))
			{
				sRetVal.append(sBuffer, iBufferPointer);
				iBufferPointer = 0;
			}

			sBuffer[iBufferPointer++] = '&';
			sBuffer[iBufferPointer++] = 'a';
			sBuffer[iBufferPointer++] = 'p';
			sBuffer[iBufferPointer++] = 'o';
			sBuffer[iBufferPointer++] = 's';
			sBuffer[iBufferPointer++] = ';';
		}
		else if (chTMP == '<')
		{
			if (iBufferPointer >= (CTPP_ESCAPE_BUFFER_LEN - 4))
			{
				sRetVal.append(sBuffer, iBufferPointer);
				iBufferPointer = 0;
			}

			sBuffer[iBufferPointer++] = '&';
			sBuffer[iBufferPointer++] = 'l';
			sBuffer[iBufferPointer++] = 't';
			sBuffer[iBufferPointer++] = ';';
		}
		else if (chTMP == '>')
		{
			if (iBufferPointer >= CTPP_ESCAPE_BUFFER_LEN - 4)
			{
				sRetVal.append(sBuffer, iBufferPointer);
				iBufferPointer = 0;
			}

			sBuffer[iBufferPointer++] = '&';
			sBuffer[iBufferPointer++] = 'g';
			sBuffer[iBufferPointer++] = 't';
			sBuffer[iBufferPointer++] = ';';
		}
		else if (chTMP == '&')
		{
			if (iBufferPointer >= (CTPP_ESCAPE_BUFFER_LEN - 5))
			{
				sRetVal.append(sBuffer, iBufferPointer);
				iBufferPointer = 0;
			}

			sBuffer[iBufferPointer++] = '&';
			sBuffer[iBufferPointer++] = 'a';
			sBuffer[iBufferPointer++] = 'm';
			sBuffer[iBufferPointer++] = 'p';
			sBuffer[iBufferPointer++] = ';';
		}
		else
		{
			sBuffer[iBufferPointer++] = *itsData;
		}

		if (iBufferPointer == CTPP_ESCAPE_BUFFER_LEN)
		{
			sRetVal.append(sBuffer, iBufferPointer);
			iBufferPointer = 0;
		}
		++itsData;
	}

	if (iBufferPointer != 0) { sRetVal.append(sBuffer, iBufferPointer); }

return sRetVal;
}

//
// Escape value
//
STLW::string WMLEscape(const STLW::string  & sData)
{
	CHAR_8 sBuffer[CTPP_ESCAPE_BUFFER_LEN + 1];

	STLW::string sRetVal = "";

	UINT_32 iBufferPointer = 0;

	// Get string to escape
	STLW::string::const_iterator itsData = sData.begin();
	while (itsData != sData.end())
	{
		UCHAR_8 chTMP = (UCHAR_8)(*itsData);

		if (chTMP == '"')
		{
			if (iBufferPointer >= (CTPP_ESCAPE_BUFFER_LEN - 6))
			{
				sRetVal.append(sBuffer, iBufferPointer);
				iBufferPointer = 0;
			}

			sBuffer[iBufferPointer++] = '&';
			sBuffer[iBufferPointer++] = 'q';
			sBuffer[iBufferPointer++] = 'u';
			sBuffer[iBufferPointer++] = 'o';
			sBuffer[iBufferPointer++] = 't';
			sBuffer[iBufferPointer++] = ';';
		}
		else if (chTMP == '\'')
		{
			if (iBufferPointer >= (CTPP_ESCAPE_BUFFER_LEN - 6))
			{
				sRetVal.append(sBuffer, iBufferPointer);
				iBufferPointer = 0;
			}

			sBuffer[iBufferPointer++] = '&';
			sBuffer[iBufferPointer++] = 'a';
			sBuffer[iBufferPointer++] = 'p';
			sBuffer[iBufferPointer++] = 'o';
			sBuffer[iBufferPointer++] = 's';
			sBuffer[iBufferPointer++] = ';';
		}
		else if (chTMP == '<')
		{
			if (iBufferPointer >= (CTPP_ESCAPE_BUFFER_LEN - 4))
			{
				sRetVal.append(sBuffer, iBufferPointer);
				iBufferPointer = 0;
			}

			sBuffer[iBufferPointer++] = '&';
			sBuffer[iBufferPointer++] = 'l';
			sBuffer[iBufferPointer++] = 't';
			sBuffer[iBufferPointer++] = ';';
		}
		else if (chTMP == '>')
		{
			if (iBufferPointer >= CTPP_ESCAPE_BUFFER_LEN - 4)
			{
				sRetVal.append(sBuffer, iBufferPointer);
				iBufferPointer = 0;
			}

			sBuffer[iBufferPointer++] = '&';
			sBuffer[iBufferPointer++] = 'g';
			sBuffer[iBufferPointer++] = 't';
			sBuffer[iBufferPointer++] = ';';
		}
		else if (chTMP == '&')
		{
			if (iBufferPointer >= (CTPP_ESCAPE_BUFFER_LEN - 5))
			{
				sRetVal.append(sBuffer, iBufferPointer);
				iBufferPointer = 0;
			}

			sBuffer[iBufferPointer++] = '&';
			sBuffer[iBufferPointer++] = 'a';
			sBuffer[iBufferPointer++] = 'm';
			sBuffer[iBufferPointer++] = 'p';
			sBuffer[iBufferPointer++] = ';';
		}
		else if (chTMP == '$')
		{
			if (iBufferPointer >= (CTPP_ESCAPE_BUFFER_LEN - 4))
			{
				sRetVal.append(sBuffer, iBufferPointer);
				iBufferPointer = 0;
			}

			sBuffer[iBufferPointer++] = '$';
			sBuffer[iBufferPointer++] = '$';
		}
		else
		{
			sBuffer[iBufferPointer++] = *itsData;
		}

		if (iBufferPointer == CTPP_ESCAPE_BUFFER_LEN)
		{
			sRetVal.append(sBuffer, iBufferPointer);
			iBufferPointer = 0;
		}
		++itsData;
	}

	if (iBufferPointer != 0) { sRetVal.append(sBuffer, iBufferPointer); }

return sRetVal;
}

//
// Escape string, if need
//
STLW::string EscapeJSONString(const STLW::string & sSource, const bool & bECMAConventions, const bool & bHTMLSafe)
{
	STLW::string sResult;

	const UINT_32 iSize = sSource.size();
	if (iSize == 0) { return sResult; }

	UINT_32 iPos     = 0;
	UINT_32 iPrevPos = 0;
	for(;;)
	{
		const UCHAR_8 uCH = sSource[iPos];
		if (uCH == '\r')
		{
			sResult.append(sSource, iPrevPos, iPos - iPrevPos);
			iPrevPos = iPos + 1;
			sResult.append("\\r", 2);
		}
		else if (uCH == '\n')
		{
			sResult.append(sSource, iPrevPos, iPos - iPrevPos);
			iPrevPos = iPos + 1;
			sResult.append("\\n", 2);
		}
		else if (uCH == '\b')
		{
			sResult.append(sSource, iPrevPos, iPos - iPrevPos);
			iPrevPos = iPos + 1;
			sResult.append("\\b", 2);
		}
		else if (uCH == '\f')
		{
			sResult.append(sSource, iPrevPos, iPos - iPrevPos);
			iPrevPos = iPos + 1;
			sResult.append("\\f", 2);
		}
		else if (uCH == '\t')
		{
			sResult.append(sSource, iPrevPos, iPos - iPrevPos);
			iPrevPos = iPos + 1;
			sResult.append("\\t", 2);
		}
		else if (uCH == '\\')
		{
			sResult.append(sSource, iPrevPos, iPos - iPrevPos);
			iPrevPos = iPos + 1;
			sResult.append("\\\\", 2);
		}
		else if (uCH == '"')
		{
			sResult.append(sSource, iPrevPos, iPos - iPrevPos);
			iPrevPos = iPos + 1;
			sResult.append("\\\"", 2);
		}
		else if (bECMAConventions)
		{
			if (uCH == '\'')
			{
				sResult.append(sSource, iPrevPos, iPos - iPrevPos);
				iPrevPos = iPos + 1;
				sResult.append("\\\'", 2);
			}
			else if (uCH == '\a')
			{
				sResult.append(sSource, iPrevPos, iPos - iPrevPos);
				iPrevPos = iPos + 1;
				sResult.append("\\a", 2);
			}
			else if (uCH == '\v')
			{
				sResult.append(sSource, iPrevPos, iPos - iPrevPos);
				iPrevPos = iPos + 1;
				sResult.append("\\v", 2);
			}
		}
		else if (bHTMLSafe)
		{
			if (uCH == '<')
			{
				sResult.append(sSource, iPrevPos, iPos - iPrevPos);
				iPrevPos = iPos + 1;
				sResult.append("\\u003c", 6);
			}
			else if (uCH == '>')
			{
				sResult.append(sSource, iPrevPos, iPos - iPrevPos);
				iPrevPos = iPos + 1;
				sResult.append("\\u003e", 6);
			}
		}

		++iPos;
		if (iPos == iSize) { break; }
	}

	if (iPos != iPrevPos) { sResult.append(sSource, iPrevPos, iPos - iPrevPos); }

return sResult;
}

//
// Dump CDT to JSON
//
void CDT2JSON(const CTPP::CDT & oCDT, STLW::string & sData)
{
	using namespace CTPP;
	switch (oCDT.GetType())
	{
		case CDT::UNDEF:
			sData.append("null");
			break;

		case CDT::INT_VAL:
		case CDT::REAL_VAL:
		case CDT::POINTER_VAL:
		case CDT::STRING_INT_VAL:
		case CDT::STRING_REAL_VAL:
			sData.append(oCDT.GetString());
			break;

		case CDT::STRING_VAL:
			sData.append("\"");
			sData.append(EscapeJSONString(oCDT.GetString(), false));
			sData.append("\"");
			break;

		case CDT::ARRAY_VAL:
			{
				sData.append("[");
				UINT_32 iJ = 0;
				while(iJ < oCDT.Size())
				{
					CDT2JSON(oCDT.GetCDT(iJ), sData);
					++iJ;
					if (iJ != oCDT.Size()) { sData.append(","); }
				}
				sData.append("]");
			}
			break;

		case CDT::HASH_VAL:
			{
				sData.append("{");
				CDT::ConstIterator itCDTCArray = oCDT.Begin();
				while (itCDTCArray != oCDT.End())
				{
					sData.append("\"");
					sData.append(EscapeJSONString(itCDTCArray -> first, false));
					sData.append("\":");

					CDT2JSON(itCDTCArray -> second, sData);

					++itCDTCArray;
					if (itCDTCArray != oCDT.End()) { sData.append(","); }
				}
				sData.append("}");
			}
			break;
	}
}

//
// Encode value
//
STLW::string Base64Encode(const STLW::string & sData)
{
	// RFC 3548 - The Base16, Base32, and Base64 Data Encodings
	static CHAR_8 aBase64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

	UINT_32 aDst[4];

	// Data
	UCCHAR_P szData = (UCCHAR_P)sData.c_str();
	// Data length
	const UINT_32 iDataLength      = sData.size();
	const UINT_32 iDataLengthFixed = 3 * (iDataLength / 3);

	// Temp buffer
	CHAR_P szBuffer = new CHAR_8[iDataLength * 4 / 3 + 4];
	CHAR_P szDst    = szBuffer;

	UINT_32 iDstLength = 0;
	UINT_32 iI         = 0;
	for (; iI < iDataLengthFixed;)
	{
		iI += 3;

		aDst[0] =   szData[0] >> 2;
		aDst[1] = ((szData[0] & 0x03) << 4) + (szData[1] >> 4);
		aDst[2] = ((szData[1] & 0x0f) << 2) + (szData[2] >> 6);
		aDst[3] =   szData[2] & 0x3f;

		*szDst++ = aBase64[ aDst[0] ];
		*szDst++ = aBase64[ aDst[1] ];
		*szDst++ = aBase64[ aDst[2] ];
		*szDst++ = aBase64[ aDst[3] ];

		szData     += 3;
		iDstLength += 4;
	}

	const UINT_32 iDelta = iDataLength - iDataLengthFixed;
	if (iDelta != 0)
	{
		UINT_32 aSrc[3];
		aSrc[0] = aSrc[1] = aSrc[2] = 0;

		for(iI = 0; iI < iDelta; ++iI) { aSrc[iI] = *szData++; }

		aDst[0] =   aSrc[0] >> 2;
		aDst[1] = ((aSrc[0] & 0x03) << 4) + (aSrc[1] >> 4);
		aDst[2] = ((aSrc[1] & 0x0f) << 2) + (aSrc[2] >> 6);

		*szDst++ = aBase64[ aDst[0] ];
		*szDst++ = aBase64[ aDst[1] ];

		if (iDelta == 1) { *szDst++ = '=';                }
		else             { *szDst++ = aBase64[ aDst[2] ]; }
		*szDst++ = '=';

		iDstLength += 4;
	}

	// Okay
	STLW::string sRetVal(szBuffer, iDstLength);

	delete [] szBuffer;

return sRetVal;
}

static UCHAR_8 aBase64Rev[256] =
{
	64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
	64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
	64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 62, 64, 64, 64, 63,
	52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 64, 64, 64, 64, 64, 64,
	64,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
	15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 64, 64, 64, 64, 64,
	64, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
	41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 64, 64, 64, 64, 64,
	64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
	64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
	64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
	64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
	64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
	64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
	64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
	64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64
};

//
// Decode value
//
STLW::string Base64Decode(const STLW::string & sData)
{
	// Data
	UCCHAR_P szData = (UCCHAR_P)sData.c_str();
	// Data length
	UINT_32 iDataLength = sData.size();

	// Temp buffer
	UCHAR_P szBuffer = new UCHAR_8[ ( (iDataLength + 3) / 4) * 3];
	UCHAR_P szDst     = szBuffer;

	UINT_32 iDstLength = 0;
	while (iDataLength > 4)
	{
		*szDst++ = (aBase64Rev[ szData[0] ] << 2 | aBase64Rev[ szData[1] ] >> 4);
		*szDst++ = (aBase64Rev[ szData[1] ] << 4 | aBase64Rev[ szData[2] ] >> 2);
		*szDst++ = (aBase64Rev[ szData[2] ] << 6 | aBase64Rev[ szData[3] ]);
		szData      += 4;
		iDataLength -= 4;
		iDstLength  += 3;
	}

	*szDst++ = (aBase64Rev[ szData[0] ] << 2 | aBase64Rev[ szData[1] ] >> 4);
	++iDstLength;

	if (szData[2] != '=') { *szDst++ = (aBase64Rev[ szData[1] ] << 4 | aBase64Rev[ szData[2] ] >> 2); ++iDstLength; }

	if (szData[3] != '=') { *szDst++ = (aBase64Rev[ szData[2] ] << 6 | aBase64Rev[ szData[3] ]);      ++iDstLength; }

	// Okay
	STLW::string sRetVal((CHAR_P)szBuffer, iDstLength);

	delete [] szBuffer;

return sRetVal;
}

//
// Length of UTF character
//
INT_32 utf_charlen(CCHAR_P szString, CCHAR_P szStringEnd)
{
	/*
	 * 0x00000000 — 0x0000007F 0x xxx xxx
	 * 0x00000080 — 0x000007FF 110xxxxx 10xxxxxx
	 * 0x00000800 — 0x0000FFFF 1110xxxx 10xxxxxx 10xxxxxx
	 * 0x00010000 — 0x001FFFFF 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
	 */

	// End of string
	if (szString == szStringEnd) { return -3; }

	// UTF8-1 0xxxxxxx
	if ((UCCHAR_8(*szString) & 0x80) == 0) { return 1; }

	// UTF8-2 110xxxxx 10xxxxxx
	if ((UCCHAR_8(*szString) & 0xE0) == 0xC0)
	{
		// Check next byte
		++szString;
		if (szString == szStringEnd) { return -3; }
		if ((UCCHAR_8(*szString) & 0xC0) != 0x80) { return -1; }

		// Ok
		return 2;
	}

	// UTF8-3 1110xxxx 10xxxxxx 10xxxxxx
	if ((UCCHAR_8(*szString) & 0xF0) == 0xE0)
	{
		// Check next byte
		++szString;
		if (szString == szStringEnd) { return -3; }
		if ((UCCHAR_8(*szString) & 0xC0) != 0x80) { return -1; }

		++szString;
		if (szString == szStringEnd) { return -3; }
		if ((UCCHAR_8(*szString) & 0xC0) != 0x80) { return -1; }

		// Ok
		return 3;
	}

	// UTF8-4 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
	if ((UCCHAR_8(*szString) & 0xF8) == 0xF0)
	{
		// Check next byte
		++szString;
		if (szString == szStringEnd) { return -3; }
		if ((UCCHAR_8(*szString) & 0xC0) != 0x80) { return -1; }

		++szString;
		if (szString == szStringEnd) { return -3; }
		if ((UCCHAR_8(*szString) & 0xC0) != 0x80) { return -1; }

		++szString;
		if (szString == szStringEnd) { return -3; }
		if ((UCCHAR_8(*szString) & 0xC0) != 0x80) { return -1; }

		// Ok
		return 4;
	}

// This is not an UTF8 character
return -2;
}
} // namespace CTPP
// End.
