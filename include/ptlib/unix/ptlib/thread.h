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
 */

///////////////////////////////////////////////////////////////////////////////
// PThread

  public:
    int PXBlockOnChildTerminate(int pid, const PTimeInterval & timeout);

    int PXBlockOnIO(
      int handle,
      int type,
      const PTimeInterval & timeout
    );

    void PXAbortBlock() const;

#ifdef P_PTHREADS

  public:
#ifndef P_HAS_SEMAPHORES
    void PXSetWaitingSemaphore(PSemaphore * sem);
#endif
    static bool PX_kill(PThreadIdentifier tid, PUniqueThreadIdentifier uid, int sig);

  protected:
    void PX_StartThread();
    void PX_ThreadBegin();
    void PX_ThreadEnd();
    void PX_Suspended();
    static void * PX_ThreadMain(void *);
    static void PX_ThreadEnd(void *);

#if P_STD_ATOMIC
    atomic<Priority> PX_priority;
#else
    atomic<int>      PX_priority;
#endif
    
#if defined(P_LINUX)
    mutable pid_t     PX_linuxId;
    PTimeInterval     PX_startTick;
    PTimeInterval     PX_endTick;
#endif
    mutable pthread_mutex_t   PX_suspendMutex;
    int               PX_suspendCount;
    enum PX_states {
      PX_firstResume,
      PX_starting,
      PX_running,
      PX_finishing,
      PX_finished
    };
#if P_STD_ATOMIC
    atomic<PX_states> PX_state;
#else
    atomic<int> PX_state;
#endif

    std::auto_ptr<PSyncPoint> PX_synchroniseThreadFinish;

#ifndef P_HAS_SEMAPHORES
    PSemaphore      * PX_waitingSemaphore;
    pthread_mutex_t   PX_WaitSemMutex;
#endif

    int unblockPipe[2];
    friend class PSocket;
    friend void PX_SuspendSignalHandler(int);

#else // P_PTHREADS

#if defined(__BEOS__)

  protected:
    static int32 ThreadFunction(void * threadPtr);
    thread_id mId;
    int32 mPriority;
    PINDEX mStackSize;
    int32 mSuspendCount;
  public:
    int unblockPipe[2];

#elif defined(VX_TASKS)
  public:
    SEM_ID syncPoint;
    static void Trace(PThreadIdentifer threadId = 0);

  private:
    static int ThreadFunction(void * threadPtr);
    long PX_threadId;
    int priority;
    PINDEX originalStackSize;

#endif

#endif // P_PTHREADS


// End Of File ////////////////////////////////////////////////////////////////
