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
 *      CDT.cpp
 *
 * $CTPP$
 */
#include "CDTSortRoutines.hpp"

#include <stdio.h>

namespace CTPP
{

// //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Class SortCompareNum
//

//
// Constructor
//
SortCompareNum::SortCompareNum(const CDT::SortingComparator::eSortingDirection  & eIDirection): eDirection(eIDirection) { ;; }

//
// Compare two values.
//
bool SortCompareNum::operator()(const CDT & oX, const CDT & oY) const
{
	INT_32 iRC = 0;

	if      (oX < oY) { iRC = -1; }
	else if (oX > oY) { iRC =  1; }

	if (eDirection == CDT::SortingComparator::DESC) { iRC = -iRC; }

return iRC < 0;
}

//
// A destructor
//
SortCompareNum::~SortCompareNum() throw() { ;; }

// //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Class SortCompareStr
//

//
// Constructor
//
SortCompareStr::SortCompareStr(const CDT::SortingComparator::eSortingDirection  & eIDirection): eDirection(eIDirection) { ;; }

//
// Compare two values.
//
bool SortCompareStr::operator()(const CDT & oX, const CDT & oY) const
{
	INT_32 iRC = 0;

	if      (oX.Less(oY))    { iRC = -1; }
	else if (oX.Greater(oY)) { iRC =  1; }

	if (eDirection == CDT::SortingComparator::DESC) { iRC = -iRC; }

return iRC < 0;
}

//
// A destructor
//
SortCompareStr::~SortCompareStr() throw() { ;; }

// //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Class SortCompareNumHashElement
//

//
// Constructor
//
SortCompareNumHashElement::SortCompareNumHashElement(const STLW::string                               & sIKey,
                                                     const CDT::SortingComparator::eSortingDirection  & eIDirection): sKey(sIKey),
                                                                                                                      eDirection(eIDirection) { ;; }

//
// Compare two values.
//
bool SortCompareNumHashElement::operator()(const CDT & oX, const CDT & oY) const
{
	INT_32 iRC = 0;

	const CDT & oXX = oX.GetCDT(sKey);
	const CDT & oYY = oY.GetCDT(sKey);

	if      (oXX < oYY) { iRC = -1; }
	else if (oXX > oYY) { iRC =  1; }

	if (eDirection == CDT::SortingComparator::DESC) { iRC = -iRC; }

return iRC < 0;
}

//
// A dertructor
//
SortCompareNumHashElement::~SortCompareNumHashElement() throw() { ;; }

// //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Class SortCompareStrHashElement
//

//
// Constructor
//
SortCompareStrHashElement::SortCompareStrHashElement(const STLW::string                               & sIKey,
                                                     const CDT::SortingComparator::eSortingDirection  & eIDirection): sKey(sIKey),
                                                                                                                      eDirection(eIDirection) { ;; }

//
// Compare two values.
//
bool SortCompareStrHashElement::operator()(const CDT & oX, const CDT & oY) const
{
	INT_32 iRC = 0;

	const CDT & oXX = oX.GetCDT(sKey);
	const CDT & oYY = oY.GetCDT(sKey);

	if      (oXX.Less(oYY))    { iRC = -1; }
	else if (oXX.Greater(oYY)) { iRC =  1; }

	if (eDirection == CDT::SortingComparator::DESC) { iRC = -iRC; }

return iRC < 0;
}

//
// A destructor
//
SortCompareStrHashElement::~SortCompareStrHashElement() throw() { ;; }

// //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Class SortCompareNumArrayElement
//

//
// Constructor
//
SortCompareNumArrayElement::SortCompareNumArrayElement(const UINT_32                                      iIIndex,
                                                       const CDT::SortingComparator::eSortingDirection  & eIDirection): iIndex(iIIndex),
                                                                                                                        eDirection(eIDirection) { ;; }

//
// Compare two values.
//
bool SortCompareNumArrayElement::operator()(const CDT & oX, const CDT & oY) const
{
	INT_32 iRC = 0;

	const CDT & oXX = oX.GetCDT(iIndex);
	const CDT & oYY = oY.GetCDT(iIndex);

	if      (oXX < oYY) { iRC = -1; }
	else if (oXX > oYY) { iRC =  1; }

	if (eDirection == CDT::SortingComparator::DESC) { iRC = -iRC; }

return iRC < 0;
}

//
// A destructor
//
SortCompareNumArrayElement::~SortCompareNumArrayElement() throw() { ;; }

// //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Class SortCompareStrArrayElement
//

//
// Constructor
//
SortCompareStrArrayElement::SortCompareStrArrayElement(const UINT_32                                      iIIndex,
                                                       const CDT::SortingComparator::eSortingDirection  & eIDirection): iIndex(iIIndex),
                                                                                                                        eDirection(eIDirection) { ;; }
//
// Compare two values.
//
bool SortCompareStrArrayElement::operator()(const CDT & oX, const CDT & oY) const
{
	INT_32 iRC = 0;

	const CDT & oXX = oX.GetCDT(iIndex);
	const CDT & oYY = oY.GetCDT(iIndex);

	if      (oXX.Less(oYY))    { iRC = -1; }
	else if (oXX.Greater(oYY)) { iRC =  1; }

	if (eDirection == CDT::SortingComparator::DESC) { iRC = -iRC; }

return iRC < 0;
}

//
// A destructor
//
SortCompareStrArrayElement::~SortCompareStrArrayElement() throw() { ;; }

} // namespace CTPP
// End.
