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
 *      CDTSortRoutines.hpp
 *
 * $CTPP$
 */
#ifndef _CDT_SORT_ROUTINES_HPP__
#define _CDT_SORT_ROUTINES_HPP__ 1

/**
  @file CDTSortRoutines.hpp
  @brief Sorting routines
*/

#include "CDT.hpp"

namespace CTPP // C++ Template Engine
{

/**
  @class SortCompareNum CDTSortRoutines.hpp <CDTSortRoutines.hpp>
  @brief Sorting comparator
*/
class SortCompareNum:
  public CDT::SortingComparator
{
public:
	/**
	  @brief Constructor
	  @param eIDirection - sorting direction
	*/
	SortCompareNum(const CDT::SortingComparator::eSortingDirection  & eIDirection = CDT::SortingComparator::ASC);

	/**
	  @brief Compare two values.
	  @param oX - first value to compare
	  @param oY - seond value to compare
	  @return true, if oX < oY, false - otherwise
	*/
	bool operator()(const CDT & oX, const CDT & oY) const;

	/**
	  @brief A destructor
	*/
	~SortCompareNum() throw();
private:
	/** Sorting direction   */
	const CDT::SortingComparator::eSortingDirection  eDirection;
};

/**
  @class SortCompareStr CDTSortRoutines.hpp <CDTSortRoutines.hpp>
  @brief Sorting comparator
*/
class SortCompareStr:
  public CDT::SortingComparator
{
public:
	/**
	  @brief Constructor
	  @param eIDirection - sorting direction
	*/
	SortCompareStr(const CDT::SortingComparator::eSortingDirection  & eIDirection = CDT::SortingComparator::ASC);

	/**
	  @brief Compare two values.
	  @param oX - first value to compare
	  @param oY - seond value to compare
	  @return true, if oX < oY, false - otherwise
	*/
	bool operator()(const CDT & oX, const CDT & oY) const;

	/**
	  @brief A destructor
	*/
	~SortCompareStr() throw();
private:
	/** Sorting direction   */
	const CDT::SortingComparator::eSortingDirection  eDirection;
};

/**
  @class SortCompareNumHashElement CDTSortRoutines.hpp <CDTSortRoutines.hpp>
  @brief Sorting comparator
*/
class SortCompareNumHashElement:
  public CDT::SortingComparator
{
public:
	/**
	  @brief Constructor
	  @param sIKey - key of key => value pair of HASH to compare
	  @param eIDirection - sorting direction
	*/
	SortCompareNumHashElement(const STLW::string                               & sIKey,
	                          const CDT::SortingComparator::eSortingDirection  & eIDirection = CDT::SortingComparator::ASC);

	/**
	  @brief Compare two values.
	  @param oX - first value to compare
	  @param oY - seond value to compare
	  @return true, if oX < oY, false - otherwise
	*/
	bool operator()(const CDT & oX, const CDT & oY) const;

	/**
	  @brief A destructor
	*/
	~SortCompareNumHashElement() throw();

private:
	/** Array element index */
	const STLW::string sKey;
	/** Sorting direction   */
	const CDT::SortingComparator::eSortingDirection  eDirection;
};

/**
  @class SortCompareStrHashElement CDTSortRoutines.hpp <CDTSortRoutines.hpp>
  @brief Sorting comparator
*/
class SortCompareStrHashElement:
  public CDT::SortingComparator
{
public:
	/**
	  @brief Constructor
	  @param sIKey - key of key => value pair of HASH to compare
	  @param eIDirection - sorting direction
	*/
	SortCompareStrHashElement(const STLW::string                               & sIKey,
	                          const CDT::SortingComparator::eSortingDirection  & eIDirection = CDT::SortingComparator::ASC);

	/**
	  @brief Compare two values.
	  @param oX - first value to compare
	  @param oY - seond value to compare
	  @return true, if oX < oY, false - otherwise
	*/
	bool operator()(const CDT & oX, const CDT & oY) const;

	/**
	  @brief A destructor
	*/
	~SortCompareStrHashElement() throw();

private:
	/** Array element index */
	const STLW::string sKey;
	/** Sorting direction   */
	const CDT::SortingComparator::eSortingDirection  eDirection;
};

/**
  @class SortCompareNumArrayElement CDTSortRoutines.hpp <CDTSortRoutines.hpp>
  @brief Sorting comparator
*/
class SortCompareNumArrayElement:
  public CDT::SortingComparator
{
public:
	/**
	  @brief Constructor
	  @param iIIndex - index of ARRAY to compare
	  @param eIDirection - sorting direction
	*/
	SortCompareNumArrayElement(const UINT_32                                      iIIndex,
	                           const CDT::SortingComparator::eSortingDirection  & eIDirection = CDT::SortingComparator::ASC);

	/**
	  @brief Compare two values.
	  @param oX - first value to compare
	  @param oY - seond value to compare
	  @return true, if oX < oY, false - otherwise
	*/
	bool operator()(const CDT & oX, const CDT & oY) const;

	/**
	  @brief A destructor
	*/
	~SortCompareNumArrayElement() throw();
private:
	/** Array element index */
	const UINT_32    iIndex;
	/** Sorting direction   */
	const CDT::SortingComparator::eSortingDirection  eDirection;
};

/**
  @class SortCompareNumArrayElement CDTSortRoutines.hpp <CDTSortRoutines.hpp>
  @brief Sorting comparator
*/
class SortCompareStrArrayElement:
  public CDT::SortingComparator
{
public:
	/**
	  @brief Constructor
	  @param iIIndex - index of ARRAY to compare
	  @param eIDirection - sorting direction
	*/
	SortCompareStrArrayElement(const UINT_32                                      iIIndex,
	                           const CDT::SortingComparator::eSortingDirection  & eIDirection = CDT::SortingComparator::ASC);

	/**
	  @brief Compare two values.
	  @param oX - first value to compare
	  @param oY - seond value to compare
	  @return true, if oX < oY, false - otherwise
	*/
	bool operator()(const CDT & oX, const CDT & oY) const;

	/**
	  @brief A destructor
	*/
	~SortCompareStrArrayElement() throw();
private:
	/** Array element index */
	const UINT_32    iIndex;
	/** Sorting direction   */
	const CDT::SortingComparator::eSortingDirection  eDirection;
};

} // namespace CTPP
#endif // _CDT_SORT_ROUTINES_HPP__
// End.
