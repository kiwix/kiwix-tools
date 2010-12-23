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
 *      CTPP2StaticData.hpp
 *
 * $CTPP$
 */
#ifndef _CTPP2_STATIC_DATA_HPP__
#define _CTPP2_STATIC_DATA_HPP__ 1

#include "CTPP2Types.h"

/**
  @file CTPP2StaticData.hpp
  @brief Staitic data segment
*/

namespace CTPP // C++ Template Engine
{
class BitIndex;

/**
  @struct StaticDataVar CTPP2StaticData.hpp <CTPP2StaticData.hpp>
  @brief Ststic data variable
*/
union StaticDataVar
{
	/** Data (integer)  */
	INT_64   i_data;
	/** Data (IEEE 754) */
	W_FLOAT  d_data;
};

/**
  @class ReducedStaticData CTPP2StaticData.hpp <CTPP2StaticData.hpp>
  @brief Static text segment with reduced functionality
*/
class CTPP2DECL ReducedStaticData
{
public:

	/**
	  @brief Constructor
	  @param aIData - text data
	  @param iIUsedDataSize - max. data size
	*/
	ReducedStaticData(const StaticDataVar  * aIData,
	                  const UINT_32          iIUsedDataSize);

	/**
	  @brief Get integer value by ID
	  @param iDataId - data ID
	  @return Signed 64-bit value
	*/
	INT_64 GetInt(const UINT_32  iDataId) const;

	/**
	  @brief Get floating point value by ID
	  @param iDataId - data ID
	  @return Wide float value
	*/
	W_FLOAT GetFloat(const UINT_32  iDataId) const;

	/**
	  @brief A destructor
	*/
	~ReducedStaticData() throw();

protected:
	friend class VMDumper;

	/** Stored data                */
	const StaticDataVar  * aData;
	/** Used data buffer length    */
	UINT_32                iUsedDataSize;

};


/**
  @class StaticData CTPP2StaticData.hpp <CTPP2StaticData.hpp>
  @brief Static text segment
*/
class CTPP2DECL StaticData
{
public:
	/**
	  @brief Constructor
	  @param iIMaxDataSize - initial text segment size
	*/
	StaticData(const UINT_32  iIMaxDataSize = 0);

	/**
	  @brief Constructor
	  @param aIData - text data
	  @param iIMaxDataSize - max. data size
	*/
	StaticData(const StaticDataVar   * aIData,
	           const UINT_32           iIMaxDataSize);

	/**
	  @brief Store data
	  @param iData - data to store, signed 64-bit value
	  @return data ID
	*/
	UINT_32 StoreInt(const UINT_64  iData);

	/**
	  @brief Store data
	  @param dData - data to store, float value
	  @return data ID
	*/
	UINT_32 StoreFloat(const W_FLOAT dData);

	/**
	  @brief Get integer value by ID
	  @param iDataId - data ID
	  @return Signed 64-bit value
	*/
	INT_64 GetInt(const UINT_32  iDataId) const;

	/**
	  @brief Get floating point value by ID
	  @param iDataId - data ID
	  @return Wide float value
	*/
	W_FLOAT GetFloat(const UINT_32  iDataId) const;

	/**
	  @brief Get bit index
	  @return Pointer to bit index object
	*/
	const BitIndex * GetBitIndex() const;

	/**
	  @brief A destructor
	*/
	~StaticData() throw();
private:
	friend class VMDumper;

	/** Max. data buffer size      */
	UINT_32          iMaxDataSize;
	/** Used data buffer length    */
	UINT_32          iUsedDataSize;
	/** Stored data                */
	StaticDataVar  * aData;
	/** Bit index                  */
	BitIndex       * oBitIndex;
};

} // namespace CTPP
#endif // _CTPP2_STATIC_DATA_HPP__
// End.
