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
 *      CTPP2StaticData.cpp
 *
 * $CTPP$
 */

#include "CTPP2StaticData.hpp"

#include <stdlib.h>
#include <string.h>

#include "CTPP2BitIndex.hpp"

namespace CTPP // C++ Template Engine
{

//
// Constructor
//
ReducedStaticData::ReducedStaticData(const StaticDataVar  * aIData,
                                     const UINT_32          iIUsedDataSize): aData(aIData),
                                                                             iUsedDataSize(iIUsedDataSize)
{
	;;
}

//
// Get integer value by ID
//
INT_64 ReducedStaticData::GetInt(const UINT_32  iDataId) const
{
	if (iDataId < iUsedDataSize) { return aData[iDataId].i_data; }

return 0;
}

//
// Get floating point value by ID
//
W_FLOAT ReducedStaticData::GetFloat(const UINT_32  iDataId) const
{
	if (iDataId < iUsedDataSize) { return aData[iDataId].d_data; }

return 0.0;
}

//
// A destructor
//
ReducedStaticData::~ReducedStaticData() throw() { ;; }

// ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Class StaticData
//

//
// Constructor
//
StaticData::StaticData(const UINT_32  iIMaxDataSize): iMaxDataSize(iIMaxDataSize),
                                                      iUsedDataSize(0),
                                                      aData(NULL)
{
	if (iMaxDataSize != 0)
	{
		aData = (StaticDataVar *)malloc(iMaxDataSize * sizeof(StaticDataVar));
		oBitIndex = new BitIndex(iMaxDataSize);
	}
	else
	{
		oBitIndex = new BitIndex(32);
	}

}

//
// Constructor
//
StaticData::StaticData(const StaticDataVar  * aIData,
                       const UINT_32          iIMaxDataSize): iMaxDataSize(iIMaxDataSize),
                                                              iUsedDataSize(iMaxDataSize),
                                                              aData(NULL)
{
	aData = (StaticDataVar *)malloc(iMaxDataSize * sizeof(StaticDataVar));
	memcpy(aData, aIData, iMaxDataSize * sizeof(StaticDataVar));
}

//
// Store data
//
UINT_32 StaticData::StoreInt(const UINT_64  iData)
{
	if (iUsedDataSize == iMaxDataSize)
	{
		iMaxDataSize = iMaxDataSize * 2 + 1;
		StaticDataVar * aTMP = (StaticDataVar *)malloc(iMaxDataSize * sizeof(StaticDataVar));
		if (aData != NULL)
		{
			memcpy(aTMP, aData, iUsedDataSize * sizeof(StaticDataVar));
			free(aData);
		}
		aData = aTMP;
	}

	// Store information about value type
	oBitIndex -> SetBit(iUsedDataSize, 0);

	// Store value
	aData[iUsedDataSize].i_data = iData;

return iUsedDataSize++;
}

//
// Store data
//
UINT_32 StaticData::StoreFloat(const W_FLOAT  dData)
{
	if (iUsedDataSize == iMaxDataSize)
	{
		iMaxDataSize = iMaxDataSize * 2 + 1;
		StaticDataVar * aTMP = (StaticDataVar *)malloc(iMaxDataSize * sizeof(StaticDataVar));
		if (aData != NULL)
		{
			memcpy(aTMP, aData, iUsedDataSize * sizeof(StaticDataVar));
			free(aData);
		}
		aData = aTMP;
	}

	// Store information about value type
	oBitIndex -> SetBit(iUsedDataSize, 1);

	// Store value
	aData[iUsedDataSize].d_data = dData;

return iUsedDataSize++;
}

//
// Get integer value by ID
//
INT_64 StaticData::GetInt(const UINT_32  iDataId) const
{
	if (iDataId < iUsedDataSize) { return aData[iDataId].i_data; }

return 0;
}

//
// Get floating point value by ID
//
W_FLOAT StaticData::GetFloat(const UINT_32  iDataId) const
{
	if (iDataId < iUsedDataSize) { return aData[iDataId].d_data; }

return 0.0;
}

//
// Get bit index
//
const BitIndex * StaticData::GetBitIndex() const { return oBitIndex; }

//
// A destructor
//
StaticData::~StaticData() throw()
{
	free(aData);
	delete oBitIndex;
}

} // namespace CTPP
// End.
