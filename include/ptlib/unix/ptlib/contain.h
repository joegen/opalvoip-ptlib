/*
 * $Id
 *
 * Portable Windows Library
 *
 * Machine dependent declarations
 *
 * Copyright 1993 by Robert Jongbloed and Craig Southeren
 *
 * $Log
 */

#ifndef _PCONTAIN
#define _PCONTAIN

#include <sys/types.h>
#include <unistd.h>
#include <ctype.h>

///////////////////////////////////////////
//
//  define TRUE and FALSE for environments that don't have them
//

#ifndef	TRUE
#define	TRUE		1
#define	FALSE		0
#endif

///////////////////////////////////////////
//
//  define a macro for declaring classes so we can bolt
//  extra things to class declarations
//

#define	PCLASS		class
#define	PSTATIC

///////////////////////////////////////////
//
//  declare PErrorStream 
//
class ostream;
extern ostream * PErrorStream;

///////////////////////////////////////////
//
// define some basic types and their limits
//

typedef int	           BOOL;
typedef unsigned char	   BYTE;	// 1 byte

typedef	signed short	   PInt16;	// 16 bit
typedef unsigned short	   WORD;

typedef	signed int         PInt32;	// 32 bit
typedef unsigned int 	   DWORD;

#ifdef P_HAS_INT64
typedef signed long long int   PInt64;
typedef unsigned long long int PUInt64;	// 64 bit
#endif

typedef size_t 		      PINDEX;
typedef int		      INT;

#define P_MAX_INDEX 		0x7fffffff
#define PABSINDEX(idx) 		(idx)		// careful - size_t may be signed!
#define PASSERTINDEX(idx)

///////////////////////////////////////////
//
//  include common declarations
//

#include "../../common/contain.h"

#endif
