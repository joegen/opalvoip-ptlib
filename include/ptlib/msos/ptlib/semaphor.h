/*
 * $Id: semaphor.h,v 1.1 1995/08/01 21:42:22 robertj Exp $
 *
 * Portable Windows Library
 *
 * User Interface Classes Interface Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: semaphor.h,v $
 * Revision 1.1  1995/08/01 21:42:22  robertj
 * Initial revision
 *
 */


#ifndef _PSEMAPHORE


///////////////////////////////////////////////////////////////////////////////
// PSemaphore

#include "../../common/semaphor.h"
#if defined(P_PLATFORM_HAS_THREADS)
  private:
    HANDLE hSemaphore;
#endif
};


#endif
