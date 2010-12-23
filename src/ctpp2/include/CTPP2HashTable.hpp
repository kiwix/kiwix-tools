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
 *      CTPP2HashTable.hpp
 *
 * $CTPP$
 */
#ifndef _CTPP2_HASH_TABLE_HPP__
#define _CTPP2_HASH_TABLE_HPP__ 1

#include "CTPP2Types.h"

/**
  @file CTPP2HashTable.hpp
  @brief Zero collision hash
*/

namespace CTPP // C++ Template Engine
{

/**
  @struct HashElement CTPP2HashTable.hpp <CTPP2HashTable.hpp>
  @brief Static data variable
*/
struct HashElement
{
	/** Hash value */
	UINT_64  hash;
	/** Data length */
	UINT_64  value;
};

/**
  @class ReducedHashTable CTPP2HashTable.hpp <CTPP2HashTable.hpp>
  @brief Hash without collisions with reduced functionality
*/
class CTPP2DECL ReducedHashTable
{
public:
	/**
	  @brief Constructor
	  @param aIElements - hash elements
	  @param iIPower - number of elements
	*/
	ReducedHashTable(const HashElement  * aIElements,
	                 const UINT_32        iIPower);

	/**
	  @brief Get value from hash
	  @param szKey - key name
	  @param iKeyLength - key length
	  @return >0 - if success, -1 - if any error occured
	*/
	UINT_64 Get(CCHAR_P        szKey,
	            const UINT_32  iKeyLength) const;

	/**
	  @brief Get hash size
	*/
	UINT_64 Size() const;

	/**
	  @brief A destructor
	*/
	~ReducedHashTable() throw();

private:
	friend class VMDumper;

	// Does not exist
	ReducedHashTable(const ReducedHashTable & oRhs);

	// Does not exist
	ReducedHashTable & operator=(const ReducedHashTable & oRhs);

	/** Hash elements      */
	const HashElement  * aElements;
	/** Power of 2         */
	UINT_32              iPower;
	/** Base of hash       */
	UINT_64              iBase;
};

/**
  @class HashTable CTPP2HashTable.hpp <CTPP2HashTable.hpp>
  @brief Hash without collisions
*/
class CTPP2DECL HashTable
{
public:
	/**
	  @brief Constructor
	*/
	HashTable();

	/**
	  @brief Get value from hash
	  @param szKey - key name
	  @param iKeyLength - key length
	  @return >0 - if success, -1 - if any error occured
	*/
	INT_64 Get(CCHAR_P        szKey,
	           const UINT_32  iKeyLength) const;

	/**
	  @brief Get hash size
	*/
	UINT_64 Size() const;

	/**
	  @brief Put value into hash
	  @param szKey - key name
	  @param iKeyLength - key length
	  @param iValue - value to store
	  @return 0 - if success, -1 - if any error occured
	*/
	UINT_32 Put(CCHAR_P        szKey,
	            const UINT_32  iKeyLength,
	            const UINT_64  iValue);

	/**
	  @brief A destructor
	*/
	~HashTable() throw();
private:
	friend class VMDumper;

	// Does not exist
	HashTable(const HashTable & oRhs);

	// Does not exist
	HashTable & operator=(const HashTable & oRhs);

	/** Hash elements           */
	HashElement  * aElements;
	/** Power of 2              */
	UINT_32        iPower;
	/** Base of hash            */
	UINT_64        iBase;
	/** Number of used elements */
	UINT_64        iUsed;

	/**
	  @brief Resize hash
	*/
	void Resize();
};

} // namespace CTPP
#endif // _CTPP2_HASH_TABLE_HPP__
// End.
