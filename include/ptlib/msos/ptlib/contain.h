/*
 * $Id: contain.h,v 1.1 1994/04/01 14:38:42 robertj Exp $
 *
 * Portable Windows Library
 *
 * PContainer Classes Interface Declarations
 *
 * Copyright 1993 by Robert Jongbloed and Craig Southeren
 *
 * $Log: contain.h,v $
 * Revision 1.1  1994/04/01 14:38:42  robertj
 * Initial revision
 *
 */

#ifndef _CONTAIN_H
#define _CONTAIN_H


#define STRICT


///////////////////////////////////////////////////////////////////////////////
// Machine & Compiler dependent declarations

#ifdef _WINDLL
#define PCLASS class __export
#else
#define PCLASS class
#endif

#define PSTATIC __near

typedef int            BOOL;  // type returned by expresion (i != j)
typedef unsigned char  BYTE;  // 8 bit quantity
typedef unsigned short WORD;  // 16 bit quantity
typedef unsigned long  DWORD; // 32 bit quantity


#ifdef _MSC_VER

#pragma warning(disable:4699)  // disable warning about precompiled headers

// Type used in array indexes especially that required by operator[] functions.
#define PINDEX            int
#define P_MAX_INDEX       32767
#define PABSINDEX(idx)    ((idx)<0?-(idx):(idx))
#define PASSERTINDEX(idx) PAssert((idx) >= 0)

#else

// Type used in array indexes especially that required by operator[] functions.
#define PINDEX            unsigned
#define P_MAX_INDEX       65535U
#define PABSINDEX(idx)    (idx)
#define PASSERTINDEX(idx)

#endif


#ifdef _MSC_VER
#pragma warning(disable:4251)  // disable warning exported structs
#pragma warning(disable:4705)  // disable warning about statement has no effect
#endif


///////////////////////////////////////////////////////////////////////////////
// Fill in common declarations

#include "../../common/contain.h"


#endif // _CONTAIN_H


// End Of File ///////////////////////////////////////////////////////////////
