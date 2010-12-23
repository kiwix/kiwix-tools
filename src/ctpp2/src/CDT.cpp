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
#include "CDT.hpp"
#include "STLFunctional.hpp"

#include <stdio.h>

namespace CTPP
{

// FWD
STLW::string EscapeJSONString(const STLW::string  & sSource,
                              const bool          & bECMAConventions = true,
                              const bool          & bHTMLSafe = true);

// ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Struct _CDT
//
/**
  @struct _CDT CDT.hpp <CDT.hpp>
  @brief Shareable datatypes
*/
struct CDT::_CDT
{
	/** References counter               */
	UINT_32           refcount;
	/** Value type fpr complex datatypes */
	mutable eValType  value_type;

	/** Union */
	union
	{
		/** String */
		String           * s_data;
		/** Array  */
		Vector           * v_data;
		/** Hash   */
		Map              * m_data;
	} u;

	/** Complex datatypes */
	mutable union
	{
		/** Signed interger                */
		INT_64                 i_data;
		/** Floating point value           */
		W_FLOAT                d_data;
	} uc;

	/** Constructor */
	_CDT();
};

//
// Constructor
//
CDT::_CDT::_CDT(): refcount(1), value_type(UNDEF)
{
	u.s_data  = NULL;
	uc.i_data = 0;
}

// ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Class Iterator
//

//
// Constructor
//
CDT::Iterator::Iterator(CDT::Map::iterator itIMap): itMap(itIMap) { ;; }

//
// Copy constructor
//
CDT::Iterator::Iterator(const CDT::Iterator & oRhs): itMap(oRhs.itMap) { ;; }

//
// Operator =
//
CDT::Iterator & CDT::Iterator::operator=(const CDT::Iterator & oRhs)
{
	if (this != &oRhs) { itMap = oRhs.itMap; }

	return *this;
}

//
// Pre-increment operator ++
//
CDT::Iterator & CDT::Iterator::operator++()
{
	++itMap;

return *this;
}

//
// Post-increment operator ++
//
CDT::Iterator CDT::Iterator::operator++(int)
{
	Iterator oTMP = *this;

	++itMap;

return oTMP;
}

//
// Access operator
//
STLW::pair<const STLW::string, CDT> * CDT::Iterator::operator->() { return &(*itMap); }

//
// Comparison operator
//
bool CDT::Iterator::operator ==(const CDT::Iterator & oRhs) { return (itMap == oRhs.itMap); }

//
// Comparison operator
//
bool CDT::Iterator::operator !=(const CDT::Iterator & oRhs) { return (itMap != oRhs.itMap); }

//
// Get iterator pointed to start of hash
//
CDT::Iterator CDT::Begin()
{
	if (eValueType != HASH_VAL) { throw CDTAccessException(); }

return Iterator(u.p_data -> u.m_data -> begin());
}

//
// Get iterator pointed to end of hash
//
CDT::Iterator CDT::End()
{
	if (eValueType != HASH_VAL) { throw CDTAccessException(); }

	return Iterator(u.p_data -> u.m_data -> end());
}

//
// Find element in hash
//
CDT::Iterator CDT::Find(const STLW::string & sKey)
{
	if (eValueType != HASH_VAL) { throw CDTAccessException(); }

return Iterator(u.p_data -> u.m_data -> find(sKey));
}

// ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Class ConstIterator
//

//
// Copy constructor
//
CDT::ConstIterator::ConstIterator(const CDT::ConstIterator & oRhs): itMap(oRhs.itMap) { ;; }

//
// Type cast constructor
//
CDT::ConstIterator::ConstIterator(const CDT::Iterator & oRhs): itMap(oRhs.itMap) { ;; }

//
// Operator =
//
CDT::ConstIterator & CDT::ConstIterator::operator=(const ConstIterator & oRhs)
{
	if (this != &oRhs) { itMap = oRhs.itMap; }

return *this;
}

//
// Operator =
//
CDT::ConstIterator & CDT::ConstIterator::operator=(const CDT::Iterator & oRhs)
{
	itMap = oRhs.itMap;

return *this;
}

//
// Pre-increment operator ++
//
CDT::ConstIterator & CDT::ConstIterator::operator++()
{
	++itMap;

return *this;
}

//
// Post-increment operator ++
//
CDT::ConstIterator CDT::ConstIterator::operator++(int)
{
	ConstIterator oTMP = *this;

	++itMap;

return oTMP;
}

//
// Access operator
//
const STLW::pair<const STLW::string, CDT> * CDT::ConstIterator::operator->() const { return &(*itMap); }

//
// Comparison operator
//
bool CDT::ConstIterator::operator ==(const CDT::ConstIterator & oRhs) const { return (itMap == oRhs.itMap); }

//
// Comparison operator
//
bool CDT::ConstIterator::operator !=(const CDT::ConstIterator & oRhs) const { return (itMap != oRhs.itMap); }

//
// Get constant iterator pointed to start of hash
//
CDT::ConstIterator CDT::Begin() const
{
	if (eValueType != HASH_VAL) { throw CDTAccessException(); }

return ConstIterator(u.p_data -> u.m_data -> begin());
}

//
// Get constant iterator pointed to end of hash
//
CDT::ConstIterator CDT::End() const
{
	if (eValueType != HASH_VAL) { throw CDTAccessException(); }

return ConstIterator(u.p_data -> u.m_data -> end());
}

//
// Find element in hash
//
CDT::ConstIterator CDT::Find(const STLW::string & sKey) const
{
	if (eValueType != HASH_VAL) { throw CDTAccessException(); }

return ConstIterator(u.p_data -> u.m_data -> find(sKey));
}

// ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Class SortHelper
//

/**
  @class SortHelper
  @brief Sorting helper
*/
class SortHelper
{
public:
	/**
	  @brief Constructor
	  @param oISortingComparator - "Real" sorting comparator
	*/
	SortHelper(const CDT::SortingComparator  & oISortingComparator);

	/**
	  @brief Comparison operator
	  @param oX - first value to compare
	  @param oY - seond value to compare
	  @return true, if oX < oY, false - otherwise
	*/
	bool operator()(const CDT & oX, const CDT & oY) const;
private:
	/** "Real" comparator */
	const CDT::SortingComparator  & oSortingComparator;
};

//
// Constructor
//
SortHelper::SortHelper(const CDT::SortingComparator  & oISortingComparator): oSortingComparator(oISortingComparator) { ;; }

//
// Comparison operator
//
bool SortHelper::operator()(const CDT & oX, const CDT & oY) const { return oSortingComparator.operator()(oX, oY); }

// ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Class CDT
//

//
// Default constructor
//
CDT::CDT(const eValType & oValue): eValueType(oValue)
{
	switch (eValueType)
	{
		case UNDEF:
			break;

		case INT_VAL:
			u.i_data = 0;
			break;

		case REAL_VAL:
			u.d_data = 0;
			break;

		case STRING_VAL:
			u.p_data = new _CDT();
			u.p_data -> u.s_data = new String;
			break;

		case STRING_INT_VAL:
			u.p_data = new _CDT();
			u.p_data -> value_type = INT_VAL;
			u.p_data -> u.s_data = new String;
			break;

		case STRING_REAL_VAL:
			u.p_data = new _CDT();
			u.p_data -> value_type = REAL_VAL;
			u.p_data -> u.s_data = new String;
			break;

		case ARRAY_VAL:
			u.p_data = new _CDT();
			u.p_data -> u.v_data = new Vector;
			break;

		case HASH_VAL:
			u.p_data = new _CDT();
			u.p_data -> u.m_data = new Map;
			break;

		case POINTER_VAL:
			u.pp_data = NULL;
			break;

		default:
			throw CDTTypeCastException("No such type");
	}
}

//
// Copy constructor
//
CDT::CDT(const CDT & oCDT)
{
	if (this == &oCDT)
	{
		eValueType = UNDEF;
		return;
	}

	eValueType = oCDT.eValueType;
	switch (eValueType)
	{
		case UNDEF:
			break;

		case INT_VAL:
			u.i_data = oCDT.u.i_data;
			break;

		case REAL_VAL:
			u.d_data = oCDT.u.d_data;
			break;

		case POINTER_VAL:
			u.pp_data = oCDT.u.pp_data;
			break;

		case STRING_VAL:
		case STRING_REAL_VAL:
		case STRING_INT_VAL:
		case ARRAY_VAL:
		case HASH_VAL:
			u.p_data = oCDT.u.p_data;
			++(u.p_data -> refcount);
			break;

		default:
			throw CDTTypeCastException("No such type");
	}
}

//
// Copy operator
//
CDT & CDT::operator=(const CDT & oCDT)
{
	if (this == &oCDT) { return *this; }

	// Destroy object if need
	if (eValueType >= STRING_VAL) { Destroy(); }

	eValueType = oCDT.eValueType;

	switch (eValueType)
	{
		case UNDEF:
			break;

		case INT_VAL:
			u.i_data = oCDT.u.i_data;
			break;

		case REAL_VAL:
			u.d_data = oCDT.u.d_data;
			break;

		case POINTER_VAL:
			u.pp_data = oCDT.u.pp_data;
			break;

		case STRING_VAL:
		case STRING_REAL_VAL:
		case STRING_INT_VAL:
		case ARRAY_VAL:
		case HASH_VAL:
			u.p_data = oCDT.u.p_data;
			++u.p_data -> refcount;
			break;

		default:
			throw CDTTypeCastException("No such type");
	}

return *this;
}

// ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//
// Type cast constructor from INT_64 type
//
CDT::CDT(const INT_64  oValue): eValueType(INT_VAL)        { u.i_data = oValue; }

//
// Type cast constructor from UINT_64 type
//
CDT::CDT(const UINT_64  oValue): eValueType(INT_VAL)       { u.i_data = oValue; }

//
// Type cast constructor from INT_32 type
//
CDT::CDT(const INT_32  oValue): eValueType(INT_VAL)        { u.i_data = oValue; }

//
// Type cast constructor from UINT_32 type
//
CDT::CDT(const UINT_32  oValue): eValueType(INT_VAL)       { u.i_data = oValue; }

//
// Type cast constructor from W_FLOAT type
//
CDT::CDT(const W_FLOAT  oValue): eValueType(REAL_VAL)      { u.d_data = oValue; }

//
// Type cast constructor from STLW::string type
//
CDT::CDT(const STLW::string & oValue): eValueType(STRING_VAL)
{
	u.p_data = new _CDT();
	u.p_data -> u.s_data = new String(oValue);
}

//
// Type cast constructor from CCHAR_P type
//
CDT::CDT(CCHAR_P oValue): eValueType(STRING_VAL)
{
	u.p_data = new _CDT();
	u.p_data -> u.s_data = new STLW::string(oValue);
}

//
// Type cast constructor
//
CDT::CDT(void * oValue): eValueType(POINTER_VAL) { u.pp_data = oValue; }

// ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//
// Operator = for type INT_64
//
CDT & CDT::operator=(const INT_64  oValue)
{
	// Destroy object if need
	if (eValueType >= STRING_VAL) { Destroy(); }

	eValueType = INT_VAL;
	u.i_data   = oValue;

return *this;
}

//
// Operator = for type UINT_64
//
CDT & CDT::operator=(const UINT_64  oValue) { return operator=(INT_64(oValue)); }

//
// Operator = for type INT_32
//
CDT & CDT::operator=(const INT_32  oValue)  { return operator=(INT_64(oValue)); }

//
// Operator = for type UINT_32
//
CDT & CDT::operator=(const UINT_32  oValue) { return operator=(UINT_64(oValue)); }

//
// Operator = for type W_FLOAT
//
CDT & CDT::operator=(const W_FLOAT  oValue)
{
	// Destroy object if need
	if (eValueType >= STRING_VAL) { Destroy(); }

	eValueType = REAL_VAL;
	u.d_data   = oValue;

return *this;
}

//
// Operator = for type STLW::string
//
CDT & CDT::operator=(const STLW::string & oValue)
{
	// Destroy object if need
	if (eValueType >= STRING_VAL) { Destroy(); }

	eValueType = STRING_VAL;
	u.p_data = new _CDT();
	u.p_data -> u.s_data = new String(oValue);

return *this;
}

//
// Operator = for old-fashion string
//
CDT & CDT::operator=(CCHAR_P oValue)
{
	// Destroy object if need
	if (eValueType >= STRING_VAL) { Destroy(); }

	eValueType = STRING_VAL;
	u.p_data = new _CDT();
	u.p_data -> u.s_data = new String(oValue);

return *this;
}

//
// Type cast constructor
//
CDT & CDT::operator=(void * oValue)
{
	// Destroy object if need
	if (eValueType >= STRING_VAL) { Destroy(); }

	eValueType = POINTER_VAL;
	u.pp_data  = oValue;

return *this;
}

// ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//
// Access operator []
//
CDT & CDT::operator[](const UINT_32  iPos)
{
	if (eValueType == UNDEF)
	{
		eValueType = ARRAY_VAL;
		u.p_data = new _CDT;
		u.p_data -> u.v_data = new Vector(iPos + 1);
	}
	else if (eValueType != ARRAY_VAL) { throw CDTAccessException(); }

	// Unshare complex type
	Unshare();

	UINT_32 iSize = u.p_data -> u.v_data -> size();

	if      (iPos == iSize) { u.p_data -> u.v_data -> push_back(CDT()); }
	else if (iPos > iSize)  { u.p_data -> u.v_data -> resize(iPos + 1); }

return u.p_data -> u.v_data -> operator[](iPos);
}

//
// Access operator []
//
CDT & CDT::operator[](const STLW::string & sKey)
{
	if (eValueType == UNDEF)
	{
		eValueType = HASH_VAL;
		u.p_data = new _CDT;
		u.p_data -> u.m_data = new Map;
	}
	else if (eValueType != HASH_VAL) { throw CDTAccessException(); }

	// Unshare complex type
	Unshare();

return u.p_data -> u.m_data -> operator[](sKey);
}

//
// Provides constant access to the data contained in CDT
//
CDT CDT::GetCDT(const UINT_32  iPos) const
{
	if (eValueType != ARRAY_VAL) { return CDT(); }

	if (iPos >= u.p_data -> u.v_data -> size()) { throw CDTRangeException(); }

return CDT(u.p_data -> u.v_data -> operator[](iPos));
}

//
// Provides constant access to the data contained in CDT
//
CDT CDT::GetCDT(const STLW::string & sKey) const
{
	bool bFlag = 0;

return GetExistedCDT(sKey, bFlag);
}

//
// Provides constant access to the data contained in CDT
//
CDT CDT::GetExistedCDT(const STLW::string & sKey, bool & bCDTExist) const
{
	// CDT Does Not exist
	if (eValueType != HASH_VAL)
	{
		bCDTExist = false;
		return CDT();
	}

	Map::const_iterator itmHash = u.p_data -> u.m_data -> find(sKey);
	if (itmHash == u.p_data -> u.m_data -> end())
	{
		bCDTExist = false;
		return CDT();
	}
	bCDTExist = true;

return CDT(itmHash -> second);
}

//
// Erase element from HASH
//
bool CDT::Erase(const STLW::string & sKey)
{
	if (eValueType != HASH_VAL) { throw CDTAccessException(); }

	Map::iterator itmHash = u.p_data -> u.m_data -> find(sKey);
	if (itmHash == u.p_data -> u.m_data -> end()) { return false; }

	Unshare();

	u.p_data -> u.m_data -> erase(itmHash);

return true;
}

//
// Check element in array
//
bool CDT::Exists(const UINT_32  iPos) const
{
	if (eValueType != ARRAY_VAL) { throw CDTAccessException(); }

	if (iPos >= u.p_data -> u.v_data -> size()) { return false; }

return true;
}

//
// Check element in hash
//
bool CDT::Exists(const STLW::string & sKey) const
{
	if (eValueType != HASH_VAL) { throw CDTAccessException(); }

	Map::const_iterator itmHash = u.p_data -> u.m_data -> find(sKey);
	if (itmHash == u.p_data -> u.m_data -> end()) { return false; }

return true;
}

//
// Push value into array
//
void CDT::PushBack(const INT_64  oValue)        { PushBack(CDT(oValue)); }

//
// Push value into array
//
void CDT::PushBack(const UINT_64  oValue)       { PushBack(CDT(oValue)); }

//
// Push value into array
//
void CDT::PushBack(const INT_32  oValue)        { PushBack(CDT(oValue)); }

//
// Push value into array
//
void CDT::PushBack(const UINT_32  oValue)       { PushBack(CDT(oValue)); }

//
// Push value into array
//
void CDT::PushBack(const W_FLOAT  oValue)       { PushBack(CDT(oValue)); }

//
// Push value into array
//
void CDT::PushBack(const STLW::string & oValue) { PushBack(CDT(oValue)); }

//
// Push value into array
//
void CDT::PushBack(CCHAR_P oValue)              { PushBack(CDT(oValue)); }

//
// Push value into array
//
void CDT::PushBack(const CDT & oValue)
{
	if      (eValueType == ARRAY_VAL) { u.p_data -> u.v_data -> push_back(oValue); }
	else if (eValueType == UNDEF)
	{
		::new (this) CDT(CDT::ARRAY_VAL);
		u.p_data -> u.v_data -> push_back(oValue);
	}
	else
	{
		throw CDTAccessException();
	}
}

//
// Returns a boolean value telling whether object has a value
//
bool CDT::Defined() const
{
	switch (eValueType)
	{
		case UNDEF:
			break;

		case INT_VAL:
			if (u.i_data != 0) { return true; }
			break;

		case REAL_VAL:
			if (u.d_data != 0) { return true; }
			break;

		case STRING_VAL:
		case STRING_REAL_VAL:
		case STRING_INT_VAL:
			if (u.p_data -> u.s_data -> size() != 0) { return true; }
			break;

		case ARRAY_VAL:
			if (u.p_data -> u.v_data -> size() != 0) { return true; }
			break;

		case HASH_VAL:
			if (u.p_data -> u.m_data -> size() != 0) { return true; }
			break;

		case POINTER_VAL:
			if (u.pp_data != NULL)                   { return true; }
			break;
	}

return false;
}

//
// Provides range-check access to the data contained in CDT
//
CDT & CDT::At(const UINT_32  iPos)
{
	if (eValueType != ARRAY_VAL) { throw CDTAccessException(); }

	if (iPos >= u.p_data -> u.v_data -> size()) { throw CDTRangeException(); }

return u.p_data -> u.v_data -> operator[](iPos);
}

//
// Provides range-check access to the data contained in CDT
//
CDT & CDT::At(const STLW::string & sKey)
{
	if (eValueType != HASH_VAL) { throw CDTAccessException(); }

	Map::iterator itmHash = u.p_data -> u.m_data -> find(sKey);
	if (itmHash == u.p_data -> u.m_data -> end()) { throw CDTRangeException(); }

return itmHash -> second;
}

// ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//
// Operator + for type INT_64
//
CDT CDT::operator+(const INT_64  oValue) const
{
	switch (eValueType)
	{
		case UNDEF:
			return CDT(oValue);

		case INT_VAL:
			return CDT(u.i_data + oValue);

		case REAL_VAL:
			return CDT(u.d_data + oValue);

		case STRING_VAL:
			{
				CheckComplexDataType();
				if (eValueType != STRING_VAL) { return this -> operator+(oValue); }

				INT_64   iData;
				W_FLOAT  dData;
				eValType eType = CastToNumber(iData, dData);

				if (eType == INT_VAL) { return CDT(iData + oValue); }

				return CDT(dData + oValue);
			}

		case STRING_INT_VAL:
			return CDT(u.p_data -> uc.i_data + oValue);


		case STRING_REAL_VAL:
			return CDT(u.p_data -> uc.d_data + oValue);

		default:
			throw CDTTypeCastException("operator+ (INT_64)");
	}

// Make stupid compiler happy
return CDT(*this);
}

//
// Operator + for type UINT_64
//
CDT CDT::operator+(const UINT_64  oValue) const { return this -> operator+(INT_64(oValue)); }

//
// Operator + for type INT_32
//
CDT CDT::operator+(const INT_32  oValue) const { return this -> operator+(INT_64(oValue));  }

//
// Operator + for type UINT_32
//
CDT CDT::operator+(const UINT_32  oValue) const { return this -> operator+(UINT_64(oValue));  }

//
// Operator + for type W_FLOAT
//
CDT CDT::operator+(const W_FLOAT  oValue) const
{
	switch (eValueType)
	{
		case UNDEF:
			return CDT(oValue);

		case INT_VAL:
			return CDT(u.i_data + oValue);

		case REAL_VAL:
			return CDT(u.d_data + oValue);

		case STRING_VAL:
			{
				CheckComplexDataType();
				if (eValueType != STRING_VAL) { return this -> operator+(oValue); }

				INT_64   iData;
				W_FLOAT  dData;
				eValType eType = CastToNumber(iData, dData);

				if (eType == INT_VAL) { return CDT(iData + oValue); }

				return CDT(dData + oValue);
			}

		case STRING_INT_VAL:
			return CDT(u.p_data -> uc.i_data + oValue);

		case STRING_REAL_VAL:
			return CDT(u.p_data -> uc.d_data + oValue);

		default:
			throw CDTTypeCastException("operator+(INT_64)");
	}

// Make stupid compiler happy
return CDT(*this);
}

//
// Operator + for type CDT
//
CDT CDT::operator+(const CDT & oCDT) const
{
	INT_64   iData1;
	W_FLOAT  dData1;
	eValType eType1 = CastToNumber(iData1, dData1);

	INT_64   iData2;
	W_FLOAT  dData2;
	eValType eType2 = oCDT.CastToNumber(iData2, dData2);

	// First argument is INT_VAL
	if (eType1 == INT_VAL)
	{
		// First argument is INT_VAL, second argument is INT_VAL
		if (eType2 == INT_VAL) { return CDT(iData1 + iData2); }

		// First argument is INT_VAL, second argument is REAL_VAL
		return CDT(iData1 + dData2);
	}

	// First argument is REAL_VAL, second argument is INT_VAL
	if (eType2 == INT_VAL) { return CDT(dData1 + iData2); }

	// First argument is REAL_VAL, second argument is REAL_VAL
	return CDT(dData1 + dData2);
}

// ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//
// Operator - for type INT_64
//
CDT CDT::operator-(const INT_64  oValue) const { return this -> operator+(-oValue);  }

//
// Operator - for type UINT_64
//
CDT CDT::operator-(const UINT_64  oValue) const { return this -> operator+(- INT_64(oValue));  }

//
// Operator - for type INT_32
//
CDT CDT::operator-(const INT_32  oValue) const { return this -> operator+(-oValue);  }

//
// Operator - for type UINT_32
//
CDT CDT::operator-(const UINT_32  oValue) const { return this -> operator+(- INT_32(oValue));  }

//
// Operator - for type W_FLOAT
//
CDT CDT::operator-(const W_FLOAT  oValue) const { return this -> operator+(-oValue);  }

//
// Operator - for type CDT
//
CDT CDT::operator-(const CDT & oCDT) const
{
	INT_64   iData1;
	W_FLOAT  dData1;
	eValType eType1 = CastToNumber(iData1, dData1);

	INT_64   iData2;
	W_FLOAT  dData2;
	eValType eType2 = oCDT.CastToNumber(iData2, dData2);

	// First argument is INT_VAL
	if (eType1 == INT_VAL)
	{
		// First argument is INT_VAL, second argument is INT_VAL
		if (eType2 == INT_VAL) { return CDT(iData1 - iData2); }

		// First argument is INT_VAL, second argument is REAL_VAL
		return CDT(iData1 - dData2);
	}

	// First argument is REAL_VAL, second argument is INT_VAL
	if (eType2 == INT_VAL) { return CDT(dData1 - iData2); }

	// First argument is REAL_VAL, second argument is REAL_VAL
	return CDT(dData1 - dData2);
}

// ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//
// Operator * for type INT_64
//
CDT CDT::operator*(const INT_64  oValue) const
{
	switch (eValueType)
	{
		case UNDEF:
			return CDT(INT_64(0));

		case INT_VAL:
			return CDT(u.i_data * oValue);

		case REAL_VAL:
			return CDT(u.d_data * oValue);

		case STRING_VAL:
			{
				CheckComplexDataType();
				if (eValueType != STRING_VAL) { return this -> operator*(oValue); }

				INT_64   iData;
				W_FLOAT  dData;
				eValType eType = CastToNumber(iData, dData);

				if (eType == INT_VAL) { return CDT(iData * oValue); }

				return CDT(dData * oValue);
			}

		case STRING_INT_VAL:
			return CDT(u.p_data -> uc.i_data * oValue);

		case STRING_REAL_VAL:
			return CDT(u.p_data -> uc.d_data * oValue);

		default:
			throw CDTTypeCastException("operator*(INT_64)");
	}

// Make stupid compiler happy
return CDT(*this);
}

//
// Operator * for type UINT_64
//
CDT CDT::operator*(const UINT_64  oValue) const { return this -> operator*(INT_64(oValue));  }

//
// Operator * for type INT_32
//
CDT CDT::operator*(const INT_32  oValue) const  { return this -> operator*(INT_64(oValue));  }

//
// Operator * for type UINT_32
//
CDT CDT::operator*(const UINT_32  oValue) const { return this -> operator*(UINT_64(oValue));  }

//
// Operator * for type W_FLOAT
//
CDT CDT::operator*(const W_FLOAT  oValue) const
{
	switch (eValueType)
	{
		case UNDEF:
			return CDT(INT_64(0));

		case INT_VAL:
			return CDT(u.i_data * oValue);

		case REAL_VAL:
			return CDT(u.d_data * oValue);

		case STRING_VAL:
			{
				CheckComplexDataType();
				if (eValueType != STRING_VAL) { return this -> operator*(oValue); }

				INT_64   iData;
				W_FLOAT  dData;
				eValType eType = CastToNumber(iData, dData);

				if (eType == INT_VAL) { return CDT(iData * oValue); }

				return CDT(dData * oValue);
			}

		case STRING_INT_VAL:
			return CDT(u.p_data -> uc.i_data * oValue);

		case STRING_REAL_VAL:
			return CDT(u.p_data -> uc.d_data * oValue);

		default:
			throw CDTTypeCastException("operator*(W_FLOAT)");
	}

// Make stupid compiler happy
return CDT(*this);
}

//
// Operator * for type CDT
//
CDT CDT::operator*(const CDT & oCDT) const
{
	INT_64   iData1;
	W_FLOAT  dData1;
	eValType eType1 = CastToNumber(iData1, dData1);

	INT_64   iData2;
	W_FLOAT  dData2;
	eValType eType2 = oCDT.CastToNumber(iData2, dData2);

	// First argument is INT_VAL
	if (eType1 == INT_VAL)
	{
		// First argument is INT_VAL, second argument is INT_VAL
		if (eType2 == INT_VAL) { return CDT(iData1 * iData2); }

		// First argument is INT_VAL, second argument is REAL_VAL
		return CDT(iData1 * dData2);
	}

	// First argument is REAL_VAL, second argument is INT_VAL
	if (eType2 == INT_VAL) { return CDT(dData1 * iData2); }

	// First argument is REAL_VAL, second argument is REAL_VAL
	return CDT(dData1 * dData2);
}

// ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//
// Operator / for type INT_64
//
CDT CDT::operator/(const INT_64  oValue) const
{
	switch (eValueType)
	{
		case UNDEF:
			return CDT(0 / oValue);

		case INT_VAL:
			return CDT(u.i_data / oValue);

		case REAL_VAL:
			return CDT(u.d_data / oValue);

		case STRING_VAL:
			{
				CheckComplexDataType();
				if (eValueType != STRING_VAL) { return this -> operator/(oValue); }

				INT_64   iData;
				W_FLOAT  dData;
				eValType eType = CastToNumber(iData, dData);

				if (eType == INT_VAL) { return CDT(iData / oValue); }

				return CDT(dData / oValue);
			}

		case STRING_INT_VAL:
			return CDT(u.p_data -> uc.i_data / oValue);


		case STRING_REAL_VAL:
			return CDT(u.p_data -> uc.d_data / oValue);

		default:
			throw CDTTypeCastException("operator/(INT_64)");
	}

// Make stupid compiler happy
return CDT(*this);
}

//
// Operator / for type UINT_64
//
CDT CDT::operator/(const UINT_64  oValue) const { return this -> operator/(INT_64(oValue));  }

//
// Operator / for type INT_32
//
CDT CDT::operator/(const INT_32  oValue) const { return this -> operator/(INT_64(oValue));  }

//
// Operator / for type UINT_32
//
CDT CDT::operator/(const UINT_32  oValue) const { return this -> operator/(UINT_64(oValue));  }

//
// Operator / for type W_FLOAT
//
CDT CDT::operator/(const W_FLOAT  oValue) const
{
	switch (eValueType)
	{
		case UNDEF:
			// Keep this
			// For explanation see IEEE standard 754 for floating-point arithmetic
			return CDT(0.0 / oValue);

		case INT_VAL:
			return CDT(u.i_data / oValue);

		case REAL_VAL:
			return CDT(u.d_data / oValue);

		case STRING_VAL:
			{
				CheckComplexDataType();
				if (eValueType != STRING_VAL) { return this -> operator/(oValue); }

				INT_64   iData;
				W_FLOAT  dData;
				eValType eType = CastToNumber(iData, dData);

				if (eType == INT_VAL) { return CDT(iData / oValue); }

				return CDT(dData / oValue);
			}

		case STRING_INT_VAL:
			return CDT(u.p_data -> uc.i_data / oValue);


		case STRING_REAL_VAL:
			return CDT(u.p_data -> uc.d_data / oValue);

		default:
			throw CDTTypeCastException("operator/(W_FLOAT)");
	}

// Make stupid compiler happy
return CDT(*this);
}

//
// Operator / for type CDT
//
CDT CDT::operator/(const CDT & oCDT) const
{
	INT_64   iData1;
	W_FLOAT  dData1;
	eValType eType1 = CastToNumber(iData1, dData1);

	INT_64   iData2;
	W_FLOAT  dData2;
	eValType eType2 = oCDT.CastToNumber(iData2, dData2);

	// First argument is INT_VAL
	if (eType1 == INT_VAL)
	{
		// First argument is INT_VAL, second argument is INT_VAL
		if (eType2 == INT_VAL) { return CDT(iData1 / iData2); }

		// First argument is INT_VAL, second argument is REAL_VAL
		return CDT(iData1 / dData2);
	}

	// First argument is REAL_VAL, second argument is INT_VAL
	if (eType2 == INT_VAL) { return CDT(dData1 / iData2); }

	// First argument is REAL_VAL, second argument is REAL_VAL
	return CDT(dData1 / dData2);
}

// ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//
// Operator += for type INT_64
//
CDT & CDT::operator+=(const INT_64  oValue)
{
	switch (eValueType)
	{
		case UNDEF:
			::new (this)CDT(oValue);
			break;

		case INT_VAL:
			u.i_data += oValue;
			break;

		case REAL_VAL:
			u.d_data += oValue;
			break;

		case STRING_VAL:
			{
				CheckComplexDataType();
				if (eValueType != STRING_VAL) { return this -> operator+=(oValue); }

				INT_64   iData;
				W_FLOAT  dData;
				eValType eType = CastToNumber(iData, dData);

				if (eType == INT_VAL) { ::new (this)CDT(iData + oValue); }
				else                  { ::new (this)CDT(dData + oValue); }
			}
			break;

		case STRING_INT_VAL:
			::new (this)CDT(u.p_data -> uc.i_data + oValue);
			break;

		case STRING_REAL_VAL:
			::new (this)CDT(u.p_data -> uc.d_data + oValue);
			break;

		default:
			throw CDTTypeCastException("operator+=(INT_64)");
	}

return *this;
}

//
// Operator += for type UINT_64
//
CDT & CDT::operator+=(const UINT_64  oValue) { return this -> operator+=(INT_64(oValue));  }

//
// Operator += for type INT_32
//
CDT & CDT::operator+=(const INT_32  oValue)  { return this -> operator+=(INT_64(oValue));  }

//
// Operator += for type UINT_32
//
CDT & CDT::operator+=(const UINT_32  oValue) { return this -> operator+=(UINT_64(oValue)); }

//
// Operator += for type W_FLOAT
//
CDT & CDT::operator+=(const W_FLOAT  oValue)
{
	switch (eValueType)
	{
		case UNDEF:
			::new (this)CDT(oValue);
			break;

		case INT_VAL:
			u.i_data += INT_64(oValue);
			break;

		case REAL_VAL:
			u.d_data += oValue;
			break;

		case STRING_VAL:
			{
				CheckComplexDataType();
				if (eValueType != STRING_VAL) { return this -> operator+=(oValue); }

				INT_64   iData;
				W_FLOAT  dData;
				eValType eType = CastToNumber(iData, dData);

				if (eType == INT_VAL) { ::new (this)CDT(iData + oValue); }
				else                  { ::new (this)CDT(dData + oValue); }
			}
			break;

		case STRING_INT_VAL:
			::new (this)CDT(u.p_data -> uc.i_data + oValue);
			break;

		case STRING_REAL_VAL:
			::new (this)CDT(u.p_data -> uc.d_data + oValue);
			break;

		default:
			throw CDTTypeCastException("operator+=(W_FLOAT)");
	}

return *this;
}

//
// Operator += for type CDT
//
CDT & CDT::operator+=(const CDT & oCDT)
{
	INT_64   iData1;
	W_FLOAT  dData1;
	eValType eType1 = CastToNumber(iData1, dData1);

	INT_64   iData2;
	W_FLOAT  dData2;
	eValType eType2 = oCDT.CastToNumber(iData2, dData2);

	// First argument is INT_VAL
	if (eType1 == INT_VAL)
	{
		// First argument is INT_VAL, second argument is INT_VAL
		if (eType2 == INT_VAL) { ::new (this)CDT(iData1 + iData2); }
		// First argument is INT_VAL, second argument is REAL_VAL
		else                   { ::new (this)CDT(iData1 + dData2); }
	}
	else
	{
		// First argument is REAL_VAL, second argument is INT_VAL
		if (eType2 == INT_VAL) { ::new (this)CDT(dData1 + iData2); }
		// First argument is REAL_VAL, second argument is REAL_VAL
		else                   { ::new (this)CDT(dData1 + dData2); }
	}

return *this;
}

// ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//
// Operator -= for type INT_32
//
CDT & CDT::operator-=(const INT_64  oValue)  { return this -> operator+=(- oValue);  }

//
// Operator -= for type UINT_32
//
CDT & CDT::operator-=(const UINT_64  oValue) { return this -> operator+=(- INT_64(oValue));  }

//
// Operator -= for type INT_32
//
CDT & CDT::operator-=(const INT_32  oValue)  { return this -> operator+=(- oValue);  }

//
// Operator -= for type UINT_32
//
CDT & CDT::operator-=(const UINT_32  oValue) { return this -> operator+=(- INT_32(oValue));  }

//
// Operator -= for type W_FLOAT
//
CDT & CDT::operator-=(const W_FLOAT  oValue) { return this -> operator+=(- oValue); }

//
// Operator -= for type CDT
//
CDT & CDT::operator-=(const CDT &  oCDT)
{
	INT_64   iData1;
	W_FLOAT  dData1;
	eValType eType1 = CastToNumber(iData1, dData1);

	INT_64   iData2;
	W_FLOAT  dData2;
	eValType eType2 = oCDT.CastToNumber(iData2, dData2);

	// First argument is INT_VAL
	if (eType1 == INT_VAL)
	{
		// First argument is INT_VAL, second argument is INT_VAL
		if (eType2 == INT_VAL) { ::new (this)CDT(iData1 - iData2); }
		// First argument is INT_VAL, second argument is REAL_VAL
		else                   { ::new (this)CDT(iData1 - dData2); }
	}
	else
	{
		// First argument is REAL_VAL, second argument is INT_VAL
		if (eType2 == INT_VAL) { ::new (this)CDT(dData1 - iData2); }
		// First argument is REAL_VAL, second argument is REAL_VAL
		else                   { ::new (this)CDT(dData1 - dData2); }
	}

return *this;
}

// ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//
// Operator *= for type INT_64
//
CDT & CDT::operator*=(const INT_64  oValue)
{
	switch (eValueType)
	{
		case UNDEF:
			::new (this)CDT(0);
			break;

		case INT_VAL:
			u.i_data *= oValue;
			break;

		case REAL_VAL:
			u.d_data *= oValue;
			break;

		case STRING_VAL:
			{
				CheckComplexDataType();
				if (eValueType != STRING_VAL) { return this -> operator*=(oValue); }

				INT_64   iData;
				W_FLOAT  dData;
				eValType eType = CastToNumber(iData, dData);

				if (eType == INT_VAL) { ::new (this)CDT(iData * oValue); }
				else                  { ::new (this)CDT(dData * oValue); }
			}
			break;

		case STRING_INT_VAL:
			::new (this)CDT(u.p_data -> uc.i_data * oValue);
			break;

		case STRING_REAL_VAL:
			::new (this)CDT(u.p_data -> uc.d_data * oValue);
			break;

		default:
			throw CDTTypeCastException("operator*=(INT_64)");
	}

return *this;
}

//
// Operator *= for type UINT_64
//
CDT & CDT::operator*=(const UINT_64  oValue) { return this -> operator*=(INT_64(oValue));  }

//
// Operator *= for type INT_32
//
CDT & CDT::operator*=(const INT_32  oValue)  { return this -> operator*=(INT_64(oValue));  }

//
// Operator *= for type UINT_32
//
CDT & CDT::operator*=(const UINT_32  oValue) { return this -> operator*=(INT_64(oValue));  }

//
// Operator *= for type W_FLOAT
//
CDT & CDT::operator*=(const W_FLOAT  oValue)
{
	switch (eValueType)
	{
		case UNDEF:
			::new (this)CDT(0.0);
			break;

		case INT_VAL:
			{
				eValueType = REAL_VAL;

				const W_FLOAT dTMP = u.i_data * oValue;
				u.d_data = dTMP;
			}
			break;

		case REAL_VAL:
			u.d_data *= oValue;
			break;

		case STRING_VAL:
			{
				CheckComplexDataType();
				if (eValueType != STRING_VAL) { return this -> operator*=(oValue); }

				INT_64   iData;
				W_FLOAT  dData;
				eValType eType = CastToNumber(iData, dData);

				if (eType == INT_VAL) { ::new (this)CDT(iData * oValue); }
				else                  { ::new (this)CDT(dData * oValue); }
			}
			break;

		case STRING_INT_VAL:
			::new (this)CDT(u.p_data -> uc.i_data * oValue);
			break;

		case STRING_REAL_VAL:
			::new (this)CDT(u.p_data -> uc.d_data * oValue);
			break;

		default:
			throw CDTTypeCastException("operator*=(W_FLOAT)");
	}

return *this;
}

//
// Operator *= for type CDT
//
CDT & CDT::operator*=(const CDT & oCDT)
{
	INT_64   iData1;
	W_FLOAT  dData1;
	eValType eType1 = CastToNumber(iData1, dData1);

	INT_64   iData2;
	W_FLOAT  dData2;
	eValType eType2 = oCDT.CastToNumber(iData2, dData2);

	// First argument is INT_VAL
	if (eType1 == INT_VAL)
	{
		// First argument is INT_VAL, second argument is INT_VAL
		if (eType2 == INT_VAL) { ::new (this)CDT(iData1 * iData2); }
		// First argument is INT_VAL, second argument is REAL_VAL
		else                   { ::new (this)CDT(iData1 * dData2); }
	}
	else
	{
		// First argument is REAL_VAL, second argument is INT_VAL
		if (eType2 == INT_VAL) { ::new (this)CDT(dData1 * iData2); }
		// First argument is REAL_VAL, second argument is REAL_VAL
		else                   { ::new (this)CDT(dData1 * dData2); }
	}

return *this;
}

// ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//
// Operator /= for type INT_64
//
CDT & CDT::operator/=(const INT_64  oValue)
{
	switch (eValueType)
	{
		case UNDEF:
			::new (this)CDT(0 / oValue);
			break;

		case INT_VAL:
			u.i_data /= oValue;
			break;

		case REAL_VAL:
			u.d_data /= oValue;
			break;

		case STRING_VAL:
			{
				CheckComplexDataType();
				if (eValueType != STRING_VAL) { return this -> operator/=(oValue); }

				INT_64   iData;
				W_FLOAT  dData;
				eValType eType = CastToNumber(iData, dData);

				if (eType == INT_VAL) { ::new (this)CDT(iData / oValue); }
				else                  { ::new (this)CDT(dData / oValue); }
			}
			break;

		case STRING_INT_VAL:
			::new (this)CDT(u.p_data -> uc.i_data / oValue);
			break;

		case STRING_REAL_VAL:
			::new (this)CDT(u.p_data -> uc.d_data / oValue);
			break;

		default:
			throw CDTTypeCastException("operator/=(INT_64)");
	}

return *this;
}

//
// Operator /= for type UINT_64
//
CDT & CDT::operator/=(const UINT_64  oValue)  { return this -> operator/=(INT_64(oValue));  }

//
// Operator /= for type INT_32
//
CDT & CDT::operator/=(const INT_32  oValue)  { return this -> operator/=(INT_64(oValue));  }

//
// Operator /= for type UINT_32
//
CDT & CDT::operator/=(const UINT_32  oValue) { return this -> operator/=(UINT_64(oValue)); }

//
// Operator /= for type W_FLOAT
//
CDT & CDT::operator/=(const W_FLOAT  oValue)
{
	switch (eValueType)
	{
		case UNDEF:
			::new (this)CDT(0.0 / oValue);
			break;

		case INT_VAL:
			{
				eValueType = REAL_VAL;

				const W_FLOAT dTMP = u.i_data / oValue;
				u.d_data = dTMP;
			}
			break;

		case REAL_VAL:
			u.d_data /= oValue;
			break;

		case STRING_VAL:
			{
				CheckComplexDataType();
				if (eValueType != STRING_VAL) { return this -> operator/=(oValue); }

				INT_64   iData;
				W_FLOAT  dData;
				eValType eType = CastToNumber(iData, dData);

				if (eType == INT_VAL) { ::new (this)CDT(iData / oValue); }
				else                  { ::new (this)CDT(dData / oValue); }
			}
			break;

		case STRING_INT_VAL:
			::new (this)CDT(u.p_data -> uc.i_data / oValue);
			break;

		case STRING_REAL_VAL:
			::new (this)CDT(u.p_data -> uc.d_data / oValue);
			break;

		default:
			throw CDTTypeCastException("operator*=(W_FLOAT)");
	}

return *this;
}

//
// Operator /= for type CDT
//
CDT & CDT::operator/=(const CDT & oCDT)
{
	INT_64   iData1;
	W_FLOAT  dData1;
	eValType eType1 = CastToNumber(iData1, dData1);

	INT_64   iData2;
	W_FLOAT  dData2;
	eValType eType2 = oCDT.CastToNumber(iData2, dData2);

	// First argument is INT_VAL
	if (eType1 == INT_VAL)
	{
		// First argument is INT_VAL, second argument is INT_VAL
		if (eType2 == INT_VAL) { ::new (this)CDT(iData1 / iData2); }
		// First argument is INT_VAL, second argument is REAL_VAL
		else                   { ::new (this)CDT(iData1 / dData2); }
	}
	else
	{
		// First argument is REAL_VAL, second argument is INT_VAL
		if (eType2 == INT_VAL) { ::new (this)CDT(dData1 / iData2); }
		// First argument is REAL_VAL, second argument is REAL_VAL
		else                   { ::new (this)CDT(dData1 / dData2); }
	}

return *this;
}

// ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//
// Operator == for type INT_32
//
bool CDT::operator==(const INT_64  oValue) const
{
	if      (eValueType == INT_VAL)         { return u.i_data == oValue;              }
	else if (eValueType == REAL_VAL)        { return u.d_data == oValue;              }
	else if (eValueType == STRING_INT_VAL)  { return u.p_data -> uc.i_data == oValue; }
	else if (eValueType == STRING_REAL_VAL) { return u.p_data -> uc.d_data == oValue; }

#if THROW_EXCEPTION_IN_COMPARATORS
	else
	{
		STLW::string sError("Can't compare ");
		sError.append(PrintableType());
		sError.append(" and INTEGER data types. Use `Equal`, `EQ` method instead.");

		throw CTPPLogicError(sError.c_str());
	}
#endif
return false;
}

//
// Operator == for type UINT_32
//
bool CDT::operator==(const UINT_64  oValue) const { return operator==(INT_64(oValue));  }

//
// Operator == for type INT_32
//
bool CDT::operator==(const INT_32  oValue)  const { return operator==(INT_64(oValue));  }

//
// Operator == for type UINT_32
//
bool CDT::operator==(const UINT_32  oValue) const { return operator==(UINT_64(oValue));  }

//
// Operator == for type W_FLOAT
//
bool CDT::operator==(const W_FLOAT  oValue) const
{
	if      (eValueType == INT_VAL)         { return u.i_data == oValue;              }
	else if (eValueType == REAL_VAL)        { return u.d_data == oValue;              }
	else if (eValueType == STRING_INT_VAL)  { return u.p_data -> uc.i_data == oValue; }
	else if (eValueType == STRING_REAL_VAL) { return u.p_data -> uc.d_data == oValue; }
#if THROW_EXCEPTION_IN_COMPARATORS
	else
	{
		STLW::string sError("Can't compare ");
		sError.append(PrintableType());
		sError.append(" and REAL data types. Use `Equal`, `EQ` method instead.");

		throw CTPPLogicError(sError.c_str());
	}
#endif
return false;
}

//
// Operator == for type STLW::string
//
bool CDT::operator==(CCHAR_P oValue) const
{
#if THROW_EXCEPTION_IN_COMPARATORS
	if      (eValueType != STRING_VAL &&
		 eValueType != STRING_INT_VAL &&
		 eValueType != STRING_REAL_VAL)
	{
		STLW::string sError("Can't compare ");
		sError.append(PrintableType());
		sError.append(" and STRING data types. Use `Greater`, `GT` method instead.");

		throw CTPPLogicError(sError.c_str());
	}
#endif

return GetString() == STLW::string(oValue);
}

//
// Operator == for type STLW::string
//
bool CDT::operator==(const STLW::string & oValue) const
{
#if THROW_EXCEPTION_IN_COMPARATORS
	if      (eValueType != STRING_VAL &&
		 eValueType != STRING_INT_VAL &&
		 eValueType != STRING_REAL_VAL)
	{

		STLW::string sError("Can't compare ");
		sError.append(PrintableType());
		sError.append(" and STRING data types. Use `Equal`, `EQ` method instead.");

		throw CTPPLogicError(sError.c_str());
	}
#endif

return GetString() == oValue;
}

//
// Operator == for type CDT
//
bool CDT::operator==(const CDT & oCDT) const
{
	if      (eValueType == UNDEF && oCDT.eValueType == UNDEF)
	{
		return true;
	}
	// Integer-to-Integer
	else if ((eValueType      == INT_VAL || eValueType      == STRING_INT_VAL) &&
	         (oCDT.eValueType == INT_VAL || oCDT.eValueType == STRING_INT_VAL)
		)
	{
		return GetInt() == oCDT.GetInt();
	}
	// (Integer, Real, String+Integer, String+Real)-to-(Integer, Real, String+Integer, String+Real)
	else if ((eValueType      == REAL_VAL || eValueType      == STRING_REAL_VAL ||
		  eValueType      == INT_VAL  || eValueType      == STRING_INT_VAL) &&
	         (oCDT.eValueType == REAL_VAL || oCDT.eValueType == STRING_REAL_VAL ||
	          oCDT.eValueType == INT_VAL  || oCDT.eValueType == STRING_INT_VAL)
		)
	{
		return GetFloat() == oCDT.GetFloat();
	}
	// (String, String+Integer, String+Real)-to-(String, String+Integer, String+Real)
	else if ((eValueType      == STRING_VAL || eValueType      == STRING_REAL_VAL || eValueType      == STRING_INT_VAL) &&
		 (oCDT.eValueType == STRING_VAL || oCDT.eValueType == STRING_REAL_VAL || oCDT.eValueType == STRING_INT_VAL))
	{
		return *(u.p_data -> u.s_data) == *(oCDT.u.p_data -> u.s_data);
	}
	else if (eValueType == POINTER_VAL && oCDT.eValueType == POINTER_VAL)
	{
		return u.pp_data == oCDT.u.pp_data;
	}
#if THROW_EXCEPTION_IN_COMPARATORS
	else
	{
		STLW::string sError("Can't compare ");
		sError.append(PrintableType());
		sError.append(" and ");
		sError.append(oCDT.PrintableType());
		sError.append(" data types. Use `Equal`, `EQ` method instead.");

		throw CTPPLogicError(sError.c_str());
	}
#endif

return false;
}

// ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//
// Operator != for type INT_32
//
bool CDT::operator!=(const INT_64  oValue)  const
{
#if THROW_EXCEPTION_IN_COMPARATORS
	try
	{
		return ! this -> operator==(oValue);
	}
	catch (CTPPLogicError & e)
	{
		STLW::string sError("Can't compare ");
		sError.append(PrintableType());
		sError.append(" and REAL data types. Use `NotEqual`, `NE` method instead.");

		throw CTPPLogicError(sError.c_str());
	}
#else
	return ! this -> operator==(oValue);
#endif
return false;
}

//
// Operator != for type UINT_32
//
bool CDT::operator!=(const UINT_64  oValue)  const
{
#if THROW_EXCEPTION_IN_COMPARATORS
	try
	{
		return ! this -> operator==(oValue);
	}
	catch (CTPPLogicError & e)
	{
		STLW::string sError("Can't compare ");
		sError.append(PrintableType());
		sError.append(" and REAL data types. Use `NotEqual`, `NE` method instead.");

		throw CTPPLogicError(sError.c_str());
	}
#else
	return ! this -> operator==(oValue);
#endif
return false;
}

//
// Operator != for type INT_32
//
bool CDT::operator!=(const INT_32  oValue)  const
{
#if THROW_EXCEPTION_IN_COMPARATORS
	try
	{
		return ! this -> operator==(oValue);
	}
	catch (CTPPLogicError & e)
	{
		STLW::string sError("Can't compare ");
		sError.append(PrintableType());
		sError.append(" and REAL data types. Use `NotEqual`, `NE` method instead.");

		throw CTPPLogicError(sError.c_str());
	}
#else
	return ! this -> operator==(oValue);
#endif
return false;
}

//
// Operator != for type UINT_32
//
bool CDT::operator!=(const UINT_32  oValue) const
{
#if THROW_EXCEPTION_IN_COMPARATORS
	try
	{
		return ! this -> operator==(oValue);
	}
	catch (CTPPLogicError & e)
	{
		STLW::string sError("Can't compare ");
		sError.append(PrintableType());
		sError.append(" and REAL data types. Use `NotEqual`, `NE` method instead.");

		throw CTPPLogicError(sError.c_str());
	}
#else
	return ! this -> operator==(oValue);
#endif
return false;
}

//
// Operator != for type W_FLOAT
//
bool CDT::operator!=(const W_FLOAT  oValue)  const
{
#if THROW_EXCEPTION_IN_COMPARATORS
	try
	{
		return ! this -> operator==(oValue);
	}
	catch (CTPPLogicError & e)
	{
		STLW::string sError("Can't compare ");
		sError.append(PrintableType());
		sError.append(" and REAL data types. Use `NotEqual`, `NE` method instead.");

		throw CTPPLogicError(sError.c_str());
	}
#else
	return ! this -> operator==(oValue);
#endif
return false;
}

//
// Operator != for type STLW::string
//
bool CDT::operator!=(CCHAR_P oValue) const
{
#if THROW_EXCEPTION_IN_COMPARATORS
	try
	{
		return ! this -> operator!=(oValue);
	}
	catch (CTPPLogicError & e)
	{
		STLW::string sError("Can't compare ");
		sError.append(PrintableType());
		sError.append(" and STRING data types. Use `NotEqual`, `NE` method instead.");

		throw CTPPLogicError(sError.c_str());
	}
#else
	return ! this -> operator==(oValue);
#endif
return false;
}

//
// Operator != for type STLW::string
//
bool CDT::operator!=(const STLW::string & oValue) const
{
#if THROW_EXCEPTION_IN_COMPARATORS
	try
	{
		return ! this -> operator!=(oValue);
	}
	catch (CTPPLogicError & e)
	{
		STLW::string sError("Can't compare ");
		sError.append(PrintableType());
		sError.append(" and STRING data types. Use `NotEqual`, `NE` method instead.");

		throw CTPPLogicError(sError.c_str());
	}
#else
	return ! this -> operator==(oValue);
#endif
return false;
}

//
// Operator != for type CDT
//
bool CDT::operator!=(const CDT & oCDT) const
{
#if THROW_EXCEPTION_IN_COMPARATORS
	try
	{
		return ! this -> operator==(oCDT);
	}
	catch (CTPPLogicError & e)
	{
		STLW::string sError("Can't compare ");
		sError.append(PrintableType());
		sError.append(" and ");
		sError.append(oCDT.PrintableType());
		sError.append(" data types. Use `NotEqual`, `NE` method instead.");

		throw CTPPLogicError(sError.c_str());
	}
#else
	return ! this -> operator==(oCDT);
#endif
return false;
}

// ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//
// Operator > for type INT_32
//
bool CDT::operator>(const INT_64  oValue)  const
{
	if      (eValueType == INT_VAL)         { return u.i_data > oValue;              }
	else if (eValueType == REAL_VAL)        { return u.d_data > oValue;              }
	else if (eValueType == STRING_INT_VAL)  { return u.p_data -> uc.i_data > oValue; }
	else if (eValueType == STRING_REAL_VAL) { return u.p_data -> uc.d_data > oValue; }
#if THROW_EXCEPTION_IN_COMPARATORS
	else
	{
		STLW::string sError("Can't compare ");
		sError.append(PrintableType());
		sError.append(" and INTEGER data types. Use `Greater`, `GT` method instead.");

		throw CTPPLogicError(sError.c_str());
	}
#endif
return false;
}

//
// Operator > for type UINT_32
//
bool CDT::operator>(const UINT_64  oValue) const { return operator>(INT_64(oValue));  }

//
// Operator > for type INT_32
//
bool CDT::operator>(const INT_32  oValue)  const { return operator>(INT_64(oValue));  }

//
// Operator > for type UINT_32
//
bool CDT::operator>(const UINT_32  oValue) const { return operator>(UINT_64(oValue));  }

//
// Operator > for type W_FLOAT
//
bool CDT::operator>(const W_FLOAT  oValue) const
{
	if      (eValueType == INT_VAL)         { return u.i_data > oValue;              }
	else if (eValueType == REAL_VAL)        { return u.d_data > oValue;              }
	else if (eValueType == STRING_INT_VAL)  { return u.p_data -> uc.i_data > oValue; }
	else if (eValueType == STRING_REAL_VAL) { return u.p_data -> uc.d_data > oValue; }
#if THROW_EXCEPTION_IN_COMPARATORS
	else
	{
		STLW::string sError("Can't compare ");
		sError.append(PrintableType());
		sError.append(" and REAL data types. Use `Greater`, `GT` method instead.");

		throw CTPPLogicError(sError.c_str());
	}
#endif
return false;
}

//
// Operator > for type STLW::string
//
bool CDT::operator>(CCHAR_P oValue) const
{
#if THROW_EXCEPTION_IN_COMPARATORS
	if      (eValueType != STRING_VAL &&
		 eValueType != STRING_INT_VAL &&
		 eValueType != STRING_REAL_VAL)
	{
		STLW::string sError("Can't compare ");
		sError.append(PrintableType());
		sError.append(" and STRING data types. Use `Greater`, `GT` method instead.");

		throw CTPPLogicError(sError.c_str());
	}
#endif

return GetString() > STLW::string(oValue);
}

//
// Operator > for type STLW::string
//
bool CDT::operator>(const STLW::string & oValue) const
{
#if THROW_EXCEPTION_IN_COMPARATORS
	if      (eValueType != STRING_VAL &&
		 eValueType != STRING_INT_VAL &&
		 eValueType != STRING_REAL_VAL)
	{
		STLW::string sError("Can't compare ");
		sError.append(PrintableType());
		sError.append(" and STRING data types. Use `Greater`, `GT` method instead.");

		throw CTPPLogicError(sError.c_str());
	}
#endif

return GetString() > oValue;
}

//
// Operator > for type CDT
//
bool CDT::operator>(const CDT & oCDT) const
{
	if      (eValueType == UNDEF && oCDT.eValueType == UNDEF)
	{
		return false;
	}
	// Integer-to-Integer
	else if ((eValueType      == INT_VAL || eValueType      == STRING_INT_VAL) &&
	         (oCDT.eValueType == INT_VAL || oCDT.eValueType == STRING_INT_VAL)
		)
	{
		return GetInt() > oCDT.GetInt();
	}
	// (Integer, Real, String+Integer, String+Real)-to-(Integer, Real, String+Integer, String+Real)
	else if ((eValueType      == REAL_VAL || eValueType      == STRING_REAL_VAL ||
		  eValueType      == INT_VAL  || eValueType      == STRING_INT_VAL) &&
	         (oCDT.eValueType == REAL_VAL || oCDT.eValueType == STRING_REAL_VAL ||
	          oCDT.eValueType == INT_VAL  || oCDT.eValueType == STRING_INT_VAL)
		)
	{
		return GetFloat() > oCDT.GetFloat();
	}
	// (String, String+Integer, String+Real)-to-(String, String+Integer, String+Real)
	else if ((eValueType      == STRING_VAL || eValueType      == STRING_REAL_VAL || eValueType      == STRING_INT_VAL) &&
		 (oCDT.eValueType == STRING_VAL || oCDT.eValueType == STRING_REAL_VAL || oCDT.eValueType == STRING_INT_VAL))
	{
		return *(u.p_data -> u.s_data) > *(oCDT.u.p_data -> u.s_data);
	}
	else if (eValueType == POINTER_VAL && oCDT.eValueType == POINTER_VAL)
	{
		return u.pp_data > oCDT.u.pp_data;
	}
#if THROW_EXCEPTION_IN_COMPARATORS
	else
	{
		STLW::string sError("Can't compare ");
		sError.append(PrintableType());
		sError.append(" and ");
		sError.append(oCDT.PrintableType());
		sError.append(" data types. Use `Greater`, `GT` method instead.");

		throw CTPPLogicError(sError.c_str());
	}
#endif
return false;
}

// ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//
// Operator < for type INT_32
//
bool CDT::operator<(const INT_64  oValue) const
{
	if      (eValueType == INT_VAL)         { return u.i_data < oValue;              }
	else if (eValueType == REAL_VAL)        { return u.d_data < oValue;              }
	else if (eValueType == STRING_INT_VAL)  { return u.p_data -> uc.i_data < oValue; }
	else if (eValueType == STRING_REAL_VAL) { return u.p_data -> uc.d_data < oValue; }
#if THROW_EXCEPTION_IN_COMPARATORS
	else
	{
		STLW::string sError("Can't compare ");
		sError.append(PrintableType());
		sError.append(" and INTEGER data types. Use `Less`, `LT` method instead.");

		throw CTPPLogicError(sError.c_str());
	}
#endif
return false;
}

//
// Operator < for type UINT_32
//
bool CDT::operator<(const UINT_64  oValue) const { return operator<(INT_64(oValue));  }

//
// Operator < for type INT_32
//
bool CDT::operator<(const INT_32  oValue)  const { return operator<(INT_64(oValue));  }

//
// Operator < for type UINT_32
//
bool CDT::operator<(const UINT_32  oValue) const { return operator<(UINT_64(oValue));  }

//
// Operator < for type W_FLOAT
//
bool CDT::operator<(const W_FLOAT  oValue) const
{
	if      (eValueType == INT_VAL)         { return u.i_data < oValue;              }
	else if (eValueType == REAL_VAL)        { return u.d_data < oValue;              }
	else if (eValueType == STRING_INT_VAL)  { return u.p_data -> uc.i_data < oValue; }
	else if (eValueType == STRING_REAL_VAL) { return u.p_data -> uc.d_data < oValue; }
#if THROW_EXCEPTION_IN_COMPARATORS
	else
	{
		STLW::string sError("Can't compare ");
		sError.append(PrintableType());
		sError.append(" and REAL data types. Use `Less`, `LT` method instead.");

		throw CTPPLogicError(sError.c_str());
	}
#endif
return false;
}

//
// Operator < for type STLW::string
//
bool CDT::operator<(CCHAR_P oValue) const
{
#if THROW_EXCEPTION_IN_COMPARATORS
	if      (eValueType != STRING_VAL &&
		 eValueType != STRING_INT_VAL &&
		 eValueType != STRING_REAL_VAL)
	{
		STLW::string sError("Can't compare ");
		sError.append(PrintableType());
		sError.append(" and STRING data types. Use `Less`, `LT` method instead.");

		throw CTPPLogicError(sError.c_str());
	}
#endif

return GetString() < STLW::string(oValue);
}

//
// Operator < for type STLW::string
//
bool CDT::operator<(const STLW::string & oValue) const
{
#if THROW_EXCEPTION_IN_COMPARATORS
	if      (eValueType != STRING_VAL &&
		 eValueType != STRING_INT_VAL &&
		 eValueType != STRING_REAL_VAL)
	{
		STLW::string sError("Can't compare ");
		sError.append(PrintableType());
		sError.append(" and STRING data types. Use `Less`, `LT` method instead.");

		throw CTPPLogicError(sError.c_str());
	}
#endif

return GetString() < oValue;
}

//
// Operator < for type CDT
//
bool CDT::operator<(const CDT & oCDT) const
{
	if      (eValueType == UNDEF && oCDT.eValueType == UNDEF)
	{
		return false;
	}
	// Integer-to-Integer
	else if ((eValueType      == INT_VAL || eValueType      == STRING_INT_VAL) &&
	         (oCDT.eValueType == INT_VAL || oCDT.eValueType == STRING_INT_VAL)
		)
	{
		return GetInt() < oCDT.GetInt();
	}
	// (Integer, Real, String+Integer, String+Real)-to-(Integer, Real, String+Integer, String+Real)
	else if ((eValueType      == REAL_VAL || eValueType      == STRING_REAL_VAL ||
		  eValueType      == INT_VAL  || eValueType      == STRING_INT_VAL) &&
	         (oCDT.eValueType == REAL_VAL || oCDT.eValueType == STRING_REAL_VAL ||
	          oCDT.eValueType == INT_VAL  || oCDT.eValueType == STRING_INT_VAL)
		)
	{
		return GetFloat() < oCDT.GetFloat();
	}
	// (String, String+Integer, String+Real)-to-(String, String+Integer, String+Real)
	else if ((eValueType      == STRING_VAL || eValueType      == STRING_REAL_VAL || eValueType      == STRING_INT_VAL) &&
		 (oCDT.eValueType == STRING_VAL || oCDT.eValueType == STRING_REAL_VAL || oCDT.eValueType == STRING_INT_VAL))
	{
		return *(u.p_data -> u.s_data) < *(oCDT.u.p_data -> u.s_data);
	}
	else if (eValueType == POINTER_VAL && oCDT.eValueType == POINTER_VAL)
	{
		return u.pp_data < oCDT.u.pp_data;
	}
#if THROW_EXCEPTION_IN_COMPARATORS
	else
	{
		STLW::string sError("Can't compare ");
		sError.append(PrintableType());
		sError.append(" and ");
		sError.append(oCDT.PrintableType());
		sError.append(" data types. Use `Less`, `LT` method instead.");

		throw CTPPLogicError(sError.c_str());
	}
#endif
return false;
}

// ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//
// Operator <= for type INT_32
//
bool CDT::operator<=(const INT_64  oValue)  const
{
	if      (eValueType == INT_VAL)         { return u.i_data <= oValue;              }
	else if (eValueType == REAL_VAL)        { return u.d_data <= oValue;              }
	else if (eValueType == STRING_INT_VAL)  { return u.p_data -> uc.i_data <= oValue; }
	else if (eValueType == STRING_REAL_VAL) { return u.p_data -> uc.d_data <= oValue; }
#if THROW_EXCEPTION_IN_COMPARATORS
	else
	{
		STLW::string sError("Can't compare ");
		sError.append(PrintableType());
		sError.append(" and INTEGER data types. Use `LessOrEqual`, `LE` method instead.");

		throw CTPPLogicError(sError.c_str());
	}
#endif
return false;
}

//
// Operator <= for type UINT_32
//
bool CDT::operator<=(const UINT_64  oValue) const { return operator<=(INT_64(oValue));  }

//
// Operator <= for type INT_32
//
bool CDT::operator<=(const INT_32  oValue)  const { return operator<=(INT_64(oValue));  }

//
// Operator <= for type UINT_32
//
bool CDT::operator<=(const UINT_32  oValue) const { return operator<=(UINT_64(oValue));  }

//
// Operator <= for type W_FLOAT
//
bool CDT::operator<=(const W_FLOAT  oValue) const
{
	if      (eValueType == INT_VAL)         { return u.i_data <= oValue;              }
	else if (eValueType == REAL_VAL)        { return u.d_data <= oValue;              }
	else if (eValueType == STRING_INT_VAL)  { return u.p_data -> uc.i_data <= oValue; }
	else if (eValueType == STRING_REAL_VAL) { return u.p_data -> uc.d_data <= oValue; }
#if THROW_EXCEPTION_IN_COMPARATORS
	else
	{
		STLW::string sError("Can't compare ");
		sError.append(PrintableType());
		sError.append(" and REAL data types. Use `LessOrEqual`, `LE` method instead.");

		throw CTPPLogicError(sError.c_str());
	}
#endif
return false;
}

//
// Operator <= for type CCHAR_P
//
bool CDT::operator<=(CCHAR_P oValue) const
{
#if THROW_EXCEPTION_IN_COMPARATORS
	if      (eValueType != STRING_VAL &&
		 eValueType != STRING_INT_VAL &&
		 eValueType != STRING_REAL_VAL)
	{
		STLW::string sError("Can't compare ");
		sError.append(PrintableType());
		sError.append(" and STRING data types. Use `LessOrEqual`, `LE` method instead.");

		throw CTPPLogicError(sError.c_str());
	}
#endif

return GetString() <= STLW::string(oValue);
}

//
// Operator < for type STLW::string
//
bool CDT::operator<=(const STLW::string & oValue) const
{
#if THROW_EXCEPTION_IN_COMPARATORS
	if      (eValueType != STRING_VAL &&
		 eValueType != STRING_INT_VAL &&
		 eValueType != STRING_REAL_VAL)
	{
		STLW::string sError("Can't compare ");
		sError.append(PrintableType());
		sError.append(" and STRING data types. Use `LessOrEqual`, `LE` method instead.");

		throw CTPPLogicError(sError.c_str());
	}
#endif

return GetString() <= oValue;
}

//
// Operator <= for type CDT
//
bool CDT::operator<=(const CDT & oCDT) const
{
	if      (eValueType == UNDEF && oCDT.eValueType == UNDEF)
	{
		return true;
	}
	// Integer-to-Integer
	else if ((eValueType      == INT_VAL || eValueType      == STRING_INT_VAL) &&
	         (oCDT.eValueType == INT_VAL || oCDT.eValueType == STRING_INT_VAL)
		)
	{
		return GetInt() <= oCDT.GetInt();
	}
	// (Integer, Real, String+Integer, String+Real)-to-(Integer, Real, String+Integer, String+Real)
	else if ((eValueType      == REAL_VAL || eValueType      == STRING_REAL_VAL ||
		  eValueType      == INT_VAL  || eValueType      == STRING_INT_VAL) &&
	         (oCDT.eValueType == REAL_VAL || oCDT.eValueType == STRING_REAL_VAL ||
	          oCDT.eValueType == INT_VAL  || oCDT.eValueType == STRING_INT_VAL)
		)
	{
		return GetFloat() <= oCDT.GetFloat();
	}
	// (String, String+Integer, String+Real)-to-(String, String+Integer, String+Real)
	else if ((eValueType      == STRING_VAL || eValueType      == STRING_REAL_VAL || eValueType      == STRING_INT_VAL) &&
		 (oCDT.eValueType == STRING_VAL || oCDT.eValueType == STRING_REAL_VAL || oCDT.eValueType == STRING_INT_VAL))
	{
		return *(u.p_data -> u.s_data) <= *(oCDT.u.p_data -> u.s_data);
	}
	else if (eValueType == POINTER_VAL && oCDT.eValueType == POINTER_VAL)
	{
		return u.pp_data <= oCDT.u.pp_data;
	}
#if THROW_EXCEPTION_IN_COMPARATORS
	else
	{
		STLW::string sError("Can't compare ");
		sError.append(PrintableType());
		sError.append(" and ");
		sError.append(oCDT.PrintableType());
		sError.append(" data types. Use `LessOrEqual`, `LE` method instead.");

		throw CTPPLogicError(sError.c_str());
	}
#endif
return false;
}

// ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//
// Operator >= for type INT_32
//
bool CDT::operator>=(const INT_64  oValue)  const
{
	if      (eValueType == INT_VAL)         { return u.i_data >= oValue;              }
	else if (eValueType == REAL_VAL)        { return u.d_data >= oValue;              }
	else if (eValueType == STRING_INT_VAL)  { return u.p_data -> uc.i_data >= oValue; }
	else if (eValueType == STRING_REAL_VAL) { return u.p_data -> uc.d_data >= oValue; }
#if THROW_EXCEPTION_IN_COMPARATORS
	else
	{
		STLW::string sError("Can't compare ");
		sError.append(PrintableType());
		sError.append(" and INTEGER data types. Use `GreaterOrEqual`, `LE` method instead.");

		throw CTPPLogicError(sError.c_str());
	}
#endif
return false;
}

//
// Operator >= for type UINT_32
//
bool CDT::operator>=(const UINT_64  oValue) const { return operator>=(INT_64(oValue));  }

//
// Operator >= for type INT_32
//
bool CDT::operator>=(const INT_32  oValue)  const { return operator>=(INT_64(oValue));  }

//
// Operator >= for type UINT_32
//
bool CDT::operator>=(const UINT_32  oValue) const { return operator>=(UINT_64(oValue));  }

//
// Operator >= for type W_FLOAT
//
bool CDT::operator>=(const W_FLOAT  oValue) const
{
	if      (eValueType == INT_VAL)         { return u.i_data >= oValue;              }
	else if (eValueType == REAL_VAL)        { return u.d_data >= oValue;              }
	else if (eValueType == STRING_INT_VAL)  { return u.p_data -> uc.i_data >= oValue; }
	else if (eValueType == STRING_REAL_VAL) { return u.p_data -> uc.d_data >= oValue; }
#if THROW_EXCEPTION_IN_COMPARATORS
	else
	{
		STLW::string sError("Can't compare ");
		sError.append(PrintableType());
		sError.append(" and REAL data types. Use `GreaterOrEqual`, `GE` method instead.");

		throw CTPPLogicError(sError.c_str());
	}
#endif
return false;
}

//
// Operator >= for type CCHAR_P
//
bool CDT::operator>=(CCHAR_P oValue) const
{
#if THROW_EXCEPTION_IN_COMPARATORS
	if      (eValueType != STRING_VAL &&
		 eValueType != STRING_INT_VAL &&
		 eValueType != STRING_REAL_VAL)
	{
		STLW::string sError("Can't compare ");
		sError.append(PrintableType());
		sError.append(" and STRING data types. Use `GreaterOrEqual`, `GE` method instead.");

		throw CTPPLogicError(sError.c_str());
	}
#endif

return GetString() >= STLW::string(oValue);
}

//
// Operator >= for type STLW::string
//
bool CDT::operator>=(const STLW::string & oValue) const
{
#if THROW_EXCEPTION_IN_COMPARATORS
	if      (eValueType != STRING_VAL &&
		 eValueType != STRING_INT_VAL &&
		 eValueType != STRING_REAL_VAL)
	{
		STLW::string sError("Can't compare ");
		sError.append(PrintableType());
		sError.append(" and STRING data types. Use `GreaterOrEqual`, `GE` method instead.");

		throw CTPPLogicError(sError.c_str());
	}
#endif

return GetString() >= oValue;
}

//
// Operator >= for type CDT
//
bool CDT::operator>=(const CDT & oCDT) const
{
	if      (eValueType == UNDEF && oCDT.eValueType == UNDEF)
	{
		return true;
	}
	// Integer-to-Integer
	else if ((eValueType      == INT_VAL || eValueType      == STRING_INT_VAL) &&
	         (oCDT.eValueType == INT_VAL || oCDT.eValueType == STRING_INT_VAL)
		)
	{
		return GetInt() >= oCDT.GetInt();
	}
	// (Integer, Real, String+Integer, String+Real)-to-(Integer, Real, String+Integer, String+Real)
	else if ((eValueType      == REAL_VAL || eValueType      == STRING_REAL_VAL ||
		  eValueType      == INT_VAL  || eValueType      == STRING_INT_VAL) &&
	         (oCDT.eValueType == REAL_VAL || oCDT.eValueType == STRING_REAL_VAL ||
	          oCDT.eValueType == INT_VAL  || oCDT.eValueType == STRING_INT_VAL)
		)
	{
		return GetFloat() >= oCDT.GetFloat();
	}
	// (String, String+Integer, String+Real)-to-(String, String+Integer, String+Real)
	else if ((eValueType      == STRING_VAL || eValueType      == STRING_REAL_VAL || eValueType      == STRING_INT_VAL) &&
		 (oCDT.eValueType == STRING_VAL || oCDT.eValueType == STRING_REAL_VAL || oCDT.eValueType == STRING_INT_VAL))
	{
		return *(u.p_data -> u.s_data) >= *(oCDT.u.p_data -> u.s_data);
	}
	else if (eValueType == POINTER_VAL && oCDT.eValueType == POINTER_VAL)
	{
		return u.pp_data >= oCDT.u.pp_data;
	}
#if THROW_EXCEPTION_IN_COMPARATORS
	else
	{
		STLW::string sError("Can't compare ");
		sError.append(PrintableType());
		sError.append(" and ");
		sError.append(oCDT.PrintableType());
		sError.append(" data types. Use `GreaterOrEqual`, `GE` method instead.");

		throw CTPPLogicError(sError.c_str());
	}
#endif
return false;
}

//
// Comparator  Equal for CDT argument
//
bool CDT::Equal(const INT_64  oValue) const
{
	switch(eValueType)
	{
		case UNDEF:
		case INT_VAL:
		case REAL_VAL:
		case POINTER_VAL:
		case STRING_INT_VAL:
		case STRING_REAL_VAL:
		case STRING_VAL:
			return GetFloat() == oValue;

		case ARRAY_VAL:
		case HASH_VAL:
		default:
			return false;
	}

return false;
}

//
// Comparator  Equal for CDT argument
//
bool CDT::Equal(const UINT_64  oValue) const { return Equal(INT_64(oValue)); }

//
// Comparator  Equal for CDT argument
//
bool CDT::Equal(const INT_32  oValue) const { return Equal(INT_64(oValue)); }

//
// Comparator  Equal for CDT argument
//
bool CDT::Equal(const UINT_32  oValue) const { return Equal(UINT_64(oValue)); }

//
// Comparator  Equal for CDT argument
//
bool CDT::Equal(const W_FLOAT  oValue) const
{
	switch(eValueType)
	{
		case UNDEF:
		case INT_VAL:
		case REAL_VAL:
		case POINTER_VAL:
		case STRING_INT_VAL:
		case STRING_REAL_VAL:
		case STRING_VAL:
			return GetFloat() == oValue;

		case ARRAY_VAL:
		case HASH_VAL:
		default:
			return false;
	}

return false;
}

//
// Comparator  Equal for old-fashion string argument
//
bool CDT::Equal(CCHAR_P oValue) const { return GetString() == STLW::string(oValue); }

//
// Comparator  Equal for string argument
//
bool CDT::Equal(const STLW::string & oValue) const { return GetString() == oValue; }

//
// Comparator  Equal for CDT argument
//
bool CDT::Equal(const CDT & oCDT) const
{
	switch(eValueType)
	{
		case UNDEF:
		case INT_VAL:
		case REAL_VAL:
		case POINTER_VAL:
		case STRING_INT_VAL:
		case STRING_REAL_VAL:
			{
				switch(oCDT.eValueType)
				{
					case UNDEF:
					case INT_VAL:
					case REAL_VAL:
					case POINTER_VAL:
					case STRING_INT_VAL:
					case STRING_REAL_VAL:
						return GetFloat() == oCDT.GetFloat();

					case STRING_VAL:
						return GetString() == oCDT.GetString();
					/*
					case ARRAY_VAL:
					case HASH_VAL:
					*/
					default:
						return false;
				}
				return false;
			}
		case STRING_VAL:
			return GetString() == oCDT.GetString();
		/*
		case ARRAY_VAL:
		case HASH_VAL:
		*/
		default:
			return false;
	}

return false;
}
// Comparator NotEqual ///////////////////////////////////

//
// Comparator  NotEqual for CDT argument
//
bool CDT::NotEqual(const INT_64  oValue) const { return ! Equal(INT_64(oValue)); }

//
// Comparator  NotEqual for CDT argument
//
bool CDT::NotEqual(const UINT_64  oValue) const { return ! Equal(INT_64(oValue)); }

//
// Comparator  NotEqual for CDT argument
//
bool CDT::NotEqual(const INT_32  oValue) const { return ! Equal(INT_64(oValue)); }

//
// Comparator  NotEqual for CDT argument
//
bool CDT::NotEqual(const UINT_32  oValue) const { return ! Equal(UINT_64(oValue)); }

//
// Comparator  NotEqual for CDT argument
//
bool CDT::NotEqual(const W_FLOAT  oValue) const { return ! Equal(oValue); }

//
// Comparator  NotEqual for old-fashion string argument
//
bool CDT::NotEqual(CCHAR_P oValue) const { return ! Equal(oValue); }

//
// Comparator  NotEqual for string argument
//
bool CDT::NotEqual(const STLW::string & oValue) const { return ! Equal(oValue); }

//
// Comparator  NotEqual for CDT argument
//
bool CDT::NotEqual(const CDT & oCDT) const { return ! Equal(oCDT); }

// Comparator Greater ///////////////////////////////////

//
// Comparator  Greater for CDT argument
//
bool CDT::Greater(const INT_64  oValue) const
{
	switch(eValueType)
	{
		case UNDEF:
		case INT_VAL:
		case REAL_VAL:
		case POINTER_VAL:
		case STRING_INT_VAL:
		case STRING_REAL_VAL:
		case STRING_VAL:
			return GetFloat() > oValue;

		case ARRAY_VAL:
		case HASH_VAL:
		default:
			return false;
	}

return false;
}

//
// Comparator  Greater for CDT argument
//
bool CDT::Greater(const UINT_64  oValue) const { return Greater(INT_64(oValue)); }

//
// Comparator  Greater for CDT argument
//
bool CDT::Greater(const INT_32  oValue) const { return Greater(INT_64(oValue)); }

//
// Comparator  Greater for CDT argument
//
bool CDT::Greater(const UINT_32  oValue) const { return Greater(UINT_64(oValue)); }

//
// Comparator  Greater for CDT argument
//
bool CDT::Greater(const W_FLOAT  oValue) const
{
	switch(eValueType)
	{
		case UNDEF:
		case INT_VAL:
		case REAL_VAL:
		case POINTER_VAL:
		case STRING_INT_VAL:
		case STRING_REAL_VAL:
		case STRING_VAL:
			return GetFloat() > oValue;

		case ARRAY_VAL:
		case HASH_VAL:
		default:
			return false;
	}

return false;
}

//
// Comparator  Greater for old-fashion string argument
//
bool CDT::Greater(CCHAR_P oValue) const { return GetString() > STLW::string(oValue); }

//
// Comparator  Greater for string argument
//
bool CDT::Greater(const STLW::string & oValue) const { return GetString() > oValue; }

//
// Comparator  Greater for CDT argument
//
bool CDT::Greater(const CDT & oCDT) const
{
	switch(eValueType)
	{
		case UNDEF:
		case INT_VAL:
		case REAL_VAL:
		case POINTER_VAL:
		case STRING_INT_VAL:
		case STRING_REAL_VAL:
			{
				switch(oCDT.eValueType)
				{
					case UNDEF:
					case INT_VAL:
					case REAL_VAL:
					case POINTER_VAL:
					case STRING_INT_VAL:
					case STRING_REAL_VAL:
						return GetFloat() > oCDT.GetFloat();

					case STRING_VAL:
						return GetString() > oCDT.GetString();
					/*
					case ARRAY_VAL:
					case HASH_VAL:
					*/
					default:
						return false;
				}
				return false;
			}
		case STRING_VAL:
			return GetString() > oCDT.GetString();

		/*
		case ARRAY_VAL:
		case HASH_VAL:
		*/
		default:
			return false;
	}

return false;
}

// Comparator Less ///////////////////////////////////

//
// Comparator  Less for CDT argument
//
bool CDT::Less(const INT_64  oValue) const
{
	switch(eValueType)
	{
		case UNDEF:
		case INT_VAL:
		case REAL_VAL:
		case POINTER_VAL:
		case STRING_INT_VAL:
		case STRING_REAL_VAL:
		case STRING_VAL:
			return GetFloat() < oValue;

		case ARRAY_VAL:
		case HASH_VAL:
		default:
			return false;
	}

return false;
}

//
// Comparator  Less for CDT argument
//
bool CDT::Less(const UINT_64  oValue) const  { return Less(INT_64(oValue)); }

//
// Comparator  Less for CDT argument
//
bool CDT::Less(const INT_32  oValue) const  { return Less(INT_64(oValue)); }

//
// Comparator  Less for CDT argument
//
bool CDT::Less(const UINT_32  oValue) const  { return Less(UINT_64(oValue)); }

//
// Comparator  Less for CDT argument
//
bool CDT::Less(const W_FLOAT  oValue) const
{
	switch(eValueType)
	{
		case UNDEF:
		case INT_VAL:
		case REAL_VAL:
		case POINTER_VAL:
		case STRING_INT_VAL:
		case STRING_REAL_VAL:
		case STRING_VAL:
			return GetFloat() < oValue;

		case ARRAY_VAL:
		case HASH_VAL:
		default:
			return false;
	}

return false;
}

//
// Comparator  Less for old-fashion string argument
//
bool CDT::Less(CCHAR_P oValue) const { return GetString() < STLW::string(oValue); }

//
// Comparator  Less for string argument
//
bool CDT::Less(const STLW::string & oValue) const { return GetString() < oValue; }

//
// Comparator  Less for CDT argument
//
bool CDT::Less(const CDT & oCDT) const
{
	switch(eValueType)
	{
		case UNDEF:
		case INT_VAL:
		case REAL_VAL:
		case POINTER_VAL:
		case STRING_INT_VAL:
		case STRING_REAL_VAL:
			{
				switch(oCDT.eValueType)
				{
					case UNDEF:
					case INT_VAL:
					case REAL_VAL:
					case POINTER_VAL:
					case STRING_INT_VAL:
					case STRING_REAL_VAL:
						return GetFloat() < oCDT.GetFloat();

					case STRING_VAL:
						return GetString() < oCDT.GetString();
					/*
					case ARRAY_VAL:
					case HASH_VAL:
					*/
					default:
						return false;
				}
				return false;
			}
		case STRING_VAL:
			return GetString() < oCDT.GetString();

		/*
		case ARRAY_VAL:
		case HASH_VAL:
		*/
		default:
			return false;
	}

return false;
}

// Comparator GreaterOrEqual ///////////////////////////////////

//
// Comparator  GreaterOrEqual for CDT argument
//
bool CDT::GreaterOrEqual(const INT_64  oValue) const { return ! Less(oValue); }

//
// Comparator  GreaterOrEqual for CDT argument
//
bool CDT::GreaterOrEqual(const UINT_64  oValue) const { return ! Less(oValue); }

//
// Comparator  GreaterOrEqual for CDT argument
//
bool CDT::GreaterOrEqual(const INT_32  oValue) const { return ! Less(oValue); }

//
// Comparator  GreaterOrEqual for CDT argument
//
bool CDT::GreaterOrEqual(const UINT_32  oValue) const { return ! Less(oValue); }

//
// Comparator  GreaterOrEqual for CDT argument
//
bool CDT::GreaterOrEqual(const W_FLOAT  oValue) const { return ! Less(oValue); }

//
// Comparator  GreaterOrEqual for old-fashion string argument
//
bool CDT::GreaterOrEqual(CCHAR_P oValue) const { return ! Less(oValue); }

//
// Comparator  GreaterOrEqual for string argument
//
bool CDT::GreaterOrEqual(const STLW::string & oValue) const { return ! Less(oValue); }

//
// Comparator  GreaterOrEqual for CDT argument
//
bool CDT::GreaterOrEqual(const CDT & oCDT) const { return ! Less(oCDT); }

// Comparator LessOrEqual ///////////////////////////////////

//
// Comparator  LessOrEqual for CDT argument
//
bool CDT::LessOrEqual(const INT_64  oValue) const { return ! Greater(oValue); }

//
// Comparator  LessOrEqual for CDT argument
//
bool CDT::LessOrEqual(const UINT_64  oValue) const { return ! Greater(oValue); }

//
// Comparator  LessOrEqual for CDT argument
//
bool CDT::LessOrEqual(const INT_32  oValue) const { return ! Greater(oValue); }

//
// Comparator  LessOrEqual for CDT argument
//
bool CDT::LessOrEqual(const UINT_32  oValue) const { return ! Greater(oValue); }

//
// Comparator  LessOrEqual for CDT argument
//
bool CDT::LessOrEqual(const W_FLOAT  oValue) const { return ! Greater(oValue); }

//
// Comparator  LessOrEqual for old-fashion string argument
//
bool CDT::LessOrEqual(CCHAR_P oValue) const { return ! Greater(oValue); }

//
// Comparator  LessOrEqual for string argument
//
bool CDT::LessOrEqual(const STLW::string & oValue) const { return ! Greater(oValue); }

//
// Comparator  LessOrEqual for CDT argument
//
bool CDT::LessOrEqual(const CDT & oCDT) const { return ! Greater(oCDT); }

//
// Pre-increment operator ++
//
CDT & CDT::operator++()
{
	switch (eValueType)
	{
		case UNDEF:
			break;

		case INT_VAL:
			++u.i_data;
			break;

		case REAL_VAL:
			++u.d_data;
			break;

		case STRING_VAL:
			{
				INT_64   iData1;
				W_FLOAT  dData1;
				if (CastToNumber(iData1, dData1) == INT_VAL) { ::new (this)CDT(++iData1); }
				else                                         { ::new (this)CDT(++dData1); }

			}
			break;

		case STRING_INT_VAL:
			{
				INT_64   iData1 = u.p_data -> uc.i_data;
				::new (this)CDT(++iData1);
			}
			break;

		case STRING_REAL_VAL:
			{
				W_FLOAT  dData1 = u.p_data -> uc.d_data;
				::new (this)CDT(++dData1);
			}
			break;

		default:
			throw CDTTypeCastException("pre-increment operator ++");
	}

return *this;
}

//
// Post-increment operator ++
//
CDT CDT::operator++(int)
{
	CDT oTMP(*this);

	switch (eValueType)
	{
		case UNDEF:
			break;

		case INT_VAL:
			++u.i_data;
			break;

		case REAL_VAL:
			++u.d_data;
			break;

		case STRING_VAL:
			{
				INT_64   iData1;
				W_FLOAT  dData1;
				if (CastToNumber(iData1, dData1) == INT_VAL) { ::new (this)CDT(++iData1); }
				else                                         { ::new (this)CDT(++dData1); }
			}
			break;

		case STRING_INT_VAL:
			{
				INT_64   iData1 = u.p_data -> uc.i_data;
			 	::new (this)CDT(++iData1);
			}
			break;

		case STRING_REAL_VAL:
			{
				W_FLOAT  dData1 = u.p_data -> uc.d_data;
				::new (this)CDT(++dData1);
			}
			break;

		default:
			throw CDTTypeCastException("post-increment operator ++");
	}

return oTMP;
}

//
// Pre-decrement operator --
//
CDT & CDT::operator--()
{
	switch (eValueType)
	{
		case UNDEF:
			break;

		case INT_VAL:
			--u.i_data;
			break;

		case REAL_VAL:
			--u.d_data;
			break;

		case STRING_VAL:
			{
				INT_64   iData1;
				W_FLOAT  dData1;
				if (CastToNumber(iData1, dData1) == INT_VAL) { ::new (this)CDT(--iData1); }
				else                                         { ::new (this)CDT(--dData1); }

			}
			break;

		case STRING_INT_VAL:
			{
				INT_64   iData1 = u.p_data -> uc.i_data;
			 	::new (this)CDT(--iData1);
			}
			break;

		case STRING_REAL_VAL:
			{
				W_FLOAT  dData1 = u.p_data -> uc.d_data;
				::new (this)CDT(--dData1);
			}
			break;

		default:
			throw CDTTypeCastException("pre-decrement operator --");
	}
return *this;
}

//
// Post-decrement operator --
//
CDT CDT::operator--(int)
{
	CDT oTMP(*this);

	switch (eValueType)
	{
		case UNDEF:
			break;

		case INT_VAL:
			--u.i_data;
			break;

		case REAL_VAL:
			--u.d_data;
			break;

		case STRING_VAL:
			{
				INT_64   iData1;
				W_FLOAT  dData1;
				if (CastToNumber(iData1, dData1) == INT_VAL) { ::new (this)CDT(--iData1); }
				else                                         { ::new (this)CDT(--dData1); }
			}
			break;

		case STRING_INT_VAL:
			{
				INT_64   iData1 = u.p_data -> uc.i_data;
			 	::new (this)CDT(--iData1);
			}
			break;

		case STRING_REAL_VAL:
			{
				W_FLOAT  dData1 = u.p_data -> uc.d_data;
				::new (this)CDT(--dData1);
			}
			break;

		default:
			throw CDTTypeCastException("post-decrement operator --");
	}

return oTMP;
}

//
// Append a string to CDT
//
CDT & CDT::Concat(const STLW::string & oValue)
{
	if (eValueType == UNDEF)
	{
		::new (this)CDT(STLW::string(oValue));
	}
	else if (eValueType == INT_VAL ||
	         eValueType == REAL_VAL)
	{
		STLW::string sTMP(GetString() + oValue);
		::new (this)CDT(sTMP);
	}
	else if (eValueType == STRING_VAL     ||
	         eValueType == STRING_INT_VAL ||
	         eValueType == STRING_REAL_VAL)
	{
		Unshare();
		u.p_data -> u.s_data -> append(oValue);
	}
	else { throw CDTTypeCastException("Concat"); }

return *this;
}

//
// Append a old-fashion string to CDT
//
CDT & CDT::Concat(CCHAR_P szData, const INT_32 iDataLength)
{
	if (iDataLength == -1) { return Concat(STLW::string(szData)); }

return Concat(STLW::string(szData, iDataLength));
}

//
// Append a string to CDT, alias for Concat
//
CDT & CDT::Append(const STLW::string & oValue) { return Concat(oValue); }

//
// Append a old-fashion string to CDT
//
CDT & CDT::Append(CCHAR_P szData, const INT_32 iDataLength)  { return Concat(szData, iDataLength); }

//
// Append a INT_64 to CDT
//
CDT & CDT::Append(const INT_64  oValue)
{
	CHAR_8 szBuf[C_MAX_SPRINTF_LENGTH + 1];
	UINT_32 iLen = snprintf(szBuf, C_MAX_SPRINTF_LENGTH, "%lli", (long long) (oValue));

	if (eValueType == UNDEF)
	{
		::new (this)CDT(STLW::string(szBuf, iLen));
	}
	else if (eValueType == INT_VAL ||
	         eValueType == REAL_VAL)
	{
		STLW::string sTMP(GetString());
		sTMP.append(szBuf, iLen);

		::new (this)CDT(sTMP);
	}
	else if (eValueType == STRING_VAL     ||
	         eValueType == STRING_INT_VAL ||
	         eValueType == STRING_REAL_VAL)
	{
		Unshare();
		u.p_data -> u.s_data -> append(szBuf, iLen);
	}
	else { throw CDTTypeCastException("Append"); }

return *this;
}

//
// Append a INT_64 to CDT
//
CDT & CDT::Append(const UINT_64  oValue) { return Append(INT_64(oValue)); }

//
// Prepend a INT_32 to CDT
//
CDT & CDT::Append(const INT_32  oValue) { return Append(INT_64(oValue)); }

//
// Prepend a UINT_32 to CDT
//
CDT & CDT::Append(const UINT_32  oValue) { return Append(INT_64(oValue)); }

//
// Prepend a W_FLOAT to CDT
//
CDT & CDT::Append(const W_FLOAT  oValue)
{
	CHAR_8 szBuf[C_MAX_SPRINTF_LENGTH + 1];
	UINT_32 iLen = snprintf(szBuf, C_MAX_SPRINTF_LENGTH, "%.*G", CTPP_FLOAT_PRECISION, oValue);

	if (eValueType == UNDEF)
	{
		::new (this)CDT(STLW::string(szBuf, iLen));
	}
	else if (eValueType == INT_VAL ||
	         eValueType == REAL_VAL)
	{
		STLW::string sTMP(GetString());
		sTMP.append(szBuf, iLen);

		::new (this)CDT(sTMP);
	}
	else if (eValueType == STRING_VAL     ||
	         eValueType == STRING_INT_VAL ||
	         eValueType == STRING_REAL_VAL)
	{
		Unshare();
		u.p_data -> u.s_data -> append(szBuf, iLen);
	}
	else { throw CDTTypeCastException("Append"); }

return *this;
}

//
// Append a string to CDT
//
CDT & CDT::Append(const CDT & oCDT)
{
	if (eValueType == UNDEF)
	{
		::new (this)CDT(STLW::string(oCDT.GetString()));
	}
	else if (eValueType == INT_VAL ||
	         eValueType == REAL_VAL)
	{
		STLW::string sTMP(GetString() + oCDT.GetString());
		::new (this)CDT(sTMP);
	}
	else if (eValueType == STRING_VAL     ||
	         eValueType == STRING_INT_VAL ||
	         eValueType == STRING_REAL_VAL)
	{
		Unshare();
		u.p_data -> u.s_data -> append(oCDT.GetString());
	}
	else { throw CDTTypeCastException("Append"); }

return *this;
}

//
// Prepend a old-fashion string to CDT
//
CDT & CDT::Prepend(CCHAR_P szData, const INT_32 iDataLength)
{
	if (iDataLength == -1) { return Prepend(STLW::string(szData)); }

return Prepend(STLW::string(szData, iDataLength));
}

//
// Prepend a string to CDT
//
CDT & CDT::Prepend(const STLW::string & oValue)
{
	if (eValueType == UNDEF)
	{
		::new (this)CDT(STLW::string(oValue));
	}
	else if (eValueType == INT_VAL ||
	         eValueType == REAL_VAL)
	{
		STLW::string sTMP(oValue + GetString());
		::new (this)CDT(sTMP);
	}
	else if (eValueType == STRING_VAL     ||
	         eValueType == STRING_INT_VAL ||
	         eValueType == STRING_REAL_VAL)
	{
		Unshare();
		STLW::string sTMP = oValue;
		sTMP.append(*(u.p_data -> u.s_data));
		u.p_data -> u.s_data -> assign(sTMP);
	}
	else { throw CDTTypeCastException("Prepend"); }

return *this;
}

//
// Prepend a string to CDT
//
CDT & CDT::Prepend(const INT_64  oValue)
{
	CHAR_8 szBuf[C_MAX_SPRINTF_LENGTH + 1];
	UINT_32 iLen = snprintf(szBuf, C_MAX_SPRINTF_LENGTH, "%lli", (long long) (oValue));

	if (eValueType == UNDEF)
	{
		::new (this)CDT(STLW::string(szBuf, iLen));
	}
	else if (eValueType == INT_VAL ||
	         eValueType == REAL_VAL)
	{
		STLW::string sTMP(STLW::string(szBuf, iLen).append(GetString()));
		::new (this)CDT(sTMP);
	}
	else if (eValueType == STRING_VAL     ||
	         eValueType == STRING_INT_VAL ||
	         eValueType == STRING_REAL_VAL)
	{
		Unshare();
		STLW::string sTMP(szBuf, iLen);
		sTMP.append(*(u.p_data -> u.s_data));
		u.p_data -> u.s_data -> assign(sTMP);
	}
	else { throw CDTTypeCastException("Prepend"); }

return *this;
}

//
// Prepend a string to CDT
//
CDT & CDT::Prepend(const UINT_64  oValue) { return Prepend(INT_64(oValue)); }

//
// Prepend a string to CDT
//
CDT & CDT::Prepend(const INT_32  oValue) { return Prepend(INT_64(oValue)); }
//
// Prepend a string to CDT
//
CDT & CDT::Prepend(const UINT_32  oValue) { return Prepend(UINT_64(oValue)); }

//
// Prepend a string to CDT
//
CDT & CDT::Prepend(const W_FLOAT  oValue)
{
	CHAR_8 szBuf[C_MAX_SPRINTF_LENGTH + 1];
	UINT_32 iLen = snprintf(szBuf, C_MAX_SPRINTF_LENGTH, "%.*G", CTPP_FLOAT_PRECISION, oValue);

	if (eValueType == UNDEF)
	{
		::new (this)CDT(STLW::string(szBuf, iLen));
	}
	else if (eValueType == INT_VAL ||
	         eValueType == REAL_VAL)
	{
		STLW::string sTMP(STLW::string(szBuf, iLen).append(GetString()));
		::new (this)CDT(sTMP);
	}
	else if (eValueType == STRING_VAL     ||
	         eValueType == STRING_INT_VAL ||
	         eValueType == STRING_REAL_VAL)
	{
		Unshare();
		STLW::string sTMP(szBuf, iLen);
		sTMP.append(*(u.p_data -> u.s_data));
		u.p_data -> u.s_data -> assign(sTMP);
	}
	else { throw CDTTypeCastException("Prepend"); }

return *this;
}

//
// Prepend a string to CDT
//
CDT & CDT::Prepend(const CDT & oCDT)
{
	if (eValueType == UNDEF)
	{
		::new (this)CDT(oCDT.GetString());
	}
	else if (eValueType == INT_VAL ||
	         eValueType == REAL_VAL)
	{
		STLW::string sTMP(oCDT.GetString() + GetString());
		::new (this)CDT(sTMP);
	}
	else if (eValueType == STRING_VAL     ||
	         eValueType == STRING_INT_VAL ||
	         eValueType == STRING_REAL_VAL)
	{
		Unshare();
		STLW::string sTMP = oCDT.GetString();
		sTMP.append(*(u.p_data -> u.s_data));
		u.p_data -> u.s_data -> assign(sTMP);
	}
	else { throw CDTTypeCastException("Prepend"); }

return *this;
}

//
// Get value as W_FLOAT
//
W_FLOAT CDT::GetFloat() const
{
	switch (eValueType)
	{
		case INT_VAL:
			return W_FLOAT(u.i_data);

		case REAL_VAL:
			return u.d_data;

		case STRING_VAL:
			{
				INT_64   iData1;
				W_FLOAT  dData1;
				if (CastToNumber(iData1, dData1) == REAL_VAL) { return dData1; }

				return W_FLOAT(iData1);
			}

		case STRING_INT_VAL:
			return W_FLOAT(u.p_data -> uc.i_data);

		case STRING_REAL_VAL:
			return u.p_data -> uc.d_data;

		case POINTER_VAL:
			return W_FLOAT((INT_64)(u.pp_data));

		default:
			return 0.0;
	}
}

//
// Get value as INT_64
//
INT_64 CDT::GetInt() const
{
	switch (eValueType)
	{
		case INT_VAL:
			return u.i_data;

		case REAL_VAL:
			return INT_64(u.d_data);

		case STRING_VAL:
			{
				INT_64   iData1;
				W_FLOAT  dData1;
				if (CastToNumber(iData1, dData1) == REAL_VAL) { return INT_64(dData1); }

				return iData1;
			}

		case STRING_INT_VAL:
			return u.p_data -> uc.i_data;

		case STRING_REAL_VAL:
			return INT_64(u.p_data -> uc.d_data);

		case POINTER_VAL:
			return (INT_64)(u.pp_data);

		default:
			return 0;
	}
}

//
// Get value as UINT_64
//
UINT_64 CDT::GetUInt() const
{
	switch (eValueType)
	{
		case INT_VAL:
			return u.i_data;

		case REAL_VAL:
			return INT_64(u.d_data);

		case STRING_VAL:
			{
				INT_64   iData1;
				W_FLOAT  dData1;
				if (CastToNumber(iData1, dData1) == REAL_VAL) { return UINT_64(dData1); }

				return iData1;
			}

		case STRING_INT_VAL:
			return u.p_data -> uc.i_data;

		case STRING_REAL_VAL:
			return UINT_64(u.p_data -> uc.d_data);

		default:
			return 0;
	}
}

//
// Cast value to W_FLOAT
//
W_FLOAT CDT::ToFloat()
{
	const W_FLOAT dTMP = GetFloat();
	
	::new(this) CDT(dTMP);
	
return dTMP;
}

//
// Cast value to INT_64
//
INT_64 CDT::ToInt()
{
	const INT_64 iTMP = GetInt();
	
	::new(this) CDT(iTMP);
	
return iTMP;
}

//
// Cast value to STLW::string
//
STLW::string CDT::ToString(CCHAR_P szFormat)
{
	const STLW::string sTMP = GetString(szFormat);
	
	::new(this) CDT(sTMP);
	
return sTMP;
}

//
// Get value as string
//
STLW::string CDT::GetString(CCHAR_P szFormat) const
{
	switch (eValueType)
	{
		case INT_VAL:
			{
				CHAR_8 szBuf[C_MAX_SPRINTF_LENGTH + 1];
				if (szFormat != NULL && *szFormat != '\0')
				{
					snprintf(szBuf, C_MAX_SPRINTF_LENGTH, szFormat, (long long) (u.i_data));
				}
				else
				{
					snprintf(szBuf, C_MAX_SPRINTF_LENGTH, "%lli", (long long) (u.i_data));
				}

			return szBuf;
			}

		case REAL_VAL:
			{
				CHAR_8 szBuf[C_MAX_SPRINTF_LENGTH + 1];
				if (szFormat != NULL && *szFormat != '\0')
				{
					snprintf(szBuf, C_MAX_SPRINTF_LENGTH, szFormat, u.d_data);
				}
				else
				{
					snprintf(szBuf, C_MAX_SPRINTF_LENGTH, "%.*G", CTPP_FLOAT_PRECISION, u.d_data);
				}
			return szBuf;
			}

		case POINTER_VAL:
			{
				CHAR_8 szBuf[C_MAX_SPRINTF_LENGTH + 1];
				if (szFormat != NULL && *szFormat != '\0')
				{
					snprintf(szBuf, C_MAX_SPRINTF_LENGTH, szFormat, u.pp_data);
				}
				else
				{
					snprintf(szBuf, C_MAX_SPRINTF_LENGTH, "%p", u.pp_data);
				}
			return szBuf;
			}


		case STRING_VAL:
		case STRING_INT_VAL:
		case STRING_REAL_VAL:
			return *(u.p_data -> u.s_data);

		case ARRAY_VAL:
			{
				CHAR_8 szBuf[C_MAX_SPRINTF_LENGTH + 1];
				snprintf(szBuf, C_MAX_SPRINTF_LENGTH, "ARRAY (%p)", (void *)(u.p_data -> u.s_data));
				return szBuf;
			}

		case HASH_VAL:
			{
				CHAR_8 szBuf[C_MAX_SPRINTF_LENGTH + 1];
				snprintf(szBuf, C_MAX_SPRINTF_LENGTH, "HASH (%p)", (void *)(u.p_data -> u.s_data));
				return szBuf;
			}

		default:
			return "";
	}
}

//
// Get generic pointer
//
const void * CDT::GetPointer() const
{
	if (eValueType != POINTER_VAL) { return NULL; }

return u.pp_data;
}

//
// Get generic pointer
//
void * CDT::GetPointer()
{
	if (eValueType != POINTER_VAL) { return NULL; }

return u.pp_data;
}

//
// Dump CDT recursively to string
//
STLW::string CDT::Dump(UINT_32 iLevel) const
{
	STLW::string sResult;
	DumpData(iLevel, 0, *this, sResult);
	return sResult;
}

//
// Dump CDT recursively to string
//
STLW::string CDT::RecursiveDump(UINT_32 iLevel) const
{
	return Dump(iLevel);
}

//
// Dump to string
//
void CDT::DumpData(UINT_32 iLevel, UINT_32 iOffset, const CDT & oData, STLW::string &sResult)
{
	CHAR_8 szBuf[512 + 1];

	++iLevel;
	switch (oData.GetType())
	{
		case UNDEF:
		case INT_VAL:
		case REAL_VAL:
		case STRING_INT_VAL:
		case STRING_REAL_VAL:
		case POINTER_VAL:
			sResult += oData.GetString();
			break;
		case STRING_VAL:
			sResult += "\"";
			sResult += EscapeJSONString(oData.GetString(), true, false);
			sResult += "\"";
			break;

		case ARRAY_VAL:
			{
				if (oData.Size() == 0) { sResult += "[ ]"; }
				else
				{
					sResult += "[\n";
					UINT_32 iI = 0;
					for (;;)
					{
						sResult.append(iOffset + iLevel * 2, ' ');

						const INT_32 iFmtLen = snprintf(szBuf, 512, "%u", iI);
						sResult += szBuf;
						sResult += " : ";
						DumpData(iLevel, iOffset + iFmtLen + 3, oData.GetCDT(iI), sResult);

						++iI;

						if (iI == oData.Size()) { break; }
						sResult += ",\n";
					}
					sResult.append("\n");
					sResult.append(iOffset + (iLevel - 1) * 2, ' ');
					sResult += "]";
				}
			}
			break;

		case HASH_VAL:
			{
				ConstIterator itHash = oData.Begin();
				if (itHash == oData.End()) { sResult += "{ }"; }
				else
				{
					sResult += "{\n";
					for(;;)
					{
						sResult.append(iOffset + iLevel * 2, ' ');
						sResult += "\"";
						sResult += EscapeJSONString(itHash -> first, true, false);
						sResult += "\" => ";
						DumpData(iLevel, iOffset + itHash -> first.size() + 6, itHash -> second, sResult);

						++itHash;

						if (itHash == oData.End()) { break; }
						sResult += ",\n";
					}
					sResult.append("\n");
					sResult.append(iOffset + (iLevel - 1) * 2, ' ');
					sResult += "}";
				}
			}
			break;

		default:
			sResult += "Invalid type";
	}
}

//
// Get value type of object
//
CDT::eValType CDT::GetType() const { return eValueType; }

//
// Get printable value type of object
//
CCHAR_P CDT::PrintableType() const { return PrintableType(eValueType); }

//
// Get printable value type
//
CCHAR_P CDT::PrintableType(eValType eType)
{
	switch(eType)
	{
		case UNDEF:           return "*UNDEF*";
		case INT_VAL:         return "INTEGER";
		case REAL_VAL:        return "REAL";
		case STRING_VAL:      return "STRING";
		case STRING_INT_VAL:  return "STRING+INT";
		case STRING_REAL_VAL: return "STRING+REAL";
		case ARRAY_VAL:       return "ARRAY";
		case HASH_VAL:        return "HASH";
		case POINTER_VAL:     return "POINTER";
		default:              return "???????";
	}
	// This should *NOT* happened;
return NULL;
}

//
// Get array or hash or string size
//
UINT_32 CDT::Size() const
{
	switch (eValueType)
	{
		case UNDEF:
		case INT_VAL:
		case REAL_VAL:
		case POINTER_VAL:
			return 0;
		case STRING_VAL:
		case STRING_INT_VAL:
		case STRING_REAL_VAL:
			return u.p_data -> u.s_data -> size();

		case ARRAY_VAL:
			return u.p_data -> u.v_data -> size();

		case HASH_VAL:
			return u.p_data -> u.m_data -> size();

		default:
			return 0;
	}

return 0;
}

//
// Swap values
//
CDT & CDT::Swap(CDT & oCDT)
{
	const CDT oTMP = oCDT;

	oCDT = *this;

	*this = oTMP;

return *this;
}

//
// Quick sort ARRAY
//
void CDT::SortArray(const CDT::SortingComparator  & oSortingComparator)
{
	if (eValueType != ARRAY_VAL || u.p_data -> u.v_data -> size() <= 1) { return; }

	STLW::sort(u.p_data -> u.v_data -> begin(), u.p_data -> u.v_data -> end(), SortHelper(oSortingComparator));
}

// ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//
// A destructor
//
CDT::SortingComparator::~SortingComparator() throw() { ;; }

//
// A destructor
//
CDT::~CDT() throw() { Destroy(); }

//
// Destroy object if need
//
void CDT::Destroy() throw()
{
	switch (eValueType)
	{
		// POD types
		case UNDEF:
		case INT_VAL:
		case REAL_VAL:
		case POINTER_VAL:
			;; // Nothing to do
			break;

		case STRING_INT_VAL:
		case STRING_REAL_VAL:
		case STRING_VAL:
			-- (u.p_data -> refcount);
			if (u.p_data -> refcount == 0)
			{
				delete u.p_data -> u.s_data;
				delete u.p_data;
			}
			break;

		case ARRAY_VAL:
			-- (u.p_data -> refcount);
			if (u.p_data -> refcount == 0)
			{
				delete u.p_data -> u.v_data;
				delete u.p_data;
			}
			break;

		case HASH_VAL:
			-- (u.p_data -> refcount);
			if (u.p_data -> refcount == 0)
			{
				delete u.p_data -> u.m_data;
				delete u.p_data;
			}
			break;

		default:
			// R.I.P.
			{ int * pI = NULL; *pI = 0xDeadBeef; }
	}
}

//
// Unshare shareable container
//
void CDT::Unshare()
{
	if (u.p_data -> refcount != 1)
	{
		_CDT * pTMP = new _CDT();

		if      (eValueType == STRING_VAL)     { pTMP -> u.s_data = new String(*(u.p_data -> u.s_data)); }
		else if (eValueType == STRING_INT_VAL)
		{
			pTMP -> u.s_data = new String(*(u.p_data -> u.s_data));
			pTMP -> uc.i_data = u.p_data -> uc.i_data;
		}
		else if (eValueType == STRING_REAL_VAL)
		{
			pTMP -> u.s_data = new String(*(u.p_data -> u.s_data));
			pTMP -> uc.d_data = u.p_data -> uc.d_data;
		}
		else if (eValueType == ARRAY_VAL)  { pTMP -> u.v_data = new Vector(*(u.p_data -> u.v_data)); }
		else if (eValueType == HASH_VAL)   { pTMP -> u.m_data = new Map(*(u.p_data -> u.m_data));    }

		-- u.p_data -> refcount;
		u.p_data = pTMP;
	}
}

//
// Try to cast value to integer or to IEEE floating point value
//
CDT::eValType CDT::CastToNumber(INT_64 & iData, W_FLOAT & dData) const
{
	iData = 0;
	dData = 0.0;

	switch (eValueType)
	{
		case UNDEF:
			return INT_VAL;

		case INT_VAL:
			iData = u.i_data;
			return INT_VAL;

		case REAL_VAL:
			dData = u.d_data;
			return REAL_VAL;

		case STRING_VAL:
			{
				CheckComplexDataType();
				if (eValueType != STRING_VAL) { return CastToNumber(iData, dData); }

				// Check integer only
				String::const_iterator        itStart = u.p_data -> u.s_data -> begin();
				const String::const_iterator  itEnd   = u.p_data -> u.s_data -> end();
				if (itStart == itEnd)
				{
					u.p_data -> uc.i_data = iData;
					eValueType = STRING_INT_VAL;
					u.p_data -> value_type = INT_VAL;
					return INT_VAL;
				}

				// [-+]?[0-9]

				// Check sign
				if (*itStart == '-' || *itStart == '+') { ++itStart; }

				// Check numbers
				while (itStart != itEnd)
				{
					if (!(*itStart >= '0' && *itStart <= '9')) { break; }
					++itStart;
				}

				// Okay, it's integer
				if (itStart == itEnd)
				{
					iData = strtoll(u.p_data -> u.s_data -> data(), NULL, 10);

					u.p_data -> uc.i_data = iData;
					eValueType = STRING_INT_VAL;
					u.p_data -> value_type = INT_VAL;
					return INT_VAL;
				}

				// [-+]?[0-9]*\.?[0-9]+([eE][-+]?[0-9]+)?

				// Check IEEE 754
				if (*itStart == '.')
				{
					++itStart;
					if (itStart == itEnd)
					{
						u.p_data -> uc.d_data = dData;
						eValueType = STRING_REAL_VAL;
						u.p_data -> value_type = REAL_VAL;
						return REAL_VAL;
					}

					while (itStart != itEnd)
					{
						if (!(*itStart >= '0' && *itStart <= '9')) { break; }
						++itStart;
					}
				}
				// Okay, it's real without exponent
				if (itStart == itEnd)
				{
					dData = strtod(u.p_data -> u.s_data -> data(), NULL);

					u.p_data -> uc.d_data = dData;
					eValueType = STRING_REAL_VAL;
					u.p_data -> value_type = REAL_VAL;
					return REAL_VAL;
				}

				// Check exponent
				if (*itStart != 'e' && *itStart != 'E')
				{
					u.p_data -> uc.d_data = dData;
					eValueType = STRING_REAL_VAL;
					u.p_data -> value_type = REAL_VAL;
					return REAL_VAL;
				}
				++itStart;

				// Check exponent sign
				if (itStart == itEnd)
				{
					u.p_data -> uc.d_data = dData;
					eValueType = STRING_REAL_VAL;
					u.p_data -> value_type = REAL_VAL;
					return REAL_VAL;
				}

				if (*itStart == '-' || *itStart == '+')
				{
					++itStart;
					if (itStart == itEnd)
					{
						u.p_data -> uc.d_data = dData;
						eValueType = STRING_REAL_VAL;
						u.p_data -> value_type = REAL_VAL;
						return REAL_VAL;
					}
				}

				while (itStart != itEnd)
				{
					if (!(*itStart >= '0' && *itStart <= '9')) { break; }
					++itStart;
				}

				// Okay, it's real with exponent
				if (itStart == itEnd)
				{
					dData = strtod(u.p_data -> u.s_data -> data(), NULL);

					u.p_data -> uc.d_data = dData;
					eValueType = STRING_REAL_VAL;
					u.p_data -> value_type = REAL_VAL;
					return REAL_VAL;
				}

			return REAL_VAL;
			}

		case STRING_INT_VAL:
			iData = u.p_data -> uc.i_data;
			return INT_VAL;

		case STRING_REAL_VAL:
			dData = u.p_data -> uc.d_data;
			return REAL_VAL;

		case ARRAY_VAL:
			iData = u.p_data -> u.v_data -> size();
			return INT_VAL;

		case HASH_VAL:
			iData = u.p_data -> u.m_data -> size();
			return INT_VAL;

		case POINTER_VAL:
			iData = UINT_64(u.pp_data);
			return INT_VAL;

		default:
			throw CDTTypeCastException("No such type");
	}
// Make compiler happy
return INT_VAL;
}

//
// Check complex data type and change value type, if need
//
void CDT::CheckComplexDataType() const
{
	if      (u.p_data -> value_type == INT_VAL)  { eValueType = STRING_INT_VAL;  }
	else if (u.p_data -> value_type == REAL_VAL) { eValueType = STRING_REAL_VAL; }
}

// ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Standalone operators
//

// ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Operator+
//

//
// Operator + for UINT_64 and CDT arguments
//
CDT operator+(const UINT_64 & oValue, const CDT & oCDT) { return CDT(oValue).operator+(oCDT); }

//
// Operator + for INT_64 and CDT arguments
//
CDT operator+(const INT_64 & oValue, const CDT & oCDT)  { return CDT(oValue).operator+(oCDT); }

//
// Operator + for UINT_32 and CDT arguments
//
CDT operator+(const UINT_32 & oValue, const CDT & oCDT) { return CDT(oValue).operator+(oCDT); }

//
// Operator + for INT_32 and CDT arguments
//
CDT operator+(const INT_32 & oValue, const CDT & oCDT) { return CDT(oValue).operator+(oCDT); }

//
// Operator + for W_FLOAT and CDT arguments
//
CDT operator+(const W_FLOAT & oValue, const CDT & oCDT) { return CDT(oValue).operator+(oCDT); }

// ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Operator-
//

//
// Operator - for UINT_64 and CDT arguments
//
CDT operator-(const UINT_64 & oValue, const CDT & oCDT) { return CDT(oValue).operator-(oCDT); }

//
// Operator - for INT_64 and CDT arguments
//
CDT operator-(const INT_64 & oValue, const CDT & oCDT)  { return CDT(oValue).operator-(oCDT); }

//
// Operator - for UINT_32 and CDT arguments
//
CDT operator-(const UINT_32 & oValue, const CDT & oCDT) { return CDT(oValue).operator-(oCDT); }

//
// Operator - for INT_32 and CDT arguments
//
CDT operator-(const INT_32 & oValue, const CDT & oCDT) { return CDT(oValue).operator-(oCDT); }

//
// Operator - for W_FLOAT and CDT arguments
//
CDT operator-(const W_FLOAT & oValue, const CDT & oCDT) { return CDT(oValue).operator-(oCDT); }

// ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Operator*
//

//
// Operator * for UINT_64 and CDT arguments
//
CDT operator*(const UINT_64 & oValue, const CDT & oCDT) { return CDT(oCDT).operator*(oValue); }

//
// Operator * for INT_64 and CDT arguments
//
CDT operator*(const INT_64 & oValue, const CDT & oCDT)  { return CDT(oCDT).operator*(oValue); }

//
// Operator * for UINT_32 and CDT arguments
//
CDT operator*(const UINT_32 & oValue, const CDT & oCDT) { return CDT(oCDT).operator*(oValue); }

//
// Operator * for INT_32 and CDT arguments
//
CDT operator*(const INT_32 & oValue, const CDT & oCDT)  { return CDT(oCDT).operator*(oValue); }

//
// Operator * for W_FLOAT and CDT arguments
//
CDT operator*(const W_FLOAT & oValue, const CDT & oCDT) { return CDT(oCDT).operator*(oValue); }

// ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Operator/
//

//
// Operator / for UINT_64 and CDT arguments
//
CDT operator/(const UINT_64 & oValue, const CDT & oCDT) { return CDT(oCDT).operator/(oValue); }

//
// Operator / for INT_64 and CDT arguments
//
CDT operator/(const INT_64 & oValue, const CDT & oCDT)  { return CDT(oCDT).operator/(oValue); }

//
// Operator / for UINT_32 and CDT arguments
//
CDT operator/(const UINT_32 & oValue, const CDT & oCDT) { return CDT(oCDT).operator/(oValue); }

//
// Operator / for INT_32 and CDT arguments
//
CDT operator/(const INT_32 & oValue, const CDT & oCDT)  { return CDT(oCDT).operator/(oValue); }

//
// Operator / for W_FLOAT and CDT arguments
//
CDT operator/(const W_FLOAT & oValue, const CDT & oCDT) { return CDT(oCDT).operator/(oValue); }
} // namespace CTPP
// End.
