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
 *      CTPP2BitIndex.hpp
 *
 * $CTPP$
 */
#ifndef _CTPP2_BIT_INDEX_HPP__
#define _CTPP2_BIT_INDEX_HPP__ 1

#include "CTPP2Types.h"

/**
  @file CTPP2BitIndex.hpp
  @brief Bit index implementation
*/

namespace CTPP // C++ Template Engine
{

/**
  @struct BitIndexData CTPP2BitIndex.hpp <CTPP2BitIndex.hpp>
  @brief Bit index
*/
struct BitIndexData
{
	/** Index size */
	UINT_64    size;    // Alignment !
	/** Index data */
	CHAR_8     data[8]; // Alignment !
};


/**
  @class ReducedBitIndex CTPP2BitIndex.hpp <CTPP2BitIndex.hpp>
  @brief Reduced bit index functionality
*/
class CTPP2DECL ReducedBitIndex
{
public:
	/**
	  @brief Constructor
	  @param aIData - index data object
	*/
	ReducedBitIndex(const BitIndexData * aIData);

	/**
	  @brief Get index data
	  @return Pointer to BitIndexData structure
	*/
	const BitIndexData * GetIndexData() const;

	/**
	  @brief  Get number of used bytes in index
	  @return Number of used bytes in index
	*/
	UINT_32 GetUsedSize() const;

	/**
	  @brief Get bit from index
	  @param iOffset - bit offset
	  @return bit from index
	*/
	CHAR_8 GetBit(const UINT_32  iOffset) const;

	/**
	  @brief A destructor
	*/
	~ReducedBitIndex() throw();
private:

	/** Bit index           */
	const BitIndexData   * aData;

};

/**
  @class BitIndex CTPP2BitIndex.hpp <CTPP2BitIndex.hpp>
  @brief Bit index
*/
class CTPP2DECL BitIndex
{
public:
	/**
	  @brief Constructor
	  @param iSize - initial index size
	*/
	BitIndex(const UINT_32  iSize);

	/**
	  @brief Get index data
	  @return Pointer to BitIndexData structure
	*/
	const BitIndexData * GetIndexData() const;

	/**
	  @brief Get number of used bytes in index
	  @return Number of used bytes in index
	*/
	UINT_32 GetUsedSize() const;

	/**
	  @brief Get bit from index
	  @param iOffset - bit offset
	  @return bit from index
	*/
	CHAR_8 GetBit(const UINT_32  iOffset) const;

	/**
	  @brief Get bit from index
	  @param iOffset - bit offset
	  @param iValue - value to set
	  @return bit from index
	*/
	void SetBit(const UINT_32  iOffset, const CHAR_8 iValue);

	/**
	  @brief A destructor
	*/
	~BitIndex() throw();
private:
	/** Bit index           */
	BitIndexData         * aData;

	/** Max. bit index size */
	UINT_32                iMaxSize;

	// Does not exist
	BitIndex(const BitIndex & oRhs);

	// Does not exist
	BitIndex & operator=(const BitIndex & oRhs);

	/**
	  @brief Resize bit index
	*/
	void ResizeBitIndex(const UINT_32 & iBytePos);
};

} // namespace CTPP
#endif // _CTPP2_BIT_INDEX_HPP__
// End.
