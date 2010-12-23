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
 *      CTPP2SymbolTable.hpp
 *
 * $CTPP$
 */
#ifndef _CTPP2_SYMBOL_TABLE_HPP__
#define _CTPP2_SYMBOL_TABLE_HPP__ 1

#include "CTPP2Types.h"

#include "STLMap.hpp"
#include "STLString.hpp"
#include "STLVector.hpp"

/**
  @file CTPP2SymbolTable.hpp
  @brief CTPP2 generic symbol table
*/

namespace CTPP // C++ Template Engine
{

/**
  @class SymbolTable CTPP2SymbolTable.hpp <CTPP2SymbolTable.hpp>
  @brief CTPP2 Symbol table with logical scopes
*/
template<typename T>class SymbolTable
{
public:
	/**
	  @struct SymbolRecord CTPP2SymbolTable.hpp <CTPP2SymbolTable.hpp>
	  @brief Symbol record
	*/
	template <typename TT>struct SymbolRecord
	{
		/** Symbol Id     */
		INT_32      symbol_id;
		/** Symbol data   */
		TT          symbol_data;

		SymbolRecord(const INT_32    iSymbolId,
		             const TT      & oT): symbol_id(iSymbolId),
		                                  symbol_data(oT) { ;; }
	};

	/**
	  @brief Constructor
	*/
	SymbolTable();

	/**
	  @brief Get symbol from symbol table
	  @param sSymbolName - symbol name
	  @return SymbolRecord object or NULL if nothing found
	*/
	const SymbolRecord<T> * GetSymbol(const STLW::string & sSymbolName) const
	{
		typename STLW::map<STLW::string, STLW::vector<SymbolRecord<T> > >::const_iterator itmSymbolTable = mSymbolTable.find(sSymbolName);
		if (itmSymbolTable == mSymbolTable.end()) { return NULL; }

		INT_32 iPos = itmSymbolTable -> second.size() - 1;
		if (iPos < 0) { return NULL; }

	return &(itmSymbolTable -> second[iPos]);
	}

	/**
	  @brief Get symbol from symbol table
	  @param szSymbolName - symbol name
	  @param iSymbolNameLength - symbol name length
	  @return SymbolRecord object or NULL if nothing found
	*/
	const SymbolRecord<T> * GetSymbol(CCHAR_P szSymbolName, const UINT_32 iSymbolNameLength) const
	{
		if (szSymbolName == NULL || iSymbolNameLength == 0) { return NULL; }

		return GetSymbol(STLW::string(szSymbolName, iSymbolNameLength));
	}

	/**
	  @brief Add symbol into symbol table
	  @param sSymbolName - symbol name
	  @param oSymbolData - symbol data
	  @return Symbol Id
	*/
	INT_32 AddSymbol(const STLW::string & sSymbolName, const T & oSymbolData);

	/**
	  @brief Add symbol into symbol table
	  @param sSymbolName - symbol name
	  @param oSymbolData - symbol data
	  @return Symbol Id
	*/
	INT_32 AddSymbol(CCHAR_P szSymbolName, const UINT_32 iSymbolNameLength, const T & oSymbolData)
	{
		return AddSymbol(STLW::string(szSymbolName, iSymbolNameLength), oSymbolData);
	}

	/**
	  @brief Mark symbol visibility scope
	  @return Symbol Id
	*/
	void MarkScope();

	/**
	  @brief Unmark (reset to previous) symbol visibility scope
	  @return Symbol Id
	*/
	void UnmarkScope();

	/**
	  @brief Destructor
	*/
	~SymbolTable() throw();

private:
	/** Last variable id                   */
	INT_32     iLastVarId;
	/** Symbol table                       */
	STLW::map<STLW::string, STLW::vector<SymbolRecord<T> > > mSymbolTable;

	/** Last variable id                   */
	INT_32     iCurrentScope;
	/** Last variable id in current scope  */
	struct ScopeVars
	{
		/** List of variables in current scope */
		STLW::vector<STLW::string>   varlist;
		/** Last scope variable Id             */
		INT_32                     last_var_id;

		/**
		  @brief Constructor
		  @param
		*/
		ScopeVars(const STLW::vector<STLW::string>  & vVarList,
		          const UINT_32                   & iLastVarId): varlist(vVarList),
		                                                         last_var_id(iLastVarId) { ;; }
	};
	STLW::vector<ScopeVars>   vScopeLastVarList;
};

//
// Constructor
//
template<typename T> SymbolTable<T>::SymbolTable():iLastVarId(0), iCurrentScope(0)
{
	vScopeLastVarList.push_back(ScopeVars(STLW::vector<STLW::string>(), 0));
}

//
// Add symbol into symbol table
//
template<typename T> INT_32 SymbolTable<T>::AddSymbol(const STLW::string & sSymbolName, const T & oSymbolData)
{
	typename STLW::map<STLW::string, STLW::vector<SymbolRecord<T> > >::const_iterator itmSymbolTable = mSymbolTable.find(sSymbolName);
	if (itmSymbolTable != mSymbolTable.end())
	{

		INT_32 iSymTablePos = itmSymbolTable -> second.size() - 1;
		INT_32 iScopeTablePos = vScopeLastVarList.size() - 1;

		if (vScopeLastVarList[iScopeTablePos].last_var_id <= itmSymbolTable -> second[iSymTablePos].symbol_id) { return -1; }
	}

	vScopeLastVarList[iCurrentScope].varlist.push_back(sSymbolName);

	mSymbolTable[sSymbolName].push_back(SymbolRecord<T>(iLastVarId, oSymbolData));

return iLastVarId++;
}

//
// Mark symbol visibility scope
//
template<typename T> void SymbolTable<T>::MarkScope()
{
	++iCurrentScope;

	vScopeLastVarList.push_back(ScopeVars(STLW::vector<STLW::string>(), iLastVarId));
}

//
// Unmark (reset to previous) symbol visibility scope
//
template<typename T> void SymbolTable<T>::UnmarkScope()
{
	typename STLW::vector<ScopeVars>::iterator itvCurrentScope = vScopeLastVarList.end();

	// Nothing to do?
	if (itvCurrentScope == vScopeLastVarList.begin()) { return ; }

	--itvCurrentScope;

	// Get list of symbols in current scope
	STLW::vector<STLW::string>::const_iterator itvVarList = itvCurrentScope -> varlist.begin();

	// Set new last variable Id
	iLastVarId = itvCurrentScope -> last_var_id;

	// Remove all variables that belong to current scope
	while (itvVarList != itvCurrentScope -> varlist.end())
	{
		typename STLW::map<STLW::string, STLW::vector<SymbolRecord<T> > >::iterator itmSymbolTable = mSymbolTable.find(*itvVarList);
		// This should never happened, but let it be
		if (itmSymbolTable != mSymbolTable.end())
		{
			typename STLW::vector<SymbolRecord<T> > & oTMP = itmSymbolTable -> second;

			typename STLW::vector<SymbolRecord<T> >::iterator itVarId = oTMP.end();

			if (itVarId != oTMP.begin())
			{
				--itVarId;
				oTMP.erase(itVarId);
			}

			// Remove empty symbols
			if (oTMP.size() == 0) { mSymbolTable.erase(itmSymbolTable); }
		}

		++itvVarList;
	}

	vScopeLastVarList.erase(itvCurrentScope);
	--iCurrentScope;
}

//
// Destructor
//
template<typename T> SymbolTable<T>::~SymbolTable() throw() { ;; }

} // namespace CTPP
#endif // _CTPP2_SYMBOL_TABLE_HPP__
// End.
