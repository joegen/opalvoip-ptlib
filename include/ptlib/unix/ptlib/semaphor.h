/*
 * semaphor.h
 *
 * Thread synchronisation semaphore class.
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
 * $Log: semaphor.h,v $
 * Revision 1.5  1998/09/24 04:11:52  robertj
 * Added open software license.
 *
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
