/*
 * (c) 1995 Microsoft Corporation. All rights reserved.
 * 		Developed by hip communications inc., http://info.hip.com/info/
 * Portions (c) 1993 Intergraph Corporation. All rights reserved.
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 */
#ifndef _CTPP2_TIME_H__
#define _CTPP2_TIME_H__ 1
/*
 * stolen from ActiveState perl sources
 */

#include <windows.h>
#include <winsock.h> /* timeval */

typedef union
{
	unsigned __int64        ft_i64;
	FILETIME                ft_val;
} FT_t;

#ifdef __GNUC__
#  define Const64(x) x##LL
#else
#  define Const64(x) x##i64
#endif
/* Number of 100 nanosecond units from 1/1/1601 to 1/1/1970 */
#define EPOCH_BIAS  Const64(116444736000000000)

/* NOTE: This does not compute the timezone info (doing so can be expensive,
 * and appears to be unsupported even by glibc) */
inline int gettimeofday(struct timeval *tp, void *not_used)
{
	FT_t ft;

	/* this returns time in 100-nanosecond units  (i.e. tens of usecs) */
	GetSystemTimeAsFileTime(&ft.ft_val);

	/* seconds since epoch */
	tp->tv_sec = (long)((ft.ft_i64 - EPOCH_BIAS) / Const64(10000000));

	/* microseconds remaining */
	tp->tv_usec = (long)((ft.ft_i64 / Const64(10)) % Const64(1000000));

return 0;
}

#endif /* _CTPP2_TIME_H__ */
/* End. */
