//
// (c) Yuriy Gorvitovskiy
// for Openh323, www.Openh323.org
//
// Windows CE Port
//
// Definitions/declarations for time routines 
//
// [Microsoft]
// [ANSI/System V]
// [Public]
// 

#ifndef _TIME_H
#define _TIME_H

#define _INC_TIME // for wce.h

#include <stdlib.h>
#include <afx.h>

#ifdef  __cplusplus
extern "C" {
#endif


#ifndef _CLOCK_T_DEFINED
typedef long clock_t;
#define _CLOCK_T_DEFINED
#endif

/* Clock ticks macro - ANSI version */
#define CLOCKS_PER_SEC  1000

/* Function prototypes */
clock_t				clock(void);
struct tm *			gmtime(const time_t* t);
inline struct tm *	localtime(const time_t* t) {return wce_localtime(t);}
inline time_t		mktime(struct tm* t)  { return wce_mktime(t); }
inline time_t		time(time_t* t)		{ return wce_time(t);}
time_t				FileTimeToTime(const FILETIME& FileTime);
time_t				SystemTimeToTime(const LPSYSTEMTIME pSystemTime);

#ifdef  __cplusplus
}
#endif

#endif  /* _INC_TIME */
