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
 *      CTPP2Compiler.cpp
 *
 * $CTPP$
 */
#include "CTPP2Compiler.hpp"

#include "CTPP2Syntax.h"
#include "CTPP2HashTable.hpp"
#include "CTPP2StaticData.hpp"
#include "CTPP2StaticText.hpp"
#include "CTPP2VMOpcodes.h"

// #define _USE_COMPILER_REPORTER 1 // Use it only for hard debugging

namespace CTPP // C++ Template Engine
{

#ifdef _USE_COMPILER_REPORTER
/**
  @class Reporter
  @brief Print template parsing syntax tree with indents
*/
class CompilerReporter
{
public:
	/**
	  @brief Constructor
	  @param sIData - method name
	  @param szCode - code to print
	*/
	CompilerReporter(const STLW::string & sIData);

	/**
	  @brief Destructor
	*/
	~CompilerReporter() throw();
private:
	// Does not exist
	CompilerReporter(const CompilerReporter & oRhs);
	CompilerReporter & operator=(const CompilerReporter & oRhs);

	/** Method name   */
	STLW::string  sData;
	/** Indent lebvel */
	static int    iLevel;
};

//
// Constructor
//
CompilerReporter::CompilerReporter(const STLW::string & sIData): sData(sIData)
{
	for (INT_32 iI = 0; iI < iLevel; ++iI) { fprintf(stderr, "    "); }
	++iLevel;
	fprintf(stderr, ">> %s\n", sData.c_str());
}

//
// Destructor
//
CompilerReporter::~CompilerReporter() throw()
{
	--iLevel;
//	for (INT_32 iI = 0; iI < iLevel; ++iI) { fprintf(stderr, "    "); }
//	fprintf(stderr, "<< %s\n", sData.c_str());
}

// Indent level
int CompilerReporter::iLevel = 0;

// Fast access macros
#define COMPILER_REPORTER(x) CompilerReporter oCompilerReporter(x);

#else // No reporter

// Fast access macros
#define COMPILER_REPORTER(x) { ;; }

#endif // Reporter class

/**
  @class DirtyRegistersFlag
  @brief Flag for register PUSH/POP optimizationr
*/
class DirtyRegistersFlag
{
public:
	/**
	  @brief Constructor
	  @param bIInForeach - flag
	*/
	DirtyRegistersFlag(bool & bIFlag);

	/**
	  @brief Destructor
	*/
	~DirtyRegistersFlag() throw();
private:
	// Does not exist
	DirtyRegistersFlag(const DirtyRegistersFlag & oRhs);
	DirtyRegistersFlag & operator=(const DirtyRegistersFlag & oRhs);

	/** Reference to foreach flag */
	bool  & bFlag;
	/** Old foreach flag state    */
	bool    bOldFlad;
};


//
// Constructor
//
DirtyRegistersFlag::DirtyRegistersFlag(bool & bIFlag): bFlag(bIFlag),
                                                       bOldFlad(bFlag)
{
	//fprintf(stderr, ">>DirtyRegistersFlag: %c -> t\n", (bOldFlad ? 't':'f'));
	bFlag = true;
}

//
// Destructor
//
DirtyRegistersFlag::~DirtyRegistersFlag() throw()
{
	//fprintf(stderr, "<<ForeachFlagLocker: t -> %c\n", (bFlag ? 't':'f'));
	bFlag = bOldFlad;
}

//
// Constructor
//
CTPP2Compiler::CTPP2Compiler(VMOpcodeCollector  & oIVMOpcodeCollector,
                             StaticText         & oISyscalls,
                             StaticData         & oIStaticData,
                             StaticText         & oIStaticText,
                             HashTable          & oIHashTable): iStackDepth(0),
                                                                iScopeNumber(0),
                                                                bRegsAreDirty(true),
                                                                oVMOpcodeCollector(oIVMOpcodeCollector),
                                                                oSyscalls(oISyscalls),
                                                                oStaticData(oIStaticData),
                                                                oStaticText(oIStaticText),
                                                                oHashTable(oIHashTable)
{
	mSyscalls["__ctpp2_emitter"]      = oSyscalls.StoreData("__ctpp2_emitter",      15);

	iZeroId = oStaticData.StoreInt(0);
	iOneId  = oStaticData.StoreInt(1);

	oVMOpcodeCollector.Insert(CreateInstruction(SYSCALL, SYSCALL_PARAMS(mSyscalls["__ctpp2_emitter"], 0), 0 ));
	oVMOpcodeCollector.Insert(CreateInstruction(MOV | ARG_SRC_STACK | ARG_DST_HR, 0, 0));
	oVMOpcodeCollector.Insert(CreateInstruction(POP, 1, 0));
}

//
// Store template source name
//
INT_32 CTPP2Compiler::StoreSourceName(CCHAR_P        szName,
                                      const UINT_32  iNameLength)
{
	return oStaticText.StoreData(szName, iNameLength);
}

//
// Send variable to standard output collector
//
INT_32 CTPP2Compiler::OutputVariable(const VMDebugInfo & oDebugInfo)
{
	COMPILER_REPORTER("OutputVariable");

	--iStackDepth;
return oVMOpcodeCollector.Insert(CreateInstruction(OUTPUT | ARG_SRC_STACK, 0, oDebugInfo.GetInfo()));
}

//
// Send text data to standard output collector
//
INT_32 CTPP2Compiler::OutputStaticData(CCHAR_P              vBuffer,
                                       const UINT_32        iBufferLength,
                                       const VMDebugInfo  & oDebugInfo)
{
	if (iBufferLength == 0) { return 0; }

	COMPILER_REPORTER("OutputStaticData/string");

	INT_32 iId = oStaticText.StoreData(vBuffer, iBufferLength);

return oVMOpcodeCollector.Insert(CreateInstruction(OUTPUT | ARG_SRC_STR, iId, oDebugInfo.GetInfo()));
}

//
// Send integer value to standard output collector
//
INT_32 CTPP2Compiler::OutputStaticData(const INT_64       & iData,
                                       const VMDebugInfo  & oDebugInfo)
{
	COMPILER_REPORTER("OutputStaticData/int");

	INT_32 iId = oStaticData.StoreInt(iData);

return oVMOpcodeCollector.Insert(CreateInstruction(OUTPUT  | ARG_SRC_INT,  iId, oDebugInfo.GetInfo()));
}

//
// Send floating point value to standard output collector
//
INT_32 CTPP2Compiler::OutputStaticData(const W_FLOAT      & dData,
                                       const VMDebugInfo  & oDebugInfo)
{
	COMPILER_REPORTER("OutputStaticData/float");

	INT_32 iId = oStaticData.StoreFloat(dData);

return oVMOpcodeCollector.Insert(CreateInstruction(OUTPUT  | ARG_SRC_FLOAT,  iId, oDebugInfo.GetInfo()));
}

//
// Change visibility scope of CDT data (for ARRAY iterating)
//
INT_32 CTPP2Compiler::ChangeScope(CCHAR_P              szScopeName,
                                  const UINT_32        iScopeNameLength,
                                  const VMDebugInfo  & oDebugInfo)
{
	COMPILER_REPORTER("ChangeScope");

	UINT_64 iDebugInfo = oDebugInfo.GetInfo();
	INT_32 iNull = iZeroId;

	// Check symbol table
	const SymbolTable<SymbolTableRec>::SymbolRecord<SymbolTableRec> * pRecord = oSymbolTable.GetSymbol(szScopeName, iScopeNameLength);
	if (pRecord != NULL)
	{
		//if(bRegsAreDirty)
		{
/*
 			++iStackDepth;
			oVMOpcodeCollector.Insert(CreateInstruction(PUSH    | ARG_SRC_HR                ,     0, iDebugInfo));
			++iStackDepth;
			oVMOpcodeCollector.Insert(CreateInstruction(PUSH    | ARG_SRC_GR                ,     0, iDebugInfo));
			++iStackDepth;
			oVMOpcodeCollector.Insert(CreateInstruction(PUSH    | ARG_SRC_FR                ,     0, iDebugInfo));
			++iStackDepth;
			oVMOpcodeCollector.Insert(CreateInstruction(PUSH    | ARG_SRC_ER                ,     0, iDebugInfo));
*/
			iStackDepth += 4;
			oVMOpcodeCollector.Insert(CreateInstruction(PUSH47                              ,     0, iDebugInfo));
		}

		INT_32 iDepth = iStackDepth - pRecord -> symbol_data.stack_depth;

		// Get from HR register indirect value (local variables)
		if (pRecord -> symbol_data.scope_number == iScopeNumber)
		{
			oVMOpcodeCollector.Insert(CreateInstruction(MOV | ARG_DST_GR | ARG_SRC_HR, 0,    iDebugInfo));
		}
		// Move data from stack to HR register
		else
		{
			oVMOpcodeCollector.Insert(CreateInstruction(MOV | ARG_DST_GR | ARG_SRC_STACK,  iDepth - 1, iDebugInfo));
		}

//		oVMOpcodeCollector.Insert(CreateInstruction(MOVSIZE | ARG_DST_FR | ARG_SRC_GR   ,     0, iDebugInfo));
//		oVMOpcodeCollector.Insert(CreateInstruction(MOV     | ARG_DST_ER | ARG_SRC_INT  , iNull, iDebugInfo));
//
//		UINT_32 iRetPoint = oVMOpcodeCollector.Insert(CreateInstruction(CMP     | ARG_SRC_ER | ARG_DST_FR   , iNull, iDebugInfo));
//
//		oVMOpcodeCollector.Insert(CreateInstruction(JE                                  , (UINT_32)-1, iDebugInfo));
//
//		oVMOpcodeCollector.Insert(CreateInstruction(MOVIREG | ARG_DST_HR | ARG_SRC_GR   , ARG_SRC_ER, iDebugInfo));

		oVMOpcodeCollector.Insert(CreateInstruction(MOVSIZE | ARG_DST_FR | ARG_SRC_GR   ,     0, iDebugInfo));
		oVMOpcodeCollector.Insert(CreateInstruction(JE,                             (UINT_32)-1, iDebugInfo));
		UINT_32 iRetPoint = oVMOpcodeCollector.Insert(CreateInstruction(MOV     | ARG_DST_ER | ARG_SRC_INT  , iNull, iDebugInfo));

		oVMOpcodeCollector.Insert(CreateInstruction(MOVIREG | ARG_DST_HR | ARG_SRC_GR   , ARG_SRC_ER, iDebugInfo));
		++iRetPoint;

		return iRetPoint;
	}

	// Remember old HR value
	// Get from DR register indirect value and store it into stack
	INT_32 iId   = oStaticText.StoreData(szScopeName, iScopeNameLength);

//
//	PUSH     HR
//	PUSH     GR
//	PUSH     FR
//	PUSH     ER
//
//  N + 0:  MOVISTR  GR, HR["ScopeName"]
//  N + 1:  JE N + 3
//  N + 2:  MOVISTR  GR, DR["ScopeName"]
//
//	MOVSIZE  FR, GR.size()
//	MOV      ER, 0
//
//  @RET_POINT
//	CMP      ER, FR
//	JE       @END
//	MOVIREG  HR, GR[ER]
//

//
//	Loop code
//

//
//	INC      ER
//	JMP      RET_POINT
//  @END:

	//bRegsAreDirty = (iScopeNumber != 0);

	//if(bRegsAreDirty)
	{
/*
		++iStackDepth;
		oVMOpcodeCollector.Insert(CreateInstruction(PUSH    | ARG_SRC_HR                ,     0, iDebugInfo));
		++iStackDepth;
		oVMOpcodeCollector.Insert(CreateInstruction(PUSH    | ARG_SRC_GR                ,     0, iDebugInfo));
		++iStackDepth;
		oVMOpcodeCollector.Insert(CreateInstruction(PUSH    | ARG_SRC_FR                ,     0, iDebugInfo));
		++iStackDepth;
		oVMOpcodeCollector.Insert(CreateInstruction(PUSH    | ARG_SRC_ER                ,     0, iDebugInfo));
*/
		iStackDepth += 4;
		oVMOpcodeCollector.Insert(CreateInstruction(PUSH47                              ,     0, iDebugInfo));

	}

	// Check from HR register indirect value (local variables)
	INT_32 iPos = oVMOpcodeCollector.Insert(CreateInstruction(MOVISTR | ARG_DST_GR | ARG_SRC_HR, iId, iDebugInfo));
	oVMOpcodeCollector.Insert(CreateInstruction(JE, iPos + 3, iDebugInfo));
	// Check from DR register indirect value (global variables)
	iPos = oVMOpcodeCollector.Insert(CreateInstruction(MOVISTR | ARG_DST_GR | ARG_SRC_DR, iId, iDebugInfo));

//	oVMOpcodeCollector.Insert(CreateInstruction(MOVSIZE | ARG_DST_FR | ARG_SRC_GR   ,     0, iDebugInfo));
//	oVMOpcodeCollector.Insert(CreateInstruction(MOV     | ARG_DST_ER | ARG_SRC_INT  , iNull, iDebugInfo));
//
//	UINT_32 iRetPoint = oVMOpcodeCollector.Insert(CreateInstruction(CMP     | ARG_SRC_ER | ARG_DST_FR   , iNull, iDebugInfo));
//
//	oVMOpcodeCollector.Insert(CreateInstruction(JE                                  , (UINT_32)-1, iDebugInfo));
//
//	oVMOpcodeCollector.Insert(CreateInstruction(MOVIREG | ARG_DST_HR | ARG_SRC_GR   , ARG_SRC_ER, iDebugInfo));

	oVMOpcodeCollector.Insert(CreateInstruction(MOVSIZE | ARG_DST_FR | ARG_SRC_GR   ,     0, iDebugInfo));
	oVMOpcodeCollector.Insert(CreateInstruction(JE,                             (UINT_32)-1, iDebugInfo));
	UINT_32 iRetPoint = oVMOpcodeCollector.Insert(CreateInstruction(MOV     | ARG_DST_ER | ARG_SRC_INT  , iNull, iDebugInfo));

	oVMOpcodeCollector.Insert(CreateInstruction(MOVIREG | ARG_DST_HR | ARG_SRC_GR   , ARG_SRC_ER, iDebugInfo));
	++iRetPoint;

return iRetPoint;
}

//
// Change visibility scope of CDT data (for ARRAY iterating)
//
INT_32 CTPP2Compiler::ChangeForeachScope(CCHAR_P              szNS,
                                         const UINT_32        iNSLength,
                                         CCHAR_P              szName,
                                         const UINT_32        iNameLength,
                                         CCHAR_P              szFullVariable,
                                         const UINT_32        iFullVariableLength,
                                         const VMDebugInfo  & oDebugInfo)
{
	COMPILER_REPORTER("ChangeForeachScope");

	const SymbolTable<SymbolTableRec>::SymbolRecord<SymbolTableRec> * pRecord = oSymbolTable.GetSymbol(szNS, iNSLength);

	if (pRecord == NULL)
	{
		INT_32 iIP = ChangeScope(szFullVariable, iFullVariableLength, oDebugInfo);

		// Scope was changed
		++iScopeNumber;
		oSymbolTable.MarkScope();

		return iIP;
	}

	UINT_64 iDebugInfo = oDebugInfo.GetInfo();
	// Remember old HR value
	// Get from DR register indirect value and store it into stack
	INT_32 iNull = iZeroId;

//
//	PUSH47
//
//	MOVISTR  GR, HR["ScopeName"]
//	RJE      @FOUND_LOCAL
//	MOVISTR  GR, DR["ScopeName"]
//
//  @FOUND_LOCAL
// MOVSIZE  FR, GR.size()
//	MOV      ER, 0
//
//  @RET_POINT
//	MOVIREG  HR, GR[ER]
//

//
//	Loop code
//

//
//	LOOP     FR, ER, @RET_POINT
//	POP47

//
//	PUSH     HR
//	PUSH     GR
//	PUSH     FR
//	PUSH     ER
//
//  N + 0:  MOVISTR  GR, HR["ScopeName"]
//  N + 1:  JE N + 3
//  N + 2:  MOVISTR  GR, DR["ScopeName"]
//
//	MOVSIZE  FR, GR.size()
//	MOV      ER, 0
//
//  @RET_POINT
//	CMP      ER, FR
//	JE       @END
//	MOVIREG  HR, GR[ER]
//

//
//	Loop code
//

//
//	INC      ER
//	JMP      RET_POINT
//  @END:

	//bRegsAreDirty = (iScopeNumber != 0);

	//if(bRegsAreDirty)
	{
/* 		++iStackDepth;
		oVMOpcodeCollector.Insert(CreateInstruction(PUSH    | ARG_SRC_HR                ,     0, iDebugInfo));
		++iStackDepth;
		oVMOpcodeCollector.Insert(CreateInstruction(PUSH    | ARG_SRC_GR                ,     0, iDebugInfo));
		++iStackDepth;
		oVMOpcodeCollector.Insert(CreateInstruction(PUSH    | ARG_SRC_FR                ,     0, iDebugInfo));
		++iStackDepth;
		oVMOpcodeCollector.Insert(CreateInstruction(PUSH    | ARG_SRC_ER                ,     0, iDebugInfo));
*/
		iStackDepth += 4;
		oVMOpcodeCollector.Insert(CreateInstruction(PUSH47                              ,     0, iDebugInfo));
	}

	INT_32 iDepth = iStackDepth - pRecord -> symbol_data.stack_depth;

	INT_32 iId = oStaticText.StoreData(szName, iNameLength);

	// Get from HR register indirect value (local variables)
	if (pRecord -> symbol_data.scope_number == iScopeNumber)
	{
		oVMOpcodeCollector.Insert(CreateInstruction(MOVISTR | ARG_DST_GR | ARG_SRC_HR, iId,    iDebugInfo));
	}
	// Move data from stack to HR register
	else
	{
		oVMOpcodeCollector.Insert(CreateInstruction(MOV | ARG_DST_HR | ARG_SRC_STACK,  iDepth - 1, iDebugInfo));
		oVMOpcodeCollector.Insert(CreateInstruction(MOVISTR | ARG_DST_GR | ARG_SRC_HR, iId,        iDebugInfo));
	}

//	oVMOpcodeCollector.Insert(CreateInstruction(MOVSIZE | ARG_DST_FR | ARG_SRC_GR   ,     0, iDebugInfo));
//	oVMOpcodeCollector.Insert(CreateInstruction(MOV     | ARG_DST_ER | ARG_SRC_INT  , iNull, iDebugInfo));
//
//	UINT_32 iRetPoint = oVMOpcodeCollector.Insert(CreateInstruction(CMP     | ARG_SRC_ER | ARG_DST_FR   , iNull, iDebugInfo));
//
//	oVMOpcodeCollector.Insert(CreateInstruction(JE                                  , (UINT_32)-1, iDebugInfo));
//
//	oVMOpcodeCollector.Insert(CreateInstruction(MOVIREG | ARG_DST_HR | ARG_SRC_GR   , ARG_SRC_ER, iDebugInfo));
//

	oVMOpcodeCollector.Insert(CreateInstruction(MOVSIZE | ARG_DST_FR | ARG_SRC_GR   ,     0, iDebugInfo));
	oVMOpcodeCollector.Insert(CreateInstruction(JE,                             (UINT_32)-1, iDebugInfo));
	UINT_32 iRetPoint = oVMOpcodeCollector.Insert(CreateInstruction(MOV     | ARG_DST_ER | ARG_SRC_INT  , iNull, iDebugInfo));

	oVMOpcodeCollector.Insert(CreateInstruction(MOVIREG | ARG_DST_HR | ARG_SRC_GR   , ARG_SRC_ER, iDebugInfo));
	++iRetPoint;

	// Scope was changed
	++iScopeNumber;
	oSymbolTable.MarkScope();

return iRetPoint;
}

//
// Change visibility scope of CDT data (for ARRAY-of-ARRAYS iterating)
//
INT_32 CTPP2Compiler::ChangeContextScope(const VMDebugInfo & oDebugInfo)
{
	COMPILER_REPORTER("ChangeContextScope");

	UINT_64 iDebugInfo = oDebugInfo.GetInfo();
	// Remember old GR value
	// Get from HR register indirect value and store it into stack
	INT_32 iNull = iZeroId;

//
//	PUSH     HR
//	PUSH     GR
//	PUSH     FR
//	PUSH     ER
//	MOV      GR, HR
//	MOVSIZE  FR, GR.size()
//	MOV      ER, 0
//
//  @RET_POINT
//	CMP      ER, FR
//	JE       @END
//	MOVIREG  HR, GR[ER]
//

//
//	Loop code
//

//
//	INC      ER
//	JMP      RET_POINT
//  @END:

	//bRegsAreDirty = (iScopeNumber != 0);

	// Store registers only if need
	//if(bRegsAreDirty)
	{
/*		++iStackDepth;
		oVMOpcodeCollector.Insert(CreateInstruction(PUSH    | ARG_SRC_HR                ,     0, iDebugInfo));
		++iStackDepth;
		oVMOpcodeCollector.Insert(CreateInstruction(PUSH    | ARG_SRC_GR                ,     0, iDebugInfo));
		++iStackDepth;
		oVMOpcodeCollector.Insert(CreateInstruction(PUSH    | ARG_SRC_FR                ,     0, iDebugInfo));
		++iStackDepth;
		oVMOpcodeCollector.Insert(CreateInstruction(PUSH    | ARG_SRC_ER                ,     0, iDebugInfo));
*/
		iStackDepth += 4;
		oVMOpcodeCollector.Insert(CreateInstruction(PUSH47                              ,     0, iDebugInfo));
	}
//	oVMOpcodeCollector.Insert(CreateInstruction(MOV     | ARG_DST_GR | ARG_SRC_HR   ,     0, iDebugInfo));
//	oVMOpcodeCollector.Insert(CreateInstruction(MOVSIZE | ARG_DST_FR | ARG_SRC_GR   ,     0, iDebugInfo));
//	oVMOpcodeCollector.Insert(CreateInstruction(MOV     | ARG_DST_ER | ARG_SRC_INT  , iNull, iDebugInfo));
//
//	UINT_32 iRetPoint = oVMOpcodeCollector.Insert(CreateInstruction(CMP     | ARG_SRC_ER | ARG_DST_FR   , iNull, iDebugInfo));
//
//	oVMOpcodeCollector.Insert(CreateInstruction(JE                                  , (UINT_32)-1, iDebugInfo));
//
//	oVMOpcodeCollector.Insert(CreateInstruction(MOVIREG | ARG_DST_HR | ARG_SRC_GR   , ARG_SRC_ER, iDebugInfo));

	oVMOpcodeCollector.Insert(CreateInstruction(MOV     | ARG_DST_GR | ARG_SRC_HR   ,     0, iDebugInfo));
	oVMOpcodeCollector.Insert(CreateInstruction(MOVSIZE | ARG_DST_FR | ARG_SRC_GR   ,     0, iDebugInfo));
	oVMOpcodeCollector.Insert(CreateInstruction(JE,                             (UINT_32)-1, iDebugInfo));
	UINT_32 iRetPoint = oVMOpcodeCollector.Insert(CreateInstruction(MOV     | ARG_DST_ER | ARG_SRC_INT  , iNull, iDebugInfo));

	oVMOpcodeCollector.Insert(CreateInstruction(MOVIREG | ARG_DST_HR | ARG_SRC_GR   , ARG_SRC_ER, iDebugInfo));
	++iRetPoint;

	++iScopeNumber;
	oSymbolTable.MarkScope();

return iRetPoint;
}

//
// Reset scope to previous CDT data (for ARRAY iterating)
//
INT_32 CTPP2Compiler::ResetScope(const UINT_32        iIP,
                                 const VMDebugInfo  & oDebugInfo)
{
	COMPILER_REPORTER("ResetScope");

	UINT_64 iDebugInfo = oDebugInfo.GetInfo();

//
//	LOOP     FR, ER, @RET_POINT
//	POP47

//
//	INC      ER
//	JMP      RET_POINT
//  @END:
//      POP      ER
//      POP      FR
//      POP      GR
//      POP      HR

//	oVMOpcodeCollector.Insert(CreateInstruction(INC     | ARG_SRC_ER                ,     0, iDebugInfo));
//	oVMOpcodeCollector.Insert(CreateInstruction(JMP                                 ,   iIP, iDebugInfo));

	oVMOpcodeCollector.Insert(CreateInstruction(LOOP | ARG_SRC_FR | ARG_DST_ER      ,   iIP, iDebugInfo));

	UINT_32 iEndPoint =
	oVMOpcodeCollector.Insert(CreateInstruction(POP47                               ,     0, iDebugInfo));
	iStackDepth -= 4;

	VMInstruction * pInstr = oVMOpcodeCollector.GetInstruction(iIP - 2);
	if (pInstr == NULL) { throw "Ouch!"; }
	pInstr -> argument = iEndPoint;

/*
//	oVMOpcodeCollector.Insert(CreateInstruction(MOV     | ARG_DST_ER | ARG_SRC_STACK,     0, iDebugInfo));
//	oVMOpcodeCollector.Insert(CreateInstruction(POP     | ARG_SRC_ER                ,     0, iDebugInfo));

	//bRegsAreDirty = (iScopeNumber != 0);

	//if(bRegsAreDirty)
	{
		--iStackDepth;
//		oVMOpcodeCollector.Insert(CreateInstruction(POP                                 ,     1, iDebugInfo));
	}
*/
//	VMInstruction * pInstr = oVMOpcodeCollector.GetInstruction(iIP + 1);
//	if (pInstr == NULL) { throw "Ouch!"; }
//	pInstr -> argument = iEndPoint;
/*
	//if(bRegsAreDirty)
	{
//		oVMOpcodeCollector.Insert(CreateInstruction(MOV     | ARG_DST_FR | ARG_SRC_STACK,     0, iDebugInfo));
		oVMOpcodeCollector.Insert(CreateInstruction(POP     | ARG_SRC_FR                ,     0, iDebugInfo));
		--iStackDepth;

//		oVMOpcodeCollector.Insert(CreateInstruction(POP                                 ,     1, iDebugInfo));

//		oVMOpcodeCollector.Insert(CreateInstruction(MOV     | ARG_DST_GR | ARG_SRC_STACK,     0, iDebugInfo));
		oVMOpcodeCollector.Insert(CreateInstruction(POP     | ARG_SRC_GR                ,     0, iDebugInfo));
		--iStackDepth;

//		oVMOpcodeCollector.Insert(CreateInstruction(POP                                 ,     1, iDebugInfo));

//		oVMOpcodeCollector.Insert(CreateInstruction(MOV     | ARG_DST_HR | ARG_SRC_STACK,     0, iDebugInfo));
		oVMOpcodeCollector.Insert(CreateInstruction(POP     | ARG_SRC_HR                ,     0, iDebugInfo));
		--iStackDepth;

//		oVMOpcodeCollector.Insert(CreateInstruction(POP                                 ,     1, iDebugInfo));
	}
*/
return iEndPoint;
}

//
// Reset scope to previous CDT data (for ARRAY iterating)
//
INT_32 CTPP2Compiler::ResetForeachScope(const UINT_32        iIP,
                                        const VMDebugInfo  & oDebugInfo)
{
	COMPILER_REPORTER("ResetForeachScope");

	--iScopeNumber;
	oSymbolTable.UnmarkScope();

return ResetScope(iIP, oDebugInfo);
}

//
// Call block by name
//
INT_32 CTPP2Compiler::CallBlock(const STLW::string   & sBlockName,
                                const bool           & bIsVariable,
                                const VMDebugInfo    & oDebugInfo)
{
	COMPILER_REPORTER("CallBlock");
	UINT_64 iDebugInfo = oDebugInfo.GetInfo();

	// Store block name
	INT_32 iId = oStaticText.StoreData(sBlockName.data(), sBlockName.size());

	if (!bIsVariable)
	{
		// Call by name
		return oVMOpcodeCollector.Insert(CreateInstruction(CALLNAME, iId, iDebugInfo));
	}
	else
	{
		// Call by indirect value
		// Check from HR register indirect value
		INT_32 iPos =
		oVMOpcodeCollector.Insert(CreateInstruction(PUSH | ARG_SRC_IND_STR | ARG_DST_HR, iId,      iDebugInfo));
		oVMOpcodeCollector.Insert(CreateInstruction(JE                                 , iPos + 3, iDebugInfo));
		oVMOpcodeCollector.Insert(CreateInstruction(REPLACE | ARG_SRC_IND_STR | ARG_DST_DR, iId,      iDebugInfo));

		++iStackDepth;
		return oVMOpcodeCollector.Insert(CreateInstruction(CALLIND | ARG_SRC_STACK     , 0,        iDebugInfo));
	}

// Make compiler happy
return -1;
}

//
// Call context block
//
INT_32 CTPP2Compiler::CallContextBlock(const VMDebugInfo & oDebugInfo)
{
	COMPILER_REPORTER("CallContextBlock");

	UINT_64 iDebugInfo = oDebugInfo.GetInfo();

return oVMOpcodeCollector.Insert(CreateInstruction(CALLIND | ARG_SRC_HR, 0, iDebugInfo));
}

//
// Start of block
//
INT_32 CTPP2Compiler::StartBlock(const STLW::string  & sBlockName,
                                 const VMDebugInfo   & oDebugInfo)
{
	COMPILER_REPORTER("StartBlock");

	UINT_64 iDebugInfo = oDebugInfo.GetInfo();
	UINT_32 iIP = oVMOpcodeCollector.Insert(CreateInstruction(JMP                                 ,   (UINT_32)-1, iDebugInfo));

	oHashTable.Put(sBlockName.data(), sBlockName.size(), oVMOpcodeCollector.GetCodeSize());

return iIP;
}

//
// End of block
//
INT_32 CTPP2Compiler::EndBlock(const VMDebugInfo & oDebugInfo)
{
	COMPILER_REPORTER("EndBlock");

	UINT_64 iDebugInfo = oDebugInfo.GetInfo();

return oVMOpcodeCollector.Insert(CreateInstruction(RET, 0, iDebugInfo));
}

//
// Get system call by id
//
INT_32 CTPP2Compiler::GetSyscallId(CCHAR_P        szSyscallName,
                                   const UINT_32  iSyscallNameLength)
{
	COMPILER_REPORTER("GetSyscallId");

	const STLW::string sTMP(szSyscallName, iSyscallNameLength);
	STLW::map<STLW::string, UINT_32>::const_iterator itmSyscalls = mSyscalls.find(sTMP);
	if (itmSyscalls != mSyscalls.end()) { return itmSyscalls -> second; }

	UINT_32 iSyscallNum = oSyscalls.StoreData(szSyscallName, iSyscallNameLength);

	mSyscalls[sTMP] = iSyscallNum;

return iSyscallNum;
}

//
// Execute system call such as HREF_PARAM, FORM_PARAM, etc
//
INT_32 CTPP2Compiler::ExecuteSyscall(CCHAR_P              szSyscallName,
                                     const UINT_32        iSyscallNameLength,
                                     const UINT_32        iArgNum,
                                     const VMDebugInfo  & oDebugInfo)
{
	COMPILER_REPORTER("ExecuteSyscall");

	iStackDepth -= iArgNum - 1;

return oVMOpcodeCollector.Insert(CreateInstruction(SYSCALL, SYSCALL_PARAMS(GetSyscallId(szSyscallName, iSyscallNameLength), iArgNum), oDebugInfo.GetInfo()));
}

//
// Push contextual variable into stack
//
INT_32 CTPP2Compiler::PushContextualVariable(const eCTPP2LoopContextVar & iTokenType,
                                             const VMDebugInfo          & oDebugInfo)
{
	COMPILER_REPORTER("PushContextualVariable");
	UINT_64 iDebugInfo = oDebugInfo.GetInfo();

	switch (iTokenType)
	{
//
//  N + 0:  MOV   AR, Zero
//  N + 1:  CMP   ER, AR
//  N + 2:  JODD  @N1
//  N + 3:  PUSH  Zero
//  N + 4:  JMP   @END
// @N1
//  N + 5:  PUSH  One
// @END
//  N + 6:  something...
//
		case TMPL_LOOP_FIRST:
			{
				++iStackDepth;
				oVMOpcodeCollector.Insert(CreateInstruction(MOV | ARG_DST_AR | ARG_SRC_INT, iZeroId, iDebugInfo));
				oVMOpcodeCollector.Insert(CreateInstruction(CMP | ARG_SRC_ER | ARG_DST_AR , 0,       iDebugInfo));
				oVMOpcodeCollector.Insert(CreateInstruction(RJE                           , 3,       iDebugInfo));
				oVMOpcodeCollector.Insert(CreateInstruction(PUSH | ARG_SRC_INT            , iZeroId, iDebugInfo));
				oVMOpcodeCollector.Insert(CreateInstruction(RJMP                          , 2,       iDebugInfo));
				return oVMOpcodeCollector.Insert(CreateInstruction(PUSH | ARG_SRC_INT     , iOneId,  iDebugInfo));
			}

//
//  N + 0:  MOV   AR, ER
//  N + 1:  INC   AR
//  N + 1:  CMP   FR, AR
//  N + 2:  JE    @N1
//  N + 3:  PUSH  Zero
//  N + 4:  JMP   @END
// @N1
//  N + 5:  PUSH  One
// @END
//  N + 6:  something...
//
		case TMPL_LOOP_LAST:
			{
				++iStackDepth;
				oVMOpcodeCollector.Insert(CreateInstruction(MOV | ARG_DST_AR | ARG_SRC_INT, iOneId, iDebugInfo));
				oVMOpcodeCollector.Insert(CreateInstruction(CMP | ARG_SRC_FR | ARG_DST_AR , 0,       iDebugInfo));
				oVMOpcodeCollector.Insert(CreateInstruction(RJE                           , 3,       iDebugInfo));
				oVMOpcodeCollector.Insert(CreateInstruction(PUSH | ARG_SRC_INT            , iZeroId, iDebugInfo));
				oVMOpcodeCollector.Insert(CreateInstruction(RJMP                          , 2,       iDebugInfo));
				return oVMOpcodeCollector.Insert(CreateInstruction(PUSH | ARG_SRC_INT     , iOneId,  iDebugInfo));
			}

//
//  N + 0:   MOV   AR, Zero  |
//  N + 1:   CMP   ER, AR    | 0
//  N + 2:   JE    @N0       | 1 -> 6
//  N + 5:   MOV   AR, ER    | 2
//  N + 6:   INC   AR        | 3
//  N + 7:   CMP   FR, AR    | 4
//  N + 8:   JN    @N1       | 5 -> 8
// N0:
//  N + 9:   PUSH  Zero      | 6
//  N + 10:  JMP   @END      | 7 -> 9
// @N1                       |
//  N + 5:   PUSH  One       | 8
// @END                      |
//  N + 6:  something...     | 9
//
		case TMPL_LOOP_INNER:
			{
				++iStackDepth;
				oVMOpcodeCollector.Insert(CreateInstruction(MOV | ARG_DST_AR | ARG_SRC_INT, iZeroId, iDebugInfo));
				oVMOpcodeCollector.Insert(CreateInstruction(CMP | ARG_SRC_ER | ARG_DST_AR , 0,       iDebugInfo));
				oVMOpcodeCollector.Insert(CreateInstruction(RJE                           , 6,       iDebugInfo));

				oVMOpcodeCollector.Insert(CreateInstruction(MOV | ARG_DST_AR | ARG_SRC_INT, iOneId,  iDebugInfo));
				oVMOpcodeCollector.Insert(CreateInstruction(CMP | ARG_SRC_FR | ARG_DST_AR , 0,       iDebugInfo));
				oVMOpcodeCollector.Insert(CreateInstruction(RJE                           , 3,       iDebugInfo));

				oVMOpcodeCollector.Insert(CreateInstruction(PUSH | ARG_SRC_INT            , iOneId,  iDebugInfo));
				oVMOpcodeCollector.Insert(CreateInstruction(RJMP                          , 2,       iDebugInfo));
				return oVMOpcodeCollector.Insert(CreateInstruction(PUSH | ARG_SRC_INT     , iZeroId, iDebugInfo));

/*

				oVMOpcodeCollector.Insert(CreateInstruction(MOV | ARG_DST_AR | ARG_SRC_INT, iZeroId, iDebugInfo));

				UINT_32 iIP =
				oVMOpcodeCollector.Insert(CreateInstruction(CMP | ARG_SRC_ER | ARG_DST_AR , 0      , iDebugInfo));
				oVMOpcodeCollector.Insert(CreateInstruction(JE                            , iIP + 6, iDebugInfo));

				oVMOpcodeCollector.Insert(CreateInstruction(MOV | ARG_DST_AR | ARG_SRC_ER , 0      , iDebugInfo));
				oVMOpcodeCollector.Insert(CreateInstruction(INC | ARG_SRC_AR              , 0      , iDebugInfo));
				oVMOpcodeCollector.Insert(CreateInstruction(CMP | ARG_SRC_FR | ARG_DST_AR , 0      , iDebugInfo));
				oVMOpcodeCollector.Insert(CreateInstruction(JN                            , iIP + 8, iDebugInfo));

				oVMOpcodeCollector.Insert(CreateInstruction(PUSH | ARG_SRC_INT            , iZeroId, iDebugInfo));
				oVMOpcodeCollector.Insert(CreateInstruction(JMP                           , iIP + 9, iDebugInfo));

				++iStackDepth;
				return oVMOpcodeCollector.Insert(CreateInstruction(PUSH | ARG_SRC_INT     , iOneId,  iDebugInfo));
*/
			}

		case TMPL_LOOP_OUTER:
			{
				++iStackDepth;
				oVMOpcodeCollector.Insert(CreateInstruction(MOV | ARG_DST_AR | ARG_SRC_INT, iZeroId, iDebugInfo));
				oVMOpcodeCollector.Insert(CreateInstruction(CMP | ARG_SRC_ER | ARG_DST_AR , 0,       iDebugInfo));
				oVMOpcodeCollector.Insert(CreateInstruction(RJN                           , 6,       iDebugInfo));

				oVMOpcodeCollector.Insert(CreateInstruction(MOV | ARG_DST_AR | ARG_SRC_INT, iOneId,  iDebugInfo));
				oVMOpcodeCollector.Insert(CreateInstruction(CMP | ARG_SRC_FR | ARG_DST_AR , 0,       iDebugInfo));
				oVMOpcodeCollector.Insert(CreateInstruction(RJN                           , 3,       iDebugInfo));

				oVMOpcodeCollector.Insert(CreateInstruction(PUSH | ARG_SRC_INT            , iOneId,  iDebugInfo));
				oVMOpcodeCollector.Insert(CreateInstruction(RJMP                          , 2,       iDebugInfo));
				return oVMOpcodeCollector.Insert(CreateInstruction(PUSH | ARG_SRC_INT     , iZeroId, iDebugInfo));
			}


		case TMPL_LOOP_EVEN:
			{
				oVMOpcodeCollector.Insert(CreateInstruction(MOV | ARG_DST_AR | ARG_SRC_INT, iZeroId, iDebugInfo));

				UINT_32 iIP =
				oVMOpcodeCollector.Insert(CreateInstruction(CMP | ARG_SRC_ER | ARG_DST_AR , 0,       iDebugInfo));
				oVMOpcodeCollector.Insert(CreateInstruction(JODD                          , iIP + 4, iDebugInfo));

				oVMOpcodeCollector.Insert(CreateInstruction(PUSH | ARG_SRC_INT            , iZeroId, iDebugInfo));
				oVMOpcodeCollector.Insert(CreateInstruction(JMP                           , iIP + 5, iDebugInfo));

				++iStackDepth;
				return oVMOpcodeCollector.Insert(CreateInstruction(PUSH | ARG_SRC_INT     , iOneId,  iDebugInfo));
			}

		case TMPL_LOOP_ODD:
			{
				oVMOpcodeCollector.Insert(CreateInstruction(MOV | ARG_DST_AR | ARG_SRC_INT, iZeroId,          iDebugInfo));
				UINT_32 iIP =
				oVMOpcodeCollector.Insert(CreateInstruction(CMP | ARG_SRC_ER | ARG_DST_AR          , 0,       iDebugInfo));
				oVMOpcodeCollector.Insert(CreateInstruction(JODD                                   , iIP + 4, iDebugInfo));

				oVMOpcodeCollector.Insert(CreateInstruction(PUSH | ARG_SRC_INT                     , iOneId,  iDebugInfo));
				oVMOpcodeCollector.Insert(CreateInstruction(JMP                                    , iIP + 5, iDebugInfo));

				++iStackDepth;
				return oVMOpcodeCollector.Insert(CreateInstruction(PUSH | ARG_SRC_INT              , iZeroId, iDebugInfo));
			}

		case TMPL_LOOP_COUNTER:
			{
				// Counter
				oVMOpcodeCollector.Insert(CreateInstruction(MOV | ARG_DST_AR | ARG_SRC_ER, 0, iDebugInfo));
				oVMOpcodeCollector.Insert(CreateInstruction(INC | ARG_SRC_AR             , 0, iDebugInfo));

				++iStackDepth;
				return oVMOpcodeCollector.Insert(CreateInstruction(PUSH | ARG_SRC_AR     , 0, iDebugInfo));
			}

		case TMPL_LOOP_RCOUNTER:
			{
				// Counter
				++iStackDepth;
				return oVMOpcodeCollector.Insert(CreateInstruction(PUSH | ARG_SRC_FR     , 0, iDebugInfo));
			}

		case TMPL_LOOP_SIZE:
			{
				// Loop size
				++iStackDepth;
				oVMOpcodeCollector.Insert(CreateInstruction(MOVSIZE | ARG_SRC_GR | ARG_DST_AR, 0, iDebugInfo));
				return oVMOpcodeCollector.Insert(CreateInstruction(PUSH | ARG_SRC_AR     , 0, iDebugInfo));
			}

		case TMPL_LOOP_CONTENT:
			{
				// Content of variable
				++iStackDepth;
				return oVMOpcodeCollector.Insert(CreateInstruction(PUSH | ARG_SRC_HR      , 0, iDebugInfo));
			}
		default:
			;
	}

return 0;
}

//
// Push variable into stack
//
INT_32 CTPP2Compiler::PushVariable(CCHAR_P              szVariableName,
                                   const UINT_32        iVariableNameLength,
                                   const VMDebugInfo  & oDebugInfo)
{
	COMPILER_REPORTER("PushVariable");
	UINT_64 iDebugInfo = oDebugInfo.GetInfo();

	// Check symbol table
	const SymbolTable<SymbolTableRec>::SymbolRecord<SymbolTableRec> * pRecord = oSymbolTable.GetSymbol(szVariableName, iVariableNameLength);
	if (pRecord == NULL)
	{
//
//  N + 0:  PUSH HR["variable"]
//  N + 1:  JE N + 3
//  N + 2:  REPLACE DR["variable"]
//  N + 3:  something...
//
		INT_32 iId = oStaticText.StoreData(szVariableName, iVariableNameLength);

		// Check from HR register indirect value
		INT_32 iPos =
		oVMOpcodeCollector.Insert(CreateInstruction(PUSH | ARG_SRC_IND_STR | ARG_DST_HR, iId,      iDebugInfo));
		oVMOpcodeCollector.Insert(CreateInstruction(JE                                 , iPos + 3, iDebugInfo));

		++iStackDepth;
		return oVMOpcodeCollector.Insert(CreateInstruction(REPLACE | ARG_SRC_IND_STR | ARG_DST_DR, iId, iDebugInfo));
	}

	// Check current stack depth
	INT_32 iDepth = iStackDepth - pRecord -> symbol_data.stack_depth;

	// Check HR and stack
	++iStackDepth;
	if (pRecord -> symbol_data.scope_number == iScopeNumber)
	{
		return oVMOpcodeCollector.Insert(CreateInstruction(PUSH    | ARG_SRC_HR,     0,          iDebugInfo));
	}

	return oVMOpcodeCollector.Insert(CreateInstruction(PUSH    | ARG_SRC_STACK,  iDepth - 1, iDebugInfo));
}

//
// Store variable in scope
//
INT_32 CTPP2Compiler::StoreScopedVariable(CCHAR_P              szNS,
	                                  const UINT_32        iNSLength,
                                          const VMDebugInfo  & oDebugInfo)
{
	COMPILER_REPORTER("StoreScopedVariable");

	INT_32 iSymbolId = oSymbolTable.AddSymbol(szNS, iNSLength, SymbolTableRec(iStackDepth, iScopeNumber));

return iSymbolId;
}

//
// Push variable into stack
//
INT_32 CTPP2Compiler::PushScopedVariable(CCHAR_P              szNS,
                                         const UINT_32        iNSLength,
                                         CCHAR_P              szName,
                                         const UINT_32        iNameLength,
                                         CCHAR_P              szFullVariable,
                                         const UINT_32        iFullVariableLength,
                                         const VMDebugInfo  & oDebugInfo)
{
	COMPILER_REPORTER("PushScopedVariable");

	// Check symbol table
	const SymbolTable<SymbolTableRec>::SymbolRecord<SymbolTableRec> * pRecord = oSymbolTable.GetSymbol(szNS, iNSLength);
	if (pRecord == NULL)
	{
		return PushVariable(szFullVariable, iFullVariableLength, oDebugInfo);
	}

	// Store debug info
	UINT_64 iDebugInfo = oDebugInfo.GetInfo();

	// Store variable name
	INT_32 iId = oStaticText.StoreData(szName, iNameLength);

	// Check current stack depth
	INT_32 iDepth = iStackDepth - pRecord -> symbol_data.stack_depth;

	++iStackDepth;
	if (pRecord -> symbol_data.scope_number == iScopeNumber)
	{
		oVMOpcodeCollector.Insert(CreateInstruction(PUSH    | ARG_SRC_HR,     0,          iDebugInfo));
	}
	else
	{
		oVMOpcodeCollector.Insert(CreateInstruction(PUSH    | ARG_SRC_STACK,  iDepth - 4, iDebugInfo));
	}

return oVMOpcodeCollector.Insert(CreateInstruction(REPLACE | ARG_SRC_IND_STR | ARG_DST_STACK, iId,    iDebugInfo));
}

//
// Push integer value into stack
//
INT_32 CTPP2Compiler::PushInt(const INT_64       & iVariable,
                              const VMDebugInfo  & oDebugInfo)
{
	COMPILER_REPORTER("PushInt");

	INT_32 iId = -1;
	// Predefined variables: '0' and '1'
	if      (iVariable == 0) { iId = iZeroId; }
	else if (iVariable == 1) { iId = iOneId;  }
	else                     { iId = oStaticData.StoreInt(iVariable); }

	++iStackDepth;
return oVMOpcodeCollector.Insert(CreateInstruction(PUSH | ARG_SRC_INT, iId, oDebugInfo.GetInfo()));
}

//
// Push Float value into stack
//
INT_32 CTPP2Compiler::PushFloat(const W_FLOAT      & dVariable,
                                const VMDebugInfo  & oDebugInfo)
{
	COMPILER_REPORTER("PushFloat");

	INT_32 iId = oStaticData.StoreFloat(dVariable);

	++iStackDepth;
return oVMOpcodeCollector.Insert(CreateInstruction(PUSH | ARG_SRC_FLOAT, iId, oDebugInfo.GetInfo()));
}

//
// Push string value into stack
//
INT_32 CTPP2Compiler::PushString(CCHAR_P              szData,
                                 const UINT_32        iDataLength,
                                 const VMDebugInfo  & oDebugInfo)
{
	COMPILER_REPORTER("PushString");

	INT_32 iId = oStaticText.StoreData(szData, iDataLength);

	++iStackDepth;
return oVMOpcodeCollector.Insert(CreateInstruction(PUSH | ARG_SRC_STR, iId, oDebugInfo.GetInfo()));
}

//
// Check existence of stack variable
//
INT_32 CTPP2Compiler::ExistStackVariable(const VMDebugInfo & oDebugInfo)
{
	COMPILER_REPORTER("ExistStackVariable");

return oVMOpcodeCollector.Insert(CreateInstruction(EXIST | ARG_SRC_STACK, 0, oDebugInfo.GetInfo()));
}

//
// Check existence of AR register variable
//
INT_32 CTPP2Compiler::ExistARReg(const VMDebugInfo & oDebugInfo)
{
	COMPILER_REPORTER("ExistARReg");

return oVMOpcodeCollector.Insert(CreateInstruction(EXIST | ARG_SRC_AR, 0, oDebugInfo.GetInfo()));
}

//
// Pop variable from stack
//
INT_32 CTPP2Compiler::PopVariable(const INT_32         iVars,
                                  const VMDebugInfo  & oDebugInfo)
{
	COMPILER_REPORTER("PopVariable");

	iStackDepth -= iVars;

return oVMOpcodeCollector.Insert(CreateInstruction(POP, iVars, oDebugInfo.GetInfo()));
}

//
// Remove last instruction from code segment
//
INT_32 CTPP2Compiler::RemoveInstruction()
{
	COMPILER_REPORTER("RemoveInstruction");

	--iStackDepth;

return oVMOpcodeCollector.Remove();
}

//
// Unconditional jump
//
INT_32 CTPP2Compiler::UncondJump(const UINT_32        iIP,
                                 const VMDebugInfo  & oDebugInfo)
{
	COMPILER_REPORTER("UncondJump");

return oVMOpcodeCollector.Insert(CreateInstruction(JMP, iIP, oDebugInfo.GetInfo()));
}

//
// Jump if NOT Equal
//
INT_32 CTPP2Compiler::NEJump(const UINT_32        iIP,
                             const VMDebugInfo  & oDebugInfo)
{
	COMPILER_REPORTER("NEJump");

return oVMOpcodeCollector.Insert(CreateInstruction(JXX | FL_NE, iIP, oDebugInfo.GetInfo()));
}

//
// Jump if Equal
//
INT_32 CTPP2Compiler::EQJump(const UINT_32        iIP,
                             const VMDebugInfo  & oDebugInfo)
{
	COMPILER_REPORTER("EQJump");

return oVMOpcodeCollector.Insert(CreateInstruction(JXX | FL_EQ, iIP, oDebugInfo.GetInfo()));
}

//
// Jump if Greater
//
INT_32 CTPP2Compiler::GTJump(const UINT_32        iIP,
                             const VMDebugInfo  & oDebugInfo)
{
	COMPILER_REPORTER("GTJump");

return oVMOpcodeCollector.Insert(CreateInstruction(JXX | FL_GT, iIP, oDebugInfo.GetInfo()));
}

//
// Jump if Greater Or Equal
//
INT_32 CTPP2Compiler::GEJump(const UINT_32        iIP,
                             const VMDebugInfo  & oDebugInfo)
{
	COMPILER_REPORTER("GEJump");

return oVMOpcodeCollector.Insert(CreateInstruction(JXX | FL_EQ | FL_GT, iIP, oDebugInfo.GetInfo()));
}

//
// Jump if Less
//
INT_32 CTPP2Compiler::LTJump(const UINT_32        iIP,
                             const VMDebugInfo  & oDebugInfo)
{
	COMPILER_REPORTER("LTJump");

return oVMOpcodeCollector.Insert(CreateInstruction(JXX | FL_LT, iIP, oDebugInfo.GetInfo()));
}

//
// Jump if Less Or Equal
//
INT_32 CTPP2Compiler::LEJump(const UINT_32        iIP,
                             const VMDebugInfo  & oDebugInfo)
{
	COMPILER_REPORTER("LEJump");

return oVMOpcodeCollector.Insert(CreateInstruction(JXX | FL_EQ | FL_LT, iIP, oDebugInfo.GetInfo()));
}

//
// Unconditional jump
//
INT_32 CTPP2Compiler::RUncondJump(const UINT_32        iIP,
                                  const VMDebugInfo  & oDebugInfo)
{
	COMPILER_REPORTER("UncondJump");

return oVMOpcodeCollector.Insert(CreateInstruction(RJMP, iIP, oDebugInfo.GetInfo()));
}

//
// Jump if NOT Equal
//
INT_32 CTPP2Compiler::RNEJump(const UINT_32        iIP,
                              const VMDebugInfo  & oDebugInfo)
{
	COMPILER_REPORTER("NEJump");

return oVMOpcodeCollector.Insert(CreateInstruction(RJN, iIP, oDebugInfo.GetInfo()));
}

//
// Jump if Equal
//
INT_32 CTPP2Compiler::REQJump(const UINT_32        iIP,
                              const VMDebugInfo  & oDebugInfo)
{
	COMPILER_REPORTER("REQJump");

return oVMOpcodeCollector.Insert(CreateInstruction(RJE, iIP, oDebugInfo.GetInfo()));
}

//
// Jump if Greater
//
INT_32 CTPP2Compiler::RGTJump(const UINT_32        iIP,
                              const VMDebugInfo  & oDebugInfo)
{
	COMPILER_REPORTER("RGTJump");

return oVMOpcodeCollector.Insert(CreateInstruction(RJG, iIP, oDebugInfo.GetInfo()));
}

//
// Jump if Greater Or Equal
//
INT_32 CTPP2Compiler::RGEJump(const UINT_32        iIP,
                              const VMDebugInfo  & oDebugInfo)
{
	COMPILER_REPORTER("RGEJump");

return oVMOpcodeCollector.Insert(CreateInstruction(RJGE, iIP, oDebugInfo.GetInfo()));
}

//
// Jump if Less
//
INT_32 CTPP2Compiler::RLTJump(const UINT_32        iIP,
                              const VMDebugInfo  & oDebugInfo)
{
	COMPILER_REPORTER("RLTJump");

return oVMOpcodeCollector.Insert(CreateInstruction(RJL, iIP, oDebugInfo.GetInfo()));
}

//
// Jump if Less Or Equal
//
INT_32 CTPP2Compiler::RLEJump(const UINT_32        iIP,
                              const VMDebugInfo  & oDebugInfo)
{
	COMPILER_REPORTER("RLEJump");

return oVMOpcodeCollector.Insert(CreateInstruction(RJLE, iIP, oDebugInfo.GetInfo()));
}


//
// Clear register
//
INT_32 CTPP2Compiler::ClearStack(const UINT_32        iStackPointer,
                                 const VMDebugInfo  & oDebugInfo)
{
	COMPILER_REPORTER("ClearStack");

return oVMOpcodeCollector.Insert(CreateInstruction(CLEAR | ARG_DST_STACK, iStackPointer, oDebugInfo.GetInfo()));
}

//
//  Stop execution
//
INT_32 CTPP2Compiler::Halt(const VMDebugInfo & oDebugInfo)
{
	COMPILER_REPORTER("Halt");

return oVMOpcodeCollector.Insert(CreateInstruction(HLT, 0, oDebugInfo.GetInfo()));
}

//
// Addition
//
INT_32 CTPP2Compiler::OpAdd(const VMDebugInfo & oDebugInfo)
{
	COMPILER_REPORTER("OpAdd");

	--iStackDepth;

return oVMOpcodeCollector.Insert(CreateInstruction(ADD | ARG_DST_STACK | ARG_SRC_STACK, 0, oDebugInfo.GetInfo()));
}

//
// Substraction
//
INT_32 CTPP2Compiler::OpSub(const VMDebugInfo & oDebugInfo)
{
	COMPILER_REPORTER("OpSub");

	--iStackDepth;

return oVMOpcodeCollector.Insert(CreateInstruction(SUB | ARG_DST_STACK | ARG_SRC_STACK, 0, oDebugInfo.GetInfo()));
}

//
// Multiplication
//
INT_32 CTPP2Compiler::OpMul(const VMDebugInfo & oDebugInfo)
{
	COMPILER_REPORTER("OpMul");

	--iStackDepth;

return oVMOpcodeCollector.Insert(CreateInstruction(MUL | ARG_DST_STACK | ARG_SRC_STACK, 0, oDebugInfo.GetInfo()));
}

//
// Division
//
INT_32 CTPP2Compiler::OpDiv(const VMDebugInfo & oDebugInfo)
{
	COMPILER_REPORTER("OpDiv");

	--iStackDepth;

return oVMOpcodeCollector.Insert(CreateInstruction(DIV | ARG_DST_STACK | ARG_SRC_STACK, 0, oDebugInfo.GetInfo()));
}

//
// Integer division
//
INT_32 CTPP2Compiler::OpIDiv(const VMDebugInfo & oDebugInfo)
{
	COMPILER_REPORTER("OpIDiv");

	--iStackDepth;

return oVMOpcodeCollector.Insert(CreateInstruction(IDIV | ARG_DST_STACK | ARG_SRC_STACK, 0, oDebugInfo.GetInfo()));
}

//
// Mod
//
INT_32 CTPP2Compiler::OpMod(const VMDebugInfo & oDebugInfo)
{
	COMPILER_REPORTER("OpMod");

	--iStackDepth;

return oVMOpcodeCollector.Insert(CreateInstruction(MOD | ARG_DST_STACK | ARG_SRC_STACK, 0, oDebugInfo.GetInfo()));
}

//
// Negation
//
INT_32 CTPP2Compiler::OpNeg(const VMDebugInfo & oDebugInfo)
{
	COMPILER_REPORTER("OpNeg");

return oVMOpcodeCollector.Insert(CreateInstruction(NEG | ARG_SRC_STACK, 0, oDebugInfo.GetInfo()));
}

//
// Logical negation
//
INT_32 CTPP2Compiler::OpNot(const VMDebugInfo & oDebugInfo)
{
	COMPILER_REPORTER("OpNot");

return oVMOpcodeCollector.Insert(CreateInstruction(CTPP2_NOT | ARG_SRC_STACK, 0, oDebugInfo.GetInfo()));
}

//
// Comparison
//
INT_32 CTPP2Compiler::OpCmp(const VMDebugInfo & oDebugInfo)
{
	COMPILER_REPORTER("OpCmp");
	--iStackDepth;
	--iStackDepth;
return oVMOpcodeCollector.Insert(CreateInstruction(CMP | ARG_DST_STACK | ARG_SRC_STACK, 0, oDebugInfo.GetInfo()));
}

//
// String comparison
//
INT_32 CTPP2Compiler::OpSCmp(const VMDebugInfo & oDebugInfo)
{
	COMPILER_REPORTER("OpSCmp");
	--iStackDepth;
	--iStackDepth;
return oVMOpcodeCollector.Insert(CreateInstruction(SCMP | ARG_DST_STACK | ARG_SRC_STACK, 0, oDebugInfo.GetInfo()));
}

//
// Get last instruction number
//
UINT_32 CTPP2Compiler::GetCodeSize() const { return oVMOpcodeCollector.GetCodeSize(); }

//
// Push variable on stack into stack
//
INT_32 CTPP2Compiler::PushStackVariable(const INT_32         iStackPos,
                                        const VMDebugInfo  & oDebugInfo)
{
	COMPILER_REPORTER("PushStackVariable");

	++iStackDepth;
return oVMOpcodeCollector.Insert(CreateInstruction(PUSH | ARG_SRC_STACK, iStackPos, oDebugInfo.GetInfo()));
}

//
// Decrease stack depth
//
void CTPP2Compiler::DecrDepth() { --iStackDepth; }

//
// Increase stack depth
//
void CTPP2Compiler::IncrDepth() { ++iStackDepth; }

//
// Get instrcution by instruction number
//
VMInstruction * CTPP2Compiler::GetInstruction(const UINT_32 iIP) { return oVMOpcodeCollector.GetInstruction(iIP); }

//
// A destructor
//
CTPP2Compiler::~CTPP2Compiler() throw()
{
	;;
}

} // namespace CTPP
// End.
