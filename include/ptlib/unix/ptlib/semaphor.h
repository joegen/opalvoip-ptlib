/*
 * $Id: semaphor.h,v 1.4 1998/01/04 10:45:01 craigs Exp $
 *
 * Portable Windows Library
 *
 * User Interface Classes Interface Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: semaphor.h,v $
 * Revision 1.4  1998/01/04 10:45:01  craigs
 * Added thread.h
 *
 * Revision 1.3  1998/01/03 23:06:32  craigs
 * Added PThread support
 *
 * Revision 1.2  1996/08/03 12:08:19  craigs
 * Changed for new common directories
 *
 * Revision 1.1  1996/01/26 11:06:31  craigs
 * Initial revision
 *
 */

#ifndef _PSEMAPHORE

#pragma interface

#include <thread.h>

#include "../../common/ptlib/semaphor.h"
#ifdef P_PTHREADS
  protected:
    pthread_mutex_t mutex;
    pthread_cond_t  condVar;
    unsigned currentCount;
    unsigned maximumCount;
    unsigned queuedLocks;
#else
    PQUEUE(ThreadQueue, PThread);
    ThreadQueue waitQueue;
#endif
friend void SemaSignal(PSemaphore & sema, BOOL PX_enableDebugOutput);
friend void SemaWait(PSemaphore & sema, BOOL PX_enableDebugOutput);
};

#endif
