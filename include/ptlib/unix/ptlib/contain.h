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
 * $Revision$
 * $Author$
 * $Date$
 */

#include "pmachdep.h"
#include <unistd.h>
#include <ctype.h>
#include <limits.h>


///////////////////////////////////////////
//
//  define TRUE and FALSE for environments that don't have them
//

#ifndef TRUE
#define TRUE    1
#define FALSE   0
#endif

#ifdef P_USE_INTEGER_BOOL
typedef int BOOL;
#endif

///////////////////////////////////////////
//
//  define a macro for declaring classes so we can bolt
//  extra things to class declarations
//

#define PEXPORT
#define PSTATIC


///////////////////////////////////////////
//
// define some basic types and their limits
//

typedef uint8_t            BYTE;
typedef int16_t            PInt16;  // 16 bit
typedef uint16_t           WORD;

typedef int32_t            PInt32;  // 32 bit
typedef uint32_t           DWORD;

#ifndef P_NEEDS_INT64
typedef   signed long long int PInt64;
typedef unsigned long long int PUInt64; // 64 bit
#endif

// Integer type that is same size as a pointer type.
#ifdef P_64BIT
typedef long          INT;
typedef unsigned long UINT;
#else
typedef int           INT;
typedef unsigned int  UINT;
#endif

typedef int PINDEX;
#define P_MAX_INDEX INT_MAX

inline PINDEX PABSINDEX(PINDEX idx) { return (idx < 0 ? -idx : idx)&P_MAX_INDEX; }
#define PASSERTINDEX(idx) PAssert((idx) >= 0, PInvalidArrayIndex)

///////////////////////////////////////////
//
// needed for STL
//
#if P_HAS_STL_STREAMS
#define __USE_STL__     1
// both gnu-c++ and stlport define __true_type normally this would be
// fine but until pwlib removes the evil using namespace std below,
// this is included here to ensure the types do not conflict.  Yes you
// get math when you don't want it but its one of the things in
// stlport that sources the native cmath and includes
// the gcc header bits/cpp_type_traits.h which has the conflicting type.
//
// the sooner the using namespace std below is removed the better.
// namespace pollution in headers is plain wrong!
// 
#include <cmath>
#endif

#define P_HAS_TYPEINFO  1

using namespace std;

