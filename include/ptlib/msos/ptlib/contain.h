/*
 * $Id: contain.h,v 1.17 1997/01/12 04:13:07 robertj Exp $
 *
 * Portable Windows Library
 *
 * PContainer Classes Interface Declarations
 *
 * Copyright 1993 by Robert Jongbloed and Craig Southeren
 *
 * $Log: contain.h,v $
 * Revision 1.17  1997/01/12 04:13:07  robertj
 * Changed library to support NT 4.0 API changes.
 *
 * Revision 1.16  1996/09/14 12:38:57  robertj
 * Moved template define from command line to code.
 * Fixed correct application of windows defines.
 *
 * Revision 1.15  1996/08/17 10:00:33  robertj
 * Changes for Windows DLL support.
 *
 * Revision 1.14  1996/08/08 10:08:58  robertj
 * Directory structure changes for common files.
 *
 * Revision 1.13  1996/07/15 10:26:55  robertj
 * MSVC 4.1 Support
 *
 * Revision 1.12  1996/03/31 09:07:29  robertj
 * Removed bad define in NT headers.
 *
 * Revision 1.11  1996/01/28 02:54:27  robertj
 * Removal of MemoryPointer classes as usage didn't work for GNU.
 *
 * Revision 1.10  1996/01/23 13:23:15  robertj
 * Added const version of PMemoryPointer
 *
 * Revision 1.9  1995/11/09 12:23:46  robertj
 * Added 64 bit integer support.
 * Added platform independent base type access classes.
 *
 * Revision 1.8  1995/04/25 11:31:18  robertj
 * Changes for DLL support.
 *
 * Revision 1.7  1995/03/12 04:59:54  robertj
 * Re-organisation of DOS/WIN16 and WIN32 platforms to maximise common code.
 * Used built-in equate for WIN32 API (_WIN32).
 *
 * Revision 1.6  1995/01/09  12:28:45  robertj
 * Moved EXPORTED definition from applicat.h
 *
 * Revision 1.5  1995/01/06  10:47:08  robertj
 * Added 64 bit integer.
 *
 * Revision 1.4  1994/11/19  00:18:26  robertj
 * Changed PInteger to be INT, ie standard type like BOOL/WORD etc.
 *
 * Revision 1.3  1994/07/02  03:18:09  robertj
 * Support for 16 bit systems.
 *
 * Revision 1.2  1994/06/25  12:13:01  robertj
 * Synchronisation.
 *
 * Revision 1.1  1994/04/01  14:38:42  robertj
 * Initial revision
 *
 */

#ifndef _OBJECT_H
#define _OBJECT_H


#ifdef _MSC_VER

#pragma warning(disable:4201)  // nonstandard extension: nameless struct/union
#pragma warning(disable:4251)  // disable warning exported structs
#pragma warning(disable:4511)  // default copy ctor not generated warning
#pragma warning(disable:4512)  // default assignment op not generated warning
#pragma warning(disable:4514)  // unreferenced inline removed
#pragma warning(disable:4699)  // precompiled headers
#pragma warning(disable:4702)  // disable warning about unreachable code
#pragma warning(disable:4705)  // disable warning about statement has no effect
#pragma warning(disable:4710)  // inline not expanded warning
#pragma warning(disable:4711)  // auto inlining warning
#pragma warning(disable:4097)  // typedef synonym for class

#if _MSC_VER>=800 && !defined(PMAKEDLL)
#define PHAS_TEMPLATES
#endif

#endif


///////////////////////////////////////////////////////////////////////////////
// Machine & Compiler dependent declarations

#if defined(_WIN32) && !defined(WIN32)
#define WIN32
#endif

#if defined(_WINDOWS) || defined(_WIN32)

#ifndef WINVER
#define WINVER 0x401
#endif

#define STRICT
#include <windows.h>

#undef DELETE   // Remove define from NT headers.

#else

typedef unsigned char  BYTE;  //  8 bit unsigned integer quantity
typedef unsigned short WORD;  // 16 bit unsigned integer quantity
typedef unsigned long  DWORD; // 32 bit unsigned integer quantity
typedef int            BOOL;  // type returned by expresion (i != j)

#define TRUE 1
#define FALSE 0

#define NEAR __near

#endif


#ifdef PMAKEDLL
#ifdef _WIN32
#define PEXPORT __declspec(dllexport)
#else
#define PEXPORT extern __export
#endif
#else
#define PEXPORT
#endif


// Declaration for exported callback functions to OS
#if defined(_WIN32)
#define PEXPORTED __stdcall
#elif defined(_WINDOWS)
#define PEXPORTED WINAPI __export
#else
#define PEXPORTED __far __pascal
#endif


// Declaration for static global variables (WIN16 compatibility)
#if defined(_WIN32)
#define PSTATIC
#else
#define PSTATIC __near
#endif


// Declaration for platform independent architectures
#define PCHAR8 PANSI_CHAR
#define PBYTE_ORDER PLITTLE_ENDIAN


// Declaration for integer that is the same size as a void *
#if defined(_WIN32)
typedef int INT;
#else
typedef long INT;   
#endif


// Declaration for signed integer that is 16 bits
typedef short PInt16;

// Declaration for signed integer that is 32 bits
typedef long PInt32;


// Declaration for 64 bit unsigned integer quantity
#if defined(_MSC_VER) && defined(_WIN32)

#define P_HAS_INT64

typedef signed __int64 PInt64;
typedef unsigned __int64 PUInt64;

class ostream;
class istream;

ostream & operator<<(ostream & s, PInt64 v);
ostream & operator<<(ostream & s, PUInt64 v);

istream & operator>>(istream & s, PInt64 v);
istream & operator>>(istream & s, PUInt64 v);

#endif


// Standard array index type (depends on compiler)
// Type used in array indexes especially that required by operator[] functions.
#ifdef _MSC_VER

#define PINDEX int
#if defined(_WIN32)
const PINDEX P_MAX_INDEX = 0x7fffffff;
#else
const PINDEX P_MAX_INDEX = 0x7fff;
#endif
inline PINDEX PABSINDEX(PINDEX idx) { return (idx < 0 ? -idx : idx)&P_MAX_INDEX; }
#define PASSERTINDEX(idx) PAssert((idx) >= 0, PInvalidArrayIndex)

#else

#define PINDEX unsigned
#if sizeof(int) == 4
const PINDEX P_MAX_INDEX = 0xffffffff;
#else
const PINDEX P_MAX_INDEX = 0xffff;
#endif
#define PABSINDEX(idx) (idx)
#define PASSERTINDEX(idx)

#endif


///////////////////////////////////////////////////////////////////////////////
// Fill in common declarations

#include "../../common/ptlib/contain.h"


#endif // _OBJECT_H


// End Of File ///////////////////////////////////////////////////////////////
