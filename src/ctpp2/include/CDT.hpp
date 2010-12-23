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
 *      CDT.hpp
 *
 * $CTPP$
 */
#ifndef _CDT_HPP__
#define _CDT_HPP__ 1

/**
  @file CDT.hpp
  @brief Common Data Type
*/

#include "STLMap.hpp"
#include "STLString.hpp"
#include "STLVector.hpp"

#include "CTPP2Exception.hpp"

namespace CTPP // C++ Template Engine
{

#define C_MAX_SPRINTF_LENGTH 128

/**
  @class CDT CDT.hpp <CDT.hpp>
  @brief Common Data Type
*/
class CTPP2DECL CDT
{
private:
	/**
	  @var typedef STLW::string<CDT> String
	  @brief internal string definition
	*/
	typedef STLW::string            String;

	/**
	  @var typedef STLW::vector<CDT> Vector
	  @brief internal array definition
	*/
	typedef STLW::vector<CDT>       Vector;

	/**
	  @var typedef STLW::map<String, CDT> Map
	  @brief internal hash definition
	*/
	typedef STLW::map<String, CDT>  Map;
public:
	/**
	  @enum eValType CDT.hpp <CDT.hpp>
	  @brief Describes type of stored value
	*/
	enum eValType { UNDEF           = 0x01,
	                INT_VAL         = 0x02,
	                REAL_VAL        = 0x04,
	                POINTER_VAL     = 0x08,
	                STRING_VAL      = 0x10,

	                STRING_INT_VAL  = 0x12,
	                STRING_REAL_VAL = 0x14,

	                ARRAY_VAL       = 0x20,
	                HASH_VAL        = 0x40 /*,
	                REFERENCE_VAL   = 0x80 */
	                 };

	// FWD
	class SortingComparator;

	/**
	  @brief Constructor
	  @param oValue - type of value
	*/
	CDT(const CDT::eValType & oValue = UNDEF);

	/**
	  @brief Copy constructor
	  @param oCDT - Object to copy
	*/
	CDT(const CDT & oCDT);

	/**
	  @brief Copy operator
	  @param oCDT - Object to copy
	  @return read/write referense to self
	*/
	CDT & operator=(const CDT & oCDT);

	/**
	  @brief Type cast constructor
	  @param oValue - INT_64 value
	*/
	CDT(const INT_64  oValue);

	/**
	  @brief Type cast constructor
	  @param oValue - UINT_64 value
	*/
	CDT(const UINT_64  oValue);

	/**
	  @brief Type cast constructor
	  @param oValue - INT_32 value
	*/
	CDT(const INT_32  oValue);

	/**
	  @brief Type cast constructor
	  @param oValue - UINT_32 value
	*/
	CDT(const UINT_32  oValue);

	/**
	  @brief Type cast constructor
	  @param oValue - W_FLOAT value
	*/
	CDT(const W_FLOAT  oValue);

	/**
	  @brief Type cast constructor
	  @param oValue - string value
	*/
	CDT(const STLW::string & oValue);

	/**
	  @brief Type cast constructor
	  @param oValue - asciz string to copy
	  @return read/write referense to self
	*/
	CDT(CCHAR_P oValue);

	/**
	  @brief Type cast constructor
	  @param oValue - generic pointer value
	*/
	CDT(void * oValue);

	/**
	  @brief Copy operator
	  @param oValue - INT_64 value to copy
	  @return read/write referense to self
	*/
	CDT & operator=(const INT_64  oValue);

	/**
	  @brief Copy operator
	  @param oValue - UINT_64 value to copy
	  @return read/write referense to self
	*/
	CDT & operator=(const UINT_64  oValue);

	/**
	  @brief Copy operator
	  @param oValue - INT_32 value to copy
	  @return read/write referense to self
	*/
	CDT & operator=(const INT_32  oValue);

	/**
	  @brief Copy operator
	  @param oValue - UINT_32 value to copy
	  @return read/write referense to self
	*/
	CDT & operator=(const UINT_32  oValue);

	/**
	  @brief Copy operator
	  @param oValue - W_FLOAT value to copy
	  @return read/write referense to self
	*/
	CDT & operator=(const W_FLOAT  oValue);

	/**
	  @brief Copy operator
	  @param oValue - string to copy
	  @return read/write referense to self
	*/
	CDT & operator=(const STLW::string & oValue);

	/**
	  @brief Copy operator
	  @param oValue - asciz string to copy
	  @return read/write referense to self
	*/
	CDT & operator=(CCHAR_P oValue);

	/**
	  @brief Copy operator
	  @param oValue - generic pointer
	  @return read/write referense to self
	*/
	CDT & operator=(void * oValue);

	/**
	  @brief Provides access to the data contained in CDT
	  @param iPos - The index of the element
	  @return Object with data
	*/
	CDT & operator[](const UINT_32  iPos);

	/**
	  @brief Provides access to the data contained in CDT
	  @param sKey - The key of the element
	  @return Object with data
	*/
	CDT & operator[](const STLW::string & sKey);

	/**
	  @brief Provides constant access to the data contained in CDT
	  @param iPos - The index of the element
	  @return Object with data
	*/
	CDT GetCDT(const UINT_32  iPos) const;

	/**
	  @brief Provides constant access to the data contained in CDT
	  @param sKey - The key of the hash
	  @return Object with data
	*/
	CDT GetCDT(const STLW::string & sKey) const;

	/**
	  @brief Provides constant access to the data contained in CDT
	  @param sKey - The key of the hash [in]
	  @param bCDTExist - Existence flag [out], is set to true if object exist or false otherwise
	  @return Object with data
	*/
	CDT GetExistedCDT(const STLW::string & sKey, bool & bCDTExist) const;

	/**
	  @brief Erase element from HASH
	  @param sKey - The key of the hash [in]
	  @return true - if key found, false - otherwise
	*/
	bool Erase(const STLW::string & sKey);

	/**
	  @brief Check element in hash
	  @param sKey - The key of the hash
	  @return true, if element present, false - otherwise
	*/
	bool Exists(const STLW::string & sKey) const;

	/**
	  @brief Check element in array
	  @param iPos - The index of the element
	  @return true, if element present, false - otherwise
	*/
	bool Exists(const UINT_32  iPos) const;

	/**
	  @brief Push value into array
	  @param oValue - INT_64 value
	*/
	void PushBack(const INT_64  oValue);

	/**
	  @brief Push value into array
	  @param oValue - UINT_64 value
	*/
	void PushBack(const UINT_64  oValue);

	/**
	  @brief Push value into array
	  @param oValue - INT_32 value
	*/
	void PushBack(const INT_32  oValue);

	/**
	  @brief Push value into array
	  @param oValue - UINT_32 value
	*/
	void PushBack(const UINT_32  oValue);

	/**
	  @brief Push value into array
	  @param oValue - W_FLOAT value
	*/
	void PushBack(const W_FLOAT  oValue);

	/**
	  @brief Push value into array
	  @param oValue - string value
	*/
	void PushBack(const STLW::string & oValue);

	/**
	  @brief Push value into array
	  @param oValue - asciz string
	*/
	void PushBack(CCHAR_P oValue);

	/**
	  @brief Push value into array
	  @param oValue - CDT object
	*/
	void PushBack(const CDT & oValue);

	/**
	  @brief Returns a boolean value telling whether object has a value
	  @return true if object has a value, false - otherwise
	*/
	bool Defined() const;

	/**
	  @brief Provides range-check access to the data contained in CDT
	  @param iPos - The index of the element
	  @return Read/write reference to data
	*/
	CDT & At(const UINT_32  iPos);

	/**
	  @brief Provides range-check access to the data contained in CDT
	  @param sKey - The key of the hash
	  @return Read/write reference to data
	*/
	CDT & At(const STLW::string & sKey);

	// Operator + ////////////////////////////////////

	/**
	  @brief Operator + for INT_64 argument
	  @param oValue - value to add
	  @return new CDT object
	*/
	CDT operator+(const INT_64  oValue) const;

	/**
	  @brief Operator + for UINT_64 argument
	  @param oValue - value to add
	  @return new CDT object
	*/
	CDT operator+(const UINT_64  oValue) const;

	/**
	  @brief Operator + for INT_32 argument
	  @param oValue - value to add
	  @return new CDT object
	*/
	CDT operator+(const INT_32  oValue) const;

	/**
	  @brief Operator + for UINT_32 argument
	  @param oValue - value to add
	  @return new CDT object
	*/
	CDT operator+(const UINT_32  oValue) const;

	/**
	  @brief Operator + for W_FLOAT argument
	  @param oValue - value to add
	  @return new CDT object
	*/
	CDT operator+(const W_FLOAT  oValue) const;

	/**
	  @brief Operator + for CDT argument
	  @param oCDT - value to add
	  @return new CDT object
	*/
	CDT operator+(const CDT & oCDT) const;

	// Operator - ////////////////////////////////////

	/**
	  @brief Operator - for INT_64 argument
	  @param oValue - value to substract
	  @return new CDT object
	*/
	CDT operator-(const INT_64  oValue) const;

	/**
	  @brief Operator - for UINT_64 argument
	  @param oValue - value to substract
	  @return new CDT object
	*/
	CDT operator-(const UINT_64  oValue) const;

	/**
	  @brief Operator - for INT_32 argument
	  @param oValue - value to substract
	  @return new CDT object
	*/
	CDT operator-(const INT_32  oValue) const;

	/**
	  @brief Operator - for UINT_32 argument
	  @param oValue - value to substract
	  @return new CDT object
	*/
	CDT operator-(const UINT_32  oValue) const;

	/**
	  @brief Operator - for W_FLOAT argument
	  @param oValue - value to substract
	  @return new CDT object
	*/
	CDT operator-(const W_FLOAT  oValue) const;

	/**
	  @brief Operator - for CDT argument
	  @param oCDT - value to substract
	  @return new CDT object
	*/
	CDT operator-(const CDT & oCDT) const;

	// Operator * ////////////////////////////////////

	/**
	  @brief Operator * for INT_64 argument
	  @param oValue - value to multiplicate
	  @return new CDT object
	*/
	CDT operator*(const INT_64  oValue) const;

	/**
	  @brief Operator * for UINT_64 argument
	  @param oValue - value to multiplicate
	  @return new CDT object
	*/
	CDT operator*(const UINT_64  oValue) const;

	/**
	  @brief Operator * for INT_32 argument
	  @param oValue - value to multiplicate
	  @return new CDT object
	*/
	CDT operator*(const INT_32  oValue) const;

	/**
	  @brief Operator * for UINT_32 argument
	  @param oValue - value to multiplicate
	  @return new CDT object
	*/
	CDT operator*(const UINT_32  oValue) const;

	/**
	  @brief Operator * for W_FLOAT argument
	  @param oValue - value to multiplicate
	  @return new CDT object
	*/
	CDT operator*(const W_FLOAT  oValue) const;

	/**
	  @brief Operator * for CDT argument
	  @param oCDT - value to multiplicate
	  @return new CDT object
	*/
	CDT operator*(const CDT & oCDT) const;

	// Operator / ////////////////////////////////////

	/**
	  @brief Operator / for INT_64 argument
	  @param oValue - value to divide
	  @return new CDT object
	*/
	CDT operator/(const INT_64  oValue) const;

	/**
	  @brief Operator / for UINT_64 argument
	  @param oValue - value to divide
	  @return new CDT object
	*/
	CDT operator/(const UINT_64  oValue) const;

	/**
	  @brief Operator / for INT_32 argument
	  @param oValue - value to divide
	  @return new CDT object
	*/
	CDT operator/(const INT_32  oValue) const;

	/**
	  @brief Operator / for UINT_32 argument
	  @param oValue - value to divide
	  @return new CDT object
	*/
	CDT operator/(const UINT_32  oValue) const;

	/**
	  @brief Operator / for W_FLOAT argument
	  @param oValue - value to divide
	  @return new CDT object
	*/
	CDT operator/(const W_FLOAT  oValue) const;

	/**
	  @brief Operator / for CDT argument
	  @param oCDT - value to divide
	  @return new CDT object
	*/
	CDT operator/(const CDT & oCDT) const;

	// ///////////////////////////////////////////////

	// Operator += ///////////////////////////////////

	/**
	  @brief Operator += for INT_64 argument
	  @param oValue - value to add
	  @return Read/write reference to self
	*/
	CDT & operator+=(const INT_64  oValue);

	/**
	  @brief Operator += for UINT_64 argument
	  @param oValue - value to add
	  @return Read/write reference to self
	*/
	CDT & operator+=(const UINT_64  oValue);

	/**
	  @brief Operator += for INT_32 argument
	  @param oValue - value to add
	  @return Read/write reference to self
	*/
	CDT & operator+=(const INT_32  oValue);

	/**
	  @brief Operator += for UINT_32 argument
	  @param oValue - value to divide
	  @return Read/write reference to self
	*/
	CDT & operator+=(const UINT_32  oValue);

	/**
	  @brief Operator += for W_FLOAT argument
	  @param oValue - value to add
	  @return Read/write reference to self
	*/
	CDT & operator+=(const W_FLOAT  oValue);

	/**
	  @brief Operator += for CDT argument
	  @param oCDT - value to add
	  @return Read/write reference to self
	*/
	CDT & operator+=(const CDT & oCDT);

	// Operator -= ///////////////////////////////////

	/**
	  @brief Operator -= for INT_64 argument
	  @param oValue - value to substract
	  @return Read/write reference to self
	*/
	CDT & operator-=(const INT_64  oValue);

	/**
	  @brief Operator -= for UINT_64 argument
	  @param oValue - value to substract
	  @return Read/write reference to self
	*/
	CDT & operator-=(const UINT_64  oValue);

	/**
	  @brief Operator -= for INT_32 argument
	  @param oValue - value to substract
	  @return Read/write reference to self
	*/
	CDT & operator-=(const INT_32  oValue);

	/**
	  @brief Operator -= for UINT_32 argument
	  @param oValue - value to substract
	  @return Read/write reference to self
	*/
	CDT & operator-=(const UINT_32  oValue);

	/**
	  @brief Operator -= for W_FLOAT argument
	  @param oValue - value to substract
	  @return Read/write reference to self
	*/
	CDT & operator-=(const W_FLOAT oValue);

	/**
	  @brief Operator -= for string argument
	  @param oCDT - value to substract
	  @return Read/write reference to self
	*/
	CDT & operator-=(const CDT & oCDT);

	// Operator *= ///////////////////////////////////

	/**
	  @brief Operator *= for INT_64 argument
	  @param oValue - value to multiplication
	  @return Read/write reference to self
	*/
	CDT & operator*=(const INT_64  oValue);

	/**
	  @brief Operator *= for UINT_64 argument
	  @param oValue - value to multiplication
	  @return Read/write reference to self
	*/
	CDT & operator*=(const UINT_64  oValue);

	/**
	  @brief Operator *= for INT_32 argument
	  @param oValue - value to multiplication
	  @return Read/write reference to self
	*/
	CDT & operator*=(const INT_32  oValue);

	/**
	  @brief Operator *= for UINT_32 argument
	  @param oValue - value to multiplication
	  @return Read/write reference to self
	*/
	CDT & operator*=(const UINT_32  oValue);

	/**
	  @brief Operator *= for W_FLOAT argument
	  @param oValue - value to multiplication
	  @return Read/write reference to self
	*/
	CDT & operator*=(const W_FLOAT  oValue);

	/**
	  @brief Operator *= for CDT argument
	  @param oCDT - value to multiplication
	  @return Read/write reference to self
	*/
	CDT & operator*=(const CDT & oCDT);

	// Operator /= ///////////////////////////////////

	/**
	  @brief Operator /= for INT_64 argument
	  @param oValue - value to division
	  @return Read/write reference to self
	*/
	CDT & operator/=(const INT_64  oValue);

	/**
	  @brief Operator /= for UINT_64 argument
	  @param oValue - value to division
	  @return Read/write reference to self
	*/
	CDT & operator/=(const UINT_64  oValue);

	/**
	  @brief Operator /= for INT_32 argument
	  @param oValue - value to division
	  @return Read/write reference to self
	*/
	CDT & operator/=(const INT_32  oValue);

	/**
	  @brief Operator /= for UINT_32 argument
	  @param oValue - value to division
	  @return Read/write reference to self
	*/
	CDT & operator/=(const UINT_32  oValue);

	/**
	  @brief Operator /= for W_FLOAT argument
	  @param oValue - value to division
	  @return Read/write reference to self
	*/
	CDT & operator/=(const W_FLOAT  oValue);

	/**
	  @brief Operator /= for CDT argument
	  @param oCDT - value to division
	  @return Read/write reference to self
	*/
	CDT & operator/=(const CDT & oCDT);

	// ///////////////////////////////////////////////

	// Operator == ///////////////////////////////////

	/**
	  @brief Comparison operator == for CDT argument
	  @param oValue - value to compare
	  @return true if values are equal
	*/
	bool operator==(const INT_64  oValue) const;

	/**
	  @brief Comparison operator == for CDT argument
	  @param oValue - value to compare
	  @return true if values are equal
	*/
	bool operator==(const UINT_64  oValue) const;

	/**
	  @brief Comparison operator == for CDT argument
	  @param oValue - value to compare
	  @return true if values are equal
	*/
	bool operator==(const INT_32  oValue) const;

	/**
	  @brief Comparison operator == for CDT argument
	  @param oValue - value to compare
	  @return true if values are equal
	*/
	bool operator==(const UINT_32  oValue) const;

	/**
	  @brief Comparison operator == for CDT argument
	  @param oValue - value to compare
	  @return true if values are equal
	*/
	bool operator==(const W_FLOAT  oValue) const;

	/**
	  @brief Comparison operator == for old-fashion string argument
	  @param oValue - value to compare
	  @return true if values are equal
	*/
	bool operator==(CCHAR_P  oValue) const;

	/**
	  @brief Comparison operator == for string argument
	  @param oValue - value to compare
	  @return true if values are equal
	*/
	bool operator==(const STLW::string & oValue) const;

	/**
	  @brief Comparison operator == for CDT argument
	  @param oCDT - value to compare
	  @return true if values are equal
	*/
	bool operator==(const CDT & oCDT) const;

	// Operator != ///////////////////////////////////

	/**
	  @brief Comparison operator != for CDT argument
	  @param oValue - value to compare
	  @return true if values are not equal
	*/
	bool operator!=(const INT_64  oValue) const;

	/**
	  @brief Comparison operator != for CDT argument
	  @param oValue - value to compare
	  @return true if values are not equal
	*/
	bool operator!=(const UINT_64  oValue) const;

	/**
	  @brief Comparison operator != for CDT argument
	  @param oValue - value to compare
	  @return true if values are not equal
	*/
	bool operator!=(const INT_32  oValue) const;

	/**
	  @brief Comparison operator != for CDT argument
	  @param oValue - value to compare
	  @return true if values are not equal
	*/
	bool operator!=(const UINT_32  oValue) const;

	/**
	  @brief Comparison operator != for CDT argument
	  @param oValue - value to compare
	  @return true if values are not equal
	*/
	bool operator!=(const W_FLOAT  oValue) const;

	/**
	  @brief Comparison operator != for old-fashion string argument
	  @param oValue - value to compare
	  @return true if values are not equal
	*/
	bool operator!=(CCHAR_P oValue) const;

	/**
	  @brief Comparison operator != for string argument
	  @param oValue - value to compare
	  @return true if values are not equal
	*/
	bool operator!=(const STLW::string & oValue) const;

	/**
	  @brief Comparison operator != for CDT argument
	  @param oCDT - value to compare
	  @return true if values are not equal
	*/
	bool operator!=(const CDT & oCDT) const;
	// ///////////////////////////////////////////////

	// Operator > ///////////////////////////////////

	/**
	  @brief Comparison operator > for CDT argument
	  @param oValue - value to compare
	  @return true if CDT is greater than value
	*/
	bool operator>(const INT_64  oValue) const;

	/**
	  @brief Comparison operator > for CDT argument
	  @param oValue - value to compare
	  @return true if CDT is greater than value
	*/
	bool operator>(const UINT_64  oValue) const;

	/**
	  @brief Comparison operator > for CDT argument
	  @param oValue - value to compare
	  @return true if CDT is greater than value
	*/
	bool operator>(const INT_32  oValue) const;

	/**
	  @brief Comparison operator > for CDT argument
	  @param oValue - value to compare
	  @return true if CDT is greater than value
	*/
	bool operator>(const UINT_32  oValue) const;

	/**
	  @brief Comparison operator > for CDT argument
	  @param oValue - value to compare
	  @return true if CDT is greater than value
	*/
	bool operator>(const W_FLOAT  oValue) const;

	/**
	  @brief Comparison operator > for old-fashion string argument
	  @param oValue - value to compare
	  @return true if CDT is greater than value
	*/
	bool operator>(CCHAR_P oValue) const;

	/**
	  @brief Comparison operator > for string argument
	  @param oValue - value to compare
	  @return true if CDT is greater than value
	*/
	bool operator>(const STLW::string & oValue) const;

	/**
	  @brief Comparison operator > for CDT argument
	  @param oCDT - value to compare
	  @return true if CDT is greater than value
	*/
	bool operator>(const CDT & oCDT) const;

	// Operator < ///////////////////////////////////

	/**
	  @brief Comparison operator < for CDT argument
	  @param oValue - value to compare
	  @return true if CDT is lesser than value
	*/
	bool operator<(const INT_64  oValue) const;

	/**
	  @brief Comparison operator < for CDT argument
	  @param oValue - value to compare
	  @return true if CDT is lesser than value
	*/
	bool operator<(const UINT_64  oValue) const;

	/**
	  @brief Comparison operator < for CDT argument
	  @param oValue - value to compare
	  @return true if CDT is lesser than value
	*/
	bool operator<(const INT_32  oValue) const;

	/**
	  @brief Comparison operator < for CDT argument
	  @param oValue - value to compare
	  @return true if CDT is lesser than value
	*/
	bool operator<(const UINT_32  oValue) const;

	/**
	  @brief Comparison operator < for CDT argument
	  @param oValue - value to compare
	  @return true if CDT is lesser than value
	*/
	bool operator<(const W_FLOAT  oValue) const;

	/**
	  @brief Comparison operator < for old-fashion string argument
	  @param oValue - value to compare
	  @return true if CDT is lesser than value
	*/
	bool operator<(CCHAR_P oValue) const;

	/**
	  @brief Comparison operator < for string argument
	  @param oValue - value to compare
	  @return true if CDT is lesser than value
	*/
	bool operator<(const STLW::string & oValue) const;

	/**
	  @brief Comparison operator < for CDT argument
	  @param oCDT - value to compare
	  @return true if CDT is lesser than value
	*/
	bool operator<(const CDT & oCDT) const;

	// Operator >= ///////////////////////////////////

	/**
	  @brief Comparison operator >= for CDT argument
	  @param oValue - value to compare
	  @return true if CDT is greater or equal than value
	*/
	bool operator>=(const INT_64  oValue) const;

	/**
	  @brief Comparison operator >= for CDT argument
	  @param oValue - value to compare
	  @return true if CDT is greater or equal than value
	*/
	bool operator>=(const UINT_64  oValue) const;

	/**
	  @brief Comparison operator >= for CDT argument
	  @param oValue - value to compare
	  @return true if CDT is greater or equal than value
	*/
	bool operator>=(const INT_32  oValue) const;

	/**
	  @brief Comparison operator >= for CDT argument
	  @param oValue - value to compare
	  @return true if CDT is greater or equal than value
	*/
	bool operator>=(const UINT_32  oValue) const;

	/**
	  @brief Comparison operator >= for CDT argument
	  @param oValue - value to compare
	  @return true if CDT is greater or equal than value
	*/
	bool operator>=(const W_FLOAT  oValue) const;

	/**
	  @brief Comparison operator >= for old-fashion string argument
	  @param oValue - value to compare
	  @return true if CDT is greater or equal than value
	*/
	bool operator>=(CCHAR_P oValue) const;

	/**
	  @brief Comparison operator >= for string argument
	  @param oValue - value to compare
	  @return true if CDT is greater or equal than value
	*/
	bool operator>=(const STLW::string & oValue) const;

	/**
	  @brief Comparison operator >= for CDT argument
	  @param oCDT - value to compare
	  @return true if CDT is greater or equal than value
	*/
	bool operator>=(const CDT & oCDT) const;

	// Operator <= ///////////////////////////////////

	/**
	  @brief Comparison operator <= for CDT argument
	  @param oValue - value to compare
	  @return true if CDT is greater or equal than value
	*/
	bool operator<=(const INT_64  oValue) const;

	/**
	  @brief Comparison operator <= for CDT argument
	  @param oValue - value to compare
	  @return true if CDT is greater or equal than value
	*/
	bool operator<=(const UINT_64  oValue) const;

	/**
	  @brief Comparison operator <= for CDT argument
	  @param oValue - value to compare
	  @return true if CDT is greater or equal than value
	*/
	bool operator<=(const INT_32  oValue) const;

	/**
	  @brief Comparison operator <= for CDT argument
	  @param oValue - value to compare
	  @return true if CDT is greater or equal than value
	*/
	bool operator<=(const UINT_32  oValue) const;

	/**
	  @brief Comparison operator <= for CDT argument
	  @param oValue - value to compare
	  @return true if CDT is greater or equal than value
	*/
	bool operator<=(const W_FLOAT  oValue) const;

	/**
	  @brief Comparison operator <= for old-fashion string argument
	  @param oValue - value to compare
	  @return true if CDT is greater or equal than value
	*/
	bool operator<=(CCHAR_P oValue) const;

	/**
	  @brief Comparison operator <= for string argument
	  @param oValue - value to compare
	  @return true if CDT is greater or equal than value
	*/
	bool operator<=(const STLW::string & oValue) const;

	/**
	  @brief Comparison operator <= for CDT argument
	  @param oCDT - value to compare
	  @return true if CDT is greater or equal than value
	*/
	bool operator<=(const CDT & oCDT) const;

	// Comparator Equal ///////////////////////////////////

	/**
	  @brief Comparator  Equal for CDT argument
	  @param oValue - value to compare
	  @return true if values are equal
	*/
	bool Equal(const INT_64  oValue) const;

	/**
	  @brief Comparator  Equal for CDT argument
	  @param oValue - value to compare
	  @return true if values are equal
	*/
	bool Equal(const UINT_64  oValue) const;

	/**
	  @brief Comparator  Equal for CDT argument
	  @param oValue - value to compare
	  @return true if values are equal
	*/
	bool Equal(const INT_32  oValue) const;

	/**
	  @brief Comparator  Equal for CDT argument
	  @param oValue - value to compare
	  @return true if values are equal
	*/
	bool Equal(const UINT_32  oValue) const;

	/**
	  @brief Comparator  Equal for CDT argument
	  @param oValue - value to compare
	  @return true if values are equal
	*/
	bool Equal(const W_FLOAT  oValue) const;

	/**
	  @brief Comparator  Equal for old-fashion string argument
	  @param oValue - value to compare
	  @return true if values are equal
	*/
	bool Equal(CCHAR_P oValue) const;

	/**
	  @brief Comparator  Equal for string argument
	  @param oValue - value to compare
	  @return true if values are equal
	*/
	bool Equal(const STLW::string & oValue) const;

	/**
	  @brief Comparator  Equal for CDT argument
	  @param oCDT - value to compare
	  @return true if values are equal
	*/
	bool Equal(const CDT & oCDT) const;

	// Comparator NotEqual ///////////////////////////////////

	/**
	  @brief Comparator  NotEqual for CDT argument
	  @param oValue - value to compare
	  @return true if values are not equal
	*/
	bool NotEqual(const INT_64  oValue) const;

	/**
	  @brief Comparator  NotEqual for CDT argument
	  @param oValue - value to compare
	  @return true if values are not equal
	*/
	bool NotEqual(const UINT_64  oValue) const;

	/**
	  @brief Comparator  NotEqual for CDT argument
	  @param oValue - value to compare
	  @return true if values are not equal
	*/
	bool NotEqual(const INT_32  oValue) const;

	/**
	  @brief Comparator  NotEqual for CDT argument
	  @param oValue - value to compare
	  @return true if values are not equal
	*/
	bool NotEqual(const UINT_32  oValue) const;

	/**
	  @brief Comparator  NotEqual for CDT argument
	  @param oValue - value to compare
	  @return true if values are not equal
	*/
	bool NotEqual(const W_FLOAT  oValue) const;

	/**
	  @brief Comparator  NotEqual for old-fashion string argument
	  @param oValue - value to compare
	  @return true if values are not equal
	*/
	bool NotEqual(CCHAR_P oValue) const;

	/**
	  @brief Comparator  NotEqual for string argument
	  @param oValue - value to compare
	  @return true if values are not equal
	*/
	bool NotEqual(const STLW::string & oValue) const;

	/**
	  @brief Comparator  NotEqual for CDT argument
	  @param oCDT - value to compare
	  @return true if values are not equal
	*/
	bool NotEqual(const CDT & oCDT) const;

	// Comparator Greater ///////////////////////////////////

	/**
	  @brief Comparator  Greater for CDT argument
	  @param oValue - value to compare
	  @return true if CDT is greater than value
	*/
	bool Greater(const INT_64  oValue) const;

	/**
	  @brief Comparator  Greater for CDT argument
	  @param oValue - value to compare
	  @return true if CDT is greater than value
	*/
	bool Greater(const UINT_64  oValue) const;

	/**
	  @brief Comparator  Greater for CDT argument
	  @param oValue - value to compare
	  @return true if CDT is greater than value
	*/
	bool Greater(const INT_32  oValue) const;

	/**
	  @brief Comparator  Greater for CDT argument
	  @param oValue - value to compare
	  @return true if CDT is greater than value
	*/
	bool Greater(const UINT_32  oValue) const;

	/**
	  @brief Comparator  Greater for CDT argument
	  @param oValue - value to compare
	  @return true if CDT is greater than value
	*/
	bool Greater(const W_FLOAT  oValue) const;

	/**
	  @brief Comparator  Greater for old-fashion string argument
	  @param oValue - value to compare
	  @return true if CDT is greater than value
	*/
	bool Greater(CCHAR_P  oValue) const;

	/**
	  @brief Comparator  Greater for string argument
	  @param oValue - value to compare
	  @return true if CDT is greater than value
	*/
	bool Greater(const STLW::string & oValue) const;

	/**
	  @brief Comparator  Greater for CDT argument
	  @param oCDT - value to compare
	  @return true if CDT is greater than value
	*/
	bool Greater(const CDT & oCDT) const;

	// Comparator Less ///////////////////////////////////

	/**
	  @brief Comparator  Less for CDT argument
	  @param oValue - value to compare
	  @return true if CDT is lesser than value
	*/
	bool Less(const INT_64  oValue) const;

	/**
	  @brief Comparator  Less for CDT argument
	  @param oValue - value to compare
	  @return true if CDT is lesser than value
	*/
	bool Less(const UINT_64  oValue) const;

	/**
	  @brief Comparator  Less for CDT argument
	  @param oValue - value to compare
	  @return true if CDT is lesser than value
	*/
	bool Less(const INT_32  oValue) const;

	/**
	  @brief Comparator  Less for CDT argument
	  @param oValue - value to compare
	  @return true if CDT is lesser than value
	*/
	bool Less(const UINT_32  oValue) const;

	/**
	  @brief Comparator  Less for CDT argument
	  @param oValue - value to compare
	  @return true if CDT is lesser than value
	*/
	bool Less(const W_FLOAT  oValue) const;

	/**
	  @brief Comparator  Less for string argument
	  @param oValue - value to compare
	  @return true if CDT is lesser than value
	*/
	bool Less(CCHAR_P  oValue) const;

	/**
	  @brief Comparator  Less for string argument
	  @param oValue - value to compare
	  @return true if CDT is lesser than value
	*/
	bool Less(const STLW::string & oValue) const;

	/**
	  @brief Comparator  Less for CDT argument
	  @param oCDT - value to compare
	  @return true if CDT is lesser than value
	*/
	bool Less(const CDT & oCDT) const;

	// Comparator GreaterOrEqual ///////////////////////////////////

	/**
	  @brief Comparator  GreaterOrEqual for CDT argument
	  @param oValue - value to compare
	  @return true if CDT is greater or equal than value
	*/
	bool GreaterOrEqual(const INT_64  oValue) const;

	/**
	  @brief Comparator  GreaterOrEqual for CDT argument
	  @param oValue - value to compare
	  @return true if CDT is greater or equal than value
	*/
	bool GreaterOrEqual(const UINT_64  oValue) const;

	/**
	  @brief Comparator  GreaterOrEqual for CDT argument
	  @param oValue - value to compare
	  @return true if CDT is greater or equal than value
	*/
	bool GreaterOrEqual(const INT_32  oValue) const;

	/**
	  @brief Comparator  GreaterOrEqual for CDT argument
	  @param oValue - value to compare
	  @return true if CDT is greater or equal than value
	*/
	bool GreaterOrEqual(const UINT_32  oValue) const;

	/**
	  @brief Comparator  GreaterOrEqual for CDT argument
	  @param oValue - value to compare
	  @return true if CDT is greater or equal than value
	*/
	bool GreaterOrEqual(const W_FLOAT  oValue) const;

	/**
	  @brief Comparator  GreaterOrEqual for old-fashion string argument
	  @param oValue - value to compare
	  @return true if CDT is greater or equal than value
	*/
	bool GreaterOrEqual(CCHAR_P oValue) const;

	/**
	  @brief Comparator  GreaterOrEqual for string argument
	  @param oValue - value to compare
	  @return true if CDT is greater or equal than value
	*/
	bool GreaterOrEqual(const STLW::string & oValue) const;

	/**
	  @brief Comparator  GreaterOrEqual for CDT argument
	  @param oCDT - value to compare
	  @return true if CDT is greater or equal than value
	*/
	bool GreaterOrEqual(const CDT & oCDT) const;

	// Comparator LessOrEqual ///////////////////////////////////

	/**
	  @brief Comparator  LessOrEqual for CDT argument
	  @param oValue - value to compare
	  @return true if CDT is greater or equal than value
	*/
	bool LessOrEqual(const INT_64  oValue) const;

	/**
	  @brief Comparator  LessOrEqual for CDT argument
	  @param oValue - value to compare
	  @return true if CDT is greater or equal than value
	*/
	bool LessOrEqual(const UINT_64  oValue) const;

	/**
	  @brief Comparator  LessOrEqual for CDT argument
	  @param oValue - value to compare
	  @return true if CDT is greater or equal than value
	*/
	bool LessOrEqual(const INT_32  oValue) const;

	/**
	  @brief Comparator  LessOrEqual for CDT argument
	  @param oValue - value to compare
	  @return true if CDT is greater or equal than value
	*/
	bool LessOrEqual(const UINT_32  oValue) const;

	/**
	  @brief Comparator  LessOrEqual for CDT argument
	  @param oValue - value to compare
	  @return true if CDT is greater or equal than value
	*/
	bool LessOrEqual(const W_FLOAT  oValue) const;

	/**
	  @brief Comparator  LessOrEqual for old-fashion string argument
	  @param oValue - value to compare
	  @return true if CDT is greater or equal than value
	*/
	bool LessOrEqual(CCHAR_P oValue) const;

	/**
	  @brief Comparator  LessOrEqual for string argument
	  @param oValue - value to compare
	  @return true if CDT is greater or equal than value
	*/
	bool LessOrEqual(const STLW::string & oValue) const;

	/**
	  @brief Comparator  LessOrEqual for CDT argument
	  @param oCDT - value to compare
	  @return true if CDT is greater or equal than value
	*/
	bool LessOrEqual(const CDT & oCDT) const;

	// ///////////////////////////////////////////////

	/**
	  @brief Pre-increment operator ++
	  @return Read/write reference to self
	*/
	CDT & operator++();

	/**
	  @brief Post-increment operator ++
	  @return Read/write reference to self
	*/
	CDT   operator++(int);

	/**
	  @brief Pre-decrement operator --
	  @return Read/write reference to self
	*/
	CDT & operator--();

	/**
	  @brief Post-decrement operator --
	  @return Read/write reference to self
	*/
	CDT   operator--(int);

	/**
	  @brief Append a string to CDT
	  @see Append(const STLW::string & oValue)
	  @param oValue - string to append
	  @return read/write referense to self
	*/
	CDT & Concat(const STLW::string & oValue);

	/**
	  @brief Append a old-fashion string to CDT
	  @see Append(CCHAR_P * szData, const INT_32 & iDataLength)
	  @param szData - string to append
	  @param iDataLength - string length
	  @return read/write referense to self
	*/
	CDT & Concat(CCHAR_P szData, const INT_32 iDataLength = -1);

	/**
	  @brief Append a string to CDT, alias for Concat
	  @see Concat(const STLW::string & oValue)
	  @param oValue - string to append
	  @return read/write referense to self
	*/
	CDT & Append(const STLW::string & oValue);

	/**
	  @brief Append a old-fashion string to CDT
	  @see Concat(CCHAR_P * szData, const INT_32 & iDataLength)
	  @param szData - string to append
	  @param iDataLength - string length
	  @return read/write referense to self
	*/
	CDT & Append(CCHAR_P szData, const INT_32 iDataLength = -1);

	/**
	  @brief Append a INT_64 to CDT
	  @param szData - string to append
	  @param iDataLength - string length
	  @return read/write referense to self
	*/
	CDT & Append(const INT_64  oValue);

	/**
	  @brief Append a INT_64 to CDT
	  @param szData - string to append
	  @param iDataLength - string length
	  @return read/write referense to self
	*/
	CDT & Append(const UINT_64  oValue);

	/**
	  @brief Append a INT_32 to CDT
	  @param szData - string to append
	  @param iDataLength - string length
	  @return read/write referense to self
	*/
	CDT & Append(const INT_32  oValue);

	/**
	  @brief Append a UINT_32 to CDT
	  @param szData - string to append
	  @param iDataLength - string length
	  @return read/write referense to self
	*/
	CDT & Append(const UINT_32  oValue);

	/**
	  @brief Append a W_FLOAT to CDT
	  @param szData - string to append
	  @param iDataLength - string length
	  @return read/write referense to self
	*/
	CDT & Append(const W_FLOAT  oValue);

	/**
	  @brief Append a string to CDT, alias for Concat
	  @see Concat(const STLW::string & oValue)
	  @param oValue - string to append
	  @return read/write referense to self
	*/
	CDT & Append(const CDT & oCDT);

	/**
	  @brief Prepend a string to CDT, alias for Concat
	  @see Concat(const STLW::string & oValue)
	  @param oValue - string to append
	  @return read/write referense to self
	*/
	CDT & Prepend(const STLW::string & oValue);

	/**
	  @brief Prepend a old-fashion string to CDT
	  @param szData - string to append
	  @param iDataLength - string length
	  @return read/write referense to self
	*/
	CDT & Prepend(CCHAR_P szData, const INT_32 iDataLength = -1);

	/**
	  @brief Prepend a INT_64 to CDT
	  @param szData - string to append
	  @param iDataLength - string length
	  @return read/write referense to self
	*/
	CDT & Prepend(const INT_64  oValue);

	/**
	  @brief Prepend a INT_64 to CDT
	  @param szData - string to append
	  @param iDataLength - string length
	  @return read/write referense to self
	*/
	CDT & Prepend(const UINT_64  oValue);

	/**
	  @brief Prepend a INT_32 to CDT
	  @param szData - string to append
	  @param iDataLength - string length
	  @return read/write referense to self
	*/
	CDT & Prepend(const INT_32  oValue);

	/**
	  @brief Prepend a UINT_32 to CDT
	  @param szData - string to append
	  @param iDataLength - string length
	  @return read/write referense to self
	*/
	CDT & Prepend(const UINT_32  oValue);

	/**
	  @brief Prepend a W_FLOAT to CDT
	  @param szData - string to append
	  @param iDataLength - string length
	  @return read/write referense to self
	*/
	CDT & Prepend(const W_FLOAT  oValue);

	/**
	  @brief Prepend a string to CDT
	  @see Concat(const STLW::string & oValue)
	  @param oValue - string to append
	  @return read/write referense to self
	*/
	CDT & Prepend(const CDT & oCDT);

	/**
	  @brief Get value as W_FLOAT
	*/
	W_FLOAT GetFloat() const;

	/**
	  @brief Get value as INT_64
	*/
	INT_64 GetInt() const;

	/**
	  @brief Get value as UINT_64
	*/
	UINT_64 GetUInt() const;

	/**
	  @brief Get value as STLW::string
	  @param szFormat - output format
	  @return String object representation
	*/
	STLW::string GetString(CCHAR_P szFormat = "") const;

	/**
	  @brief Cast value to W_FLOAT
        */
	W_FLOAT ToFloat();

	/**
	  @brief Cast value to INT_64
	*/
	INT_64 ToInt();

	/**
	  @brief Cast value to STLW::string
	  @param szFormat - output format
	  @return String object representation
	*/
	STLW::string ToString(CCHAR_P szFormat = "");

	/**
	  @brief Get generic pointer
	  @return generic pointer
	*/
	const void * GetPointer() const;

	/**
	  @brief Get object
	  @return pointer to specified type
	*/
	template<typename T> const T * GetObject() const { return (T*)GetPointer(); }

	/**
	  @brief Get generic pointer
	  @return generic pointer
	*/
	void * GetPointer();

	/**
	  @brief Get object
	  @return pointer to specified type
	*/
	template<typename T> T * GetObject() { return (T*)GetPointer(); }

	/**
	  @brief Recursively dump CDT into string
	  @param iLevel - left margin
	  @return printable string
	*/
	STLW::string Dump(UINT_32 iLevel = 0) const;

	/**
	  @brief Alias for RecursiveDump, deprecated
	  @param iLevel - left margin
	  @return printable string
	*/
	STLW::string RecursiveDump(UINT_32 iLevel = 0) const;

	/**
	  @brief Get value type of object
	*/
	eValType GetType() const;

	/**
	  @brief Get printable value type of object
	  @return printable name of data type
	*/
	CCHAR_P PrintableType() const;

	/**
	  @brief Get printable value type
	  @return printable name of data type
	*/
	static CCHAR_P PrintableType(eValType eType);

	/**
	  @brief Get array of hash size
	  @return Size of array
	*/
	UINT_32 Size() const;

	/**
	  @brief Swap values
	  @param oCDT - value to swap
	  @return Reference to self
	*/
	CDT & Swap(CDT & oCDT);

	/**
	  @brief Quick sort ARRAY
	  @param oSortingComparator - variables comparator
	*/
	void SortArray(const SortingComparator & oSortingComparator);

	/**
	  @class SortingComparator CDT.hpp <CDT.hpp>
	  @brief Sorting comparator
	*/
	class SortingComparator
	{
	public:
		/**
		  @brief Sorting direction
		*/
		enum eSortingDirection { ASC, DESC };

		/**
		  @brief Compare two values.
		  @param oX - first value to compare
		  @param oY - seond value to compare
		  @return 0 - if equel, -1 - if oX < oY, 1 - if ox > oY
		*/
		virtual bool operator()(const CDT & oX, const CDT & oY) const = 0;

		/**
		  @brief A dertructor
		*/
		virtual ~SortingComparator() throw();
	};

	// FWD
	class ConstIterator;

	/**
	  @class Iterator CDT.hpp <CDT.hpp>
	  @brief CDT[HASH] forward iterator
	*/
	class Iterator
	{
	private:
		friend class CDT;
		friend class ConstIterator;

		/** Hash iterator */
		CDT::Map::iterator itMap;

		/**
		  @brief Constructor
		  @param itIMap - map iterator
		*/
		Iterator(CDT::Map::iterator itIMap);
	public:
		/**
		  @brief Copy constructor
		  @param oRhs - object to copy
		*/
		Iterator(const Iterator & oRhs);

		/**
		  @brief Operator =
		  @param oRhs - object to copy
		*/
		Iterator & operator=(const Iterator & oRhs);

		/**
		  @brief Pre-increment operator ++
		*/
		Iterator & operator++();

		/**
		  @brief Post-increment operator ++
		*/
		Iterator operator++(int);

		/**
		  @brief Access operator
		  @return Pair of key => value
		*/
		STLW::pair<const STLW::string, CDT> * operator->();

		/**
		  @brief Comparison operator
		  @param oRhs - object to compare
		  @return true if objects are equal
		*/
		bool operator ==(const Iterator & oRhs);

		/**
		  @brief Comparison operator
		  @param oRhs - object to compare
		  @return true if objects are NOT equal
		*/
		bool operator !=(const Iterator & oRhs);
	};

	/**
	  @brief Get iterator pointed to start of hash
	*/
	Iterator Begin();

	/**
	  @brief Get iterator pointed to end of hash
	*/
	Iterator End();

	/**
	  @brief Find element in hash
	  @param sKey - element name
	  @return Iterator pointed to element or to end of hash if nothing found
	*/
	Iterator Find(const STLW::string & sKey);

	/**
	  @class ConstIterator CDT.hpp <CDT.hpp>
	  @brief CDT[HASH] forward constant iterator
	*/
	class ConstIterator
	{
	private:
		friend class CDT;

		/** Hash iterator */
		CDT::Map::const_iterator itMap;

	public:
		/**
		  @brief Copy constructor
		  @param oRhs - object to copy
		*/
		ConstIterator(const ConstIterator & oRhs);

		/**
		  @brief Type cast constructor
		  @param oRhs - object to copy
		*/
		ConstIterator(const Iterator & oRhs);

		/**
		  @brief Operator =
		  @param oRhs - object to copy
		*/
		ConstIterator & operator=(const ConstIterator & oRhs);

		/**
		  @brief Operator =
		  @param oRhs - object to copy
		*/
		ConstIterator & operator=(const Iterator & oRhs);

		/**
		  @brief Pre-increment operator ++
		*/
		ConstIterator & operator++();

		/**
		  @brief Post-increment operator ++
		*/
		ConstIterator operator++(int);

		/**
		  @brief Access operator
		  @return Pair of key => value
		*/
		const STLW::pair<const STLW::string, CDT> * operator->() const;

		/**
		  @brief Comparison operator
		  @param oRhs - object to compare
		  @return true if objects are equal
		*/
		bool operator ==(const ConstIterator & oRhs) const;

		/**
		  @brief Comparison operator
		  @param oRhs - object to compare
		  @return true if objects are NOT equal
		*/
		bool operator !=(const ConstIterator & oRhs) const;
	};

	/**
	  @brief Get constant iterator pointed to start of hash
	*/
	ConstIterator Begin() const;

	/**
	  @brief Get constant iterator pointed to end of hash
	*/
	ConstIterator End() const;

	/**
	  @brief Find element in hash
	  @param sKey - element name
	  @return Iterator pointed to element or to end of hash if nothing found
	*/
	ConstIterator Find(const STLW::string & sKey) const;

	/**
	  @brief Try to cast value to integer or to IEEE floating point value
	  @return Cast result type
	*/
	eValType CastToNumber(INT_64   & iData,
	                      W_FLOAT  & dData) const;

	/**
	  @brief A destructor
	*/
	~CDT() throw();

private:
	// Friends
	friend CDT operator-(const UINT_64 & oValue, const CDT & oCDT);
	friend CDT operator-(const INT_64  & oValue, const CDT & oCDT);
	friend CDT operator-(const W_FLOAT & oValue, const CDT & oCDT);

	friend CDT operator/(const UINT_64 & oValue, const CDT & oCDT);
	friend CDT operator/(const INT_64  & oValue, const CDT & oCDT);
	friend CDT operator/(const W_FLOAT & oValue, const CDT & oCDT);

	// FWD
	struct _CDT;

	/** Plain Old datatypes */
	union
	{
		/** Signed interger                */
		INT_64                 i_data;
		/** Floating point value           */
		W_FLOAT                d_data;
		/** Pointer to shareable container */
		_CDT                 * p_data;
		/** Generic pointer                */
		void                 * pp_data;
	} u;

	/** Value type */
	mutable eValType               eValueType;

	/**
	  @brief Destroy object if need
	*/
	void Destroy() throw();

	/**
	  @brief Unshare shareable container
	*/
	void Unshare();

	/**
	  @brief Dump CDT into string
	  @param iLevel  - level of recursion
	  @param oData   - data to dump
	  @param sResult - string to put result in
	*/
	static void DumpData(UINT_32 iLevel, UINT_32 iOffset, const CDT & oData, STLW::string & sResult);

	/**
	  @brief Check complex data type and change value type, if need
	*/
	void CheckComplexDataType() const;

};

// ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Realization
//

// ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
  @brief Operator + for UINT_64 and CDT arguments
  @param oValue - 1-st argument
  @param oCDT - 2-nd argument
  @return result of addition with same type of oCDT
*/
CDT operator+(const UINT_64 & oValue, const CDT & oCDT);

/**
  @brief Operator + for INT_64 and CDT arguments
  @param oValue - 1-st argument
  @param oCDT - 2-nd argument
  @return result of addition with same type of oCDT
*/
CDT operator+(const INT_64 & oValue, const CDT & oCDT);

/**
  @brief Operator + for UINT_32 and CDT arguments
  @param oValue - 1-st argument
  @param oCDT - 2-nd argument
  @return result of addition with same type of oCDT
*/
CDT operator+(const UINT_32 & oValue, const CDT & oCDT);

/**
  @brief Operator + for INT_32 and CDT arguments
  @param oValue - 1-st argument
  @param oCDT - 2-nd argument
  @return result of addition with same type of oCDT
*/
CDT operator+(const INT_32 & oValue, const CDT & oCDT);

/**
  @brief Operator + for W_FLOAT and CDT arguments
  @param oValue - 1-st argument
  @param oCDT - 2-nd argument
  @return result of addition with same type of oCDT
*/
CDT operator+(const W_FLOAT & oValue, const CDT & oCDT);

// ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
  @brief Operator - for UINT_64 and CDT arguments
  @param oValue - 1-st argument
  @param oCDT - 2-nd argument
  @return result of substraction with same type of oCDT
*/
CDT operator-(const UINT_64 & oValue, const CDT & oCDT);

/**
  @brief Operator - for INT_64 and CDT arguments
  @param oValue - 1-st argument
  @param oCDT - 2-nd argument
  @return result of substraction with same type of oCDT
*/
CDT operator-(const INT_64 & oValue, const CDT & oCDT);

/**
  @brief Operator - for UINT_32 and CDT arguments
  @param oValue - 1-st argument
  @param oCDT - 2-nd argument
  @return result of substraction with same type of oCDT
*/
CDT operator-(const UINT_32 & oValue, const CDT & oCDT);

/**
  @brief Operator - for INT_32 and CDT arguments
  @param oValue - 1-st argument
  @param oCDT - 2-nd argument
  @return result of substraction with same type of oCDT
*/
CDT operator-(const INT_32 & oValue, const CDT & oCDT);

/**
  @brief Operator - for W_FLOAT and CDT arguments
  @param oValue - 1-st argument
  @param oCDT - 2-nd argument
  @return result of substraction with same type of oCDT
*/
CDT operator-(const W_FLOAT & oValue, const CDT & oCDT);

// ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
  @brief Operator * for UINT_64 and CDT arguments
  @param oValue - 1-st argument
  @param oCDT - 2-nd argument
  @return result of multiplication with same type of oCDT
*/
CDT operator*(const UINT_64 & oValue, const CDT & oCDT);

/**
  @brief Operator * for INT_64 and CDT arguments
  @param oValue - 1-st argument
  @param oCDT - 2-nd argument
  @return result of multiplication with same type of oCDT
*/
CDT operator*(const INT_64 & oValue, const CDT & oCDT);

/**
  @brief Operator * for UINT_32 and CDT arguments
  @param oValue - 1-st argument
  @param oCDT - 2-nd argument
  @return result of multiplication with same type of oCDT
*/
CDT operator*(const UINT_32 & oValue, const CDT & oCDT);

/**
  @brief Operator * for INT_32 and CDT arguments
  @param oValue - 1-st argument
  @param oCDT - 2-nd argument
  @return result of multiplication with same type of oCDT
*/
CDT operator*(const INT_32 & oValue, const CDT & oCDT);

/**
  @brief Operator * for W_FLOAT and CDT arguments
  @param oValue - 1-st argument
  @param oCDT - 2-nd argument
  @return result of multiplication with same type of oCDT
*/
CDT operator*(const W_FLOAT & oValue, const CDT & oCDT);

// ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
  @brief Operator / for UINT_64 and CDT arguments
  @param oValue - 1-st argument
  @param oCDT - 2-nd argument
  @return result of division with same type of oCDT
*/
CDT operator/(const UINT_64 & oValue, const CDT & oCDT);

/**
  @brief Operator / for INT_64 and CDT arguments
  @param oValue - 1-st argument
  @param oCDT - 2-nd argument
  @return result of division with same type of oCDT
*/
CDT operator/(const INT_64 & oValue, const CDT & oCDT);

/**
  @brief Operator / for UINT_32 and CDT arguments
  @param oValue - 1-st argument
  @param oCDT - 2-nd argument
  @return result of division with same type of oCDT
*/
CDT operator/(const UINT_32 & oValue, const CDT & oCDT);

/**
  @brief Operator / for INT_32 and CDT arguments
  @param oValue - 1-st argument
  @param oCDT - 2-nd argument
  @return result of division with same type of oCDT
*/
CDT operator/(const INT_32 & oValue, const CDT & oCDT);

/**
  @brief Operator / for W_FLOAT and CDT arguments
  @param oValue - 1-st argument
  @param oCDT - 2-nd argument
  @return result of division with same type of oCDT
*/
CDT operator/(const W_FLOAT & oValue, const CDT & oCDT);

} // namespace CTPP
#endif // _CDT_HPP__
// End.
