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

typedef int		   BOOL;
typedef unsigned char	   BYTE;	// 1 byte

typedef	short	   	   PInt16;	// 16 bit
typedef unsigned short	   WORD;

typedef	int    	           PInt32;	// 32 bit
typedef unsigned int 	   DWORD;

#define P_HAS_INT64
typedef signed long long   PInt64;
typedef unsigned long long PUInt64;	// 8 bytes

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
