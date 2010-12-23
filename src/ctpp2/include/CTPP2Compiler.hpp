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
 *      CTPP2Compiler.hpp
 *
 * $CTPP$
 */
#ifndef _CTPP2_COMPILER_HPP__
#define _CTPP2_COMPILER_HPP__ 1

/**
  @file CTPP2Compiler.hpp
  @brief CTPP2 temlate-to-bytecode compiler
*/

#include "CTPP2SymbolTable.hpp"
#include "CTPP2Syntax.h"
#include "CTPP2VMDebugInfo.hpp"
#include "CTPP2VMOpcodeCollector.hpp"

#include "STLMap.hpp"
#include "STLString.hpp"

namespace CTPP // C++ Template Engine
{
// FWD
class HashTable;
class StaticText;
class StaticData;
class CTPP2Parser;

/**
  @class CTPP2Compiler CTPP2Parser.hpp <CTPP2Parser.hpp>
  @brief CTRPP2 compiler
*/
class CTPP2DECL CTPP2Compiler
{
public:
	/**
	  @brief Constructor
	  @param oIVMOpcodeCollector - code segment collector
	  @param oISyscalls - syscalls segment
	  @param oIStaticData - static data segment
	  @param oIStaticText - static text segment
	  @param oIHashTable - hash table
	*/
	CTPP2Compiler(VMOpcodeCollector  & oIVMOpcodeCollector,
	              StaticText         & oISyscalls,
	              StaticData         & oIStaticData,
	              StaticText         & oIStaticText,
	              HashTable          & oIHashTable);

	/**
	  @brief Store template source name
	  @param szName -  name
	  @param iNameLength - name length
	  @return Name Id in static text segment, or -1 if any error occured
	*/
	INT_32 StoreSourceName(CCHAR_P        szName,
	                       const UINT_32  iNameLength);

	/**
	  @brief  Send variable to standard output collector
	  @param oDebugInfo - debug information object
	*/
	INT_32 OutputVariable(const VMDebugInfo & oDebugInfo = VMDebugInfo());

	/**
	  @brief Send text data to standard output collector
	  @param vBuffer - data
	  @param iBufferLength - data size
	  @param oDebugInfo - debug information object
	  @return instruction pointer if success, -1 if any error occured
	*/
	INT_32 OutputStaticData(CCHAR_P              vBuffer,
	                        const UINT_32        iBufferLength,
	                        const VMDebugInfo  & oDebugInfo = VMDebugInfo());

	/**
	  @brief Send integer value to standard output collector
	  @param iData - value to print
	  @param oDebugInfo - debug information object
	  @return instruction pointer if success, -1 if any error occured
	*/
	INT_32 OutputStaticData(const INT_64       & iData,
	                        const VMDebugInfo  & oDebugInfo = VMDebugInfo());

	/**
	  @brief Send floating point value to standard output collector
	  @param sData - value to print
	  @param oDebugInfo - debug information object
	  @return instruction pointer if success, -1 if any error occured
	*/
	INT_32 OutputStaticData(const W_FLOAT      & sData,
	                        const VMDebugInfo  & oDebugInfo = VMDebugInfo());

	/**
	  @brief Change visibility scope of CDT data (for ARRAY iterating)
	  @param szScopeName - ARRAY name
	  @param iScopeNameLength - Length of ARRAY name
	  @param oDebugInfo - debug information object
	  @return instruction pointer if success, -1 if any error occured
	*/
	INT_32 ChangeScope(CCHAR_P              szScopeName,
	                   const UINT_32        iScopeNameLength,
	                   const VMDebugInfo  & oDebugInfo = VMDebugInfo());

	/**
	  @brief Change visibility scope of CDT data (for ARRAY iterating)
	  @param szNS - namespace
	  @param iNSLength - namespace length
	  @param szName - variable name (everything after namespace)
	  @param iNameLength - variable name length
	  @param szFullVariable - full variable name (ns + name)
	  @param iFullVariableLength -  full variable name length
	  @param oDebugInfo - debug information object
	  @return instruction pointer if success, -1 if any error occured
	*/
	INT_32 ChangeForeachScope(CCHAR_P              szNS,
	                          const UINT_32        iNSLength,
	                          CCHAR_P              szName,
	                          const UINT_32        iNameLength,
	                          CCHAR_P              szFullVariable,
	                          const UINT_32        iFullVariableLength,
	                          const VMDebugInfo  & oDebugInfo);

	/**
	  @brief Change visibility scope of CDT data (for ARRAY-of-ARRAYS iterating)
	  @param oDebugInfo - debug information object
	  @return instruction pointer if success, -1 if any error occured
	*/
	INT_32 ChangeContextScope(const VMDebugInfo & oDebugInfo);

	/**
	  @brief Reset scope to previous CDT data (for ARRAY iterating)
	  @param iIP - instruction pointer
	  @param oDebugInfo - debug information object
	  @return instruction pointer if success, -1 if any error occured
	*/
	INT_32 ResetScope(const UINT_32        iIP,
	                  const VMDebugInfo  & oDebugInfo = VMDebugInfo());

	/**
	  @brief Reset scope to previous CDT data (for ARRAY/FOREACH iterating)
	  @param iIP - instruction pointer
	  @param oDebugInfo - debug information object
	  @return instruction pointer if success, -1 if any error occured
	*/
	INT_32 ResetForeachScope(const UINT_32        iIP,
	                         const VMDebugInfo  & oDebugInfo = VMDebugInfo());

	/**
	  @brief Execute system call such as HREF_PARAM, FORM_PARAM, etc
	  @param szSyscallName - syscall name
	  @param iSyscallNameLength - Length of syscall name
	  @param iArgNum - number of arguments
	  @param oDebugInfo - debug information object
	  @return instruction pointer if success, -1 if any error occured
	*/
	INT_32 ExecuteSyscall(CCHAR_P              szSyscallName,
	                      const UINT_32        iSyscallNameLength,
	                      const UINT_32        iArgNum,
	                      const VMDebugInfo  & oDebugInfo = VMDebugInfo());

	/**
	  @brief Push variable into stack
	  @param szVariableName - variable name
	  @param iVariableNameLength - Length of variable name
	  @param oDebugInfo - debug information object
	  @return instruction pointer if success, -1 if any error occured
	*/
	INT_32 PushVariable(CCHAR_P              szVariableName,
	                    const UINT_32        iVariableNameLength,
	                    const VMDebugInfo  & oDebugInfo = VMDebugInfo());

	/**
	  @brief Push contextual variable into stack
	  @param iTokenType - type of token
	  @param oDebugInfo - debug information object
	  @return instruction pointer if success, -1 if any error occured
	*/
	INT_32 PushContextualVariable(const eCTPP2LoopContextVar  & iTokenType,
	                              const VMDebugInfo           & oDebugInfo = VMDebugInfo());

	/**
	  @brief Push integer value into stack
	  @param iVariable - value to push
	  @param oDebugInfo - debug information object
	  @return instruction pointer if success, -1 if any error occured
	*/
	INT_32 PushInt(const INT_64       & iVariable,
	               const VMDebugInfo  & oDebugInfo = VMDebugInfo());

	/**
	  @brief Push Float value into stack
	  @param dVariable - value to push
	  @param oDebugInfo - debug information object
	  @return instruction pointer if success, -1 if any error occured
	*/
	INT_32 PushFloat(const W_FLOAT      & dVariable,
	                 const VMDebugInfo  & oDebugInfo = VMDebugInfo());

	/**
	  @brief Push string variable into stack
	  @param szData - data
	  @param iDataLength - data length
	  @param oDebugInfo - debug information object
	  @return instruction pointer if success, -1 if any error occured
	*/
	INT_32 PushString(CCHAR_P              szData,
	                  const UINT_32        iDataLength,
	                  const VMDebugInfo  & oDebugInfo = VMDebugInfo());

	/**
	  @brief Pop variable from stack
	  @param iVars - number of variables to clear from stack
	  @param oDebugInfo - debug information object
	  @return instruction pointer if success, -1 if any error occured
	*/
	INT_32 PopVariable(const INT_32         iVars = 1,
	                   const VMDebugInfo  & oDebugInfo = VMDebugInfo());

	/**
	  @brief Check existence of stack variable
	  @param oDebugInfo - debug information object
	  @return instruction pointer if success, -1 if any error occured
	*/
	INT_32 ExistStackVariable(const VMDebugInfo & oDebugInfo = VMDebugInfo());

	/**
	  @brief Check existence of AR register variable
	  @param oDebugInfo - debug information object
	  @return instruction pointer if success, -1 if any error occured
	*/
	INT_32 ExistARReg(const VMDebugInfo & oDebugInfo = VMDebugInfo());

	/**
	  @brief Call block by name
	  @param sBlockName - block name
	  @param bIsVariable - is true if block name is variable, not plain text
	  @param oDebugInfo - debug information object
	  @return instruction pointer if success, -1 if any error occured
	*/
	INT_32 CallBlock(const STLW::string   & sBlockName,
	                 const bool           & bIsVariable,
	                 const VMDebugInfo    & oDebugInfo = VMDebugInfo());

	/**
	  @brief Call block by __CONTENT__ variable
	  @param oDebugInfo - debug information object
	  @return instruction pointer if success, -1 if any error occured
	*/
	INT_32 CallContextBlock(const VMDebugInfo & oDebugInfo = VMDebugInfo());

	/**
	  @brief Start of block
	  @param sBlockName - block name
	  @param oDebugInfo - debug information object
	  @return instruction pointer if success, -1 if any error occured
	*/
	INT_32 StartBlock(const STLW::string  & sBlockName,
	                  const VMDebugInfo   & oDebugInfo = VMDebugInfo());

	/**
	  @brief End of block
	  @param oDebugInfo - debug information object
	  @return instruction pointer if success, -1 if any error occured
	*/
	INT_32 EndBlock(const VMDebugInfo & oDebugInfo = VMDebugInfo());

	// ////////////////////////////////////////////////////////////////////////////////

	/**
	  @brief Get system call by id
	  @param szSyscallName - syscall name
	  @param iSyscallNameLength - Length of syscall name
	  @return instruction pointer if success, -1 if any error occured
	*/
	INT_32 GetSyscallId(CCHAR_P         szSyscallName,
	                    const UINT_32   iSyscallNameLength);

	/**
	  @brief Get last instruction number
	*/
	UINT_32 GetCodeSize() const;

	/**
	  @brief Get instruction by instruction number
	  @param iIP - instruction number
	  @return pointer to instruction or NULL if instruction does not exist
	*/
	VMInstruction * GetInstruction(const UINT_32   iIP);

	/**
	  @brief Remove last instruction from code segment
	  @return instruction pointer if success, -1 if any error occured
	*/
	INT_32 RemoveInstruction();

	/**
	  @brief Addition
	  @param oDebugInfo - debug information object
	  @return instruction pointer if success, -1 if any error occured
	*/
	INT_32 OpAdd(const VMDebugInfo & oDebugInfo = VMDebugInfo());

	/**
	  @brief Substraction
	  @param oDebugInfo - debug information object
	  @return instruction pointer if success, -1 if any error occured
	*/
	INT_32 OpSub(const VMDebugInfo & oDebugInfo = VMDebugInfo());

	/**
	  @brief Multiplication
	  @param oDebugInfo - debug information object
	  @return instruction pointer if success, -1 if any error occured
	*/
	INT_32 OpMul(const VMDebugInfo & oDebugInfo = VMDebugInfo());

	/**
	  @brief Dividion
	  @param oDebugInfo - debug information object
	  @return instruction pointer if success, -1 if any error occured
	*/
	INT_32 OpDiv(const VMDebugInfo & oDebugInfo = VMDebugInfo());

	/**
	  @brief Integer dividion
	  @param oDebugInfo - debug information object
	  @return instruction pointer if success, -1 if any error occured
	*/
	INT_32 OpIDiv(const VMDebugInfo & oDebugInfo = VMDebugInfo());

	/**
	  @brief MOD
	  @param oDebugInfo - debug information object
	  @return instruction pointer if success, -1 if any error occured
	*/
	INT_32 OpMod(const VMDebugInfo & oDebugInfo = VMDebugInfo());

	/**
	  @brief Negation
	  @param oDebugInfo - debug information object
	  @return instruction pointer if success, -1 if any error occured
	*/
	INT_32 OpNeg(const VMDebugInfo & oDebugInfo = VMDebugInfo());

	/**
	  @brief Logical Negation
	  @param oDebugInfo - debug information object
	  @return instruction pointer if success, -1 if any error occured
	*/
	INT_32 OpNot(const VMDebugInfo & oDebugInfo = VMDebugInfo());

	/**
	  @brief Comparison
	  @param oDebugInfo - debug information object
	  @return instruction pointer if success, -1 if any error occured
	*/
	INT_32 OpCmp(const VMDebugInfo & oDebugInfo = VMDebugInfo());

	/**
	  @brief String comparison
	  @param oDebugInfo - debug information object
	  @return instruction pointer if success, -1 if any error occured
	*/
	INT_32 OpSCmp(const VMDebugInfo & oDebugInfo = VMDebugInfo());

	/**
	  @brief Store variable in scope
	  @param sNS - namespace
	  @param oDebugInfo - debug information object
	  @return Variable scope Id
	*/
	INT_32 StoreScopedVariable(CCHAR_P              szNS,
	                           const UINT_32        iNSLength,
	                           const VMDebugInfo  & oDebugInfo);

	/**
	  @brief Push variable into stack
	  @param szNS - namespace
	  @param iNSLength - namespace length
	  @param szVariableName - variable name
	  @param iVariableNameLength - variable name length
	  @param oDebugInfo - debug information object
	  @return instruction pointer if success, -1 if any error occured
	*/
	INT_32 PushScopedVariable(CCHAR_P              szNS,
	                          const UINT_32        iNSLength,
	                          CCHAR_P              szName,
	                          const UINT_32        iNameLength,
	                          CCHAR_P              szFullVariable,
	                          const UINT_32        iFullVariableLength,
	                          const VMDebugInfo  & oDebugInfo);

	/**
	  @brief Push variable on stack into stack
	  @param iStackPos - stack position
	  @param oDebugInfo - debug information object
	  @return instruction pointer if success, -1 if any error occured
	*/
	INT_32 PushStackVariable(const INT_32         iStackPos,
	                         const VMDebugInfo  & oDebugInfo = VMDebugInfo());

	/**
	  @brief Unconditional jump
	  @param iIP - new instruction pointer
	  @param oDebugInfo - debug information object
	  @return instruction pointer if success, -1 if any error occured
	*/
	INT_32 UncondJump(const UINT_32        iIP,
	                  const VMDebugInfo  & oDebugInfo = VMDebugInfo());
	/**
	  @brief Jump if NOT Equal
	  @param iIP - new instruction pointer
	  @param oDebugInfo - debug information object
	  @return instruction pointer if success, -1 if any error occured
	*/
	INT_32 NEJump(const UINT_32        iIP,
	              const VMDebugInfo  & oDebugInfo = VMDebugInfo());

	/**
	  @brief Jump if Equal
	  @param iIP - new instruction pointer
	  @param oDebugInfo - debug information object
	  @return instruction pointer if success, -1 if any error occured
	*/
	INT_32 EQJump(const UINT_32        iIP,
	              const VMDebugInfo  & oDebugInfo = VMDebugInfo());

	/**
	  @brief Jump if Greater
	  @param iIP - new instruction pointer
	  @param oDebugInfo - debug information object
	  @return instruction pointer if success, -1 if any error occured
	*/
	INT_32 GTJump(const UINT_32        iIP,
	              const VMDebugInfo  & oDebugInfo = VMDebugInfo());

	/**
	  @brief Jump if Greater Or Equal
	  @param iIP - new instruction pointer
	  @param oDebugInfo - debug information object
	  @return instruction pointer if success, -1 if any error occured
	*/
	INT_32 GEJump(const UINT_32        iIP,
	              const VMDebugInfo  & oDebugInfo = VMDebugInfo());

	/**
	  @brief Jump if Less
	  @param iIP - new instruction pointer
	  @param oDebugInfo - debug information object
	  @return instruction pointer if success, -1 if any error occured
	*/
	INT_32 LTJump(const UINT_32        iIP,
	              const VMDebugInfo  & oDebugInfo = VMDebugInfo());

	/**
	  @brief Jump if Less Or Equal
	  @param iIP - new instruction pointer
	  @param oDebugInfo - debug information object
	  @return instruction pointer if success, -1 if any error occured
	*/
	INT_32 LEJump(const UINT_32        iIP,
	              const VMDebugInfo  & oDebugInfo = VMDebugInfo());

	/**
	  @brief Unconditional jump
	  @param iIP - new instruction pointer
	  @param oDebugInfo - debug information object
	  @return instruction pointer if success, -1 if any error occured
	*/
	INT_32 RUncondJump(const UINT_32        iIP,
	                   const VMDebugInfo  & oDebugInfo = VMDebugInfo());
	/**
	  @brief Jump if NOT Equal
	  @param iIP - new instruction pointer
	  @param oDebugInfo - debug information object
	  @return instruction pointer if success, -1 if any error occured
	*/
	INT_32 RNEJump(const UINT_32        iIP,
	               const VMDebugInfo  & oDebugInfo = VMDebugInfo());

	/**
	  @brief Jump if Equal
	  @param iIP - new instruction pointer
	  @param oDebugInfo - debug information object
	  @return instruction pointer if success, -1 if any error occured
	*/
	INT_32 REQJump(const UINT_32        iIP,
	               const VMDebugInfo  & oDebugInfo = VMDebugInfo());

	/**
	  @brief Jump if Greater
	  @param iIP - new instruction pointer
	  @param oDebugInfo - debug information object
	  @return instruction pointer if success, -1 if any error occured
	*/
	INT_32 RGTJump(const UINT_32        iIP,
	               const VMDebugInfo  & oDebugInfo = VMDebugInfo());

	/**
	  @brief Jump if Greater Or Equal
	  @param iIP - new instruction pointer
	  @param oDebugInfo - debug information object
	  @return instruction pointer if success, -1 if any error occured
	*/
	INT_32 RGEJump(const UINT_32        iIP,
	               const VMDebugInfo  & oDebugInfo = VMDebugInfo());

	/**
	  @brief Jump if Less
	  @param iIP - new instruction pointer
	  @param oDebugInfo - debug information object
	  @return instruction pointer if success, -1 if any error occured
	*/
	INT_32 RLTJump(const UINT_32        iIP,
	               const VMDebugInfo  & oDebugInfo = VMDebugInfo());

	/**
	  @brief Jump if Less Or Equal
	  @param iIP - new instruction pointer
	  @param oDebugInfo - debug information object
	  @return instruction pointer if success, -1 if any error occured
	*/
	INT_32 RLEJump(const UINT_32        iIP,
	               const VMDebugInfo  & oDebugInfo = VMDebugInfo());

	/**
	  @brief Clear stack
	  @param iStackPointer - stack pointer
	  @param oDebugInfo - debug information object
	  @return instruction pointer if success, -1 if any error occured
	*/
	INT_32 ClearStack(const UINT_32        iStackPointer = 0,
	                  const VMDebugInfo  & oDebugInfo = VMDebugInfo());

	/**
	  @brief Stop execution
	  @param oDebugInfo - debug information object
	  @return instruction pointer if success, -1 if any error occured
	*/
	INT_32 Halt(const VMDebugInfo & oDebugInfo = VMDebugInfo());

	/**
	  @brief Decrease stack depth
	*/
	void DecrDepth();

	/**
	  @bref Increase stack depth
	*/
	void IncrDepth();

	/**
	  @brief A destructor
	*/
	~CTPP2Compiler() throw();

private:
	friend class CTPP2Parser;

	struct SymbolTableRec
	{
		/** Relative depth of stack */
		UINT_32         stack_depth;
		/** Scope number            */
		UINT_32         scope_number;

		inline SymbolTableRec(UINT_32  iStackDepth = 0,
		                      UINT_32  iScopeNumber = 0): stack_depth(iStackDepth),
		                                                  scope_number(iScopeNumber)
		{ ;; }
	};

	/** Current stack usage factor */
	UINT_32                            iStackDepth;
	UINT_32                            iScopeNumber;
	/** RRegisters are dirty; need to */
	bool                               bRegsAreDirty;

	/** Symbol table               */
	SymbolTable<SymbolTableRec>        oSymbolTable;
	/** Opcodes collector          */
	VMOpcodeCollector                & oVMOpcodeCollector;
	/** Syscalls                   */
	StaticText                       & oSyscalls;
	/** Static data segment        */
	StaticData                       & oStaticData;
	/** Static text segment        */
	StaticText                       & oStaticText;
	/** Hash table for calls       */
	HashTable                        & oHashTable;

	/** Syscall cache              */
	STLW::map<STLW::string,  UINT_32>  mSyscalls;
	/** Id of stored 0 to compare  */
	UINT_32                            iZeroId;
	/** Id of stored 1 to compare  */
	UINT_32                            iOneId;
};

} // namespace CTPP
#endif // _CTPP2_COMPILER_H__
// End.
