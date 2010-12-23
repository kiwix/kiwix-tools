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
 *      CTPP2Parser.cpp
 *
 * $CTPP$
 */

#include "CTPP2Parser.hpp"

#include "CTPP2ParserException.hpp"
#include "CTPP2SourceLoader.hpp"

#include <stdio.h>

// #define _USE_REPORTER 1 // Use it only for hard debugging

#define VM_DEBUG(y) VMDebugInfo((y), iSourceNameId)


/*
<TMPL_var  Factor | "(" LogicalOrExpr ")">

<TMPL_if  Factor | "(" LogicalOrExpr ")">
	...
<TMPL_elsif Factor | "(" LogicalOrExpr ")">
	...
<TMPL_else>
	...
</TMPL_if>

<TMPL_unless  Factor | "(" LogicalOrExpr ")">
	...
<TMPL_elsif Factor | "(" LogicalOrExpr ")">
	...
<TMPL_else>
	...
</TMPL_unless>

<TMPL_loop variable>
	...
</TMPL_loop>

<TMPL_foreach loop_var as iterator>
	...
</TMPL_foreach>

<TMPL_include filename.tmpl [map{ a => b, c => d }] > - old-fashion style
<TMPL_include "filename.tmpl" [map{ a => b, c => d }]> - new-fashion style

<TMPL_break Factor | "(" LogicalOrExpr ")">

LogicalOrExpr  = LogicalAndExpr { "||" LogicalAndExpr }

LogicalAndExpr = RelExpr { "&&" RelExpr }

RelExpr        = SimpleExpr [Relation SimpleExpr]

SimpleExpr     = Term {AddOp Term}

Term           = Factor {MulOp Factor}

Factor         = Function "(" [ExprList] ")" | number | character | string | "(" LogicalOrExpr ")" | [ UNPLUS | UNMINUS | NOT] Factor

Relation       = N_EQ | N_NE | N_GT | N_LT | N_GE | N_LE | EQ | NE | GT | LT | GE | LE

AddOp          = ADD | SUB

MulOp          = MUL | DIV | IDIV | MOD

ExprList       = LogicalOrExpr {"," LogicalOrExpr}
*/

namespace CTPP // C++ Template Engine
{

#ifdef _USE_REPORTER
/**
  @class Reporter
  @brief Print template parsing syntax tree with indents
*/
class Reporter
{
public:
	/**
	  @brief Constructor
	  @param sIData - method name
	  @param szCode - code to print
	*/
	Reporter(const STLW::string & sIData, CCharIterator szCode);

	/**
	  @brief Destructor
	*/
	~Reporter() throw();
private:
	// Does not exist
	Reporter(const Reporter & oRhs);
	Reporter & operator=(const Reporter & oRhs);

	/** Method name   */
	STLW::string sData;
	/** Indent lebvel */
	static int iLevel;
};

//
// Constructor
//
Reporter::Reporter(const STLW::string & sIData, CCharIterator szCode): sData(sIData)
{
	for (INT_32 iI = 0; iI < iLevel; ++iI) { fprintf(stderr, "    "); }
	++iLevel;
	fprintf(stderr, ">> %s: `%s`\n", sData.c_str(), szCode());
}

//
// Destructor
//
Reporter::~Reporter() throw() { --iLevel; fprintf(stderr, "<< %s\n", sData.c_str());  }

// Indent level
int Reporter::iLevel = 0;

// Fast access macros
#define REPORTER(x, y) Reporter oReporter((x), (y));

#else // No reporter

// Fast access macros
#define REPORTER(x, y) { ; }

#endif // Reporter class


/**
  @class ForeachFlagLocker
  @brief Lock flag for foreach iterator
*/
class ForeachFlagLocker
{
public:
	/**
	  @brief Constructor
	  @param bIInForeach - flag
	*/
	ForeachFlagLocker(bool & bIInForeach);

	/**
	  @brief Destructor
	*/
	~ForeachFlagLocker() throw();
private:
	// Does not exist
	ForeachFlagLocker(const ForeachFlagLocker & oRhs);
	ForeachFlagLocker & operator=(const ForeachFlagLocker & oRhs);

	/** Reference to foreach flag */
	bool  & bInForeach;
	/** Old foreach flag state    */
	bool    bOldInForeach;
};

//
// Constructor
//
ForeachFlagLocker::ForeachFlagLocker(bool & bIInForeach): bInForeach(bIInForeach),
                                                          bOldInForeach(bInForeach)
{
	//fprintf(stderr, ">>ForeachFlagLocker: %c -> t\n", (bOldInForeach ? 't':'f'));
	bInForeach = true;
}

//
// Destructor
//
ForeachFlagLocker::~ForeachFlagLocker() throw()
{
	//fprintf(stderr, "<<ForeachFlagLocker: t -> %c\n", (bOldInForeach ? 't':'f'));
	bInForeach = bOldInForeach;
}

//
// CTPP2 reserved words
//
CTPP2Parser::CTPP2Relation CTPP2Parser::aCTPP2Relations [] =
{
	{ TMPL_EQ,   sizeof(TMPL_EQ),    R_EQ    },
	{ TMPL_NE,   sizeof(TMPL_NE),    R_NE    },
	{ TMPL_GT,   sizeof(TMPL_GT),    R_GT    },
	{ TMPL_LT,   sizeof(TMPL_LT),    R_LT    },
	{ TMPL_GE,   sizeof(TMPL_GE),    R_GE    },
	{ TMPL_LR,   sizeof(TMPL_LR),    R_LE    },
	{ TMPL_N_EQ, sizeof(TMPL_N_EQ),  R_N_EQ  },
	{ TMPL_N_NE, sizeof(TMPL_N_NE),  R_N_NE  },
	{ TMPL_N_GE, sizeof(TMPL_N_GE),  R_N_GE  },
	{ TMPL_N_LE, sizeof(TMPL_N_LE),  R_N_LE  },
	{ TMPL_N_GT, sizeof(TMPL_N_GT),  R_N_GT  },
	{ TMPL_N_LT, sizeof(TMPL_N_LT),  R_N_LT  },
	// Terminator
	{ NULL,             0,           R_UNDEF }
};

//
// CTPP2 reserved words
//
CTPP2Parser::CTPP2Keyword CTPP2Parser::aKeywords [] =
{
	{ TMPL_VAR_TOK,     sizeof(TMPL_VAR_TOK),     TMPL_var     },
	{ TMPL_IF_TOK,      sizeof(TMPL_IF_TOK),      TMPL_if      },
	{ TMPL_UNLESS_TOK,  sizeof(TMPL_UNLESS_TOK),  TMPL_unless  },
	{ TMPL_ELSE_TOK,    sizeof(TMPL_ELSE_TOK),    TMPL_else    },
	{ TMPL_ELSIF_TOK,   sizeof(TMPL_ELSIF_TOK),   TMPL_elsif   },
	{ TMPL_LOOP_TOK,    sizeof(TMPL_LOOP_TOK),    TMPL_loop    },
	{ TMPL_FOREACH_TOK, sizeof(TMPL_FOREACH_TOK), TMPL_foreach },
	{ TMPL_INCLUDE_TOK, sizeof(TMPL_INCLUDE_TOK), TMPL_include },
	{ TMPL_CALL_TOK,    sizeof(TMPL_CALL_TOK),    TMPL_call    },
	{ TMPL_UDF_TOK,     sizeof(TMPL_UDF_TOK),     TMPL_udf     },
	{ TMPL_COMMENT_TOK, sizeof(TMPL_COMMENT_TOK), TMPL_comment },
	{ TMPL_BREAK_TOK,   sizeof(TMPL_BREAK_TOK),   TMPL_break   },
	{ TMPL_BLOCK_TOK,   sizeof(TMPL_BLOCK_TOK),   TMPL_block   },
	// Terminator
	{ NULL,             0, UNDEF }
};

//
// Loop modifiers
//
CTPP2Parser::CTPP2LoopKeyword CTPP2Parser::aLoopKeywords [] =
{
	{ TMPL_GLOBAL_VARS_TOK,      sizeof(TMPL_GLOBAL_VARS_TOK),     TMPL_USE_GLOBAL_VARS  },
	{ TMPL_CONTEXT_VARS_TOK,     sizeof(TMPL_CONTEXT_VARS_TOK),    TMPL_USE_CONTEXT_VARS },
	{ TMPL_NO_GLOBAL_VARS_TOK,   sizeof(TMPL_NO_GLOBAL_VARS_TOK),  TMPL_NO_GLOBAL_VARS   },
	{ TMPL_NO_CONTEXT_VARS_TOK,  sizeof(TMPL_NO_CONTEXT_VARS_TOK), TMPL_NO_CONTEXT_VARS  },
	// Terminator
	{ NULL,                      0, 0 }
};

//
// Loop modifiers
//
CTPP2Parser::CTPP2LoopContextVar CTPP2Parser::aLoopContextVars [] =
{
	{ TMPL_LOOP_FIRST_TOK,       sizeof(TMPL_LOOP_FIRST_TOK),     TMPL_LOOP_FIRST    },
	{ TMPL_LOOP_LAST_TOK,        sizeof(TMPL_LOOP_LAST_TOK),      TMPL_LOOP_LAST     },
	{ TMPL_LOOP_OUTER_TOK,       sizeof(TMPL_LOOP_OUTER_TOK),     TMPL_LOOP_OUTER    },
	{ TMPL_LOOP_INNER_TOK,       sizeof(TMPL_LOOP_INNER_TOK),     TMPL_LOOP_INNER    },
	{ TMPL_LOOP_ODD_TOK,         sizeof(TMPL_LOOP_ODD_TOK),       TMPL_LOOP_ODD      },
	{ TMPL_LOOP_COUNTER_TOK,     sizeof(TMPL_LOOP_COUNTER_TOK),   TMPL_LOOP_COUNTER  },
	{ TMPL_LOOP_RCOUNTER_TOK,    sizeof(TMPL_LOOP_RCOUNTER_TOK),  TMPL_LOOP_RCOUNTER },
	{ TMPL_LOOP_EVEN_TOK,        sizeof(TMPL_LOOP_EVEN_TOK),      TMPL_LOOP_EVEN     },
	{ TMPL_LOOP_SIZE_TOK,        sizeof(TMPL_LOOP_SIZE_TOK),      TMPL_LOOP_SIZE     },
	{ TMPL_LOOP_CONTENT_TOK,     sizeof(TMPL_LOOP_CONTENT_TOK),   TMPL_LOOP_CONTENT  },
	// Terminator
	{ NULL,                      0, TMPL_LOOP_UNDEF }
};

//
// Constructor
//
CTPP2Parser::CTPP2Parser(CTPP2SourceLoader   * pISourceLoader,
                         CTPP2Compiler       * pICTPP2Compiler,
                         const::STLW::string  & sISourceName,
                         const bool          & bIInForeach,
                         const INT_32          iIRecursionLevel): pSourceLoader(pISourceLoader),
                                                                  pCTPP2Compiler(pICTPP2Compiler),
                                                                  oSyscalls(pCTPP2Compiler -> oSyscalls),
                                                                  oStaticData(pCTPP2Compiler -> oStaticData),
                                                                  oStaticText(pCTPP2Compiler -> oStaticText),
                                                                  sSourceName(sISourceName),
                                                                  bInForeach(bIInForeach),
                                                                  iRecursionLevel(iIRecursionLevel)
{
	iSourceNameId = pCTPP2Compiler -> StoreSourceName(sSourceName.c_str(), sSourceName.size());
}

//
// Compile CTPP2 template
//
INT_32 CTPP2Parser::Compile(const UINT_32 & iHalt)
{
	UINT_32 iTemplateSize = 0;
	CCharIterator szData = pSourceLoader -> GetTemplate(iTemplateSize);

	// Impossible happened.
	if (szData == NULL) { return -1; }

	CCharIterator szEnd = szData() + iTemplateSize;
	CCharIterator sTMP = Parse(szData, szEnd);

	// Check end of template
	if (sTMP != szEnd)
	{
		if (sTMP != NULL)
		{
			if (eBreakFound != UNDEF) { throw CTPPParserOperatorsMismatch("TMPL prefix", GetOperatorName(eBreakFound), sTMP.GetLine(), sTMP.GetLinePos()); }
		}

		throw CTPPParserSyntaxError("syntax error", sTMP.GetLine(), sTMP.GetLinePos());
	}

	// Check end of template
	if (eBreakFound != UNDEF)
	{
		STLW::string sError("Found unclosed tag </");
		sError.append(GetOperatorName(eBreakFound));
		sError.append(">");
		throw CTPPParserSyntaxError(sError.c_str(), sTMP.GetLine(), sTMP.GetLinePos());
	}

	if (iHalt == 1)
	{
		// Halt program
		pCTPP2Compiler -> Halt();
	}

return 0;
}

//
// Set parameter names translation map
//
void CTPP2Parser::SetParamMap(const CTPP2Parser::IncludeMapType & oIParamTranslationMap) { oParamTranslationMap = oIParamTranslationMap; }

// Simple tokens: open and close tags, operators, variables, strings and numbers //////////////////////////////////////////////////////////////////////////////////

//
// Open tag ([Tt][Mm][Pp][Ll])
//
CCharIterator CTPP2Parser::IsOpenTag(CCharIterator szData, CCharIterator szEnd)
{
	CCharIterator szIter1 = szData;
	CCharIterator szIter2 = CTPP2_OPEN_TAG;

	while (szIter1 != szEnd)
	{
		if ((*szIter1 | 0x20) != *szIter2) { break; }
		++szIter1; ++szIter2;
		if (*szIter2 == '\0')
		{
			if (*szIter1 == '_') { return szIter1; }
			return NULL;
		}
	}

return NULL;
}

//
// Close tag (/[Tt][Mm][Pp][Ll])
//
CCharIterator CTPP2Parser::IsCloseTag(CCharIterator szData, CCharIterator szEnd)
{
	CCharIterator szIter2 = CTPP2_CLOSE_TAG;

	if (szData != szEnd && *szData != '/') { return NULL; }
	++szData;

	while (szData != szEnd)
	{
		if ((*szData | 0x20) != *szIter2) { break; }
		++szData; ++szIter2;
		if (*szIter2 == '\0')
		{
			if (*szData == '_') { return szData; }
			return NULL;
		}
	}
return NULL;
}

//
// Operator (var | if | else | unless | loop | foreach | udf | comment | call | break)
//
CCharIterator CTPP2Parser::IsOperator(CCharIterator szData, CCharIterator szEnd, eCTPP2Operator & oOperatorType)
{
	oOperatorType = UNDEF;

	CTPP2Keyword * oKeyword = aKeywords;
	while (oKeyword -> keyword != NULL)
	{
		CCharIterator szIter1 = szData;
		CCharIterator szIter2 = oKeyword -> keyword;
		while (szIter1 != szEnd)
		{
			if ((*szIter1 | 0x20) != *szIter2) { break; }
			++szIter1; ++szIter2;
			if (*szIter2 == '\0')
			{
				oOperatorType = oKeyword -> keyword_operator;
				return szIter1;
			}
		}
		++oKeyword;
	}

return NULL;
}

//
// Function [a-zA-Z_][a-zA-Z0-9_]( Expr [, Expr2 ... ] )
//
CCharIterator CTPP2Parser::IsFunc(CCharIterator szData, CCharIterator szEnd, CCharIterator & sFuncEnd, UINT_32 & iFunctionParams)
{
	REPORTER("IsFunc", szData());

	eCTPP2ExprOperator eResultOperator = EXPR_UNDEF;
	CCharIterator sTMP = IsFuncName(szData, szEnd);

	if (sTMP != NULL)
	{
		sFuncEnd = sTMP;
		szData   = sTMP;
		// Skip white space
		sTMP = IsWhiteSpace(szData, szEnd, 0);

		// '(' blah, clah, mlah ')'
		// Not a function; perhaps, variable, value, or string constant
		if (sTMP == NULL || *sTMP != '(') { return NULL; }

		// OK
		++sTMP;
		if (sTMP == szEnd) { throw CTPPParserSyntaxError("expected primary-expression", sTMP.GetLine(), sTMP.GetLinePos()); }

		// ... blah, clah, mlah ')'
		if (*sTMP == ')') { return ++sTMP; }

		for(;;)
		{
			szData = sTMP;
			// Expression
			sTMP = LogicalOrExpr(szData, szEnd, eResultOperator);
			// Cannot parse expression or variable name after ',' token
			if (sTMP == NULL) { throw CTPPParserSyntaxError("expected expression after ','", szData.GetLine(), szData.GetLinePos()); }

			++iFunctionParams;
			// Skip white space
			sTMP = IsWhiteSpace(sTMP, szEnd, 0);

			// , clah, mlah ')'
			if      (*sTMP == ',') { ++sTMP; }
			else if (*sTMP == ')') { ++sTMP; break; }
		}
		return sTMP;
	}

return NULL;
}

//
// CTPP2 function a-zA-Z_[a-zA-Z0-9_]
//
CCharIterator CTPP2Parser::IsFuncName(CCharIterator szData, CCharIterator szEnd)
{
	if (szData != szEnd &&
	    !((*szData >= 'a' && *szData <= 'z') ||
	      (*szData >= 'A' && *szData <= 'Z') || *szData == '_')
	    )
	{
		return NULL;
	}
	++szData;

	while (szData != szEnd)
	{
		if (!((*szData >= 'a' && *szData <= 'z') ||
		      (*szData >= 'A' && *szData <= 'Z') ||
		      (*szData >= '0' && *szData <= '9') ||
		       *szData == '_')
		    )
		{
			return szData;
		}
		++szData;
	}
return NULL;
}

//
// CTPP2 iterator [a-zA-Z_][a-zA-Z0-9_]
//
CCharIterator CTPP2Parser::IsIterator(CCharIterator szData, CCharIterator szEnd)
{
	if (szData != szEnd &&
	    !((*szData >= 'a' && *szData <= 'z') ||
	      (*szData >= 'A' && *szData <= 'Z') || *szData == '_')
	    )
	{
		return NULL;
	}
	++szData;

	while (szData != szEnd)
	{
		if (!((*szData >= 'a' && *szData <= 'z') ||
		      (*szData >= 'A' && *szData <= 'Z') ||
		      (*szData >= '0' && *szData <= '9') ||
		       *szData == '_')
		    )
		{
			if (*szData == '.' || *szData == ':')
			{
				throw CTPPParserSyntaxError("iterator name should not contains '.' or ':' symbols", szData.GetLine(), szData.GetLinePos());
			}
			return szData;
		}
		++szData;
	}
return NULL;
}

//
// CTPP2 variable [a-zA-Z_][a-zA-Z0-9_.:]
//
CCharIterator CTPP2Parser::IsVar(CCharIterator szData, CCharIterator szEnd, CCharIterator & szNS)
{
	REPORTER("IsVar", szData());

	bool bIsFirstIteration = true;
	while (szData != szEnd)
	{
		// First symbol should be [a-zA-Z_]
		if (szData != szEnd &&
		    !((*szData >= 'a' && *szData <= 'z') ||
		      (*szData >= 'A' && *szData <= 'Z') || *szData == '_' ||
		      // Allo foo.2bar variables (ugly fix, but let it be)
		      (!bIsFirstIteration && (*szData >= '0' && *szData <= '9')))
		    )
		{
			return NULL;
		}
		++szData;

		// [a-zA-Z0-9_.:]
		while (szData != szEnd)
		{
			if (!((*szData >= 'a' && *szData <= 'z') ||
			      (*szData >= 'A' && *szData <= 'Z') ||
			      (*szData >= '0' && *szData <= '9') ||
			       *szData == '_')
			    )
			{
				break;
			}
			++szData;
		}

		// Check namespace delimiters
		if (szData == szEnd) { return NULL; }

		if      (*szData != '.' && *szData != ':') { return szData; }
		else if (szNS == NULL)                     { szNS = szData; }
		++szData;
		bIsFirstIteration = false;
	}

return NULL;
}

//
// CTPP2 string "blah-blah \" clah-clah " | 'blah-blah \' clah-clah '
//
CCharIterator CTPP2Parser::IsString(CCharIterator szData, CCharIterator szEnd)
{
	REPORTER("IsString", szData());
	sTMPBuf.erase();

	INT_32 bEscaped = 0;
	// End-of-String
	if (szData == szEnd) { return NULL; }
	CHAR_8 chEOS = *szData;
	if (chEOS != '"' && chEOS != '\'') { return NULL; }
	++szData;
	if (szData == szEnd) { throw CTPPParserSyntaxError("missing terminating character", szData.GetLine(), szData.GetLinePos()); }

	while (szData != szEnd)
	{
		// Escaper
		if (*szData == '\\' && bEscaped == 0)
		{
			++szData;
			bEscaped = 1;
		}
		// Other cases
		else
		{
			// End-of-String ?
			if (*szData == chEOS && bEscaped == 0) { return ++szData; }
			if (bEscaped == 1)
			{
				switch (*szData)
				{
					// Octal value
					case '0':
					case '1':
					case '2':
					case '3':
					case '4':
					case '5':
					case '6':
					case '7':
					{
						UINT_32 iTMP = 0;
						for (INT_32 iI = 2; iI >= 0; --iI)
						{
							UCHAR_8 iCh = *szData;

							if (iCh >= '0' && iCh <= '7')
							{
								iTMP = (iTMP << 3);
								iTMP += (iCh - '0');
							}
							else { throw CTPPParserSyntaxError("invalid octal escape sequence", szData.GetLine(), szData.GetLinePos()); }

							++szData;
						}
						if (iTMP > 255) { throw CTPPParserSyntaxError("octal escape sequence out of range", szData.GetLine(), szData.GetLinePos()); }
						sTMPBuf += UCHAR_8(iTMP);
					}
					break;
					// Error
					case '8':
					case '9':
						throw CTPPParserSyntaxError("invalid octal escape sequence", szData.GetLine(), szData.GetLinePos());
						break;
					// Hexadecimal value
					case 'x':
					case 'X':
					{
						++szData;
						UINT_32 iTMP = 0;
						for (INT_32 iI = 1; iI >= 0; --iI)
						{
							if (szData == szEnd) { throw CTPPParserSyntaxError("invalid hexadecimal escape sequence", szData.GetLine(), szData.GetLinePos()); }

							UCHAR_8 iCh = *szData;

							if (iCh >= '0' && iCh <= '9') { iTMP += ((iCh - '0') << (iI * 4)); }
							else
							{
								iCh |= 0x20;
								if (*szData >= 'a' && *szData <= 'f') { iTMP += ((iCh - 'a' + 10) << (iI * 4)); }
								else                                  { throw CTPPParserSyntaxError("invalid hexadecimal escape sequence", szData.GetLine(), szData.GetLinePos()); }
							}

							++szData;
						}
						sTMPBuf += UCHAR_8(iTMP);
					}
					break;

					// man ascii
					case 'a': sTMPBuf += '\a'; ++szData; break;
					case 'b': sTMPBuf += '\b'; ++szData; break;
					case 't': sTMPBuf += '\t'; ++szData; break;
					case 'n': sTMPBuf += '\n'; ++szData; break;
					case 'v': sTMPBuf += '\v'; ++szData; break;
					case 'f': sTMPBuf += '\f'; ++szData; break;
					case 'r': sTMPBuf += '\r'; ++szData; break;
					// Unicode + 4 digits
					case 'u':
					{
						++szData;
						UINT_32 iTMP = 0;
						for (INT_32 iI = 3; iI >= 0; --iI)
						{
							if (szData == szEnd) { throw CTPPParserSyntaxError("invalid unicode escape sequence", szData.GetLine(), szData.GetLinePos()); }

							UCHAR_8 iCh = *szData;

							if (iCh >= '0' && iCh <= '9') { iTMP += ((iCh - '0') << (iI * 4)); }
							else
							{
								iCh |= 0x20;
								if (*szData >= 'a' && *szData <= 'f') { iTMP += ((iCh - 'a' + 10) << (iI * 4)); }
								else                                  { throw CTPPParserSyntaxError("invalid unicode escape sequence", szData.GetLine(), szData.GetLinePos()); }
							}

							++szData;
						}
						UCHAR_8 iCh = ((iTMP & 0x0000FF00) >> 8);
						if (iCh != 0) { sTMPBuf += iCh; }

						iCh = (iTMP & 0x000000FF);
						sTMPBuf += iCh;
					}
					break;
					default:
						sTMPBuf += *szData;
						++szData;
				}
			}
			else
			{
				sTMPBuf += *szData;
				++szData;
			}

			bEscaped = 0;
		}
	}

return NULL;
}

//
// CTPP2 old fashion string blah-blah.clah-clah
//
CCharIterator CTPP2Parser::IsStringCompatOldVersion(CCharIterator szData, CCharIterator szEnd)
{
	sTMPBuf.erase();
	if (szData == szEnd) { return NULL; }
	while (szData != szEnd)
	{
		if (*szData == ' ' || *szData == '\t' || *szData == TMPL_CLOSE_SYMBOL) { return szData; }
		sTMPBuf += *szData;
		szData++;
	}

return NULL;
}

//
// Parse Float value
//
void CTPP2Parser::ParseFloat(CCharIterator & szStart, CCharIterator & szData)
{
	INT_32 iDigitSize = szData() - szStart();
	CHAR_P szDigit = new CHAR_8[iDigitSize + 1];
	memcpy(szDigit, szStart(), iDigitSize);
	szDigit[iDigitSize] = '\0';

	double iFF = 0;
	sscanf(szDigit, "%lg", &iFF);

	dFloatData = iFF;
	iParsedNumberType = 1;

	delete [] szDigit;
}

//
// Number [-]0-9[. [0-9] ] [E [+-] 0-9]
//
CCharIterator CTPP2Parser::IsNum(CCharIterator szData, CCharIterator szEnd)
{
	REPORTER("IsNum", szData());

	CCharIterator szStart = szData;

	// First digit
	if (szData != szEnd && !(*szData >= '0' && *szData <= '9')) { return NULL; }
	++szData;

	// Digits, optional
	while (szData != szEnd)
	{
		if (!(*szData >= '0' && *szData <= '9')) { break; }
		++szData;
	}

	// Integer?
	if (szData == szEnd || (*szData != '.' && *szData != 'e' && *szData != 'E'))
	{
		INT_32 iDigitSize = szData() - szStart();
		CHAR_P szDigit = new CHAR_8[iDigitSize + 1];
		memcpy(szDigit, szStart(), iDigitSize);
		szDigit[iDigitSize] = '\0';

		long long iLL = 0;
		sscanf(szDigit, "%lli", &iLL);

		iIntData = iLL;
		iParsedNumberType = 0;

		delete [] szDigit;
		return szData;
	}

	// Check '.'
	if (*szData == '.') { ++szData; }
	if (szData == szEnd)
	{
		ParseFloat(szStart, szData);
		return szData;
	}

	// Digits, optional
	while (szData != szEnd)
	{
		if (!(*szData >= '0' && *szData <= '9')) { break; }
		++ szData;
	}

	// Exponent, optional
	if (szData == szEnd || (*szData != 'e' && *szData != 'E'))
	{
		ParseFloat(szStart, szData);
		return szData;
	}

	++szData;
	// Exponent sign, optional
	if (szData == szEnd) { throw CTPPParserSyntaxError("expected has no digits", szData.GetLine(), szData.GetLinePos()); }

	bool bSignPresent = false;
	if (*szData == '+' || *szData == '-')
	{
		bSignPresent = true;
		++szData;
	}
	if (szData == szEnd) { throw CTPPParserSyntaxError("expected has no digits", szData.GetLine(), szData.GetLinePos()); }

	// Digits, required
	INT_32 iFoundDigits = 0;
	while (szData != szEnd)
	{
		if (!(*szData >= '0' && *szData <= '9')) { break; }
		iFoundDigits = 1;
		++ szData;
	}

	if (iFoundDigits == 0)
	{
		if (bSignPresent) { throw CTPPParserSyntaxError("exponent has no digits after sign", szData.GetLine(), szData.GetLinePos()); }
		else              { throw CTPPParserSyntaxError("exponent has no digits", szData.GetLine(), szData.GetLinePos());            }
	}

	ParseFloat(szStart, szData);

return szData;
}

//
// CTPP2 Loop contextual variable (__first__ | __last__ | __inner__ | __odd__ | __even__ | __counter__ | __size__ | __content__ )
//
CCharIterator CTPP2Parser::IsContextVar(CCharIterator szData, CCharIterator szEnd, eCTPP2LoopContextVar & eContextVariable)
{
	REPORTER("IsContextVar", szData());

	if (szData == szEnd || *szData != '_') { return NULL; }

	CTPP2LoopContextVar * oKeyword = aLoopContextVars;
	while (oKeyword -> keyword != NULL)
	{
		CCharIterator szIter1 = szData;
		CCharIterator szIter2 = oKeyword -> keyword;
		while (szIter1 != szEnd)
		{
			if (!((*szIter1 | 0x20) == *szIter2 || (*szIter2 == '_' && *szIter1 == *szIter2))) { break; }
			++szIter1; ++szIter2;
			if (*szIter2 == '\0')
			{
				eContextVariable = oKeyword -> keyword_operator;
				return szIter1;
			}
		}
		++oKeyword;
	}

	throw CTPPParserSyntaxError("Invalid contextual variable", szData.GetLine(), szData.GetLinePos());

return NULL;
}

//
// Loop operator keyword (__global_vars__ | __context_vars__ | __no_global_vars__ | __no_context_vars__ )
//
CCharIterator CTPP2Parser::IsLoopKeyword(CCharIterator szData, CCharIterator szEnd, UINT_32 & iLoopKeyword)
{
	if (szData == szEnd || *szData != '_') { return NULL; }

	CTPP2LoopKeyword * oKeyword = aLoopKeywords;
	while (oKeyword -> keyword != NULL)
	{
		CCharIterator szIter1 = szData;
		CCharIterator szIter2 = oKeyword -> keyword;
		while (szIter1 != szEnd)
		{
			if (((*szIter1 | 0x20) != *szIter2) &&
			     (*szIter2 == '_' && *szIter1 != *szIter2)) { break; }
			++szIter1; ++szIter2;
			if (*szIter2 == '\0')
			{
				iLoopKeyword |= oKeyword -> keyword_operator;
				return szIter1;
			}
		}
		++oKeyword;
	}

return NULL;
}

//
// Foreach operator keyword (as)
//
CCharIterator CTPP2Parser::IsForeachKeyword(CCharIterator szData, CCharIterator szEnd)
{
	if (szData == szEnd) { return NULL; }
	if (*szData != 'A' && *szData != 'a') { return NULL; }
	++szData;

	if (szData == szEnd) { return NULL; }
	if (*szData != 'S' && *szData != 's') { return NULL; }

	++szData;

return szData;
}

//
// White space (Tab | ' ')
//
CCharIterator CTPP2Parser::IsWhiteSpace(CCharIterator szData, CCharIterator szEnd, const UINT_32 & iSkipFirst)
{
	if (iSkipFirst == 1)
	{
		if (szData != szEnd && (*szData != ' ' && *szData != '\t')) { return NULL; }
		++szData;
	}

	while (szData != szEnd)
	{
		if (*szData != ' ' && *szData != '\t') { break; }
		++szData;
	}
return szData;
}

// Expressions ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//
// Expression ( Function ( expression ) | string | variable | number )
//
CCharIterator CTPP2Parser::IsExpr(CCharIterator szData, CCharIterator szEnd, eCTPP2ExprOperator & eResultOperator)
{
	REPORTER("IsExpr", szData());

	// Skip white space
	szData = IsWhiteSpace(szData, szEnd, 0);

	if (szData == szEnd) { return NULL; }
	if (*szData == '(')
	{
		++szData;
		// LogicalOrExpr
		CCharIterator sTMP = LogicalOrExpr(szData, szEnd, eResultOperator);

		if (sTMP == NULL) { throw CTPPParserSyntaxError("IsExpr", szData.GetLine(), szData.GetLinePos()); }

		if (*sTMP != ')') { throw CTPPParserSyntaxError("expected ')' token", sTMP.GetLine(), sTMP.GetLinePos()); }

		return ++sTMP;
	}

return IsFactor(szData, szEnd, eResultOperator);
}

//
// LogicalOrExpr  = LogicalAndExpr { "||" LogicalAndExpr }
//
CCharIterator CTPP2Parser::LogicalOrExpr(CCharIterator szData, CCharIterator szEnd, eCTPP2ExprOperator & eResultOperator)
{
//
// if (stmt1 || stmt2 || stmt3 || stmt4 || stmt5) { expr1; } else { expr2; }
//
// if (stmt1) { goto @expr1; }
// if (stmt2) { goto @expr1; }
// if (stmt3) { goto @expr1; }
// if (stmt4) { goto @expr1; }
// if (stmt5) { goto @expr1; }
// expr2; // false
// goto @end:
// @expr1:
// expr1; // true
// @end:
// ....something else...
//

// if (stmt)

// EXIST  STACK[0]
// POP 1
// JE END

//
//   PUSH      0                  //
//   JXX       @END               // 1 | N + 4
//@EXPR1:
//   PUSH      1                  // 2
//@END:
//   XXXXXXX                      // 6
//

	REPORTER("LogicalOrExpr", szData());

	STLW::vector<INT_32> vJMPPoints;

	// Factor
	CCharIterator sTMP = LogicalAndExpr(szData, szEnd, eResultOperator);
	if (sTMP == NULL) { throw CTPPParserSyntaxError("1 !IsTerm!", szData.GetLine(), szData.GetLinePos()); }

	// {IsLogicalOp LogicalAndExpr}
	for (;;)
	{
		szData = sTMP;
		// Skip white space
		szData = IsWhiteSpace(szData, szEnd, 0);

		// IsLogicalOp
		eCTPP2ArOp eArOp = OP_UNDEF;
		sTMP = IsLogicalOp(szData, szEnd, eArOp);
		if (sTMP == NULL || eArOp != OP_LOR)
		{
			// Nothing to do
			if (vJMPPoints.size() == 0) { return szData; }

			// At least one logical operation

			// Store prev. expression code point
			pCTPP2Compiler -> ExistStackVariable(VM_DEBUG(szData));
			pCTPP2Compiler -> PopVariable(1, VM_DEBUG(szData));
			vJMPPoints.push_back(pCTPP2Compiler -> EQJump(UINT_32(-1), VM_DEBUG(szData)));

			// Expr2
			INT_32 iNewIP = pCTPP2Compiler -> PushInt(0, VM_DEBUG(szData));
			pCTPP2Compiler -> UncondJump(iNewIP + 3, VM_DEBUG(szData));
			// Expr1
			iNewIP = pCTPP2Compiler -> PushInt(1, VM_DEBUG(szData));
			pCTPP2Compiler -> DecrDepth();

			INT_32 iLastJMPPos = vJMPPoints.size() - 1;
			// Okay, set JMP points
			for(INT_32 iI = 0; iI <= iLastJMPPos; ++iI)
			{
				VMInstruction * pInstruction = pCTPP2Compiler -> GetInstruction(vJMPPoints[iI]);
				pInstruction -> argument = iNewIP;
			}

			eResultOperator = EXPR_VARIABLE;
			return szData;
		}

		// Store prev. expression code point
		pCTPP2Compiler -> ExistStackVariable(VM_DEBUG(szData));
		pCTPP2Compiler -> PopVariable(1, VM_DEBUG(szData));
		vJMPPoints.push_back(pCTPP2Compiler -> EQJump(UINT_32(-1), VM_DEBUG(szData)));

		// LogicalAndExpr
		szData = sTMP;
		sTMP = LogicalAndExpr(szData, szEnd, eResultOperator);

		// Error?
		if (sTMP == NULL) { throw CTPPParserSyntaxError("Rvalue expected after \"||\"", szData.GetLine(), szData.GetLinePos()); }
	}

// This should *not* happened
return NULL;
}

//
// LogicalAndExpr = RelExpr { "&&" RelExpr }
//
CCharIterator CTPP2Parser::LogicalAndExpr(CCharIterator szData, CCharIterator szEnd, eCTPP2ExprOperator & eResultOperator)
{
//
// if (stmt1 && stmt2 && stmt3 && stmt4 && stmt5) { expr1; } else { expr2; }
//
// if (!stmt1) { goto @expr2; }
// if (!stmt2) { goto @expr2; }
// if (!stmt3) { goto @expr2; }
// if (!stmt4) { goto @expr2; }
// if (!stmt5) { goto @expr2; }
// expr1; // true
// goto @end;
// @expr2:
// expr2; // false
// @end:
// ....something else...
//

// if (stmt)

// EXIST  STACK[0]
// POP 1
// JN END

//
//   PUSH      1                  //
//   JXX       @END               // 1 | N + 4
//@EXPR1:
//   PUSH      0                  // 2
//@END:
//   XXXXXXX                      // 6
//
	REPORTER("LogicalAndExpr", szData());

	STLW::vector<INT_32> vJMPPoints;
	// Factor
	CCharIterator sTMP = RelExpr(szData, szEnd, eResultOperator);
	if (sTMP == NULL) { throw CTPPParserSyntaxError("1 !IsTerm!", szData.GetLine(), szData.GetLinePos()); }

	// {IsLogicalOp RelExpr}
	for (;;)
	{
		szData = sTMP;
		// Skip white space
		szData = IsWhiteSpace(szData, szEnd, 0);

		// IsLogicalOp
		// Initialization is need to avoid warning "warning: 'eArOp' may be used uninitialized in this function" on gcc 44
		eCTPP2ArOp eArOp = OP_UNDEF;

		sTMP = IsLogicalOp(szData, szEnd, eArOp);
		if (sTMP == NULL || eArOp != OP_LAND)
		{
			// Nothing to do
			if (vJMPPoints.size() == 0)
			{
				return szData;
			}

			// At least one logical operation

			// Store prev. expression code point
			pCTPP2Compiler -> ExistStackVariable(VM_DEBUG(szData));
			pCTPP2Compiler -> PopVariable(1, VM_DEBUG(szData));
			vJMPPoints.push_back(pCTPP2Compiler -> NEJump(UINT_32(-1), VM_DEBUG(szData)));

			// Expr1
			INT_32 iNewIP = pCTPP2Compiler -> PushInt(1, VM_DEBUG(szData));
			pCTPP2Compiler -> UncondJump(iNewIP + 3, VM_DEBUG(szData));
			// Expr2
			iNewIP = pCTPP2Compiler -> PushInt(0, VM_DEBUG(szData));
			pCTPP2Compiler -> DecrDepth();

			INT_32 iLastJMPPos = vJMPPoints.size() - 1;
			// Okay, set JMP points
			for(INT_32 iI = 0; iI <= iLastJMPPos; ++iI)
			{
				VMInstruction * pInstruction = pCTPP2Compiler -> GetInstruction(vJMPPoints[iI]);
				pInstruction -> argument = iNewIP;
			}

			eResultOperator = EXPR_VARIABLE;
			return szData;
		}

		// Store prev. expression code point
		pCTPP2Compiler -> ExistStackVariable();
		pCTPP2Compiler -> PopVariable();
		vJMPPoints.push_back(pCTPP2Compiler -> NEJump(UINT_32(-1), VM_DEBUG(szData)));

		// RelExpr
		szData = sTMP;
		sTMP = RelExpr(szData, szEnd, eResultOperator);
		// Error?
		if (sTMP == NULL) { throw CTPPParserSyntaxError("Rvalue expected after \"&&\"", szData.GetLine(), szData.GetLinePos()); }
	}

return NULL;
}

//
// RelExpr          = SimpleExpr [Relation SimpleExpr]
//
CCharIterator CTPP2Parser::RelExpr(CCharIterator szData, CCharIterator szEnd, eCTPP2ExprOperator & eResultOperator)
{
	REPORTER("RelExpr", szData());

	// Skip white space
	szData = IsWhiteSpace(szData, szEnd, 0);

	CCharIterator sTMP = IsSimpleExpr(szData, szEnd, eResultOperator);
	if (sTMP == NULL)
	{
		throw CTPPParserSyntaxError("1 !RelExpr!", szData.GetLine(), szData.GetLinePos());
	}

	// All Okay
	szData = sTMP;

	// Skip white space
	szData = IsWhiteSpace(szData, szEnd, 0);

	eCTPP2Relation eRelation = R_UNDEF;
	sTMP = IsRelation(szData, szEnd, eRelation);
	// Relation not found
	if (sTMP == NULL) { return szData; }

	// All Okay
	szData = sTMP;

	// Skip white space
	szData = IsWhiteSpace(szData, szEnd, 0);

	// Second argument
	sTMP = IsSimpleExpr(szData, szEnd, eResultOperator);

//
//   CMP STACK[0], STACK[1]       //
//   RJXX      @L0                // 0 | N + 3
//   PUSH      0                  // 1
//   RJMP      @L1                // 2 | N + 2
//@L0
//   PUSH      1                  // 3
//@L1:
//   XXXXXXX                      // 4
//
	// Compare variables
	eResultOperator = EXPR_VARIABLE;

	switch(eRelation)
	{
		case R_EQ:
			pCTPP2Compiler -> OpSCmp(VM_DEBUG(szData));
			pCTPP2Compiler -> REQJump(3, VM_DEBUG(szData));
//			fprintf(stderr, "R_EQ\n");
			break;
		case R_NE:
			pCTPP2Compiler -> OpSCmp(VM_DEBUG(szData));
			pCTPP2Compiler -> RNEJump(3, VM_DEBUG(szData));
//			fprintf(stderr, "R_NE\n");
			break;
		case R_GT:
			pCTPP2Compiler -> OpSCmp(VM_DEBUG(szData));
			pCTPP2Compiler -> RGTJump(3, VM_DEBUG(szData));
//			fprintf(stderr, "R_GT\n");
			break;
		case R_LT:
			pCTPP2Compiler -> OpSCmp(VM_DEBUG(szData));
			pCTPP2Compiler -> RLTJump(3, VM_DEBUG(szData));
//			fprintf(stderr, "R_LT\n");
			break;
		case R_GE:
			pCTPP2Compiler -> OpSCmp(VM_DEBUG(szData));
			pCTPP2Compiler -> RGEJump(3, VM_DEBUG(szData));
//			fprintf(stderr, "R_GE\n");
			break;
		case R_LE:
			pCTPP2Compiler -> OpSCmp(VM_DEBUG(szData));
			pCTPP2Compiler -> RLEJump(3, VM_DEBUG(szData));
//			fprintf(stderr, "R_LE\n");
			break;
		case R_N_EQ:
			pCTPP2Compiler -> OpCmp(VM_DEBUG(szData));
			pCTPP2Compiler -> REQJump(3, VM_DEBUG(szData));
//			fprintf(stderr, "R_N_EQ\n");
			break;
		case R_N_NE:
			pCTPP2Compiler -> OpCmp(VM_DEBUG(szData));
			pCTPP2Compiler -> RNEJump(3, VM_DEBUG(szData));
//			fprintf(stderr, "R_N_NE\n");
			break;
		case R_N_GT:
			pCTPP2Compiler -> OpCmp(VM_DEBUG(szData));
			pCTPP2Compiler -> RGTJump(3, VM_DEBUG(szData));
//			fprintf(stderr, "R_N_GT\n");
			break;
		case R_N_LT:
			pCTPP2Compiler -> OpCmp(VM_DEBUG(szData));
			pCTPP2Compiler -> RLTJump(3, VM_DEBUG(szData));
//			fprintf(stderr, "R_N_LT\n");
			break;
		case R_N_GE:
			pCTPP2Compiler -> OpCmp(VM_DEBUG(szData));
			pCTPP2Compiler -> RGEJump(3, VM_DEBUG(szData));
//			fprintf(stderr, "R_N_GE\n");
			break;
		case R_N_LE:
			pCTPP2Compiler -> OpCmp(VM_DEBUG(szData));
			pCTPP2Compiler -> RLEJump(3, VM_DEBUG(szData));
//			fprintf(stderr, "R_N_LE\n");
			break;
		default:
			throw CTPPParserSyntaxError("Undefined comparison expression", szData.GetLine(), szData.GetLinePos());
	}

	pCTPP2Compiler -> PushInt(0, VM_DEBUG(szData));
	pCTPP2Compiler -> RUncondJump(2, VM_DEBUG(szData));
	pCTPP2Compiler -> PushInt(1, VM_DEBUG(szData));

	// Decrease stack depth because really only one variable is PUSHed
	pCTPP2Compiler -> DecrDepth();

	if (sTMP == NULL)
	{
		throw CTPPParserSyntaxError("2 !RelExpr!", szData.GetLine(), szData.GetLinePos());
	}

return sTMP;
}

//
// SimpleExpr    = [UNPLUS | UNMINUS] Term {AddOp Term}
//
CCharIterator CTPP2Parser::IsSimpleExpr(CCharIterator szData, CCharIterator szEnd, eCTPP2ExprOperator & eResultOperator)
{
	REPORTER("IsSimpleExpr", szData());

	// Term
	CCharIterator sTMP = IsTerm(szData, szEnd, eResultOperator);
	if (sTMP == NULL) { throw CTPPParserSyntaxError("2 !IsSimpleExpr!", szData.GetLine(), szData.GetLinePos()); }

	// {AddOp Term}
	for (;;)
	{
		szData = sTMP;
		// Skip white space
		szData = IsWhiteSpace(szData, szEnd, 0);

		// AddOp
		eCTPP2ArOp eArOp = OP_UNDEF;
		sTMP = IsAddOp(szData, szEnd, eArOp);
		if (sTMP == NULL) { return szData; }

		// Factor
		szData = sTMP;
		sTMP = IsTerm(szData, szEnd, eResultOperator);
		if (sTMP == NULL) { throw CTPPParserSyntaxError("3 !IsSimpleExpr!", szData.GetLine(), szData.GetLinePos()); }

		// Operation
		switch(eArOp)
		{
			case OP_PLUS:
				pCTPP2Compiler -> OpAdd(VM_DEBUG(szData));
//				fprintf(stderr, "OP_PLUS\n");
				break;

			case OP_MINUS:
				pCTPP2Compiler -> OpSub(VM_DEBUG(szData));
//				fprintf(stderr, "OP_MINUS\n");
				break;
			default:
				// This should *never* happened
				throw "Ouch!";
		}
	}

return NULL;
}

//
// Term          = Factor {MulOp Factor}
//
CCharIterator CTPP2Parser::IsTerm(CCharIterator szData, CCharIterator szEnd, eCTPP2ExprOperator & eResultOperator)
{
	REPORTER("IsTerm", szData());

	// Factor
	CCharIterator sTMP = IsFactor(szData, szEnd, eResultOperator);
	if (sTMP == NULL) { throw CTPPParserSyntaxError("expected variable, value, function or expression", szData.GetLine(), szData.GetLinePos()); }

	// {MulOp Factor}
	for (;;)
	{
		szData = sTMP;
		// Skip white space
		szData = IsWhiteSpace(szData, szEnd, 0);

		// MulOp
		// Initialization is need to avoid warning "warning: 'eArOp' may be used uninitialized in this function" on gcc 44
		eCTPP2ArOp eArOp = OP_UNDEF;

		sTMP = IsMulOp(szData, szEnd, eArOp);
		if (sTMP == NULL) { return szData; }

		// Factor
		szData = sTMP;
		sTMP = IsFactor(szData, szEnd, eResultOperator);
		// Error?
		if (sTMP == NULL) { throw CTPPParserSyntaxError("Rvalue expected", szData.GetLine(), szData.GetLinePos()); }

		// Variable on stack
		eResultOperator = EXPR_VARIABLE;

		// Operation
		switch(eArOp)
		{
			case OP_MUL:
				pCTPP2Compiler -> OpMul(VM_DEBUG(szData));
//				fprintf(stderr, "OP_MUL\n");
				break;

			case OP_DIV:
				pCTPP2Compiler -> OpDiv(VM_DEBUG(szData));
//				fprintf(stderr, "OP_DIV\n");
				break;

			case OP_IDIV:
				pCTPP2Compiler -> OpIDiv(VM_DEBUG(szData));
//				fprintf(stderr, "OP_IDIV\n");
				break;

			case OP_MOD:
				pCTPP2Compiler -> OpMod(VM_DEBUG(szData));
//				fprintf(stderr, "OP_MOD\n");
				break;
			default:
				// This should *never* happened
				throw "Ouch!";
		}
	}

return NULL;
}

//
// Factor        = Function "(" [ Expr {"," Expr} ] ")" | number | character | string | "(" Expr ")" | NOT Factor
//
CCharIterator CTPP2Parser::IsFactor(CCharIterator szData, CCharIterator szEnd, eCTPP2ExprOperator & eResultOperator)
{
	REPORTER("IsFactor", szData());

	// Skip white space
	szData = IsWhiteSpace(szData, szEnd, 0);

	CCharIterator sFuncEnd = NULL;

	UINT_32 iFunctionParams = 0;
	// Check function
	CCharIterator sTMP = IsFunc(szData, szEnd, sFuncEnd, iFunctionParams);
	if (sTMP != NULL)
	{
		// Push syscall
		pCTPP2Compiler -> ExecuteSyscall(szData(), sFuncEnd() - szData(), iFunctionParams, VM_DEBUG(szData));
		eResultOperator = EXPR_VARIABLE;
		return sTMP;
	}

	eCTPP2LoopContextVar eContextVar;
	// Check context variable
	sTMP = IsContextVar(szData, szEnd, eContextVar);
	if (sTMP != NULL)
	{
		pCTPP2Compiler -> PushContextualVariable(eContextVar, VM_DEBUG(szData));
		eResultOperator = EXPR_VARIABLE;
		return sTMP;
	}

	CCharIterator szNS(NULL);
	sTMP = IsVar(szData, szEnd, szNS);
	if (sTMP != NULL)
	{
		// Check iterator name
		if (bInForeach && szNS != NULL)
		{
			// Push iterator -> variable pair
			pCTPP2Compiler -> PushScopedVariable(szData(),   szNS() - szData(),
			                                     szNS() + 1, sTMP() - szNS() - 1,
			                                     szData(),   sTMP() - szData(),
			                                     VM_DEBUG(szData));
		}
		// Check translation map
		else
		{
			// Push regular variable
			STLW::string sVarName(szData(), sTMP() - szData());
			CheckParamMap(sVarName);
			pCTPP2Compiler -> PushVariable(sVarName.data(), sVarName.size(), VM_DEBUG(szData));
		}

		eResultOperator = EXPR_VARIABLE;
		return sTMP;
	}

	// Check number
	sTMP = IsNum(szData, szEnd);
	if (sTMP != NULL)
	{
		if (iParsedNumberType == 0)
		{
			pCTPP2Compiler -> PushInt(iIntData, VM_DEBUG(szData));
			eResultOperator = EXPR_INT_VALUE;
		}
		else
		{
			pCTPP2Compiler -> PushFloat(dFloatData, VM_DEBUG(szData));
			eResultOperator = EXPR_FLOAT_VALUE;
		}

		return sTMP;
	}

	// Check string
	sTMP = IsString(szData, szEnd);
	if (sTMP != NULL)
	{
		pCTPP2Compiler -> PushString(sTMPBuf.c_str(), sTMPBuf.size(), VM_DEBUG(szData));
		eResultOperator = EXPR_STRING_VALUE;
		return sTMP;
	}

	// Check brackets
	if (*szData == '(')
	{
		sTMP = LogicalOrExpr(++szData, szEnd, eResultOperator);
		if (sTMP == NULL)
		{
			throw CTPPParserSyntaxError("1 !IsFactor!", szData.GetLine(), szData.GetLinePos());
		}

		// Skip white space
		szData = IsWhiteSpace(szData, szEnd, 0);

		if (*sTMP != ')')
		{
			throw CTPPParserSyntaxError("2 !IsFactor!", szData.GetLine(), szData.GetLinePos());
		}
		return ++sTMP;
	}

	eCTPP2ArOp eArOp = OP_UNDEF;
	sTMP = IsUnaryOp(szData, szEnd, eArOp);
	// Optional
	if (sTMP != NULL)
	{
		szData = sTMP;
		// Skip white space
		szData = IsWhiteSpace(szData, szEnd, 0);

		sTMP = IsFactor(szData, szEnd, eResultOperator);
		if (sTMP == NULL) { throw CTPPParserSyntaxError("Expected rvalue after unary operation", szData.GetLine(), szData.GetLinePos()); }

		switch(eArOp)
		{
			case OP_UNPLUS:
				// Do nothing
//				fprintf(stderr, "OP_UNPLUS\n");
				break;

			case OP_UNMINUS:
				pCTPP2Compiler -> OpNeg(VM_DEBUG(szData));
				// Negate value
//				fprintf(stderr, "OP_UNMINUS\n");
				break;

			case OP_NOT:
				pCTPP2Compiler -> OpNot(VM_DEBUG(szData));
				// Logical negation
//				fprintf(stderr, "OP_NOT\n");
				break;
			default:
				throw CTPPParserSyntaxError("Incorrect operator", szData.GetLine(), szData.GetLinePos());
		}

		return sTMP;
	}


return NULL;
}

//
// Relation      = N_EQ | N_NE | N_GT | N_LT | N_GE | N_LE | EQ | NE | GT | LT | GE | LE
//
CCharIterator CTPP2Parser::IsRelation(CCharIterator szData, CCharIterator szEnd, eCTPP2Relation & eRelation)
{
	REPORTER("IsRelation", szData());

	if (szData == szEnd) { return NULL; }

	CTPP2Relation * oRelation = aCTPP2Relations;
	while (oRelation -> keyword != NULL)
	{
		CCharIterator szIter1 = szData;
		CCharIterator szIter2 = oRelation -> keyword;
		while (szIter1 != szEnd)
		{
			if ((*szIter1 | 0x20) != *szIter2)  { break; }
			++szIter1; ++szIter2;
			if (*szIter2 == '\0')
			{
				eRelation = oRelation -> keyword_operator;
				return szIter1;
			}
		}
		++oRelation;
	}

return NULL;
}

//
// UnaryOp         = UNPLUS | UNMINUS | NOT
//
CCharIterator CTPP2Parser::IsUnaryOp(CCharIterator szData, CCharIterator szEnd, eCTPP2ArOp & eArOp)
{
	REPORTER("IsUnaryOp", szData());

	// Skip white space
	szData = IsWhiteSpace(szData, szEnd, 0);

	if (szData == szEnd) { return NULL; }

	if      (*szData == '+') { eArOp = OP_UNPLUS;  }
	else if (*szData == '-') { eArOp = OP_UNMINUS; }
	else if (*szData == '!') { eArOp = OP_NOT;     }
	else
	{
		return NULL;
	}

	++szData;

return szData;
}

//
// AddOp         = ADD | SUB
//
CCharIterator CTPP2Parser::IsAddOp(CCharIterator szData, CCharIterator szEnd, eCTPP2ArOp & eArOp)
{
	REPORTER("IsAddOp", szData());

	// Skip white space
	szData = IsWhiteSpace(szData, szEnd, 0);

	if (szData == szEnd) { return NULL; }

	if      (*szData == '+') { eArOp = OP_PLUS;  }
	else if (*szData == '-') { eArOp = OP_MINUS; }
	else
	{
		return NULL;
	}

	++szData;

return szData;
}

//
// LogicalOp         = && | ||
//
CCharIterator CTPP2Parser::IsLogicalOp(CCharIterator szData, CCharIterator szEnd, eCTPP2ArOp & eArOp)
{
	REPORTER("LogicalOp", szData());

	static CCHAR_P szKeyTable[]    = { "&&",    "||",   "and",   "or",   NULL     };
	static eCTPP2ArOp oArOpTable[] = { OP_LAND, OP_LOR, OP_LAND, OP_LOR, OP_UNDEF };

	if (szData == szEnd) { return NULL; }

	CCHAR_P    * sKey  = szKeyTable;
	eCTPP2ArOp * oArOp = oArOpTable;

	while (*sKey != NULL)
	{
		CCharIterator szIter1 = szData;
		CCharIterator szIter2 = *sKey;
		while (szIter1 != szEnd)
		{
			if ((*szIter1 | 0x20) != *szIter2) { break; }
			++szIter1; ++szIter2;
			if (*szIter2 == '\0')
			{
				eArOp = *oArOp;
				return szIter1;
			}
		}
		++sKey; ++oArOp;
	}
	return NULL;

return szData;
}

//
// MulOp         = MUL | DIV | IDIV | MOD
//
CCharIterator CTPP2Parser::IsMulOp(CCharIterator szData, CCharIterator szEnd, eCTPP2ArOp & eArOp)
{
	REPORTER("IsMulOp", szData());

	static CCHAR_P szKeyTable[]    = { "mod",    "div", NULL     };
	static eCTPP2ArOp oArOpTable[] = { OP_MOD, OP_IDIV, OP_UNDEF };

	if (szData == szEnd) { return NULL; }

	if      (*szData == '*') { eArOp = OP_MUL; }
	else if (*szData == '/') { eArOp = OP_DIV; }
	else
	{
		CCHAR_P    * sKey  = szKeyTable;
		eCTPP2ArOp * oArOp = oArOpTable;

		while (*sKey != NULL)
		{
			CCharIterator szIter1 = szData;
			CCharIterator szIter2 = *sKey;
			while (szIter1 != szEnd)
			{
				if ((*szIter1 | 0x20) != *szIter2) { break; }
				++szIter1; ++szIter2;
				if (*szIter2 == '\0')
				{
					eArOp = *oArOp;
					return szIter1;
				}
			}
			++sKey; ++oArOp;
		}
		return NULL;
	}
	++szData;

return szData;
}

// Operator expressions ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//
// CTPP2 branch expression
//
CCharIterator CTPP2Parser::IsIfExpr(CCharIterator szData, CCharIterator szEnd, eCTPP2ExprOperator & eResultOperator)
{
	CCharIterator sTMP = IsExpr(szData, szEnd, eResultOperator);
	if (sTMP == NULL) { throw CTPPParserSyntaxError("expected '<TMPL_if expression>'", szData.GetLine(), szData.GetLinePos()); }

	// All Okay
	szData = sTMP;

	// Skip white space
	szData = IsWhiteSpace(szData, szEnd, 0);

	// Check close symbol
	if (*szData != TMPL_CLOSE_SYMBOL) { throw CTPPParserSyntaxError("expected '>'", szData.GetLine(), szData.GetLinePos()); }

return szData;
}

//
// CTPP2 reverse branch expression
//
CCharIterator CTPP2Parser::IsUnlessExpr(CCharIterator szData, CCharIterator szEnd, eCTPP2ExprOperator & eResultOperator)
{
	return IsExpr(szData, szEnd, eResultOperator);
}

//
// CTPP2 loop expression
//
CCharIterator CTPP2Parser::IsLoopExpr(CCharIterator szData, CCharIterator szEnd, UINT_32 & iRetPoint)
{
	UINT_32 iLoopKeyword = 0;
	// Find modifiers
	// First loop modifier
	CCharIterator sTMP = IsLoopKeyword(szData, szEnd, iLoopKeyword);
	if (sTMP != NULL)
	{
		szData = sTMP;
		// Skip white space
		sTMP = IsWhiteSpace(szData, szEnd);
		if (sTMP == NULL) { throw CTPPParserSyntaxError("expected at least one space symbol", szData.GetLine(), szData.GetLinePos()); }
		szData = sTMP;
		// Second loop modifier
		sTMP = IsLoopKeyword(szData, szEnd, iLoopKeyword);
		if (sTMP != NULL)
		{
			szData = sTMP;

			sTMP = IsWhiteSpace(szData, szEnd);
			if (sTMP == NULL) { throw CTPPParserSyntaxError("expected at least one space symbol", szData.GetLine(), szData.GetLinePos()); }
			szData = sTMP;
		}
	}

	if ((iLoopKeyword & (TMPL_USE_GLOBAL_VARS | TMPL_NO_GLOBAL_VARS)) == (TMPL_USE_GLOBAL_VARS | TMPL_NO_GLOBAL_VARS))
	{
		throw CTPPParserSyntaxError("cannot combine '" TMPL_GLOBAL_VARS_TOK "' and '" TMPL_NO_GLOBAL_VARS_TOK "' in one loop", szData.GetLine(), szData.GetLinePos());
	}

	if ((iLoopKeyword & (TMPL_USE_CONTEXT_VARS | TMPL_NO_CONTEXT_VARS)) == (TMPL_USE_CONTEXT_VARS | TMPL_NO_CONTEXT_VARS))
	{
		throw CTPPParserSyntaxError("cannot combine '" TMPL_CONTEXT_VARS_TOK "' and '" TMPL_NO_CONTEXT_VARS_TOK "' in one loop", szData.GetLine(), szData.GetLinePos());
	}

	// Check contextual variable
	eCTPP2LoopContextVar eContextVar = TMPL_LOOP_UNDEF;
	sTMP = IsContextVar(szData, szEnd, eContextVar);
	if (sTMP != NULL)
	{
		if (eContextVar != TMPL_LOOP_CONTENT)
		{
			throw CTPPParserSyntaxError("only __CONTENT__ variable is allowed to use in <TMPL_loop>", szData.GetLine(), szData.GetLinePos());
		}
		// Remember RET_POINT
		iRetPoint = pCTPP2Compiler -> ChangeContextScope(VM_DEBUG(szData));
		return sTMP;
	}

	// Find loop name
	CCharIterator szNS(NULL);
	sTMP = IsVar(szData, szEnd, szNS);
	if (sTMP == NULL) { throw CTPPParserSyntaxError("incorrect loop condition", szData.GetLine(), szData.GetLinePos()); }

	// Check translation map, TBD
	// ................................

	if (bInForeach && szNS != NULL)
	{
		iRetPoint = pCTPP2Compiler -> ChangeForeachScope(szData(),   szNS() - szData(),
		                                                 szNS() + 1, sTMP() - szNS() - 1,
		                                                 szData(),   sTMP() - szData(),
		                                                 VM_DEBUG(szData));
	}
	else
	{
		// Check translation map
		STLW::string sVarName(szData(), sTMP() - szData());
		CheckParamMap(sVarName);

		// Remember RET_POINT
		iRetPoint = pCTPP2Compiler -> ChangeForeachScope(NULL,     0,
		                                                 NULL,     0,
		                                                 sVarName.data(), sVarName.size(),
		                                                 VM_DEBUG(szData));
	}

return sTMP;
}

//
// CTPP2 foreach expression
//
CCharIterator CTPP2Parser::IsForeachExpr(CCharIterator szData, CCharIterator szEnd, UINT_32 & iRetPoint)
{
	// Check contextual variable
	eCTPP2LoopContextVar eContextVar = TMPL_LOOP_UNDEF;
	CCharIterator sTMP = IsContextVar(szData, szEnd, eContextVar);
	if (sTMP != NULL)
	{
		if (eContextVar != TMPL_LOOP_CONTENT)
		{
			throw CTPPParserSyntaxError("only __CONTENT__ variable is allowed to use in <TMPL_foreach>", szData.GetLine(), szData.GetLinePos());
		}
		// Remember RET_POINT
		iRetPoint = pCTPP2Compiler -> ChangeContextScope(VM_DEBUG(szData));
		return sTMP;
	}

	// Find loop name
	CCharIterator szLoopNS(NULL);
	sTMP = IsVar(szData, szEnd, szLoopNS);
	if (sTMP == NULL) { throw CTPPParserSyntaxError("incorrect foreach condition", szData.GetLine(), szData.GetLinePos()); }

	CCharIterator szLoopName    = szData;
	CCharIterator szLoopNameEnd = sTMP;

	// Check translation map, TBD
	// ................................

	// Skip white space
	szData = IsWhiteSpace(sTMP, szEnd, 0);

	// Check 'as' keyword
	sTMP = IsForeachKeyword(szData, szEnd);
	if (sTMP == NULL) { throw CTPPParserSyntaxError("incorrect foreach condition. expected `as` keyword", szData.GetLine(), szData.GetLinePos()); }

	// Skip white space
	szData = IsWhiteSpace(sTMP, szEnd, 0);

	// Find iterator name
	sTMP = IsIterator(szData, szEnd);
	if (sTMP == NULL) { throw CTPPParserSyntaxError("incorrect foreach iterator name", szData.GetLine(), szData.GetLinePos()); }

	// Remember RET_POINT
	if (szLoopNS != NULL)
	{
		iRetPoint = pCTPP2Compiler -> ChangeForeachScope(szLoopName(),   szLoopNS()      - szLoopName(),
		                                                 szLoopNS() + 1, szLoopNameEnd() - szLoopNS() - 1,
		                                                 szLoopName(),   szLoopNameEnd() - szLoopName(),
		                                                 VM_DEBUG(szData));
	}
	else
	{
		iRetPoint = pCTPP2Compiler -> ChangeForeachScope(NULL,   0,
		                                                 NULL,   0,
		                                                 szLoopName(), szLoopNameEnd() - szLoopName(),
		                                                 VM_DEBUG(szData));
	}

	// Store new prefix
	pCTPP2Compiler -> StoreScopedVariable(szData(), sTMP() - szData(), VM_DEBUG(szData));

return sTMP;
}

//
// CTPP2 break expression
//
CCharIterator IsBreakExpr(CCharIterator szData, CCharIterator szEnd, UINT_32 & iRetPoint)
{
	/*
	  TBD in version 2.5.0
	*/
return NULL;
}

// Operators //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//
// Variable/Function/Exression output operator <TMPL_var Expr>
//
CCharIterator CTPP2Parser::VarOperator(CCharIterator szData, CCharIterator szEnd)
{
	eCTPP2ExprOperator eResultOperator;
	// Skip white space
	CCharIterator sTMP = IsWhiteSpace(szData, szEnd);
	if (sTMP == NULL) { throw CTPPParserSyntaxError("expected at least one space symbol", szData.GetLine(), szData.GetLinePos()); }
	szData = sTMP;

	sTMP = IsExpr(szData, szEnd, eResultOperator);
	// Cannot parse expression or variable name
	if (sTMP == NULL) { throw CTPPParserSyntaxError("expected variable or expression", szData.GetLine(), szData.GetLinePos()); }
	szData = sTMP;

	// Skip white space
	szData = IsWhiteSpace(szData, szEnd, 0);

	if (*szData != TMPL_CLOSE_SYMBOL) { throw CTPPParserSyntaxError("expected '>'", szData.GetLine(), szData.GetLinePos()); }

	// Output variable
	pCTPP2Compiler -> OutputVariable(VM_DEBUG(szData));

return szData;
}

//
// Branch operator <TMPL_if IfExpr> blah-blah-blah [ <TMPL_else> clah-clah-clah ] </TMPL_if>
//
CCharIterator CTPP2Parser::IfOperator(CCharIterator szData, CCharIterator szEnd)
{
	STLW::vector<INT_32> vJMPPoints;

	eCTPP2ExprOperator eResultOperator = EXPR_UNDEF;
	// Skip white space
	CCharIterator sTMP = IsWhiteSpace(szData, szEnd);
	if (sTMP == NULL) { throw CTPPParserSyntaxError("expected at least one space symbol", szData.GetLine(), szData.GetLinePos()); }
	szData = sTMP;

	// Condition
	szData = IsIfExpr(szData, szEnd, eResultOperator);

	// Syntax ok, store result
	INT_32 iBranchCallerIP = StoreIfComparisonResult(szData, eResultOperator);

	eBreakFound = TMPL_if;

	// Recursive descend
	sTMP = Parse(++szData, szEnd);
	// This should *never* happened
	if (sTMP == NULL) { throw "Ouch!"; }
	szData = sTMP;

	if (eBreakFound != TMPL_else  &&
	    eBreakFound != TMPL_elsif &&
	    eBreakFound != TMPL_if) { throw CTPPParserOperatorsMismatch("<TMPL_else> OR <TMPL_elsif expression> OR </TMPL_if>", GetOperatorName(eBreakFound), szData.GetLine(), szData.GetLinePos()); }

	// Jump to end of branch
	UINT_32 iTMP = pCTPP2Compiler -> UncondJump(UINT_32(-1), VM_DEBUG(szData));
	vJMPPoints.push_back(iTMP);

	while (eBreakFound == TMPL_elsif)
	{
		eResultOperator = EXPR_UNDEF;

		// Skip white space
		CCharIterator sTMP = IsWhiteSpace(szData, szEnd);
		if (sTMP == NULL) { throw CTPPParserSyntaxError("expected at least one space symbol", szData.GetLine(), szData.GetLinePos()); }
		szData = sTMP;

		INT_32 iBranchEntryPoint = pCTPP2Compiler -> GetCodeSize();

		// Condition
		szData = IsIfExpr(szData, szEnd, eResultOperator);

		// Syntax ok, store result
		INT_32 iBranchCodePoint = StoreIfComparisonResult(szData, eResultOperator);

		if (iBranchCallerIP != -1)
		{
			// Set JXX point for previous branch
			VMInstruction * pInstruction = pCTPP2Compiler -> GetInstruction(iBranchCallerIP);
			pInstruction -> argument = iBranchEntryPoint;
		}
		iBranchCallerIP = iBranchCodePoint;

		eBreakFound = TMPL_elsif;

		// Recursive descend
		sTMP = Parse(++szData, szEnd);
		// This should *never* happened
		if (sTMP == NULL) { throw "Ouch!"; }

		if (eBreakFound != TMPL_else  &&
		    eBreakFound != TMPL_elsif &&
		    eBreakFound != TMPL_if) { throw CTPPParserOperatorsMismatch("<TMPL_else> OR <TMPL_elsif expression> OR </TMPL_if>", GetOperatorName(eBreakFound), szData.GetLine(), szData.GetLinePos()); }

		szData = sTMP;

		// Jump to end of branch
		iTMP = pCTPP2Compiler -> UncondJump(UINT_32(-1), VM_DEBUG(szData));
		vJMPPoints.push_back(iTMP);
	}

	// Jump to else-branch if condition is false
	INT_32 iExitPoint = pCTPP2Compiler -> GetCodeSize();
	if (iBranchCallerIP != -1)
	{
		VMInstruction * pInstruction = pCTPP2Compiler -> GetInstruction(iBranchCallerIP);
		pInstruction -> argument = iExitPoint;
	}

	// Else branch
	if (eBreakFound == TMPL_else)
	{
		// Skip white space
		szData = IsWhiteSpace(szData, szEnd, 0);

		// '>'
		if (*szData != TMPL_CLOSE_SYMBOL) { throw CTPPParserSyntaxError("expected '>'", szData.GetLine(), szData.GetLinePos()); }

		sTMP = Parse(++szData, szEnd);
		// This should *never* happened
		if (sTMP == NULL) { throw "Ouch!"; }

		szData = sTMP;
		// Jump to end of branch
		iTMP = pCTPP2Compiler -> UncondJump(UINT_32(-1), VM_DEBUG(szData));
		vJMPPoints.push_back(iTMP);
		iExitPoint = pCTPP2Compiler -> GetCodeSize();
	}

	// /TMPL_if
	if (eBreakFound != TMPL_if)
	{
		if (szData == szEnd) { throw CTPPParserSyntaxError("expected </TMPL_if>", szData.GetLine(), szData.GetLinePos()); }

		throw CTPPParserOperatorsMismatch("</TMPL_if>", GetOperatorName(eBreakFound), szData.GetLine(), szData.GetLinePos());
	}
	eBreakFound = UNDEF;

	INT_32 iLastJMPPos = vJMPPoints.size() - 1;

	// Okay, set JMP points
	for(INT_32 iI = 0; iI <= iLastJMPPos; ++iI)
	{
		VMInstruction * pInstruction = pCTPP2Compiler -> GetInstruction(vJMPPoints[iI]);
		pInstruction -> argument = iExitPoint;
	}

return szData;
}

//
// Reverse branch operator <TMPL_unless IfExpr> blah-blah-blah [ <TMPL_else> clah-clah-clah ] </TMPL_unless>
//
CCharIterator CTPP2Parser::UnlessOperator(CCharIterator szData, CCharIterator szEnd)
{
	STLW::vector<INT_32> vJMPPoints;

	eCTPP2ExprOperator eResultOperator = EXPR_UNDEF;
	// Skip white space
	CCharIterator sTMP = IsWhiteSpace(szData, szEnd);
	if (sTMP == NULL) { throw CTPPParserSyntaxError("expected at least one space symbol", szData.GetLine(), szData.GetLinePos()); }
	szData = sTMP;

	// Condition
	szData = IsIfExpr(szData, szEnd, eResultOperator);

	// Syntax ok, store result
	INT_32 iBranchCallerIP = StoreUnlessComparisonResult(szData, eResultOperator);

	eBreakFound = TMPL_if;

	// Recursive descend
	sTMP = Parse(++szData, szEnd);
	// This should *never* happened
	if (sTMP == NULL) { throw "Ouch!"; }
	szData = sTMP;

	if (eBreakFound != TMPL_else  &&
	    eBreakFound != TMPL_elsif &&
	    eBreakFound != TMPL_unless) { throw CTPPParserOperatorsMismatch("<TMPL_else> OR <TMPL_elsif expression> OR </TMPL_if>", GetOperatorName(eBreakFound), szData.GetLine(), szData.GetLinePos()); }

	// Jump to end of branch
	UINT_32 iTMP = pCTPP2Compiler -> UncondJump(UINT_32(-1), VM_DEBUG(szData));
	vJMPPoints.push_back(iTMP);

	while (eBreakFound == TMPL_elsif)
	{
		eResultOperator = EXPR_UNDEF;

		// Skip white space
		CCharIterator sTMP = IsWhiteSpace(szData, szEnd);
		if (sTMP == NULL) { throw CTPPParserSyntaxError("expected at least one space symbol", szData.GetLine(), szData.GetLinePos()); }
		szData = sTMP;

		INT_32 iBranchEntryPoint = pCTPP2Compiler -> GetCodeSize();

		// Condition
		szData = IsIfExpr(szData, szEnd, eResultOperator);

		// Syntax ok, store result
		INT_32 iBranchCodePoint = StoreIfComparisonResult(szData, eResultOperator);

		// Set JXX point for previous branch
		if (iBranchCallerIP != -1)
		{
			VMInstruction * pInstruction = pCTPP2Compiler -> GetInstruction(iBranchCallerIP);
			pInstruction -> argument = iBranchEntryPoint;
		}

		iBranchCallerIP = iBranchCodePoint;

		eBreakFound = TMPL_elsif;

		// Recursive descend
		sTMP = Parse(++szData, szEnd);
		// This should *never* happened
		if (sTMP == NULL) { throw "Ouch!"; }

		if (eBreakFound != TMPL_else  &&
		    eBreakFound != TMPL_elsif &&
		    eBreakFound != TMPL_unless) { throw CTPPParserOperatorsMismatch("<TMPL_else> OR <TMPL_elsif expression> OR </TMPL_unless>", GetOperatorName(eBreakFound), szData.GetLine(), szData.GetLinePos()); }

		szData = sTMP;

		// Jump to end of branch
		iTMP = pCTPP2Compiler -> UncondJump(UINT_32(-1), VM_DEBUG(szData));
		vJMPPoints.push_back(iTMP);
	}

	// Jump to else-branch if condition is false
	INT_32 iExitPoint = pCTPP2Compiler -> GetCodeSize();
	if (iBranchCallerIP != -1)
	{
		VMInstruction * pInstruction = pCTPP2Compiler -> GetInstruction(iBranchCallerIP);
		pInstruction -> argument = iExitPoint;
	}

	// Else branch
	if (eBreakFound == TMPL_else)
	{
		// Skip white space
		szData = IsWhiteSpace(szData, szEnd, 0);

		// '>'
		if (*szData != TMPL_CLOSE_SYMBOL) { throw CTPPParserSyntaxError("expected '>'", szData.GetLine(), szData.GetLinePos()); }

		sTMP = Parse(++szData, szEnd);
		// This should *never* happened
		if (sTMP == NULL) { throw "Ouch!"; }

		szData = sTMP;
		// Jump to end of branch
		iTMP = pCTPP2Compiler -> UncondJump(UINT_32(-1), VM_DEBUG(szData));
		vJMPPoints.push_back(iTMP);
		iExitPoint = pCTPP2Compiler -> GetCodeSize();
	}

	// /TMPL_unless
	if (eBreakFound != TMPL_unless)
	{
		if (szData == szEnd) { throw CTPPParserSyntaxError("expected </TMPL_if>", szData.GetLine(), szData.GetLinePos()); }

		throw CTPPParserOperatorsMismatch("</TMPL_unless>", GetOperatorName(eBreakFound), szData.GetLine(), szData.GetLinePos());
	}
	eBreakFound = UNDEF;

	INT_32 iLastJMPPos = vJMPPoints.size() - 1;

	// Okay, set JMP points
	for(INT_32 iI = 0; iI <= iLastJMPPos; ++iI)
	{
		VMInstruction * pInstruction = pCTPP2Compiler -> GetInstruction(vJMPPoints[iI]);
		pInstruction -> argument = iExitPoint;
	}

return szData;
}

//
// CTPP2 loop <TMPL_loop Expr> blah-blah-blah </TMPL_loop>
//
CCharIterator CTPP2Parser::LoopOperator(CCharIterator szData, CCharIterator szEnd)
{
	CCharIterator sTMP = IsWhiteSpace(szData, szEnd);
	if (sTMP == NULL) { throw CTPPParserSyntaxError("expected at least one space symbol", szData.GetLine(), szData.GetLinePos()); }
	szData = sTMP;

	UINT_32 iRetPoint = 0;
	// Parse loop expression
	szData = IsLoopExpr(szData, szEnd, iRetPoint);

	// Skip white space
	szData = IsWhiteSpace(szData, szEnd, 0);

	if (*szData != TMPL_CLOSE_SYMBOL) { throw CTPPParserSyntaxError("expected '>'", szData.GetLine(), szData.GetLinePos()); }

	// Recursive descend
	sTMP = Parse(++szData, szEnd);
	// This should *never* happened
	if (sTMP == NULL) { throw "Ouch!"; }

	if (eBreakFound != TMPL_loop ) { throw CTPPParserOperatorsMismatch("</TMPL_loop>", GetOperatorName(eBreakFound), sTMP.GetLine(), sTMP.GetLinePos()); }
	eBreakFound = UNDEF;

	// Store point
	pCTPP2Compiler -> ResetForeachScope(iRetPoint, VM_DEBUG(szData));

return sTMP;
}

//
// Foreach operator <TMPL_foreach var_array as var> blah-blah-blah </TMPL_foreach>
//
CCharIterator CTPP2Parser::ForeachOperator(CCharIterator szData, CCharIterator szEnd)
{
	CCharIterator sTMP = IsWhiteSpace(szData, szEnd);
	if (sTMP == NULL) { throw CTPPParserSyntaxError("expected at least one space symbol", szData.GetLine(), szData.GetLinePos()); }
	szData = sTMP;

	UINT_32 iRetPoint = 0;

	// Parse foreach expression
	szData = IsForeachExpr(szData, szEnd, iRetPoint);

	// Skip white space
	szData = IsWhiteSpace(szData, szEnd, 0);

	if (*szData != TMPL_CLOSE_SYMBOL) { throw CTPPParserSyntaxError("expected '>'", szData.GetLine(), szData.GetLinePos()); }

	// Lock foreach flag
	ForeachFlagLocker oLocker(bInForeach);

	// Recursive descend
	sTMP = Parse(++szData, szEnd);
	// This should *never* happened
	if (sTMP == NULL) { throw "Ouch!"; }

	if (eBreakFound != TMPL_foreach) { throw CTPPParserOperatorsMismatch("</TMPL_foreach>", GetOperatorName(eBreakFound), sTMP.GetLine(), sTMP.GetLinePos()); }
	eBreakFound = UNDEF;

	// Store point
	pCTPP2Compiler -> ResetForeachScope(iRetPoint, VM_DEBUG(szData));

return sTMP;
}

//
// Include operator <TMPL_include Expr>
//
CCharIterator CTPP2Parser::IncludeOperator(CCharIterator szData, CCharIterator szEnd)
{
	IncludeMapType oIncludeTranslationMap;

	CCharIterator sTMP = IsWhiteSpace(szData, szEnd);
	if (sTMP == NULL) { throw CTPPParserSyntaxError("expected at least one space symbol", szData.GetLine(), szData.GetLinePos()); }
	szData = sTMP;

	// Check include file name
	sTMP = IsString(szData, szEnd);
	if (sTMP == NULL)
	{
		// Check old-fashion syntax
		sTMP = IsStringCompatOldVersion(szData, szEnd);

		if (sTMP == NULL) { throw CTPPParserSyntaxError("incorrect include file name", szData.GetLine(), szData.GetLinePos()); }
	}

	// Skip white space
	sTMP = IsWhiteSpace(sTMP, szEnd, 0);

	szData = sTMP;
	sTMP = IncludeMap(szData, szEnd, oIncludeTranslationMap);
	if (sTMP == NULL) { sTMP = szData; }
	else
	{
		// Skip white space
		sTMP = IsWhiteSpace(sTMP, szEnd, 0);
	}

	// '>'
	if (*sTMP != TMPL_CLOSE_SYMBOL) { throw CTPPParserSyntaxError("expected '>'", sTMP.GetLine(), sTMP.GetLinePos()); }

	// Max, recursion level reached?
	if (iRecursionLevel == CTPP_MAX_TEMPLATE_RECURSION_DEPTH - 1)
	{
		throw CTPPParserSyntaxError("Max. recursion level of template reached", sTMP.GetLine(), sTMP.GetLinePos());
	}

	// All okay, need to parse include file
	CTPP2SourceLoader * pTMPSourceLoader = NULL;
	try
	{
		// Create loader
		pTMPSourceLoader = pSourceLoader -> Clone();
		// Load template
		pTMPSourceLoader -> LoadTemplate(sTMPBuf.c_str());
		// Create parser
		CTPP2Parser oTMPParser(pTMPSourceLoader, pCTPP2Compiler, sTMPBuf, bInForeach, iRecursionLevel + 1);
		// Set translation map
		oTMPParser.SetParamMap(oIncludeTranslationMap);
		// No HLT at end of code
		oTMPParser.Compile(0);
	}
	catch(CTPPLogicError & e)
	{
		delete pTMPSourceLoader;

		CHAR_8 szTMPBuffer[2048 + 1];
		snprintf(szTMPBuffer, 2048, "Cannot load template file `%s`: %s", sTMPBuf.c_str(), e.what());

		throw CTPPParserSyntaxError(szTMPBuffer, sTMP.GetLine(), sTMP.GetLinePos());
	}
	catch(CTPPUnixException & e)
	{
		delete pTMPSourceLoader;

		CHAR_8 szTMPBuffer[2048 + 1];
		snprintf(szTMPBuffer, 2048, "Cannot load template file `%s`: %s", sTMPBuf.c_str(), strerror(e.ErrNo()));

		throw CTPPParserSyntaxError(szTMPBuffer, sTMP.GetLine(), sTMP.GetLinePos());
	}
	catch(CTPPParserOperatorsMismatch & e)
	{
		delete pTMPSourceLoader;

		CHAR_8 szTMPBuffer[2048 + 1];
		snprintf(szTMPBuffer, 2048, "In include file '%s' at line %d, pos %d: expected \"%s\", but found \"%s\"", sTMPBuf.c_str(), e.GetLine(), e.GetLinePos(), e.Expected(), e.Found());

		throw CTPPParserSyntaxError(szTMPBuffer, sTMP.GetLine(), sTMP.GetLinePos());
	}
	catch(CTPPParserException & e)
	{
		delete pTMPSourceLoader;

		CHAR_8 szTMPBuffer[2048 + 1];
		snprintf(szTMPBuffer, 2048, "In include file '%s' at line %d, pos %d: %s", sTMPBuf.c_str(), e.GetLine(), e.GetLinePos(), e.what());

		throw CTPPParserSyntaxError(szTMPBuffer, sTMP.GetLine(), sTMP.GetLinePos());
	}

	delete pTMPSourceLoader;

return sTMP;
}

//
// Call operator <TMPL_call Expr>
//
CCharIterator CTPP2Parser::CallOperator(CCharIterator szData, CCharIterator szEnd)
{
	CCharIterator sTMP = IsWhiteSpace(szData, szEnd);
	if (sTMP == NULL) { throw CTPPParserSyntaxError("expected at least one space symbol", szData.GetLine(), szData.GetLinePos()); }
	szData = sTMP;

	bool bIsVariable        = false;
	bool bIsContextVariable = false;
	// Check block name
	sTMP = IsString(szData, szEnd);
	if (sTMP == NULL)
	{
		// Check contextual variable
		eCTPP2LoopContextVar eContextVar = TMPL_LOOP_UNDEF;
		sTMP = IsContextVar(szData, szEnd, eContextVar);
		if (sTMP != NULL)
		{
			if (eContextVar != TMPL_LOOP_CONTENT)
			{
				throw CTPPParserSyntaxError("only __CONTENT__ variable is allowed to use in <TMPL_call>", szData.GetLine(), szData.GetLinePos());
			}
			bIsContextVariable = true;
		}
		else
		{
			// Search variable
			CCharIterator szNS(NULL);
			sTMP = IsVar(szData, szEnd, szNS);
			if (sTMP == NULL) { throw CTPPParserSyntaxError("incorrect block name", szData.GetLine(), szData.GetLinePos()); }

			sTMPBuf.assign(szData(), sTMP() - szData());
			bIsVariable = true;
		}
	}

	// Skip white space
	sTMP = IsWhiteSpace(sTMP, szEnd, 0);

	// '>'
	if (*sTMP != TMPL_CLOSE_SYMBOL) { throw CTPPParserSyntaxError("expected '>'", sTMP.GetLine(), sTMP.GetLinePos()); }

	// Store instruction
	if (bIsContextVariable) { pCTPP2Compiler -> CallContextBlock(VM_DEBUG(szData));                }
	else                    { pCTPP2Compiler -> CallBlock(sTMPBuf, bIsVariable, VM_DEBUG(szData)); }

return sTMP;
}

//
// CTPP2 comment <TMPL_comment> blah-blah-blah </TMPL_comment>
//
CCharIterator CTPP2Parser::CommentOperator(CCharIterator szData, CCharIterator szEnd)
{
	// Skip white space
	szData = IsWhiteSpace(szData, szEnd, 0);

	// '>'
	if (*szData != TMPL_CLOSE_SYMBOL) { throw CTPPParserSyntaxError("expected '>'", szData.GetLine(), szData.GetLinePos()); }

	++szData;

	// Skip code
	for (;;)
	{
		// Open symbol
		while (szData != szEnd && *szData != TMPL_OPEN_SYMBOL) { ++szData; }
		// Unexpected end of template
		if (szData == szEnd) { throw CTPPParserSyntaxError("expected '</TMPL_comment>'", szData.GetLine(), szData.GetLinePos()); }

		CCharIterator szTMP = IsCloseTag(++szData, szEnd);
		if (szTMP == NULL) { ++szData; }
		else
		{
			eCTPP2Operator oOperatorType = UNDEF;
			szData = szTMP;
			szTMP = IsOperator(++szData, szEnd, oOperatorType);
			//
			if (szTMP == szEnd) { throw CTPPParserSyntaxError("unexpected end of file", szData.GetLine(), szData.GetLinePos()); }
			// Operator NOT found after opener tag
			if (szTMP != NULL)
			{
				if (oOperatorType == TMPL_comment)
				{
					szTMP = IsWhiteSpace(szTMP, szEnd, 0);
					if (*szTMP != TMPL_CLOSE_SYMBOL) { throw CTPPParserSyntaxError("expected '>'", szTMP.GetLine(), szTMP.GetLinePos()); }
					return szTMP;
				}
				szData = szTMP;
			}

		}
	}
return NULL;
}

//
// Block of code <TMPL_block "blockname"> ..... </TMPL_block>
//
CCharIterator CTPP2Parser::BlockOperator(CCharIterator szData, CCharIterator szEnd)
{
	CCharIterator sTMP = IsWhiteSpace(szData, szEnd);
	if (sTMP == NULL) { throw CTPPParserSyntaxError("expected at least one space symbol", szData.GetLine(), szData.GetLinePos()); }
	szData = sTMP;

	// Check block name
	sTMP = IsString(szData, szEnd);
	if (sTMP == NULL)
	{
		sTMP = IsStringCompatOldVersion(szData, szEnd);
		if (sTMP == NULL) { throw CTPPParserSyntaxError("incorrect block name", szData.GetLine(), szData.GetLinePos()); }
	}

	// Skip white space
	sTMP = IsWhiteSpace(sTMP, szEnd, 0);
	// '>'
	if (*sTMP != TMPL_CLOSE_SYMBOL) { throw CTPPParserSyntaxError("expected '>'", sTMP.GetLine(), sTMP.GetLinePos()); }

	// Start block
	UINT_32 iStartBlockIP = pCTPP2Compiler -> StartBlock(sTMPBuf, sTMP);

	// Recursive descend
	sTMP = Parse(++sTMP, szEnd);
	// This should *never* happened
	if (sTMP == NULL) { throw "Ouch!"; }

	if (eBreakFound != TMPL_block ) { throw CTPPParserOperatorsMismatch("</TMPL_block>", GetOperatorName(eBreakFound), sTMP.GetLine(), sTMP.GetLinePos()); }
	eBreakFound = UNDEF;

	pCTPP2Compiler -> EndBlock(VM_DEBUG(sTMP));

	VMInstruction * pInstruction = pCTPP2Compiler -> GetInstruction(iStartBlockIP);
	pInstruction -> argument = pCTPP2Compiler -> GetCodeSize();

return sTMP;
}

// Other stuff ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//
// Parse CTPP2 source code
//
CCharIterator CTPP2Parser::Parse(CCharIterator szData, CCharIterator szEnd)
{
	eBreakFound = UNDEF;

	CCharIterator szIter = szData;
	CCharIterator szText = szData;

	for (;;)
	{
		// Open tag
		while (szIter != szEnd)
		{
			if (*szIter == TMPL_OPEN_SYMBOL) { break; }
			++szIter;
		}
		if (szIter == szEnd)
		{
			// Store static data
			pCTPP2Compiler -> OutputStaticData(szText(), szEnd() - szText(), VM_DEBUG(szText));
			return szIter;
		}
		++szIter;

		// Check open tag
		CCharIterator sTMP = IsOpenTag(szIter, szEnd);
		if (sTMP != NULL)
		{
			// Store static data
			pCTPP2Compiler -> OutputStaticData(szText(), szIter() - szText() - 1, VM_DEBUG(szText));

			eCTPP2Operator oOperatorType = UNDEF;
			szIter = sTMP;
			// Check operator
			sTMP = IsOperator(++szIter, szEnd, oOperatorType);

			// Operator NOT found after opener tag
			if (sTMP == NULL)  { throw CTPPParserSyntaxError("incorrect operator", szIter.GetLine(), szIter.GetLinePos()); }

			if (sTMP == szEnd) { throw CTPPParserSyntaxError("unexpected end of file", szIter.GetLine(), szIter.GetLinePos()); }
			// Operator found
			szIter = sTMP;

			switch (oOperatorType)
			{
				// Print variable or execute function
				// User defined function
				case TMPL_udf:
				// Variable
				case TMPL_var:
					sTMP = VarOperator(szIter, szEnd);
					break;

				// Branch
				case TMPL_if:
					sTMP = IfOperator(szIter, szEnd);
					break;

				// Next branch
				case TMPL_elsif:
					eBreakFound = TMPL_elsif;
					return sTMP;
					break;

				// Branch with inverted condition
				case TMPL_unless:
					sTMP = UnlessOperator(szIter, szEnd);
					break;

				// Else-branch
				case TMPL_else:
					eBreakFound = TMPL_else;
					return szIter;

				// Loop
				case TMPL_loop:
					eBreakFound = TMPL_loop;
					sTMP = LoopOperator(szIter, szEnd);
					break;

				// Foreach
				case TMPL_foreach:
					eBreakFound = TMPL_foreach;
					sTMP = ForeachOperator(szIter, szEnd);
					break;

				// Include file
				case TMPL_include:
					sTMP = IncludeOperator(szIter, szEnd);
					break;

				// Call operator
				case TMPL_call:
					sTMP = CallOperator(szIter, szEnd);
					break;

				// Comment
				case TMPL_comment:
					sTMP = CommentOperator(szIter, szEnd);
					break;

				// Break
				case TMPL_break:
					throw CTPPParserSyntaxError("Support of TMPL_break operator requires version 2.6.0 or higher", szIter.GetLine(), szIter.GetLinePos());
					break;

				// Block
				case TMPL_block:
					eBreakFound = TMPL_block;
					sTMP = BlockOperator(szIter, szEnd);
					break;

				// Error!
				default:
					throw CTPPParserSyntaxError("impossible happened", szIter.GetLine(), szIter.GetLinePos());
					break;
			}
			szIter = sTMP;
			// End of stream
			if (szIter == szEnd) { return szIter; }

			++szIter;
			szText = szIter;
		}
		else
		{
			sTMP = IsCloseTag(szIter, szEnd);
			if (sTMP != NULL)
			{
				// Store static data
				pCTPP2Compiler -> OutputStaticData(szText(), szIter() - szText() - 1, VM_DEBUG(szText));

				eCTPP2Operator oOperatorType = UNDEF;

				szIter = sTMP;
				// Check operator
				sTMP = IsOperator(++szIter, szEnd, oOperatorType);

				switch (oOperatorType)
				{
					// Error!
					case UNDEF:
						throw CTPPParserSyntaxError("impossible happened", szIter.GetLine(), szIter.GetLinePos());

					case TMPL_udf:
						throw CTPPParserSyntaxError("operator '</TMPL_udf>' does not exist", szIter.GetLine(), szIter.GetLinePos());

					case TMPL_var:
						throw CTPPParserSyntaxError("operator '</TMPL_var>' does not exist", szIter.GetLine(), szIter.GetLinePos());

					case TMPL_else:
						throw CTPPParserSyntaxError("operator '</TMPL_else>' does not exist", szIter.GetLine(), szIter.GetLinePos());

					case TMPL_elsif:
						throw CTPPParserSyntaxError("operator '</TMPL_elsif>' does not exist", szIter.GetLine(), szIter.GetLinePos());

					case TMPL_include:
						throw CTPPParserSyntaxError("operator '</TMPL_include>' does not exist", szIter.GetLine(), szIter.GetLinePos());

					case TMPL_break:
						throw CTPPParserSyntaxError("operator '</TMPL_break>' does not exist", szIter.GetLine(), szIter.GetLinePos());

					case TMPL_comment:
						throw CTPPParserSyntaxError("operator '</TMPL_comment>' cannot use here", szIter.GetLine(), szIter.GetLinePos());
					case TMPL_call:
						throw CTPPParserSyntaxError("operator '</TMPL_call>' does not exist", szIter.GetLine(), szIter.GetLinePos());

					// Branch
					case TMPL_if:
						sTMP =  IsWhiteSpace(sTMP, szEnd, 0);
						if (*sTMP != TMPL_CLOSE_SYMBOL) { throw CTPPParserSyntaxError("expected '>'", sTMP.GetLine(), sTMP.GetLinePos()); }
						eBreakFound = TMPL_if;
						return sTMP;

					// Branch with inverted condition
					case TMPL_unless:
						sTMP =  IsWhiteSpace(sTMP, szEnd, 0);
						if (*sTMP != TMPL_CLOSE_SYMBOL) { throw CTPPParserSyntaxError("expected '>'", sTMP.GetLine(), sTMP.GetLinePos()); }
						eBreakFound = TMPL_unless;
						return sTMP;

					// Loop
					case TMPL_loop:
						sTMP =  IsWhiteSpace(sTMP, szEnd, 0);
						if (*sTMP != TMPL_CLOSE_SYMBOL) { throw CTPPParserSyntaxError("expected '>'", sTMP.GetLine(), sTMP.GetLinePos()); }
						eBreakFound = TMPL_loop;
						return sTMP;

					// Foreach
					case TMPL_foreach:
						sTMP =  IsWhiteSpace(sTMP, szEnd, 0);
						if (*sTMP != TMPL_CLOSE_SYMBOL) { throw CTPPParserSyntaxError("expected '>'", sTMP.GetLine(), sTMP.GetLinePos()); }
						eBreakFound = TMPL_foreach;
						return sTMP;

					// Block
					case TMPL_block:
						sTMP =  IsWhiteSpace(sTMP, szEnd, 0);
						if (*sTMP != TMPL_CLOSE_SYMBOL) { throw CTPPParserSyntaxError("expected '>'", sTMP.GetLine(), sTMP.GetLinePos()); }
						eBreakFound = TMPL_block;
						return sTMP;

					default:
						throw "UndefOperator";
				}
				++ szIter;
				szText = szIter;
			}
			else
			{
				if (szIter == szEnd)
				{
					// Store static data
					pCTPP2Compiler -> OutputStaticData(szText(), szEnd() - szText(), VM_DEBUG(szText));
					break;
				}
			}
		}
	}

// This should not happened
return NULL;
}

//
// CTPP2 include map: map { a = b, c = d } OR map { b : a, d : c }
//
CCharIterator CTPP2Parser::IncludeMap(CCharIterator szData, CCharIterator szEnd, CTPP2Parser::IncludeMapType & oIncludeTranslationMap)
{
	static CCHAR_P szName = "map";
	CCharIterator szIter2 = szName;
	while (szData != szEnd)
	{
		if ((*szData | 0x20) != *szIter2) { return NULL; }
		++szData; ++szIter2;
		if (*szIter2 == '\0')
		{
			break;
		}
	}

	// Skip white space
	szData = IsWhiteSpace(szData, szEnd, 0);
	// Check open tag
	if (szData == szEnd || *szData != '(') { throw CTPPParserSyntaxError("need '(' token after \"map\"", szData.GetLine(), szData.GetLinePos()); }
	++szData;

	INT_32 iRenamingOrder = 0;
	STLW::string sKey;
	for (;;)
	{
		// Skip white space
		szData = IsWhiteSpace(szData, szEnd, 0);
		// Search variable
		CCharIterator szNS(NULL);
		CCharIterator sTMP = IsVar(szData, szEnd, szNS);
		if (sTMP == NULL) { break; }

		sKey.assign(szData(), sTMP() - szData());

		szData = sTMP;
		// Skip white space
		szData = IsWhiteSpace(szData, szEnd, 0);

		// Check delimiter
		if (szData == szEnd) { throw CTPPParserSyntaxError("Unexpected end of stream (need ':' OR '=' token)", szData.GetLine(), szData.GetLinePos()); }

		if      (*szData == '=') { iRenamingOrder = 1; }
		else if (*szData == ':') { iRenamingOrder = 2; }
		else    { throw CTPPParserSyntaxError("need '=' OR ':' token after variable name", szData.GetLine(), szData.GetLinePos()); }

		++szData;
		// Skip white space
		szData = IsWhiteSpace(szData, szEnd, 0);
		// Search variable
		sTMP = IsVar(szData, szEnd, szNS);
		if (sTMP == NULL) { throw CTPPParserSyntaxError("need variable name", szData.GetLine(), szData.GetLinePos()); }

		if (iRenamingOrder == 1) { oIncludeTranslationMap[sKey] = STLW::string(szData(), sTMP() - szData()); }
		else                     { oIncludeTranslationMap[STLW::string(szData(), sTMP() - szData())] = sKey;  }

		szData = sTMP;
		// Check delimiter
		if (szData == szEnd || *szData != ',') { break; }
		++szData;
	}

	// Check close tag
	if (szData == szEnd || *szData != ')') { throw CTPPParserSyntaxError("need ')' token after translation map", szData.GetLine(), szData.GetLinePos()); }

return ++szData;
}

//
// Store comparison result (if branch)
//
UINT_32 CTPP2Parser::StoreIfComparisonResult(CCharIterator szData, const eCTPP2ExprOperator & eResultOperator)
{
	UINT_32 iElseBranchCallerIP = (UINT_32)-1;

	// Compare values
	switch (eResultOperator)
	{
		// Simple case: comparison result is always true or always false
		// 0 ?
		case EXPR_INT_VALUE:
			fprintf(stderr, "WARNING: near line %d, pos. %d: comparison result of INTEGER VALUE is always ", szData.GetLine(),  szData.GetLinePos());
			pCTPP2Compiler -> RemoveInstruction();

			if (iIntData != 0) { fprintf(stderr, "true\n"); }
			else
			{
				// Store point to unconditional jump (to ELSE branch)
				iElseBranchCallerIP = pCTPP2Compiler -> UncondJump(UINT_32(-1), VM_DEBUG(szData));
				fprintf(stderr, "false\n");
			}

			break;

		// 0.0 ?
		case EXPR_FLOAT_VALUE:
			fprintf(stderr, "WARNING: near line %d, pos. %d: comparison result of FLOAT VALUE is always ", szData.GetLine(),  szData.GetLinePos());
			pCTPP2Compiler -> RemoveInstruction();

			if (dFloatData != 0.0) { fprintf(stderr, "true\n");  }
			else
			{
				// Store point to unconditional jump (to ELSE branch)
				iElseBranchCallerIP = pCTPP2Compiler -> UncondJump(UINT_32(-1), VM_DEBUG(szData));
				fprintf(stderr, "true\n");
			}
			break;

		// Empty string?
		case EXPR_STRING_VALUE:
			fprintf(stderr, "WARNING: near line %d, pos. %d: comparison result of STRING VALUE is always ", szData.GetLine(),  szData.GetLinePos());
			pCTPP2Compiler -> RemoveInstruction();

			if (sTMPBuf.size() != 0) { fprintf(stderr, "true\n");  }
			else
			{
				// Store point to unconditional jump (to ELSE branch)
				iElseBranchCallerIP = pCTPP2Compiler -> UncondJump(UINT_32(-1), VM_DEBUG(szData));
				fprintf(stderr, "true\n");
			}
			break;

		// Complex case: compare variable
		case EXPR_VARIABLE:
			pCTPP2Compiler -> ExistStackVariable(VM_DEBUG(szData));
			pCTPP2Compiler -> PopVariable(1, VM_DEBUG(szData));
			iElseBranchCallerIP = pCTPP2Compiler -> NEJump(UINT_32(-1), VM_DEBUG(szData));
			break;

		default:
			// This should *never* happened
			throw "Ouch!";
	}

return iElseBranchCallerIP;
}

//
// Store comparison result (unless branch)
//
UINT_32 CTPP2Parser::StoreUnlessComparisonResult(CCharIterator szData, const eCTPP2ExprOperator & eResultOperator)
{
	UINT_32 iElseBranchCallerIP = (UINT_32)-1;

	// Compare values
	switch (eResultOperator)
	{
		// Simple case: comparison result is always true or always false
		// 0 ?
		case EXPR_INT_VALUE:
			fprintf(stderr, "WARNING: near line %d, pos. %d: comparison result of INTEGER VALUE is always ", szData.GetLine(),  szData.GetLinePos());
			pCTPP2Compiler -> RemoveInstruction();

			if (iIntData == 0) { fprintf(stderr, "true\n"); }
			else
			{
				// Store point to unconditional jump (to ELSE branch)
				iElseBranchCallerIP = pCTPP2Compiler -> UncondJump(UINT_32(-1), VM_DEBUG(szData));
				fprintf(stderr, "false\n");
			}
			break;

		// 0.0 ?
		case EXPR_FLOAT_VALUE:
			fprintf(stderr, "WARNING: near line %d, pos. %d: comparison result of FLOAT VALUE is always ", szData.GetLine(),  szData.GetLinePos());
			pCTPP2Compiler -> RemoveInstruction();

			if (dFloatData == 0.0) { fprintf(stderr, "true\n");  }
			else
			{
				// Store point to unconditional jump (to ELSE branch)
				iElseBranchCallerIP = pCTPP2Compiler -> UncondJump(UINT_32(-1), VM_DEBUG(szData));
				fprintf(stderr, "true\n");
			}
			break;

		// Empty string?
		case EXPR_STRING_VALUE:
			fprintf(stderr, "WARNING: near line %d, pos. %d: comparison result of STRING VALUE is always ", szData.GetLine(),  szData.GetLinePos());
			pCTPP2Compiler -> RemoveInstruction();

			if (sTMPBuf.size() == 0) { fprintf(stderr, "true\n");  }
			else
			{
				// Store point to unconditional jump (to ELSE branch)
				iElseBranchCallerIP = pCTPP2Compiler -> UncondJump(UINT_32(-1), VM_DEBUG(szData));
				fprintf(stderr, "true\n");
			}
			break;

		// Complex case: compare variable
		case EXPR_VARIABLE:
			pCTPP2Compiler -> ExistStackVariable(VM_DEBUG(szData));
			pCTPP2Compiler -> PopVariable(1, VM_DEBUG(szData));
			iElseBranchCallerIP = pCTPP2Compiler -> EQJump(UINT_32(-1), VM_DEBUG(szData));
			break;

		default:
			// This should *never* happened
			throw "Ouch!";
	}

return iElseBranchCallerIP;
}

//
// Check translation map
//
void CTPP2Parser::CheckParamMap(STLW::string & sParam)
{
	IncludeMapType::const_iterator itMap = oParamTranslationMap.find(sParam);
	if (itMap != oParamTranslationMap.end()) { sParam = itMap -> second; }
}

//
// Get printable operator name
//
CCHAR_P CTPP2Parser::GetOperatorName(const eCTPP2Operator & oOperator)
{
	switch(oOperator)
	{
		case TMPL_var:     return "TMPL_var";
		case TMPL_if:      return "TMPL_if";
		case TMPL_unless:  return "TMPL_unless";
		case TMPL_else:    return "TMPL_else";
		case TMPL_elsif:   return "TMPL_elsif";
		case TMPL_loop:    return "TMPL_loop";
		case TMPL_foreach: return "TMPL_foreach";
		case TMPL_include: return "TMPL_include";
		case TMPL_udf:     return "TMPL_udf";
		case TMPL_comment: return "TMPL_comment";
		case TMPL_break:   return "TMPL_break";
		case TMPL_block:   return "TMPL_block";
		case TMPL_call:    return "TMPL_call";
		default:
			 return "*UNDEF*";
	}
// Make stupid compiler happy
return NULL;
}

//
// A destructor
//
CTPP2Parser::~CTPP2Parser() throw()
{
	;;
}

} // namespace CTPP
// End.
