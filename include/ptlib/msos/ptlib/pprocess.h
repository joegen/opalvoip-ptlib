/*
 * $Id: pprocess.h,v 1.4 1994/07/27 06:00:10 robertj Exp $
 *
 * Portable Windows Library
 *
 * User Interface Classes Interface Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: pprocess.h,v $
 * Revision 1.4  1994/07/27 06:00:10  robertj
 * Backup
 *
 * Revision 1.3  1994/07/21  12:35:18  robertj
 * *** empty log message ***
 *
 * Revision 1.2  1994/07/02  03:18:09  robertj
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
#ifdef _WINDOWS
  friend int PASCAL WinMain(HINSTANCE, HINSTANCE, LPSTR, int);
#endif
};


#ifndef _WINDLL

extern PProcess * PSTATIC PProcessInstance;

inline PProcess::PProcess()
  {
    PProcessInstance = this;
  }

inline PProcess * PProcess::Current()
  {
    return PProcessInstance;
  }


#endif


#endif
