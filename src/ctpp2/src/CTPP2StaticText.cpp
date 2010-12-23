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
 *      CTPP2StaticText.cpp
 *
 * $CTPP$
 */
#include "CTPP2StaticText.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

namespace CTPP // C++ Template Engine
{

//
// Constructor
//
ReducedStaticText::ReducedStaticText(CCHAR_P                sIData,
                                     const UINT_32          iIUsedDataOffsetsSize,
                                     const TextDataIndex  * aIDataOffsets): sData(sIData),
                                                                            iUsedDataOffsetsSize(iIUsedDataOffsetsSize),
                                                                            aDataOffsets(aIDataOffsets)
{
	;;
}

//
// GetData by ID
//
CCHAR_P ReducedStaticText::GetData(const UINT_32    iDataId,
                                   UINT_32        & iDataSize) const
{
	if (iDataId >= iUsedDataOffsetsSize) { return NULL; }

	iDataSize = aDataOffsets[iDataId].length;

return &sData[aDataOffsets[iDataId].offset];
}

//
// Get number of stored records
//
UINT_32 ReducedStaticText::GetRecordsNum() const { return iUsedDataOffsetsSize; }

//
// A destructor
//
ReducedStaticText::~ReducedStaticText() throw() { ;; }

// ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Class StaticText
//

//
// Constructor
//
StaticText::StaticText(const UINT_32  iIMaxDataSize,
                       const UINT_32  iIMaxDataOffsetsSize): iMaxDataSize(iIMaxDataSize),
                                                             iMaxDataOffsetsSize(iIMaxDataOffsetsSize),
                                                             iUsedDataSize(0),
                                                             iUsedDataOffsetsSize(0),
                                                             sData(NULL),
                                                             aDataOffsets(NULL)
{
	if (iMaxDataSize != 0)        { sData        = (CHAR_P)malloc(iMaxDataSize);                 }
	if (iMaxDataOffsetsSize != 0) { aDataOffsets = (TextDataIndex *)malloc(iMaxDataOffsetsSize); }
}

//
// Constructor
//
StaticText::StaticText(CCHAR_P                sIData,
                       const TextDataIndex  * aIDataOffsets,
                       const UINT_32          iIMaxDataSize,
                       const UINT_32          iIMaxDataOffsetsSize): iMaxDataSize(iIMaxDataSize),
                                                                     iMaxDataOffsetsSize(iIMaxDataOffsetsSize),
                                                                     iUsedDataSize(iIMaxDataSize),
                                                                     iUsedDataOffsetsSize(iIMaxDataOffsetsSize),
                                                                     sData(NULL),
                                                                     aDataOffsets(NULL)
{
	if (iMaxDataSize != 0)        { sData        = (CHAR_P)malloc(iMaxDataSize); }
	if (iMaxDataOffsetsSize != 0) { aDataOffsets = (TextDataIndex *)malloc(iMaxDataOffsetsSize * sizeof(TextDataIndex)); }

	memcpy(sData, sIData, iMaxDataSize);
	memcpy(aDataOffsets, aIDataOffsets, iMaxDataOffsetsSize * sizeof(TextDataIndex));
}

//
// Store data
//
UINT_32 StaticText::StoreData(CCHAR_P sStoreData, const UINT_32  iDataLength)
{
	// New data offset
	UINT_32 iDataOffset = iUsedDataSize + iDataLength;

	// Resize data storage
	if (iDataOffset >= iMaxDataSize)
	{
		iMaxDataSize = iDataOffset * 2 + 1;
		CHAR_P sTMP = (CHAR_P)malloc(iMaxDataSize);

		if (sData != NULL)
		{
			memcpy(sTMP, sData, iUsedDataSize);
			free(sData);
		}
		sData = sTMP;
	}

	// Copy data
	memcpy(sData + iUsedDataSize, sStoreData, iDataLength);
	sData[iDataOffset] = '\0';

	if (iUsedDataOffsetsSize == iMaxDataOffsetsSize)
	{
		iMaxDataOffsetsSize = iMaxDataOffsetsSize * 2 + 1;
		TextDataIndex * aTMP = (TextDataIndex *)malloc(iMaxDataOffsetsSize * sizeof(TextDataIndex));
		if (aDataOffsets != NULL)
		{
			memcpy(aTMP, aDataOffsets, iUsedDataOffsetsSize * sizeof(TextDataIndex));
			free(aDataOffsets);
		}
		aDataOffsets = aTMP;
	}

	aDataOffsets[iUsedDataOffsetsSize].offset = iUsedDataSize;
	aDataOffsets[iUsedDataOffsetsSize].length = iDataLength;

	iUsedDataSize = iDataOffset + 1;

return iUsedDataOffsetsSize++;
}

//
// GetData by ID
//
CCHAR_P StaticText::GetData(const UINT_32    iDataId,
                            UINT_32        & iDataSize) const
{
	if (iDataId >= iUsedDataOffsetsSize) { return NULL; }

	iDataSize = aDataOffsets[iDataId].length;

return &sData[aDataOffsets[iDataId].offset];
}

//
// Get number of stored records
//
UINT_32 StaticText::GetRecordsNum() const { return iUsedDataOffsetsSize; }

//
// A destructor
//
StaticText::~StaticText() throw()
{
	free(sData);
	free(aDataOffsets);
}

} // namespace CTPP
// End.
