/*
 * tlibthrd.cxx
 *
 * Routines for pre-emptive threading system
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
 * $Log: tlibthrd.cxx,v $
 * Revision 1.16  1999/05/12 03:29:20  robertj
 * Fixed problem with semaphore free, done at wrong time.
 *
 * Revision 1.15  1999/04/29 08:41:26  robertj
 * Fixed problems with uninitialised mutexes in PProcess.
 *
 * Revision 1.14  1999/03/16 10:54:16  robertj
 * Added parameterless version of WaitForTermination.
 *
 * Revision 1.13  1999/03/16 10:30:37  robertj
 * Added missing PThread::WaitForTermination function.
 *
 * Revision 1.12  1999/01/12 12:09:51  robertj
 * Removed redundent member variable, was in common.
 * Fixed BSD threads compatibility.
 *
 * Revision 1.11  1999/01/11 12:05:56  robertj
 * Fixed some more race conditions in threads.
 *
 * Revision 1.10  1999/01/11 03:42:26  robertj
 * Fixed problem with destroying thread automatically.
 *
 * Revision 1.9  1999/01/09 03:37:28  robertj
 * Fixed problem with closing thread waiting on semaphore.
 * Improved efficiency of mutex to use pthread functions directly.
 *
 * Revision 1.8  1999/01/08 01:31:03  robertj
 * Support for pthreads under FreeBSD
 *
 * Revision 1.7  1998/12/15 12:41:07  robertj
 * Fixed signal handling so can now ^C a pthread version.
 *
 * Revision 1.6  1998/11/05 09:45:04  robertj
 * Removed StartImmediate option in thread construction.
 *
 * Revision 1.5  1998/09/24 04:12:25  robertj
 * Added open software license.
 *
 */

#include <pthread.h>
#include <sys/resource.h>

#define	SUSPEND_SIG	SIGUSR1
#define	RESUME_SIG	SIGUSR2

PDECLARE_CLASS(HouseKeepingThread, PThread)
  public:
    HouseKeepingThread()
      : PThread(1000) { Resume(); }
    void Main();
};


int PThread::PXBlockOnIO(int handle, int type, const PTimeInterval & timeout)
{
  // make sure we flush the buffer before doing a write
  fd_set tmp_rfd, tmp_wfd, tmp_efd;
  fd_set * read_fds      = &tmp_rfd;
  fd_set * write_fds     = &tmp_wfd;
  fd_set * exception_fds = &tmp_efd;

  FD_ZERO (read_fds);
  FD_ZERO (write_fds);
  FD_ZERO (exception_fds);

  switch (type) {
    case PChannel::PXReadBlock:
    case PChannel::PXAcceptBlock:
      FD_SET (handle, read_fds);
      break;
    case PChannel::PXWriteBlock:
      FD_SET (handle, write_fds);
      break;
    case PChannel::PXConnectBlock:
      FD_SET (handle, write_fds);
      FD_SET (handle, exception_fds);
      break;
    default:
      PAssertAlways(PLogicError);
      return 0;
  }

  struct timeval * tptr = NULL;
  struct timeval   timeout_val;
  if (timeout != PMaxTimeInterval) {
    static const PTimeInterval oneDay(0, 0, 0, 0, 1);
    if (timeout < oneDay) {
      timeout_val.tv_usec = (timeout.GetMilliSeconds() % 1000) * 1000;
      timeout_val.tv_sec  = timeout.GetSeconds();
      tptr                = &timeout_val;
    }
  }

  int retval = ::select(handle+1, read_fds, write_fds, exception_fds, tptr);
  PProcess::Current().PXCheckSignals();
  return retval;
}


static void sigSuspendHandler(int)
{
  // wait for a resume signal
  sigset_t waitSignals;
  sigemptyset(&waitSignals);
  sigaddset(&waitSignals, RESUME_SIG);
  sigwait(&waitSignals);
}


void HouseKeepingThread::Main()
{
  PProcess & process = PProcess::Current();

  // In this thread we really do want these signals
  sigset_t blockedSignals;
  sigemptyset(&blockedSignals);
  sigaddset(&blockedSignals, SIGHUP);
  sigaddset(&blockedSignals, SIGINT);
  sigaddset(&blockedSignals, SIGQUIT);
  sigaddset(&blockedSignals, SIGTERM);
  PAssertOS(pthread_sigmask(SIG_UNBLOCK, &blockedSignals, NULL) == 0);

  for (;;) {
    PTimeInterval waitTime = process.timers.Process();
    if (waitTime == PMaxTimeInterval)
      process.timerChangeSemaphore.Wait();
    else
      process.timerChangeSemaphore.Wait(waitTime);
  }
}


void PProcess::Construct()
{
  // make sure we don't get upset by resume signals
  sigset_t blockedSignals;
  sigemptyset(&blockedSignals);
  sigaddset(&blockedSignals, SIGHUP);
  sigaddset(&blockedSignals, SIGINT);
  sigaddset(&blockedSignals, SIGQUIT);
  sigaddset(&blockedSignals, SIGTERM);
  sigaddset(&blockedSignals, RESUME_SIG);
  PAssertOS(pthread_sigmask(SIG_BLOCK, &blockedSignals, NULL) == 0);

  // set the file descriptor limit to something sensible
  struct rlimit rl;
  PAssertOS(getrlimit(RLIMIT_NOFILE, &rl) == 0);
  rl.rlim_cur = rl.rlim_max;
  PAssertOS(setrlimit(RLIMIT_NOFILE, &rl) == 0);

  // initialise the housekeeping thread
  housekeepingThread = PNEW HouseKeepingThread;

  CommonConstruct();
}


PProcess::~PProcess()
{
  if (housekeepingThread != NULL)
    delete housekeepingThread;
  CommonDestruct();
}


PThread::PThread()
{
  // see InitialiseProcessThread()
}


void PThread::InitialiseProcessThread()
{
  PX_origStackSize    = 0;
  autoDelete          = FALSE;
  PX_waitingSemaphore = NULL;
  PX_threadId         = pthread_self();
  PX_suspendCount     = 0;

  PAssertOS(pthread_mutex_init(&PX_WaitSemMutex, NULL) == 0);
  PAssertOS(pthread_mutex_init(&PX_suspendMutex, NULL) == 0);

  ((PProcess *)this)->activeThreads.DisallowDeleteObjects();
  ((PProcess *)this)->activeThreads.SetAt((unsigned)PX_threadId, this);
}


PThread::PThread(PINDEX stackSize,
                 AutoDeleteFlag deletion,
                 Priority /*priorityLevel*/)
{
  PAssert(stackSize > 0, PInvalidParameter);

  PX_origStackSize = stackSize;
  autoDelete       = (deletion == AutoDeleteThread);

  PX_waitingSemaphore = NULL;
  pthread_mutex_init(&PX_WaitSemMutex, NULL);

  PAssertOS(pthread_mutex_init(&PX_suspendMutex, NULL) == 0);

  // throw the new thread
  PX_NewThread(TRUE);
}


PThread::~PThread()
{
  if (!IsTerminated())
    Terminate();

  PAssertOS(pthread_mutex_destroy(&PX_WaitSemMutex) == 0);
  PAssertOS(pthread_mutex_destroy(&PX_suspendMutex) == 0);
}


void PThread::PX_NewThread(BOOL startSuspended)
{
  // initialise suspend counter and create mutex
  PX_suspendCount = startSuspended ? 1 : 0;

  // throw the thread
  pthread_attr_t threadAttr;
  pthread_attr_init(&threadAttr);
  PAssertOS(pthread_create(&PX_threadId, NULL, PX_ThreadStart, this) == 0);
}


void * PThread::PX_ThreadStart(void * arg)
{ 
  pthread_t threadId = pthread_self();

  // self-detach
  pthread_detach(threadId);

  PThread * thread = (PThread *)arg;

  PProcess & process = PProcess::Current();

  // block RESUME_SIG
  sigset_t blockedSignals;
  sigemptyset(&blockedSignals);
  sigaddset(&blockedSignals, SIGHUP);
  sigaddset(&blockedSignals, SIGINT);
  sigaddset(&blockedSignals, SIGQUIT);
  sigaddset(&blockedSignals, SIGTERM);
  sigaddset(&blockedSignals, RESUME_SIG);
  PAssertOS(pthread_sigmask(SIG_BLOCK, &blockedSignals, NULL) == 0);

  // add thread to thread list
  process.threadMutex.Wait();
  process.activeThreads.SetAt((unsigned)threadId, thread);
  process.threadMutex.Signal();

  // make sure the cleanup routine is called when the thread exits
  pthread_cleanup_push(PThread::PX_ThreadEnd, arg);

  // if we are not supposed to start suspended, then don't wait
  // if we are supposed to start suspended, then wait for a resume
  PAssertOS(pthread_mutex_lock(&thread->PX_suspendMutex) == 0);
  if (thread->PX_suspendCount ==  0) 
    PAssertOS(pthread_mutex_unlock(&thread->PX_suspendMutex) == 0);
  else {
    PAssertOS(pthread_mutex_unlock(&thread->PX_suspendMutex) == 0);
    sigset_t waitSignals;
    sigemptyset(&waitSignals);
    sigaddset(&waitSignals, RESUME_SIG);
    sigwait(&waitSignals);
  }

  // set the signal handler for SUSPEND_SIG
  struct sigaction action;
  memset(&action, 0, sizeof(action));
  action.sa_handler = sigSuspendHandler;
  sigaction(SUSPEND_SIG, &action, 0);

  // now call the the thread main routine
  thread->Main();

  // execute the cleanup routine
  pthread_cleanup_pop(1);

  return NULL;
}


void PProcess::SignalTimerChange()
{
  timerChangeSemaphore.Signal();
}


void PThread::PX_ThreadEnd(void * arg)
{
  PThread * thread = (PThread *)arg;
  PProcess & process = PProcess::Current();

  // remove this thread from the thread list
  process.threadMutex.Wait();
  process.activeThreads.SetAt(thread->PX_GetThreadId(), NULL);
  process.threadMutex.Signal();
  
  // delete the thread if required
  if (thread->autoDelete) {
    thread->PX_threadId = 0;  // Prevent terminating terminated thread
    delete thread;
//    printf("auto deleted thread object\n");
  }
}


unsigned PThread::PX_GetThreadId() const
{
  return (unsigned)PX_threadId;
}


void PThread::Restart()
{
  PAssert(IsTerminated(), "Cannot restart running thread");
  PX_NewThread(FALSE);
}


void PThread::Terminate()
{
  if (PX_origStackSize <= 0)
    return;

  PAssert(!IsTerminated(), "Cannot terminate a thread which is already terminated");
  if (Current() == this)
    pthread_exit(NULL);
  else {
    PAssertOS(pthread_mutex_lock(&PX_WaitSemMutex) == 0);
    if (PX_waitingSemaphore != NULL) {
      PAssertOS(pthread_mutex_lock(&PX_waitingSemaphore->mutex) == 0);
      PX_waitingSemaphore->queuedLocks--;
      PAssertOS(pthread_mutex_unlock(&PX_waitingSemaphore->mutex) == 0);
      PX_waitingSemaphore = NULL;
    }
    PAssertOS(pthread_mutex_unlock(&PX_WaitSemMutex) == 0);

    pthread_kill(PX_threadId, SIGKILL);
  }
}


void PThread::PXSetWaitingSemaphore(PSemaphore * sem)
{
  PAssertOS(pthread_mutex_lock(&PX_WaitSemMutex) == 0);
  PX_waitingSemaphore = sem;
  PAssertOS(pthread_mutex_unlock(&PX_WaitSemMutex) == 0);
}


BOOL PThread::IsTerminated() const
{
  return PX_threadId == 0 || pthread_kill(PX_threadId, 0) != 0;
}


void PThread::Suspend(BOOL susp)
{
  PAssertOS(pthread_mutex_lock(&PX_suspendMutex) == 0);
  BOOL unlock = TRUE;

  if (pthread_kill(PX_threadId, 0) == 0) {

    // if suspending, then see if already suspended
    if (susp) {
      PX_suspendCount++;
      if (PX_suspendCount == 1) {
        if (PX_threadId != pthread_self()) 
          pthread_kill(PX_threadId, SUSPEND_SIG);
        else {
          unlock = FALSE;
          PAssertOS(pthread_mutex_unlock(&PX_suspendMutex) == 0);
          sigset_t waitSignals;
          sigemptyset(&waitSignals);
          sigaddset(&waitSignals, RESUME_SIG);
          sigwait(&waitSignals);
        }
      }
    }

    // if resuming, then see if to really resume
    else if (PX_suspendCount > 0) {
      PX_suspendCount--;
      if (PX_suspendCount == 0) 
        pthread_kill(PX_threadId, RESUME_SIG);
    }
  }
  if (unlock)
    PAssertOS(pthread_mutex_unlock(&PX_suspendMutex) == 0);
}


void PThread::Resume()
{
  Suspend(FALSE);
}


BOOL PThread::IsSuspended() const
{
  PAssert(!IsTerminated(), "Operation on terminated thread");
  PAssertOS(pthread_mutex_lock((pthread_mutex_t *)&PX_suspendMutex) == 0);
  BOOL suspended = PX_suspendCount > 0;
  PAssertOS(pthread_mutex_unlock((pthread_mutex_t *)&PX_suspendMutex) == 0);
  return suspended;
}


void PThread::SetPriority(Priority /*priorityLevel*/)
{
  PAssert(!IsTerminated(), "Cannot set priority of terminated thread");
}


PThread::Priority PThread::GetPriority() const
{
  PAssert(!IsTerminated(), "Cannot get priority of terminated thread");
  return LowestPriority;
}


void PThread::Yield()
{
  ::sleep(0);
}


PThread * PThread::Current()
{
  PProcess & process = PProcess::Current();
  process.threadMutex.Wait();
  PThread * thread = process.activeThreads.GetAt((unsigned)pthread_self());
  process.threadMutex.Signal();
  return PAssertNULL(thread);
}


void PThread::Sleep(const PTimeInterval & timeout)
{
  struct timeval * tptr = NULL;

  struct timeval   timeout_val;
  if (timeout != PMaxTimeInterval) {
    if (timeout.GetMilliSeconds() < 1000L*60L*60L*24L) {
      timeout_val.tv_usec = (timeout.GetMilliSeconds() % 1000) * 1000;
      timeout_val.tv_sec  = timeout.GetSeconds();
      tptr                = &timeout_val;
    }
  }
  while (::select(0, NULL, NULL, NULL, tptr) != 0)
    PProcess::Current().PXCheckSignals();
}


void PThread::WaitForTermination() const
{
  while (!IsTerminated())
    ::sleep(1);
}


BOOL PThread::WaitForTermination(const PTimeInterval & maxWait) const
{
  PTimer timeout = maxWait;
  while (!IsTerminated()) {
    if (timeout == 0)
      return FALSE;
    ::sleep(1);
  }
  return TRUE;
}


///////////////////////////////////////////////////////////////////////////////

PSemaphore::PSemaphore(unsigned initial, unsigned maxCount)
{
  PAssert(maxCount > 0, "Invalid semaphore maximum.");
  if (initial > maxCount)
    initial = maxCount;

  currentCount = initial;
  maximumCount = maxCount;
  queuedLocks  = 0;

  //pthread_mutexattr_t mutexAttr;
  //pthread_mutexattr_init(&mutexAttr);
  //pthread_mutexattr_setpshared(&mutexAttr, PTHREAD_PROCESS_PRIVATE);
  pthread_mutex_init(&mutex, NULL);

  //pthread_condattr_t condAttr;
  //pthread_condattr_init(&condAttr);
  PAssertOS(pthread_cond_init(&condVar, NULL) == 0);
}


PSemaphore::~PSemaphore()
{
  PAssertOS(pthread_mutex_lock(&mutex) == 0);
  PAssert(queuedLocks == 0, "Semaphore destroyed with queued locks");
  PAssertOS(pthread_cond_destroy(&condVar) == 0);
  PAssertOS(pthread_mutex_destroy(&mutex) == 0);
}


void PSemaphore::Wait()
{
  PAssertOS(pthread_mutex_lock(&mutex) == 0);

  queuedLocks++;
  PThread::Current()->PXSetWaitingSemaphore(this);

  while (currentCount == 0) {
    int err = pthread_cond_wait(&condVar, &mutex);
    PProcess::Current().PXCheckSignals();
    PAssertOS(err == 0);
  }

  PThread::Current()->PXSetWaitingSemaphore(NULL);
  queuedLocks--;

  currentCount--;

  PAssertOS(pthread_mutex_unlock(&mutex) == 0);
}


BOOL PSemaphore::Wait(const PTimeInterval & waitTime)
{
  if (waitTime == PMaxTimeInterval) {
    Wait();
    return TRUE;
  }

  struct timeval valTime;
  ::gettimeofday(&valTime, NULL);
  valTime.tv_sec += waitTime.GetSeconds();
  valTime.tv_usec += waitTime.GetMilliSeconds() % 1000L;
  if (valTime.tv_usec > 1000000) {
    valTime.tv_usec -= 1000000;
    valTime.tv_sec++;
  }

  struct timespec absTime;
  absTime.tv_sec = valTime.tv_sec;
  absTime.tv_nsec = valTime.tv_usec * 1000;

  PAssertOS(pthread_mutex_lock(&mutex) == 0);

  PThread::Current()->PXSetWaitingSemaphore(this);
  queuedLocks++;

  BOOL ok = TRUE;
  while (currentCount == 0) {
    int err = pthread_cond_timedwait(&condVar, &mutex, &absTime);
    PProcess::Current().PXCheckSignals();
    if (err == ETIMEDOUT) {
      ok = FALSE;
      break;
    }
    PAssertOS(err == 0);
  }

  PThread::Current()->PXSetWaitingSemaphore(NULL);
  queuedLocks--;

  if (ok)
    currentCount--;

  PAssertOS(pthread_mutex_unlock((pthread_mutex_t *)&mutex) == 0);

  return ok;
}


void PSemaphore::Signal()
{
  PAssertOS(pthread_mutex_lock(&mutex) == 0);

  if (currentCount < maximumCount)
    currentCount++;

  if (queuedLocks > 0) 
    pthread_cond_signal(&condVar);

  PAssertOS(pthread_mutex_unlock(&mutex) == 0);
}


BOOL PSemaphore::WillBlock() const
{
  return currentCount == 0;
}


PMutex::PMutex()
  : PSemaphore(1, 1)
{
}


void PMutex::Wait()
{
  PAssertOS(pthread_mutex_lock(&mutex) == 0);
}


BOOL PMutex::Wait(const PTimeInterval & waitTime)
{
  if (waitTime == PMaxTimeInterval) {
    Wait();
    return TRUE;
  }

  PTimeInterval sleepTime = waitTime/100;
  if (sleepTime > 1000)
    sleepTime = 1000;
  int subdivision = waitTime.GetMilliSeconds()/sleepTime.GetMilliSeconds();
  for (int count = 0; count < subdivision; count++) {
    if (pthread_mutex_trylock(&mutex) == 0)
      return TRUE;
    PThread::Current()->Sleep(sleepTime);
  }

  return FALSE;
}


void PMutex::Signal()
{
  PAssertOS(pthread_mutex_unlock(&mutex) == 0);
}


BOOL PMutex::WillBlock() const
{
  pthread_mutex_t * mp = (pthread_mutex_t*)&mutex;
  if (pthread_mutex_trylock(mp) != 0)
    return TRUE;
  return pthread_mutex_unlock(mp) != 0;
}


PSyncPoint::PSyncPoint()
  : PSemaphore(0, 1)
{
}



