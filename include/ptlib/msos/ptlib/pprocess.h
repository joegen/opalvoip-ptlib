/*
 * $Id: pprocess.h,v 1.2 1994/07/02 03:18:09 robertj Exp $
 *
 * Portable Windows Library
 *
 * User Interface Classes Interface Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: pprocess.h,v $
 * Revision 1.2  1994/07/02 03:18:09  robertj
 * Prevent WinMain in pure MSDOS versions.
 *
 * Revision 1.1  1994/06/25  12:13:01  robertj
 * Initial revision
 *
 */


#ifndef _PPROCESS

///////////////////////////////////////////////////////////////////////////////
// PProcess

#include "../../common/process.h"
  private:
    PThread * currentThread;

  friend class PThread;

#ifdef _WINDOWS
  friend int PASCAL WinMain(HINSTANCE, HINSTANCE, LPSTR, int);
#endif
};


#ifndef _WINDLL

extern PProcess * PSTATIC PProcessInstance;

inline PProcess::PProcess()
  { PProcessInstance = this; }

inline PProcess * PProcess::Current()
  { return PProcessInstance; }


#endif


#endif
