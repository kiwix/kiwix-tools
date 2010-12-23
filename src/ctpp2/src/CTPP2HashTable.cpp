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
 *      CTPP2HashTable.cpp
 *
 * $CTPP$
 */

#include "CTPP2HashTable.hpp"

#include <stdio.h>

namespace CTPP // C++ Template Engine
{

//
// hash function
//
UINT_64 HashFunc(CCHAR_P        sKey,
                 const UINT_32  iLength)
{
	UINT_64 ulHash = 5381;
	UCCHAR_P sEnd = (UCCHAR_P)sKey + iLength;

	while ((UCCHAR_P)sKey != sEnd)
	{
		ulHash += (ulHash << 5);
		ulHash ^= (UINT_64) *(UCCHAR_P)sKey++;
	}
return ulHash;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Class ReducedHashTable
//

//
// Constructor
//
ReducedHashTable::ReducedHashTable(const HashElement  * aIElements,
                                   const UINT_32        iIPower): aElements(aIElements),
                                                                  iPower(iIPower)
{
	iBase = (1 << iPower) - 1;
}

//
// Get value from hash
//
UINT_64 ReducedHashTable::Get(CCHAR_P        szKey,
                              const UINT_32  iKeyLength) const
{
	// 1. Calculate hash
	const UINT_64 iHash = HashFunc(szKey, iKeyLength);
	// 2. Get position in array
	const UINT_64 iPos = iHash & iBase;
	// 3. Check hash value
	const HashElement & oElement = aElements[iPos];

	// Mistake?
	if (iHash != oElement.hash) { return (UINT_64)-1; }

return oElement.value;
}

//
// Get hash size
//
UINT_64 ReducedHashTable::Size() const { return UINT_64(1 << iPower); }

//
// A destructor
//
ReducedHashTable::~ReducedHashTable() throw() { ;; }

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Class HashTable
//

//
// Constructor
//
HashTable::HashTable(): iPower(1)
{
	const UINT_32 iNewElements = 1 << iPower;
	aElements = new HashElement[iNewElements];
	iBase = iNewElements - 1;
	iUsed = 0;

	UINT_32 iPos = 0;
	for(; iPos < iNewElements; ++iPos)
	{
		HashElement & oNewElement = aElements[iPos];

		oNewElement.hash  = (UINT_64)-1;
		oNewElement.value = (UINT_64)-1;
	}
}

//
// Get value from hash
//
INT_64 HashTable::Get(CCHAR_P        szKey,
                      const UINT_32  iKeyLength) const
{
	// 1. Calculate hash
	const UINT_64 iHash = HashFunc(szKey, iKeyLength);
	// 2. Get position in array
	const UINT_64 iPos = iHash & iBase;
	// 3. Check hash value
	const HashElement & oElement = aElements[iPos];

	// Mistake?
	if (iHash != oElement.hash) { return -1; }

return oElement.value;
}

//
// Get hash size
//
UINT_64 HashTable::Size() const { return UINT_64(1 << iPower); }

//
// Put value into hash
//
UINT_32 HashTable::Put(CCHAR_P        szKey,
                       const UINT_32  iKeyLength,
                       const UINT_64  iValue)
{
	// 1. Calculate hash
	const UINT_64 iHash = HashFunc(szKey, iKeyLength);

	for (;;)
	{
		// 2. Get position in array
		const UINT_64 iPos = iHash & iBase;
		// 3. Check hash value
		HashElement & oElement = aElements[iPos];

		// Duplicate!
		if (oElement.hash == iHash) { return (UINT_32)-1; }

		// Resize hash
		if (oElement.value != (UINT_64)-1) { Resize(); }
		else
		{
			++iUsed;
			oElement.hash  = iHash;
			oElement.value = iValue;
			return 0;
		}
	}

// Make compiler happy
return (UINT_32)-1;
}

//
// Resize hash
//
void HashTable::Resize()
{
	const UINT_32 iOldElements = 1 << iPower;

	++iPower;
	const UINT_32 iNewElements = 1 << iPower;

	// Create new array of elements
	HashElement  * aTMP = new HashElement[iNewElements];
	iBase = iNewElements - 1;

	UINT_32 iPos = 0;
	for(; iPos < iNewElements; ++iPos)
	{
		HashElement & oNewElement = aTMP[iPos];

		oNewElement.hash  = (UINT_64)-1;
		oNewElement.value = (UINT_64)-1;
	}

	for(iPos = 0; iPos < iOldElements; ++iPos)
	{
		// Element in old array
		const HashElement & oOldElement = aElements[iPos];
		if (oOldElement.hash != (UINT_64)-1)
		{
			// Get new position in array
			const UINT_64 iNewPos = oOldElement.hash & iBase;

			HashElement & oNewElement = aTMP[iNewPos];

			oNewElement.hash  = oOldElement.hash;
			oNewElement.value = oOldElement.value;
		}
	}

	delete [] aElements;
	aElements = aTMP;
}

//
// A destructor
//
HashTable::~HashTable() throw()
{
	delete [] aElements;
}

} // namespace CTPP
// End.
