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
 *      CTPP2GlobalDefines.h
 *
 * $CTPP$
 */
#ifndef _CTPP_GLOBAL_DEFINES_H__
#define _CTPP_GLOBAL_DEFINES_H__ 1

#include "CTPP2SysHeaders.h"

/**
  @namespace CTPP
  @brief CTPP - C++ Template Engine. Main namespace of CTPP2 project.
*/

/**
  @file CTPP2GlobalDefines.h
  @brief Global definitions for CTPP2
*/

/*
 * Define this if you want to use STL STLW::exception class as base class of all CTPP2 exceptions
 */
#define _USE_STL_AS_BASE_EXCEPTION 1

/*
 * Define this if you want to build project with extended debug information
 */
/* #define _DEBUG 1 */
#ifdef DEBUG_MODE
    #define _DEBUG 1
#endif // DEBUG_MODE

/*
 * Define this if your compiler does not support namesapce std for STL
 */
#ifdef NO_STL_STD_PREFIX
    #define _NO_STL_STD_NS_PREFIX 1
#endif

/*
 * Define this if you want to use C++ operator new / operator delete instead of  malloc / free functions
 */
#define _OPERATOR_NEW_INSTEAD_OF_MALLOC 1

/*
 * Defines for Microsoft (R) Windows
 */
#ifdef _MSC_VER
    #pragma warning (disable : 4996)   // deprecated: strdup, snprintf
    #pragma warning (disable : 4251)   // class 'Class::Name' needs to have dll-interface to be used by clients of class 'Another::Name'
    #define strcasecmp(x,y)      _stricmp((x),(y))
    #define snprintf(x,y,z,...)  _snprintf((x),(y),(z), __VA_ARGS__ )
#endif

#ifdef CTPP2_DLL
    #ifdef ctpp2_EXPORTS
        #define CTPP2DECL   __declspec(dllexport)
    #else
        #define CTPP2DECL   __declspec(dllimport)
    #endif
#else
    #define CTPP2DECL
#endif

/*
 * Header files
 */
#ifndef WIN32

    #ifndef HAVE_SYS_TIME_H
        #error "Cannot find header file `sys/time.h'"
    #endif

    #ifndef HAVE_SYS_UIO_H
        #error "Cannot find header file `sys/uio.h'"
    #endif

    #ifndef HAVE_SYS_TYPES_H
        #error "Cannot find header file `sys/types.h'"
    #endif

    #ifndef HAVE_UNISTD_H
        #error "Cannot find header file `unistd.h'"
    #endif

#endif

#ifndef HAVE_FCNTL_H
    #error "Cannot find header file `fcntl.h'"
#endif

#ifndef HAVE_MATH_H
    #error "Cannot find header file `math.h'"
#endif

#ifndef HAVE_STDIO_H
    #error "Cannot find header file `stdio.h'"
#endif

#ifndef HAVE_STDLIB_H
    #error "Cannot find header file `stdlib.h'"
#endif

#ifndef HAVE_STRING_H
    #error "Cannot find header file `string.h'"
#endif

#ifndef HAVE_TIME_H
    #error "Cannot find header file `time.h'"
#endif

#ifdef HAVE_SYSEXITS_H
    #include <sysexits.h>
#else
/*
 * Copyright (c) 1987, 1993
 *      The Regents of the University of California.  All rights reserved.
 */
    #define EX_OK           0       /* successful termination                 */

    #define EX__BASE        64      /* base value for error messages          */

    #define EX_USAGE        64      /* command line usage error               */
    #define EX_DATAERR      65      /* data format error                      */
    #define EX_NOINPUT      66      /* cannot open input                      */
    #define EX_NOUSER       67      /* addressee unknown                      */
    #define EX_NOHOST       68      /* host name unknown                      */
    #define EX_UNAVAILABLE  69      /* service unavailable                    */
    #define EX_SOFTWARE     70      /* internal software error                */
    #define EX_OSERR        71      /* system error (e.g., can't fork)        */
    #define EX_OSFILE       72      /* critical OS file missing               */
    #define EX_CANTCREAT    73      /* can't create (user) output file        */
    #define EX_IOERR        74      /* input/output error                     */
    #define EX_TEMPFAIL     75      /* temp failure; user is invited to retry */
    #define EX_PROTOCOL     76      /* remote error in protocol               */
    #define EX_NOPERM       77      /* permission denied                      */
    #define EX_CONFIG       78      /* configuration error                    */

    #define EX__MAX 78              /* maximum listed value                   */

#endif

#endif /* _GLOBAL_DEFINES_H__ */
/* End. */
