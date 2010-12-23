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
 *      CTPP2Parser.hpp
 *
 * $CTPP$
 */
#ifndef _CTPP2_PARSER_HPP__
#define _CTPP2_PARSER_HPP__ 1

#include "CTPP2CharIterator.hpp"
#include "CTPP2Compiler.hpp"
#include "CDT.hpp"

/**
  @file CTPP2Parser.hpp
  @brief CTPP2 syntax parser
*/

namespace CTPP // C++ Template Engine
{

// FWD
class StaticData;
class StaticText;
class CTPP2SourceLoader;

/**
  @class CTPP2Parser CTPP2Parser.hpp <CTPP2Parser.hpp>
  @brief CTPP2 source code parser
*/
class CTPP2DECL CTPP2Parser
{
public:
	/** Type for include map */
	typedef STLW::map<STLW::string, STLW::string>  IncludeMapType;

	/**
	  @brief Constructor
	  @param pISourceLoader - source loader
	  @param pICTPP2Compiler - compiler object
	  @param sISourceName - template source name
	  @param bIInForeach - foreach flag
	  @param iIRecursionLevel - recursion level
	*/
	CTPP2Parser(CTPP2SourceLoader    * pISourceLoader,
	            CTPP2Compiler        * pICTPP2Compiler,
	            const::STLW::string  & sISourceName,
	            const bool           & bIInForeach      = false,
	            const INT_32           iIRecursionLevel = 0);

	/**
	  @brief Compile CTPP2 template
	  @param iHalt - if set to 1, add to end of code HLT instruction
	  @return 0 if success, -1 if any error occured
	*/
	INT_32 Compile(const UINT_32 & iHalt = 1);

	/**
	  @brief Set parameter names translation map
	  @param oIParamTranslationMap - translation map
	*/
	void SetParamMap(const IncludeMapType & oIParamTranslationMap);

	/**
	  @brief A destructor
	*/
	~CTPP2Parser() throw();
private:
	enum eCTPP2Operator { UNDEF        = 0,
	                      TMPL_var     = 1,
	                      TMPL_if      = 2,
	                      TMPL_unless  = 3,
	                      TMPL_else    = 4,
	                      TMPL_elsif   = 5,
	                      TMPL_loop    = 6,
	                      TMPL_foreach = 7,
	                      TMPL_include = 8,
	                      TMPL_call    = 9,
	                      TMPL_udf     = 10,
	                      TMPL_comment = 11,
	                      TMPL_break   = 12,
	                      TMPL_block   = 13 };

	enum eCTPP2ExprOperator { EXPR_UNDEF        = 0,
	                          EXPR_INT_VALUE    = 1,
	                          EXPR_FLOAT_VALUE  = 2,
	                          EXPR_STRING_VALUE = 3,
	                          EXPR_VARIABLE     = 4,
	                          EXPR_FUNCTION     = 5,
	                          EXPR_BRANCH       = 6 };

	enum eCTPP2Relation { R_UNDEF = 0,
	                      R_EQ    = 1,
	                      R_NE    = 2,
	                      R_GT    = 3,
	                      R_LT    = 4,
	                      R_GE    = 5,
	                      R_LE    = 6,
	                      R_N_EQ  = 7,
	                      R_N_NE  = 8,
	                      R_N_GT  = 9,
	                      R_N_LT  = 10,
	                      R_N_GE  = 11,
	                      R_N_LE  = 12 };

	enum eCTPP2ArOp { OP_UNDEF   = 0,
	                  OP_PLUS    = 1,
	                  OP_MINUS   = 2,
	                  OP_DIV     = 3,
	                  OP_MUL     = 4,
	                  OP_IDIV    = 5,
	                  OP_MOD     = 6,
	                  OP_UNPLUS  = 7,
	                  OP_UNMINUS = 8,
	                  OP_NOT     = 9,
	                  OP_LAND    = 10,
	                  OP_LOR     = 11 };

	/**
	  @struct CTPP2Keyword CTPP2Parser.hpp <CTPP2Parser.hpp>
	  @brief CTPP2 reserverd words
	*/
	struct CTPP2Keyword
	{
		/** keyword                        */
		CCHAR_P          keyword;
		/** keyword length                 */
		UINT_32          keyword_length;
		/** keyword corresponding operator */
		eCTPP2Operator   keyword_operator;
	};
	/** Keywords table */
	static CTPP2Keyword aKeywords[];

	/**
	  @class CTPP2LoopKeyword CTPP2Parser.hpp <CTPP2Parser.hpp>
	  @brief CTPP2 reserverd words for loops
	*/
	struct CTPP2LoopKeyword
	{
		/** keyword                        */
		CCHAR_P          keyword;
		/** keyword length                 */
		UINT_32          keyword_length;
		/** keyword corresponding operator */
		UINT_32          keyword_operator;
	};

	/** Loop modifiers        */
	static CTPP2LoopKeyword aLoopKeywords [];

	/**
	  @struct CTPP2Relation CTPP2Parser.hpp <CTPP2Parser.hpp>
	  @brief CTPP2 Relation
	*/
	struct CTPP2Relation
	{
		/** keyword                        */
		CCHAR_P               keyword;
		/** keyword length                 */
		UINT_32               keyword_length;
		/** keyword corresponding operator */
		eCTPP2Relation        keyword_operator;
	};

	/** Relations */
	static CTPP2Relation aCTPP2Relations [];

	/**
	  @struct CTPP2LoopContextVar CTPP2Parser.hpp <CTPP2Parser.hpp>
	  @brief CTPP2 Loop context variable
	*/
	struct CTPP2LoopContextVar
	{
		/** keyword                        */
		CCHAR_P               keyword;
		/** keyword length                 */
		UINT_32               keyword_length;
		/** keyword corresponding operator */
		eCTPP2LoopContextVar  keyword_operator;
	};

	/** Loop context variable   */
	static CTPP2LoopContextVar aLoopContextVars [];

	/** Current operator        */
	eCTPP2Operator  eBreakFound;

	/** Temp. buffer            */
	STLW::string    sTMPBuf;
	/** Parsed integer value    */
	INT_64         iIntData;
	/** Parsed floating value   */
	W_FLOAT        dFloatData;
	/** Type of parsed value    */
	INT_32         iParsedNumberType;

	/** Source loader object    */
	CTPP2SourceLoader  * pSourceLoader;
	/** Compiler                */
	CTPP2Compiler      * pCTPP2Compiler;
	/** Syscalls                */
	StaticText         & oSyscalls;
	/** Static data segment     */
	StaticData         & oStaticData;
	/** Static text segment     */
	StaticText         & oStaticText;

	/** Translation map         */
	IncludeMapType       oParamTranslationMap;

	/** Template source name    */
	STLW::string         sSourceName;
	/** Template source name ID */
	UINT_32             iSourceNameId;
	/** Foreach flag            */
	bool                bInForeach;
	/** Recursion level         */
	INT_32              iRecursionLevel;

	// Simple tokens: open and close tags, operators, variables, strings and numbers //////////////////////////////////////////////////////////////////////////////////

	/**
	  @brief Open tag ([Tt][Mm][Pp][Ll])
	  @param szData - current stream position
	  @param szEnd - end of stream position
	  @return NULL if sequence not found or pointer to end of sequence
	*/
	CCharIterator IsOpenTag(CCharIterator szData, CCharIterator szEnd);

	/**
	  @brief Close tag (/[Tt][Mm][Pp][Ll])
	  @param szData - current stream position
	  @param szEnd - end of stream position
	  @return NULL if sequence not found or pointer to end of sequence
	*/
	CCharIterator IsCloseTag(CCharIterator szData, CCharIterator szEnd);

	/**
	  @brief Operator (var | if | else | unless | loop | foreach | udf | comment | call | break)
	  @param szData - current stream position
	  @param szEnd - end of stream position
	  @param oOperatorType - operator type
	  @return NULL if sequence not found or pointer to end of sequence
	*/
	CCharIterator IsOperator(CCharIterator szData, CCharIterator szEnd, eCTPP2Operator & oOperatorType);

	/**
	  @brief Function [a-zA-Z_][a-zA-Z0-9_]( Expr [, Expr2 ... ] )
	  @param szData - current stream position
	  @param szEnd - end of stream position
	  @param sFuncEnd - end of function name
	  @param iFunctionParams - number of function parameters
	  @return NULL if sequence not found or pointer to end of sequence
	*/
	CCharIterator IsFunc(CCharIterator szData, CCharIterator szEnd, CCharIterator & sFuncEnd, UINT_32 & iFunctionParams);

	/**
	  @brief CTPP2 function a-zA-Z_[a-zA-Z0-9_]
	  @param szData - current stream position
	  @param szEnd - end of stream position
	  @param szNS - namespace, if present
	  @return NULL if sequence not found or pointer to end of sequence
	*/
	CCharIterator IsFuncName(CCharIterator szData, CCharIterator szEnd);

	/**
	  @brief CTPP2 iterator [a-zA-Z_][a-zA-Z0-9_]
	  @param szData - current stream position
	  @param szEnd - end of stream position
	  @return NULL if sequence not found or pointer to end of sequence
	*/
	CCharIterator IsIterator(CCharIterator szData, CCharIterator szEnd);

	/**
	  @brief CTPP2 variable a-zA-Z_[a-zA-Z0-9_|.|:]
	  @param szData - current stream position
	  @param szEnd - end of stream position
	  @param szNS - namespace, if present
	  @return NULL if sequence not found or pointer to end of sequence
	*/
	CCharIterator IsVar(CCharIterator szData, CCharIterator szEnd, CCharIterator & szNS);

	/**
	  @brief CTPP2 string "blah-blah \" clah-clah " | 'blah-blah \' clah-clah '
	  @param szData - current stream position
	  @param szEnd - end of stream position
	  @return NULL if sequence not found or pointer to end of sequence
	*/
	CCharIterator IsString(CCharIterator szData, CCharIterator szEnd);

	/**
	  @brief CTPP2 old fashion string blah-blah.clah-clah
	  @param szData - current stream position
	  @param szEnd - end of stream position
	  @return NULL if sequence not found or pointer to end of sequenc
	*/
	CCharIterator IsStringCompatOldVersion(CCharIterator szData, CCharIterator szEnd);

	/**
	  @brief Number [-]0-9[. [0-9] ] [E [+-] 0-9]
	  @param szData - current stream position
	  @param szEnd - end of stream position
	  @return NULL if sequence not found or pointer to end of sequence
	*/
	CCharIterator IsNum(CCharIterator szData, CCharIterator szEnd);

	/**
	  @brief Parse Float value
	  @param szData - current stream position
	  @param szEnd - end of stream position
	*/
	void ParseFloat(CCharIterator & szData, CCharIterator & szEnd);

	/**
	  @brief CTPP2 Loop contextual variable (__first__ | __last__ | __inner__ | __odd__ | __even__ | __counter__ | __size__ )
	  @param szData - current stream position
	  @param szEnd - end of stream position
	  @param eContextVariable - contextual variable that was found
	  @return NULL if sequence not found or pointer to end of sequence
	*/
	CCharIterator IsContextVar(CCharIterator szData, CCharIterator szEnd, eCTPP2LoopContextVar & eContextVariable);

	/**
	  @brief Loop operator keyword (__global_vars__ | __context_vars__ | __no_global_vars__ | __no_context_vars__ )
	  @param szData - current stream position
	  @param szEnd - end of stream position
	  @param iLoopKeyword - keyword name
	  @return NULL if sequence not found or pointer to end of sequence
	*/
	CCharIterator IsLoopKeyword(CCharIterator szData, CCharIterator szEnd, UINT_32 & iLoopKeyword);

	/**
	  @brief Foreach operator keyword (as)
	  @param szData - current stream position
	  @param szEnd - end of stream position
	  @return NULL if sequence not found or pointer to end of sequence
	*/
	CCharIterator IsForeachKeyword(CCharIterator szData, CCharIterator szEnd);

	/**
	  @brief White space (Tab | ' ')
	  @param szData - current stream position
	  @param szEnd - end of stream position
	  @param iSkipFirst - if set to 0, does not check existence of first space
	  @return NULL if sequence not found or pointer to end of sequence
	*/
	CCharIterator IsWhiteSpace(CCharIterator szData, CCharIterator szEnd, const UINT_32 & iSkipFirst = 1);

	// Expressions ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/**
	  @brief Expression ( Function ( expression ) | string | variable | number )
	  @param szData - current stream position
	  @param szEnd - end of stream position
	  @param eResultOperator - Result operand of expression
	  @return NULL if sequence not found or pointer to end of sequence
	*/
	CCharIterator IsExpr(CCharIterator szData, CCharIterator szEnd, eCTPP2ExprOperator & eResultOperator);

	/**
	  @brief LogicalOrExpr  = LogicalAndExpr { "||" LogicalAndExpr }
	  @param szData - current stream position
	  @param szEnd - end of stream position
	  @param eResultOperator - expression type
	  @return NULL if sequence not found or pointer to end of sequence
	*/
	CCharIterator LogicalOrExpr(CCharIterator szData, CCharIterator szEnd, eCTPP2ExprOperator & eResultOperator);

	/**
	  @brief LogicalAndExpr = RelExpr { "&&" RelExpr }
	  @param szData - current stream position
	  @param szEnd - end of stream position
	  @param eResultOperator - expression type
	  @return NULL if sequence not found or pointer to end of sequence
	*/
	CCharIterator LogicalAndExpr(CCharIterator szData, CCharIterator szEnd, eCTPP2ExprOperator & eResultOperator);

	/**
	  @brief RelExpr          = SimpleExpr [Relation SimpleExpr]
	  @param szData - current stream position
	  @param szEnd - end of stream position
	  @param eResultOperator - expression type
	  @return NULL if sequence not found or pointer to end of sequence
	*/
	CCharIterator RelExpr(CCharIterator szData, CCharIterator szEnd, eCTPP2ExprOperator & eResultOperator);

	/**
	  @brief SimpleExpr    = [UNPLUS | UNMINUS] Term {AddOp Term}
	  @param szData - current stream position
	  @param szEnd - end of stream position
	  @param eResultOperator - expression type
	  @return NULL if sequence not found or pointer to end of sequence
	*/
	CCharIterator IsSimpleExpr(CCharIterator szData, CCharIterator szEnd, eCTPP2ExprOperator & eResultOperator);

	/**
	  @brief Term          = Factor {MulOp Factor}
	  @param szData - current stream position
	  @param szEnd - end of stream position
	  @param eResultOperator - expression type
	  @return NULL if sequence not found or pointer to end of sequence
	*/
	CCharIterator IsTerm(CCharIterator szData, CCharIterator szEnd, eCTPP2ExprOperator & eResultOperator);

	/**
	  @brief Factor        = Function "(" [ Expr {"," Expr} ] ")" | number | character | string | "(" Expr ")" | NOT Factor
	  @param szData - current stream position
	  @param szEnd - end of stream position
	  @param eResultOperator - expression type
	  @return NULL if sequence not found or pointer to end of sequence
	*/
	CCharIterator IsFactor(CCharIterator szData, CCharIterator szEnd, eCTPP2ExprOperator & eResultOperator);

	/**
	  @brief Relation      = N_EQ | N_NE | N_GT | N_LT | N_GE | N_LE | EQ | NE | GT | LT | GE | LE
	  @param szData - current stream position
	  @param szEnd - end of stream position
	  @param eRelation - relation id [out]
	  @return NULL if sequence not found or pointer to end of sequence
	*/
	CCharIterator IsRelation(CCharIterator szData, CCharIterator szEnd, eCTPP2Relation & eRelation);

	/**
	  @brief UnaryOp         = UNPLUS | UNMINUS
	  @param szData - current stream position
	  @param szEnd - end of stream position
	  @param eArOp - type of arithmetic operation
	  @return NULL if sequence not found or pointer to end of sequence
	*/
	CCharIterator IsUnaryOp(CCharIterator szData, CCharIterator szEnd, eCTPP2ArOp & eArOp);

	/**
	  @brief AddOp         = ADD | SUB
	  @param szData - current stream position
	  @param szEnd - end of stream position
	  @param eArOp - type of arithmetic operation
	  @return NULL if sequence not found or pointer to end of sequence
	*/
	CCharIterator IsAddOp(CCharIterator szData, CCharIterator szEnd, eCTPP2ArOp & eArOp);

	/**
	  @brief LogicalOp         = && | ||
	  @param szData - current stream position
	  @param szEnd - end of stream position
	  @param eArOp - type of arithmetic operation
	  @return NULL if sequence not found or pointer to end of sequence
	*/
	CCharIterator IsLogicalOp(CCharIterator szData, CCharIterator szEnd, eCTPP2ArOp & eArOp);

	/**
	  @brief MulOp         = MUL | DIV | IDIV | MOD
	  @param szData - current stream position
	  @param szEnd - end of stream position
	  @param eArOp - type of arithmetic operation
	  @return NULL if sequence not found or pointer to end of sequence
	*/
	CCharIterator IsMulOp(CCharIterator szData, CCharIterator szEnd, eCTPP2ArOp & eArOp);

	// Operator expressions ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/**
	  @brief CTPP2 branch expression; &lt;TMPL_if IfExpr&gt;
	  @param szData - current stream position
	  @param szEnd - end of stream position
	  @param eResultOperator - Result operand of expression
	  @return NULL if sequence not found or pointer to end of sequence
	*/
	CCharIterator IsIfExpr(CCharIterator szData, CCharIterator szEnd, eCTPP2ExprOperator & eResultOperator);

	/**
	  @brief CTPP2 reverse branch expression; &lt;TMPL_unless UnlessExpr&gt;
	  @param szData - current stream position
	  @param szEnd - end of stream position
	  @param eResultOperator - Result operand of expression
	  @return NULL if sequence not found or pointer to end of sequence
	*/
	CCharIterator IsUnlessExpr(CCharIterator szData, CCharIterator szEnd, eCTPP2ExprOperator & eResultOperator);

	/**
	  @brief CTPP2 loop expression; &lt;TMPL_loop LoopExpr&gt;
	  @param szData - current stream position
	  @param szEnd - end of stream position
	  @param iRetPoint - return point instruction number [out]
	  @return NULL if sequence not found or pointer to end of sequence
	*/
	CCharIterator IsLoopExpr(CCharIterator szData, CCharIterator szEnd, UINT_32 & iRetPoint);

	/**
	  @brief Foreach expression &lt;TMPL_loop ForeachExpr&gt;
	  @param szData - current stream position
	  @param szEnd - end of stream position
	  @param iRetPoint - return point instruction number [out]
	  @return NULL if sequence not found or pointer to end of sequence
	*/
	CCharIterator IsForeachExpr(CCharIterator szData, CCharIterator szEnd, UINT_32 & iRetPoint);

	/**
	  @brief Break expression; &lt;TMPL_break BreakExpr&gt;
	  @param szData - current stream position
	  @param szEnd - end of stream position
	  @param iRetPoint - return point instruction number
	  @return NULL if sequence not found or pointer to end of sequence
	*/
	CCharIterator IsBreakExpr(CCharIterator szData, CCharIterator szEnd, UINT_32 & iRetPoint);

	// Operators //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/**
	  @brief Variable/Function/Exression output operator&lt;TMPL_var Expr&gt;
	  @param szData - current stream position
	  @param szEnd - end of stream position
	  @return NULL if sequence not found or pointer to end of sequence
	*/
	CCharIterator VarOperator(CCharIterator szData, CCharIterator szEnd);

	/**
	  @brief Branch operator &lt;TMPL_if IfExpr&gt; blah-blah-blah [ &lt;TMPL_else&gt; clah-clah-clah ] &lt;/TMPL_if&gt;
	  @param szData - current stream position
	  @param szEnd - end of stream position
	  @return NULL if sequence not found or pointer to end of sequence
	*/
	CCharIterator IfOperator(CCharIterator szData, CCharIterator szEnd);

	/**
	  @brief Reverse branch operator &lt;TMPL_unless IfExpr&gt; blah-blah-blah [ &lt;TMPL_else&gt; clah-clah-clah ] &lt;/TMPL_unless&gt;
	  @param szData - current stream position
	  @param szEnd - end of stream position
	  @return NULL if sequence not found or pointer to end of sequence
	*/
	CCharIterator UnlessOperator(CCharIterator szData, CCharIterator szEnd);

	/**
	  @brief Loop operator &lt;TMPL_loop Expr&gt; blah-blah-blah &lt/TMPL_loop&gt;
	  @param szData - current stream position
	  @param szEnd - end of stream position
	  @return NULL if sequence not found or pointer to end of sequence
	*/
	CCharIterator LoopOperator(CCharIterator szData, CCharIterator szEnd);

	/**
	  @brief Foreach operator &lt;TMPL_foreach var_array as var&gt; blah-blah-blah &lt/TMPL_foreach&gt;
	  @param szData - current stream position
	  @param szEnd - end of stream position
	  @return NULL if sequence not found or pointer to end of sequence
	*/
	CCharIterator ForeachOperator(CCharIterator szData, CCharIterator szEnd);

	/**
	  @brief Include operator &lt;TMPL_include Expr&gt;
	  @param szData - current stream position
	  @param szEnd - end of stream position
	  @return NULL if sequence not found or pointer to end of sequence
	*/
	CCharIterator IncludeOperator(CCharIterator szData, CCharIterator szEnd);

	/**
	  @brief Call operator &lt;TMPL_call Expr&gt;
	  @param szData - current stream position
	  @param szEnd - end of stream position
	  @return NULL if sequence not found or pointer to end of sequence
	*/
	CCharIterator CallOperator(CCharIterator szData, CCharIterator szEnd);

	/**
	  @brief Comment &lt;TMPL_comment&gt; ..... &lt/TMPL_comment&gt;
	  @param szData - current stream position
	  @param szEnd - end of stream position
	  @return NULL if sequence not found or pointer to end of sequence
	*/
	CCharIterator CommentOperator(CCharIterator szData, CCharIterator szEnd);

	/**
	  @brief Block of code &lt;TMPL_block "blockname"&gt; ..... &lt/TMPL_block&gt;
	  @param szData - current stream position
	  @param szEnd - end of stream position
	  @return NULL if sequence not found or pointer to end of sequence
	*/
	CCharIterator BlockOperator(CCharIterator szData, CCharIterator szEnd);

	// Other stuff ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/**
	  @brief Parse CTPP2 template
	  @param szData - template start
	  @param szEnd - template end
	  @return 0 if success, -1 if any error occured
	*/
	CCharIterator Parse(CCharIterator szData, CCharIterator szEnd);

	/**
	  @brief CTPP2 include map: map { a:b, c:d }
	  @param szData - current stream position
	  @param szEnd - end of stream position
	  @param oIncludeTranslationMap - translation map
	  @return NULL if sequence not found or pointer to end of sequence
	*/
	CCharIterator IncludeMap(CCharIterator     szData,
	                         CCharIterator     szEnd,
	                         IncludeMapType  & oIncludeTranslationMap);

	/**
	  @brief Store comparison result (if branch)
	  @param szData - current position in stream
	  @param eResultOperator - type of parsed expression
	  @return instruction pointer to JMP or JXX instrution
	*/
	UINT_32 StoreIfComparisonResult(CCharIterator szData, const eCTPP2ExprOperator & eResultOperator);

	/**
	  @brief Store comparison result (unless branch)
	  @param szData - current position in stream
	  @param eResultOperator - type of parsed expression
	  @return instruction pointer to JMP or JXX instrution
	*/
	UINT_32 StoreUnlessComparisonResult(CCharIterator szData, const eCTPP2ExprOperator & eResultOperator);

	/**
	  @brief Check translation map
	  @param sParam - parameter to check
	*/
	void CheckParamMap(STLW::string & sParam);

	/**
	  @brief Get printable operator name
	  @param oOperator  operator type
	  @return printable operator name
	*/
	CCHAR_P GetOperatorName(const eCTPP2Operator & oOperator);
};

} // namespace CTPP
#endif // _CTPP2_PARSER_H__
// End.
