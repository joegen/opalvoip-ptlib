/*
 * $Id: semaphor.h,v 1.3 1996/08/08 10:09:11 robertj Exp $
 *
 * Portable Windows Library
 *
 * User Interface Classes Interface Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: semaphor.h,v $
 * Revision 1.3  1996/08/08 10:09:11  robertj
 * Directory structure changes for common files.
 *
 * Revision 1.2  1996/06/13 13:32:10  robertj
 * Rewrite of auto-delete threads, fixes Windows95 total crash.
 *
 * Revision 1.1  1995/08/01 21:42:22  robertj
 * Initial revision
 *
 */


#ifndef _PSEMAPHORE


///////////////////////////////////////////////////////////////////////////////
// PSemaphore

#include "../../common/ptlib/semaphor.h"
#if defined(P_PLATFORM_HAS_THREADS)
  public:
    HANDLE GetHandle() const { return hSemaphore; }
  private:
    HANDLE hSemaphore;
#endif
};


#endif
