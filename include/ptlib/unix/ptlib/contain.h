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
typedef unsigned short	   WORD;	// 2 bytes
typedef unsigned int 	   DWORD;	// 4 bytes
typedef unsigned long long QWORD;	// 8 bytes
typedef size_t 		   PINDEX;
typedef int		   INT;

#define P_MAX_INDEX 		0x7fffffff
#define PABSINDEX(idx) 		(idx)		// careful - size_t may be signed!
#define PASSERTINDEX(idx)

///////////////////////////////////////////
//
//  include common declarations
//

#include "../../common/contain.h"

#endif
