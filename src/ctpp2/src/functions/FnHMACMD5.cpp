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
 *      FnHMACMD5.cpp
 *
 * $CTPP$
 */

#include "CDT.hpp"
#include "CTPP2Logger.hpp"
#include "FnHMACMD5.hpp"

#ifdef MD5_SUPPORT
    #ifndef WIN32
        #ifdef MD5_WITHOUT_OPENSSL
            #include <md5.h>
            #define MD5_Init    MD5Init
            #define MD5_Update  MD5Update
            #define MD5_Final   MD5Final
        #else
            #include <openssl/md5.h>
        #endif
    #endif // ifndef WIN32
#endif // MD5_SUPPORT

namespace CTPP // C++ Template Engine
{
#ifdef MD5_SUPPORT
//
// Constructor
//
FnHMACMD5::FnHMACMD5()
{
#ifdef WIN32
	// http://msdn.microsoft.com/en-us/library/aa382375(VS.85).aspx
	// Get a handle to the default PROV_RSA_FULL provider.
	if(CryptAcquireContext(&hCryptProv, NULL, NULL, PROV_RSA_FULL, 0))
	{
		// Okay
	}
	else
	{
		if (GetLastError() == NTE_BAD_KEYSET)
		{
			// No default container was found. Attempt to create it.
			if(CryptAcquireContext(&hCryptProv, NULL, NULL, PROV_RSA_FULL, CRYPT_NEWKEYSET))
			{
				// Okay
			}
			else
			{
				throw CTPPUnixException("Could not create the default key container.", GetLastError());
			}
		}
		else
		{
			throw CTPPUnixException("A general error running CryptAcquireContext", GetLastError());
		}
	}
#else
	;;
#endif
}

//
// Handler
//
#ifndef WIN32
INT_32 FnHMACMD5::Handler(CDT            * aArguments,
                          const UINT_32    iArgNum,
                          CDT            & oCDTRetVal,
                          Logger         & oLogger)
{
	static CHAR_8 aHex[] = "0123456789abcdef";
	CHAR_8 szMD5[32];

	// Only 2 args allowed
	if (iArgNum != 2)
	{
		oLogger.Emerg("Usage: HMAC_MD5(key, message)");
		return -1;
	}

	MD5_CTX   oContext;
	UCHAR_8  sIPad[65];
	UCHAR_8  sOPad[65];

	// Construct key
	STLW::string sKey = aArguments[0].GetString();
	// If key is longer than 64 bytes reset it to key=MD5(key)
	if (sKey.size() > 64)
	{
		UCHAR_8  sTempKey[16];
		MD5_Init(&oContext);
		MD5_Update(&oContext, sKey.data(), sKey.size());
		MD5_Final(sTempKey, &oContext);

		sKey.assign((CCHAR_P)sTempKey, 16);
	}

	/*
	 * MD5(K XOR opad, MD5(K XOR ipad, text))
	 *
	 * where K is an n byte key
	 * ipad is the byte 0x36 repeated 64 times
	 * opad is the byte 0x5c repeated 64 times
	 * and text is the data being protected
	 */
	UINT_32 iI = 0;
	for(; iI < sKey.size(); ++iI)
	{
		sIPad[iI] = 0x36 ^ UCHAR_8(sKey[iI]);
		sOPad[iI] = 0x5C ^ UCHAR_8(sKey[iI]);
	}

	for(; iI < 64; ++iI)
	{
		sIPad[iI] = 0x36;
		sOPad[iI] = 0x5C;
	}

	unsigned char sDigest[16];

	STLW::string sSource = aArguments[1].GetString();
	// Inner MD5
	MD5_Init(&oContext);
	MD5_Update(&oContext, sIPad, 64);
	MD5_Update(&oContext, sSource.data(), sSource.size());
	MD5_Final(sDigest, &oContext);

	// Outer MD5
	MD5_Init(&oContext);
	MD5_Update(&oContext, sOPad, 64);
	MD5_Update(&oContext, sDigest, 16);
	MD5_Final(sDigest, &oContext);

	INT_32 iJ = 0;
	for (iI = 0; iI < 16; ++iI)
	{
		szMD5[ iJ++] = aHex[ (sDigest[iI] >> 4 )& 0x0F ];
		szMD5[ iJ++] = aHex[  sDigest[iI]       & 0x0F ];
	}

	// Okay
	oCDTRetVal = STLW::string(szMD5, 32);

return 0;
}

#else // WIN32

INT_32 FnHMACMD5::Handler(CDT            * aArguments,
                          const UINT_32    iArgNum,
                          CDT            & oCDTRetVal,
                          Logger         & oLogger)
{
	static CHAR_8 aHex[] = "0123456789abcdef";
	CHAR_8 szMD5[32];
	// >0 arguments need
	if (iArgNum != 2 || hCryptProv == 0) { return -1; }

	HCRYPTHASH hHash;
	UCHAR_8  sIPad[65];
	UCHAR_8  sOPad[65];

	// Construct key
	STLW::string sKey = aArguments[0].GetString();
	// If key is longer than 64 bytes reset it to key=MD5(key)
	if (sKey.size() > 64)
	{
		if (::CryptCreateHash(hCryptProv, CALG_MD5, 0, 0, &hHash) == 0) { return -1; }

		if (::CryptHashData(hHash, (BYTE *)sKey.c_str(), (DWORD)sKey.size(), 0) == 0)
		{
			::CryptDestroyHash(hHash);
			return -1;
		}

		BYTE sDigest[16];
		DWORD dwCount = 16;
		if (::CryptGetHashParam(hHash, HP_HASHVAL, sDigest, &dwCount, 0) == 0)
		{
			::CryptDestroyHash(hHash);
			return -1;
		}
		::CryptDestroyHash(hHash);
		sKey.assign((CCHAR_P)sDigest, 16);
	}

	/*
	 * MD5(K XOR opad, MD5(K XOR ipad, text))
	 *
	 * where K is an n byte key
	 * ipad is the byte 0x36 repeated 64 times
	 * opad is the byte 0x5c repeated 64 times
	 * and text is the data being protected
	 */
	UINT_32 iI = 0;
	for(; iI < sKey.size(); ++iI)
	{
		sIPad[iI] = 0x36 ^ UCHAR_8(sKey[iI]);
		sOPad[iI] = 0x5C ^ UCHAR_8(sKey[iI]);
	}

	for(; iI < 64; ++iI)
	{
		sIPad[iI] = 0x36;
		sOPad[iI] = 0x5C;
	}


	BYTE sDigest[16];
	DWORD dwCount = 16;

	STLW::string sSource = aArguments[1].GetString();
	// Inner MD5
	if (::CryptCreateHash(hCryptProv, CALG_MD5, 0, 0, &hHash) == 0) { return -1; }

	if (::CryptHashData(hHash, (BYTE *)sIPad, (DWORD)64, 0) == 0)
	{
		::CryptDestroyHash(hHash);
		return -1;
	}

	if (::CryptHashData(hHash, (BYTE *)sSource.data(), (DWORD)sSource.size(), 0) == 0)
	{
		::CryptDestroyHash(hHash);
		return -1;
	}

	if (::CryptGetHashParam(hHash, HP_HASHVAL, sDigest, &dwCount, 0) == 0)
	{
		::CryptDestroyHash(hHash);
		return -1;
	}
	::CryptDestroyHash(hHash);

	if (::CryptCreateHash(hCryptProv, CALG_MD5, 0, 0, &hHash) == 0) { return -1; }

	if (::CryptHashData(hHash, (BYTE *)sOPad, (DWORD)64, 0) == 0)
	{
		::CryptDestroyHash(hHash);
		return -1;
	}

	if (::CryptHashData(hHash, (BYTE *)sDigest, (DWORD)16, 0) == 0)
	{
		::CryptDestroyHash(hHash);
		return -1;
	}

	if (::CryptGetHashParam(hHash, HP_HASHVAL, sDigest, &dwCount, 0) == 0)
	{
		::CryptDestroyHash(hHash);
		return -1;
	}
	::CryptDestroyHash(hHash);

	INT_32 iJ = 0;
	for (iI = 0; iI < 16; ++iI)
	{
		szMD5[ iJ++] = aHex[ (sDigest[iI] >> 4 )& 0x0F ];
		szMD5[ iJ++] = aHex[  sDigest[iI]       & 0x0F ];
	}

	// Okay
	oCDTRetVal = STLW::string(szMD5, 32);

return 0;
}
#endif // WIN32
//
// Get function name
//
CCHAR_P FnHMACMD5::GetName() const { return "hmac_md5"; }

//
// A destructor
//
FnHMACMD5::~FnHMACMD5() throw()
{
#ifdef WIN32
	if (hCryptProv != 0) { ::CryptReleaseContext(hCryptProv, 0); }
#else
	;;
#endif
}

#endif // MD5_SUPPORT
} // namespace CTPP
// End.
