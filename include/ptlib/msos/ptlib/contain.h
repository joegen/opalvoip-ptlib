/*
 * $Id: contain.h,v 1.6 1995/01/09 12:28:45 robertj Exp $
 *
 * Portable Windows Library
 *
 * PContainer Classes Interface Declarations
 *
 * Copyright 1993 by Robert Jongbloed and Craig Southeren
 *
 * $Log: contain.h,v $
 * Revision 1.6  1995/01/09 12:28:45  robertj
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


///////////////////////////////////////////////////////////////////////////////
// Machine & Compiler dependent declarations

#ifdef _WINDOWS

#define STRICT
#include <windows.h>

#define EXPORTED WINAPI _export

#else

#define TRUE 1
#define FALSE 0

#define NEAR __near

#endif


#ifdef _WINDLL
#define PCLASS class __export
#else
#define PCLASS class
#endif


#define PSTATIC __near


class PInteger64 {
  public:
    PInteger64() { }
    PInteger64(unsigned long l) : low(l), high(0) { }
    operator unsigned long() const { return high != 0 ? 0xffffffff : low; }
    unsigned long GetLow() const { return low; }
    unsigned long GetHigh() const { return high; }
    void SetLow(unsigned long l) { low = l; }
    void SetHigh(unsigned long h) { high = h; }
  private:
    unsigned long low, high;
};


typedef unsigned char  BYTE;  //  8 bit unsigned integer quantity
typedef unsigned short WORD;  // 16 bit unsigned integer quantity
typedef unsigned long  DWORD; // 32 bit unsigned integer quantity
typedef PInteger64     QWORD; // 64 bit unsigned integer quantity
typedef long           INT;   // Integer that is the same size as a void *
typedef int            BOOL;  // type returned by expresion (i != j)


#ifdef _MSC_VER

#pragma warning(disable:4699)  // disable warning about precompiled headers

// Type used in array indexes especially that required by operator[] functions.
#define PINDEX            int
#define P_MAX_INDEX       32767
#define PABSINDEX(idx)    (((idx)<0?-(idx):(idx))&0x7fff)
#define PASSERTINDEX(idx) PAssert((idx) >= 0, PInvalidArrayIndex)

#else

// Type used in array indexes especially that required by operator[] functions.
#define PINDEX            unsigned
#define P_MAX_INDEX       65535U
#define PABSINDEX(idx)    (idx)
#define PASSERTINDEX(idx)

#endif


#ifdef _MSC_VER
#pragma warning(disable:4251)  // disable warning exported structs
#pragma warning(disable:4702)  // disable warning about unreachable code
#pragma warning(disable:4705)  // disable warning about statement has no effect
#pragma warning(disable:4511)  // default copy ctor not generated warning
#pragma warning(disable:4512)  // default assignment op not generated warning
#pragma warning(disable:4710)  // inline not expanded warning
#pragma warning(disable:4711)  // auto inlining warning
#endif


///////////////////////////////////////////////////////////////////////////////
// Fill in common declarations

#include "../../common/object.h"


#endif // _OBJECT_H


// End Of File ///////////////////////////////////////////////////////////////
