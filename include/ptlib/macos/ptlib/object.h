/*
 * $Id: object.h,v 1.1 1996/01/02 13:10:31 robertj Exp $
 *
 * Portable Windows Library
 *
 * PContainer Classes Interface Declarations
 *
 * Copyright 1993 by Robert Jongbloed and Craig Southeren
 *
 * $Log: object.h,v $
 * Revision 1.1  1996/01/02 13:10:31  robertj
 * Initial revision
 *
 */

#ifndef _OBJECT_H
#define _OBJECT_H

#include <istream>
#include <ostream>


///////////////////////////////////////////////////////////////////////////////
// Machine & Compiler dependent declarations

typedef unsigned char  BYTE;  //  8 bit unsigned integer quantity
typedef unsigned short WORD;  // 16 bit unsigned integer quantity
typedef unsigned long  DWORD; // 32 bit unsigned integer quantity
typedef int            BOOL;  // type returned by expresion (i != j)

//#define TRUE 1
//#define FALSE 0

#define PCLASS class


// Declaration for exported callback functions to OS
#define EXPORTED


// Declaration for static global variables (WIN16 compatibility)
#define PSTATIC


// Declaration for platform independent architectures
#define PCHAR8 PANSI_CHAR
#define PBYTE_ORDER PBIG_ENDIAN


// Declaration for integer that is the same size as a void *
typedef int INT;


// Declaration for a pointer to arbitrary blocks of memory
typedef unsigned char * PMemoryPointer;


// Declaration for signed integer that is 16 bits
typedef short PInt16;

// Declaration for signed integer that is 32 bits
typedef long PInt32;


// Declaration for 64 bit unsigned integer quantity
#if 0
#define P_HAS_INT64
typedef signed __int64 PInt64;
typedef unsigned __int64 PUInt64;
#endif


// Standard array index type (depends on compiler)
// Type used in array indexes especially that required by operator[] functions.
#define PINDEX unsigned
const PINDEX P_MAX_INDEX = 0xffffffff;
#define PABSINDEX(idx) (idx)
#define PASSERTINDEX(idx)


#ifdef __MWERKS__
// Seems to be missing iostream........
class iostream : public istream, public ostream {
  public:
    iostream(streambuf * s) : istream(s), ostream(s) { }
  private:
    iostream() : istream(0), ostream(0) { }
};

typedef long off_t;

#endif


///////////////////////////////////////////////////////////////////////////////
// Fill in common declarations

#include "::common:object.h"


#endif // _OBJECT_H


// End Of File ///////////////////////////////////////////////////////////////
