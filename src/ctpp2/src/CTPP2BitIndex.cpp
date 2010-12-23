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
 *      CTPP2BitIndex.cpp
 *
 * $CTPP$
 */
#include "CTPP2BitIndex.hpp"

#include <stdlib.h>
#include <string.h>

#include <stdio.h>

namespace CTPP // C++ Template Engine
{

//
// Constructor
//
ReducedBitIndex::ReducedBitIndex(const BitIndexData * aIData): aData(aIData) { ;; }

//
// Get index data
//
const BitIndexData * ReducedBitIndex::GetIndexData() const { return aData; }

//
// Get number of used bytes in index
//
UINT_32 ReducedBitIndex::GetUsedSize() const { return UINT_32(aData -> size); }

//
// Get bit from index
//
CHAR_8 ReducedBitIndex::GetBit(const UINT_32  iOffset) const
{
	UINT_32 iBytePos = iOffset >> 3;

	if (iBytePos >= aData -> size) { return 0; }

	UINT_32 iBitPos = iOffset & 7;
	CHAR_8  iResult = aData -> data[iBytePos];

return (iResult & (1 << iBitPos));
}

//
//  A destructor
//
ReducedBitIndex::~ReducedBitIndex() throw() { ;; }

// ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Class BitIndex
//

//
// Constructor
//
BitIndex::BitIndex(const UINT_32  iSize): aData(NULL)
{
	UINT_32 iRealSize = (iSize >> 3) + 1;

	void * vTMP = malloc(sizeof(UINT_64) + iRealSize);

	memset(vTMP, 0, sizeof(UINT_64) + iRealSize);

	aData = (BitIndexData *)vTMP;

	aData -> size = 0;
	iMaxSize = iRealSize;
}

//
// Get index data
//
const BitIndexData * BitIndex::GetIndexData() const { return aData; }

//
// Get number of used bytes in index
//
UINT_32 BitIndex::GetUsedSize() const { return UINT_32(aData -> size); }

//
// Get bit from index
//
CHAR_8 BitIndex::GetBit(const UINT_32  iOffset) const
{
	UINT_32 iBytePos = iOffset >> 3;

	if (iBytePos >= iMaxSize) { return 0; }

	UINT_32 iBitPos  = iOffset & 7;
	CHAR_8 iResult = aData -> data[iBytePos];

return (iResult & (1 << iBitPos));
}

//
// Get bit from index
//
void BitIndex::SetBit(const UINT_32  iOffset,
                      const CHAR_8   iValue)
{
	UINT_32 iBytePos = iOffset >> 3;
	UINT_32 iBitPos  = iOffset & 7;

	if (iBytePos >= iMaxSize) { ResizeBitIndex(iBytePos); }

	if (aData -> size <= iBytePos)
	{
		aData -> size = iBytePos + 1;
	}

	CHAR_8 & iResult = aData -> data[iBytePos];

	if (iValue != 0) { iResult |=         (1 << iBitPos);  } // Set bit
	else             { iResult &= (0xFF ^ (1 << iBitPos)); } // Remove bit
}

//
// Resize bit index
//
void BitIndex::ResizeBitIndex(const UINT_32 & iBytePos)
{
	UINT_32 iNewSize = (iBytePos << 1);

	void * vTMP = malloc(sizeof(UINT_64) + iNewSize);
	memset(vTMP, 0, sizeof(UINT_64) + iNewSize);

	UINT_32 iUsedSize = UINT_32(aData -> size);
	memcpy((CHAR_P)vTMP + sizeof(UINT_64), aData -> data, iMaxSize);
	free(aData);

	aData = (BitIndexData *)vTMP;
	aData -> size = iUsedSize;

	iMaxSize = iNewSize;
}

//
// A destructor
//
BitIndex::~BitIndex() throw() { free(aData); }

} // namespace CTPP
// End.
