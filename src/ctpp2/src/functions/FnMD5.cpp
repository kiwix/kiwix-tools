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
 *      FnMD5.cpp
 *
 * $CTPP$
 */

#include "CDT.hpp"
#include "CTPP2Logger.hpp"
#include "FnMD5.hpp"

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
FnMD5::FnMD5()
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
//
// Handler, using Win32 CryptoAPI
//
INT_32 FnMD5::Handler(CDT            * aArguments,
                      const UINT_32    iArgNum,
                      CDT            & oCDTRetVal,
                      Logger         & oLogger)
{
	static CHAR_8 aHex[] = "0123456789abcdef";
	CHAR_8 szMD5[32];
	// >0 arguments need
	if (iArgNum == 0)
	{
		oLogger.Emerg("Usage: MD5(data1, data2, ..., dataN); at least 1 argument need");
		return -1;
	}

	MD5_CTX oMD5Context;
	MD5_Init(&oMD5Context);

	INT_32 iI = iArgNum - 1;
	for (; iI >= 0; --iI)
	{
		// Data
		const STLW::string & sTMP = aArguments[iI].GetString();
		CCHAR_P szData = sTMP.c_str();
		// Data length
		const UINT_32 iDataLength = sTMP.size();

		MD5_Update(&oMD5Context, (const unsigned char *)szData, iDataLength);
	}

	unsigned char sDigest[16];
	MD5_Final(sDigest, &oMD5Context);

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

//
// Handler, using Win32 CryptoAPI
//
INT_32 FnMD5::Handler(CDT            * aArguments,
                      const UINT_32    iArgNum,
                      CDT            & oCDTRetVal,
                      Logger         & oLogger)
{
	static CHAR_8 aHex[] = "0123456789abcdef";
	CHAR_8 szMD5[32];
	HCRYPTHASH hHash;

	// >0 arguments need
	if (iArgNum == 0 || hCryptProv == 0) { fprintf(stderr, "FnMD5 iArgNum = %d hCryptProv = %d\n", iArgNum, hCryptProv); return -1; }

	if (::CryptCreateHash(hCryptProv, CALG_MD5, 0, 0, &hHash) == 0) { fprintf(stderr, "CryptCreateHash\n"); return -1; }

	INT_32 iI = iArgNum - 1;
	for (; iI >= 0; --iI)
	{
		// Data
		const STLW::string & sTMP = aArguments[iI].GetString();
		if (::CryptHashData(hHash, (BYTE *)sTMP.c_str(), (DWORD)sTMP.size(), 0) == 0)
		{
			fprintf(stderr, "CryptHashData\n");
			::CryptDestroyHash(hHash);
			return -1;
		}
	}

	BYTE sDigest[16];
	DWORD dwCount = 16;
	if (::CryptGetHashParam(hHash, HP_HASHVAL, sDigest, &dwCount, 0) == 0)
	{
		fprintf(stderr, "CryptGetHashParam\n");
		::CryptDestroyHash(hHash);
		return -1;
	}

	INT_32 iJ = 0;
	for (iI = 0; iI < 16; ++iI)
	{
		szMD5[ iJ++] = aHex[ (sDigest[iI] >> 4 )& 0x0F ];
		szMD5[ iJ++] = aHex[  sDigest[iI]       & 0x0F ];
	}

	::CryptDestroyHash(hHash);

	// Okay
	oCDTRetVal = STLW::string(szMD5, 32);

return 0;
}
#endif // WIN32

//
// Get function name
//
CCHAR_P FnMD5::GetName() const { return "md5"; }

//
// A destructor
//
FnMD5::~FnMD5() throw()
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
