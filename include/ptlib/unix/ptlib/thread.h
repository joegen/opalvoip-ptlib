/*
 * thread.h
 *
 * Thread of execution control class.
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
 * $Log: thread.h,v $
 * Revision 1.12  1999/01/09 03:35:52  robertj
 * Fixed problem with closing thread waiting on semaphore.
 *
 * Revision 1.11  1998/11/30 22:07:23  robertj
 * New directory structure.
 *
 * Revision 1.10  1998/09/24 04:12:03  robertj
 * Added open software license.
 *
 * Revision 1.9  1998/01/03 23:06:32  craigs
 * Added PThread support
 *
 * Revision 1.8  1997/04/22 11:00:44  craigs
 * Added FreeStack function
 *
 * Revision 1.7  1996/12/30 03:23:52  robertj
 * Added timeout to block on child process function.
 *
 * Revision 1.6  1996/08/03 12:09:51  craigs
 * Changed for new common directories
 *
 * Revision 1.5  1996/04/15 10:50:48  craigs
 * Last revision prior to release of MibMaster
 *
 * Revision 1.4  1996/01/26 11:08:45  craigs
 * Fixed problem with blocking Accept calls
 *
 * Revision 1.3  1995/12/08 13:16:38  craigs
 * Added semaphore include and friend class
 *
 * Revision 1.2  1995/07/09 00:35:00  craigs
 * Latest and greatest omnibus change
 *
 * Revision 1.1  1995/01/23 18:43:27  craigs
 * Initial revision
 *
 * Revision 1.1  1994/04/12  08:31:05  robertj
 * Initial revision
 *
 */

#ifndef _PTHREAD

#pragma interface

#include <setjmp.h>

class PProcess;
class PSemaphore;

///////////////////////////////////////////////////////////////////////////////
// PThread

#include "../../thread.h"
  public:
    int PXBlockOnChildTerminate(int pid, const PTimeInterval & timeout);

  public:
    int PXBlockOnIO(int handle,
                    int type,
                   const PTimeInterval & timeout);

    int PXBlockOnIO(int maxHandle,
               fd_set & readBits,
               fd_set & writeBits,
               fd_set & execptionBits,
               const PTimeInterval & timeout,
               const PIntArray & osHandles);

#ifdef P_PTHREADS

  public:
    void PXSetWaitingSemaphore(PSemaphore * sem);
    //void InitialiseProcessThread();
    static void * PX_ThreadStart(void *);
    static void PX_ThreadEnd(void *);

  protected:
    void PX_NewThread(BOOL startSuspended);
    unsigned PX_GetThreadId() const;

    PINDEX     PX_origStackSize;
    int        PX_suspendCount;
    BOOL       PX_autoDelete;

    pthread_t       PX_threadId;
    pthread_mutex_t PX_suspendMutex;

    PSemaphore * PX_waitingSemaphore;
    pthread_mutex_t PX_WaitSemMutex;

#else

  protected:
    void FreeStack();
    void PXSetOSHandleBlock  (int fd, int type);
    void PXClearOSHandleBlock(int fd, int type);

    PTimer ioTimer;
    BOOL   hasIOTimer;
    int    waitPid;

    fd_set * read_fds;
    fd_set * write_fds;
    fd_set * exception_fds;
    int    handleWidth;

    int    selectReturnVal;
    int    selectErrno;
#endif
};

#endif
