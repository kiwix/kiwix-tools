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
 *      CTPP2SysTypes.h
 *
 * $CTPP$
 */
#ifndef _CTPP2_SYS_TYPES_H__
#define _CTPP2_SYS_TYPES_H__ 1

/**
  @file CTPP2SysTypes.h
  @brief Base platform-dependent type definitions
*/

// C Includes
#include <sys/types.h>
#include <stdint.h>

#if defined(__linux__) || defined(linux) || defined(CYGWIN) || defined(__CYGWIN__)

/**
  @var typedef __int16_t    INT_16
  @brief Signed 16-bit integer (linux)
*/
typedef __int16_t                   INT_16;

/**
  @var typedef __uint16_t   UINT_16
  @brief Unsigned 16-bit integer (linux)
*/
typedef __uint16_t                  UINT_16;

/**
  @var typedef __int32_t    INT_32
  @brief Signed 32-bit integer (linux)
*/
typedef __int32_t                   INT_32;

/**
  @var typedef __uint32_t   UINT_32
  @brief Unsigned 32-bit integer (linux)
*/
typedef __uint32_t                  UINT_32;

/**
  @var typedef __int64_t    INT_64
  @brief Signed 64-bit integer (linux)
*/
typedef __int64_t                   INT_64;

/**
  @var typedef __uint64_t   UINT_64
  @brief Unsigned 64-bit integer (linux)
*/
typedef __uint64_t                  UINT_64;

/**
  @var typedef double              W_FLOAT
  @brief Floafing point variable (linux)
*/
typedef double                      W_FLOAT;

/**
  @var typedef char                CHAR_8
  @brief Signed 8-bit character    (linux)
*/
typedef char                        CHAR_8;

/**
  @var unsigned char       UCHAR_8
  @brief Unsigned 8-bit character (linux)
*/
typedef unsigned char               UCHAR_8;

/**
  @var typedef const char  CCHAR_8
  @brief Constant character (linux)
*/
typedef const char                  CCHAR_8;

/**
  @var typedef const unsigned char UCCHAR_8
  @brief Unsigned constant character (linux)
*/
typedef const unsigned char         UCCHAR_8;

/**
  @var typedef const unsigned char UCCHAR_8
  @brief Address (linux)
*/
typedef CHAR_8                    * CHAR_P;

/**
  @var typedef const unsigned char UCHAR_P
  @brief Address of unsigned char (linux)
*/
typedef UCHAR_8                   * UCHAR_P;

/**
  @var typedef const unsigned char CCHAR_P
  @brief Address, pointer to const (linux)
*/
typedef CCHAR_8                   * CCHAR_P;

/**
  @var typedef const unsigned char UCCHAR_P
  @brief Address, pointer to unsigned const (linux)
*/
typedef UCCHAR_8                  * UCCHAR_P;


#elif defined(__FreeBSD__) || defined(__APPLE__) /* Linux End, start of FreeBSD / Mac OS declarations */

/**
  @var typedef int16_t    INT_16
  @brief Signed 16-bit integer (FreeBSD OS)
*/
typedef int16_t                     INT_16;

/**
  @var typedef uint16_t   UINT_16
  @brief Unsigned 16-bit integer (FreeBSD OS)
*/
typedef uint16_t                    UINT_16;

/**
  @var typedef int32_t    INT_32
  @brief Signed 32-bit integer (FreeBSD OS)
*/
typedef int32_t                     INT_32;

/**
  @var typedef uint32_t   UINT_32
  @brief Unsigned 32-bit integer (FreeBSD OS)
*/
typedef uint32_t                    UINT_32;

/**
  @var typedef int64_t    INT_64
  @brief Signed 64-bit integer (FreeBSD OS)
*/
typedef int64_t                     INT_64;

/**
  @var typedef uint64_t    UINT_64
  @brief Unsigned 64-bit integer (FreeBSD OS)
*/
typedef uint64_t                    UINT_64;

/**
  @var typedef double              W_FLOAT
  @brief Floafing point variable (FreeBSD OS)
*/
typedef double                      W_FLOAT;

/**
  @var typedef char                CHAR_8
  @brief Signed 8-bit character    (FreeBSD OS)
*/
typedef char                        CHAR_8;

/**
  @var typedef unsigned char UCHAR_8
  @brief Unsigned 8-bit character  (FreeBSD OS)
*/
typedef unsigned char               UCHAR_8;

/**
  @var typedef const char  CCHAR_8
  @brief Constant character (FreeBSD OS)
*/
typedef const char                  CCHAR_8;

/**
  @var typedef const unsigned char UCCHAR_8
  @brief Unsigned constant character (FreeBSD OS)
*/
typedef const unsigned char         UCCHAR_8;

/**
  @var typedef CHAR_8 * CHAR_P
  @brief Address (FreeBSD OS)
*/
typedef CHAR_8                    * CHAR_P;

/**
  @var typedef UCHAR_8    * UCHAR_P
  @brief Address of unsigned char (FreeBSD OS)
*/
typedef UCHAR_8                   * UCHAR_P;

/**
  @var typedef CCHAR_8    * CCHAR_P
  @brief Address, pointer to const (FreeBSD OS)
*/
typedef CCHAR_8                   * CCHAR_P;

/**
  @var typedef UCCHAR_8 * UCCHAR_P
  @brief Address, pointer to unsigned const (FreeBSD OS)
*/
typedef UCCHAR_8                  * UCCHAR_P;


#elif WIN32 /* FreeBSD End, start of Win32 declarations */

/**
  @var typedef __int16           INT_16
  @brief Signed 16-bit integer (Win32)
*/
typedef __int16                     INT_16;

/**
  @var typedef unsigned __int16   UINT_16
  @brief Unsigned 16-bit integer (Win32)
*/
typedef unsigned __int16            UINT_16;

/**
  @var typedef __int32    INT_32
  @brief Signed 32-bit integer (Win32)
*/
typedef __int32                     INT_32;

/**
  @var typedef unsigned __int32   UINT_32
  @brief Unsigned 32-bit integer (Win32)
*/
typedef unsigned __int32            UINT_32;

/**
  @var typedef __int64    INT_64
  @brief Signed 64-bit integer (Win32)
*/
typedef __int64                     INT_64;

/**
  @var typedef uint64_t    UINT_64
  @brief Unsigned 64-bit integer (Win32)
*/
typedef unsigned __int64            UINT_64;

/**
  @var typedef double              W_FLOAT
  @brief Floafing point variable (Win32)
*/
typedef double                      W_FLOAT;

/**
  @var typedef char                CHAR_8
  @brief Signed 8-bit character    (Win32)
*/
typedef char                        CHAR_8;

/**
  @var typedef unsigned char UCHAR_8
  @brief Unsigned 8-bit character  (Win32)
*/
typedef unsigned char               UCHAR_8;

/**
  @var typedef const char  CCHAR_8
  @brief Constant character (Win32)
*/
typedef const char                  CCHAR_8;

/**
  @var typedef const unsigned char UCCHAR_8
  @brief Unsigned constant character (Win32)
*/
typedef const unsigned char         UCCHAR_8;

/**
  @var typedef CHAR_8 * CHAR_P
  @brief Address (Win32)
*/
typedef CHAR_8                    * CHAR_P;

/**
  @var typedef UCHAR_8    * UCHAR_P
  @brief Address of unsigned char (Win32)
*/
typedef UCHAR_8                   * UCHAR_P;

/**
  @var typedef CCHAR_8    * CCHAR_P
  @brief Address, pointer to const (Win32)
*/
typedef CCHAR_8                   * CCHAR_P;

/**
  @var typedef UCCHAR_8 * UCCHAR_P
  @brief Address, pointer to unsigned const (Win32)
*/
typedef UCCHAR_8                  * UCCHAR_P;

#elif sun /* Win32 End, start of Sun Solaris declarations */

/**
  @var typedef int16_t    INT_16
  @brief Signed 16-bit integer (Sun OS)
*/
typedef int16_t                     INT_16;

/**
  @var typedef uint16_t   UINT_16
  @brief Unsigned 16-bit integer (Sun OS)
*/
typedef uint16_t                    UINT_16;

/**
  @var typedef int32_t    INT_32
  @brief Signed 32-bit integer (Sun OS)
*/
typedef int32_t                     INT_32;

/**
  @var typedef uint32_t   UINT_32
  @brief Unsigned 32-bit integer (Sun OS)
*/
typedef uint32_t                    UINT_32;

/**
  @var typedef int64_t    INT_64
  @brief Signed 64-bit integer (Sun OS)
*/
typedef int64_t                     INT_64;

/**
  @var typedef uint64_t    UINT_64
  @brief Unsigned 64-bit integer (Sun OS)
*/
typedef uint64_t                    UINT_64;

/**
  @var typedef double              W_FLOAT
  @brief Floafing point variable (Sun OS)
*/
typedef double                      W_FLOAT;

/**
  @var typedef char                CHAR_8
  @brief Signed 8-bit character    (Sun OS)
*/
typedef char                        CHAR_8;

/**
  @var typedef unsigned char UCHAR_8
  @brief Unsigned 8-bit character  (Sun OS)
*/
typedef unsigned char               UCHAR_8;

/**
  @var typedef const char  CCHAR_8
  @brief Constant character (Sun OS)
*/
typedef const char                  CCHAR_8;

/**
  @var typedef const unsigned char UCCHAR_8
  @brief Unsigned constant character (Sun OS)
*/
typedef const unsigned char         UCCHAR_8;

/**
  @var typedef CHAR_8 * CHAR_P
  @brief Address (Sun OS)
*/
typedef CHAR_8                    * CHAR_P;

/**
  @var typedef UCHAR_8    * UCHAR_P
  @brief Address of unsigned char (Sun OS)
*/
typedef UCHAR_8                   * UCHAR_P;

/**
  @var typedef CCHAR_8    * CCHAR_P
  @brief Address, pointer to const (Sun OS)
*/
typedef CCHAR_8                   * CCHAR_P;

/**
  @var typedef UCCHAR_8 * UCCHAR_P
  @brief Address, pointer to unsigned const (Sun OS)
*/
typedef UCCHAR_8                  * UCCHAR_P;

#elif __hpux /* Sun OS End, start of HP-UX declarations */

/**
  @var typedef int16_t    INT_16
  @brief Signed 16-bit integer (HP-UX)
*/
typedef int16_t                     INT_16;

/**
  @var typedef uint16_t   UINT_16
  @brief Unsigned 16-bit integer (HP-UX)
*/
typedef uint16_t                    UINT_16;

/**
  @var typedef int32_t    INT_32
  @brief Signed 32-bit integer (HP-UX)
*/
typedef int32_t                     INT_32;

/**
  @var typedef uint32_t   UINT_32
  @brief Unsigned 32-bit integer (HP-UX)
*/
typedef uint32_t                    UINT_32;

/**
  @var typedef int64_t    INT_64
  @brief Signed 64-bit integer (HP-UX)
*/
typedef int64_t                     INT_64;

/**
  @var typedef uint64_t    UINT_64
  @brief Unsigned 64-bit integer (HP-UX)
*/
typedef uint64_t                    UINT_64;
/**
  @var typedef double              W_FLOAT
  @brief Floafing point variable (HP-UX)
*/
typedef double                      W_FLOAT;

/**
  @var typedef char                CHAR_8
  @brief Signed 8-bit character    (HP-UX)
*/
typedef char                        CHAR_8;

/**
  @var typedef unsigned char UCHAR_8
  @brief Unsigned 8-bit character  (HP-UX)
*/
typedef unsigned char               UCHAR_8;

/**
  @var typedef const char  CCHAR_8
  @brief Constant character (HP-UX)
*/
typedef const char                  CCHAR_8;
/**
  @var typedef const unsigned char UCCHAR_8
  @brief Unsigned constant character (HP-UX)
*/
typedef const unsigned char         UCCHAR_8;

/**
  @var typedef CHAR_8 * CHAR_P
  @brief Address (HP-UX)
*/
typedef CHAR_8                    * CHAR_P;

/**
  @var typedef UCHAR_8    * UCHAR_P
  @brief Address of unsigned char (HP-UX)
*/
typedef UCHAR_8                   * UCHAR_P;

/**
  @var typedef CCHAR_8    * CCHAR_P
  @brief Address, pointer to const (HP-UX)
*/
typedef CCHAR_8                   * CCHAR_P;

/**
  @var typedef UCCHAR_8 * UCCHAR_P
  @brief Address, pointer to unsigned const (HP-UX)
*/
typedef UCCHAR_8                  * UCCHAR_P;

#else  /* HP-UX End */

#error "Cannot compile on this platform"

#endif  /* Platform      */

#ifndef NULL
  #define NULL 0x0
#endif

#endif /* _CTPP2_SYS_TYPES_H__ */
/* End. */
