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
 *      CTPP2StaticText.hpp
 *
 * $CTPP$
 */
#ifndef _CTPP2_STATIC_TEXT_HPP__
#define _CTPP2_STATIC_TEXT_HPP__ 1

#include "CTPP2Types.h"

/**
  @file CTPP2StaticText.hpp
  @brief Staitic text segment
*/

namespace CTPP // C++ Template Engine
{

/**
  @struct TextDataIndex CTPP2StaticText.hpp <CTPP2StaticText.hpp>
  @brief Index for static data segment
*/
struct TextDataIndex
{
	/** Data offset */
	UINT_32   offset;
	/** Data length */
	UINT_32   length;
};

/**
  @class ReducedStaticText CTPP2StaticText.hpp <CTPP2StaticText.hpp>
  @brief Static text segment with reduced functionality
*/
class CTPP2DECL ReducedStaticText
{
public:
	/**
	  @brief Constructor
	  @param sIData - stored text
	  @param iIUsedDataOffsetsSize - number of cells in aIDataOffsets array
	  @param aIDataOffsets - data offsets index
	*/
	ReducedStaticText(CCHAR_P                sIData,
	                  const UINT_32          iIUsedDataOffsetsSize,
	                  const TextDataIndex  * aIDataOffsets);

	/**
	  @brief GetData by ID
	  @param iDataId - data ID
	  @param iDataSize - data length
	  @return pointer to data of NULL if any error occured
	*/
	CCHAR_P GetData(const UINT_32    iDataId,
	                UINT_32        & iDataSize) const;

	/**
	  @brief Get number of stored records
	  @return number of stored records
	*/
	UINT_32 GetRecordsNum() const;

	/**
	  @brief A destructor
	*/
	~ReducedStaticText() throw();

protected:
	friend class VMDumper;
	/** Stored data                */
	CCHAR_P                sData;
	/** Number of used cells in TextDataIndex array */
	const UINT_32          iUsedDataOffsetsSize;
	/** Stored data offsets        */
	const TextDataIndex  * aDataOffsets;
};

/**
  @class StaticText CTPP2StaticText.hpp <CTPP2StaticText.hpp>
  @brief Static text segment
*/
class CTPP2DECL StaticText
{
public:
	/**
	  @brief Constructor
	  @param iIMaxDataSize - initial text segment size
	  @param iIMaxDataOffsetsSize - initial index size
	*/
	StaticText(const UINT_32  iIMaxDataSize = 0,
	           const UINT_32  iIMaxDataOffsetsSize = 0);

	/**
	  @brief Constructor
	  @param sData - text data
	  @param aDataOffsets - array with data offsets
	  @param iMaxDataSize - max. data size
	  @param iMaxDataOffsetsSize - max size of data offsets array
	*/
	StaticText(CCHAR_P                sData,
	           const TextDataIndex  * aDataOffsets,
	           const UINT_32          iMaxDataSize,
	           const UINT_32          iMaxDataOffsetsSize);

	/**
	  @brief Store data
	  @param sStoreData - text data
	  @param iDataLength - data length
	  @return data ID
	*/
	UINT_32 StoreData(CCHAR_P          sStoreData,
	                  const UINT_32    iDataLength);

	/**
	  @brief GetData by ID
	  @param iDataId - data ID
	  @param iDataSize - data length
	  @return pointer to data of NULL if any error occured
	*/
	CCHAR_P GetData(const UINT_32    iDataId,
	                UINT_32        & iDataSize) const;

	/**
	  @brief Get number of stored records
	  @return number of stored records
	*/
	UINT_32 GetRecordsNum() const;

	/**
	  @brief A destructor
	*/
	~StaticText() throw();
private:
	friend class VMDumper;

	/** Max. data buffer size      */
	UINT_32          iMaxDataSize;
	/** Max. offsets array length  */
	UINT_32          iMaxDataOffsetsSize;
	/** Used data buffer length    */
	UINT_32          iUsedDataSize;
	/** Number of used cells
	        in TextDataIndex array */
	UINT_32          iUsedDataOffsetsSize;
	/** Stored data                */
	CHAR_P           sData;
	/** Stored data offsets        */
	TextDataIndex  * aDataOffsets;
};

} // namespace CTPP
#endif // _CTPP2_STATIC_TEXT_HPP__
// End.
