/*
 * contain.h
 *
 * Low level object and container definitions.
 *
 * Portable Windows Library
 *
 * Copyright (c) 1993-1998 Equivalence Pty. Ltd.
 *
 * The contents of this file are subject to the Mozilla Public License
 * Version 1.0 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS"
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See
 * the License for the specific language governing rights and limitations
 * under the License.
 *
 * The Original Code is Portable Windows Library.
 *
 * The Initial Developer of the Original Code is Equivalence Pty. Ltd.
 *
 * Portions are Copyright (C) 1993 Free Software Foundation, Inc.
 * All Rights Reserved.
 *
 * Contributor(s): ______________________________________.
 *
 * $Log: contain.h,v $
 * Revision 1.10  1998/11/03 10:56:33  robertj
 * Removed unused extern for PErrorStream
 *
 * Revision 1.9  1998/09/24 04:11:31  robertj
 * Added open software license.
 *
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

#define	PEXPORT
#define	PSTATIC


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

#include "../../common/ptlib/contain.h"

#endif
