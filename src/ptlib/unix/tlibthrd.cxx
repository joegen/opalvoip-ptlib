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
 * Revision 1.36  2000/03/20 22:56:34  craigs
 * Fixed problems with race conditions caused by testing or changing
 * attributes on a terminated thread. Only occured on a fast machine!
 *
 * Revision 1.35  2000/03/17 03:45:40  craigs
 * Fixed problem with connect call hanging
 *
 * Revision 1.34  2000/03/08 12:17:09  rogerh
 * Add OpenBSD support
 *
 * Revision 1.33  2000/02/29 13:18:21  robertj
 * Added named threads to tracing, thanks to Dave Harvey
 *
 * Revision 1.32  2000/01/20 08:20:57  robertj
 * FreeBSD v3 compatibility changes, thanks Roger Hardiman & Motonori Shindo
 *
 * Revision 1.31  1999/11/18 14:02:57  craigs
 * Fixed problem with houskeeping thread termination
 *
 * Revision 1.30  1999/11/15 01:12:56  craigs
 * Fixed problem with PSemaphore::Wait consuming 100% CPU
 *
 * Revision 1.29  1999/10/30 13:44:11  craigs
 * Added correct method of aborting socket operations asynchronously
 *
 * Revision 1.28  1999/10/24 13:03:30  craigs
 * Changed to capture io break signal
 *
 * Revision 1.27  1999/09/23 06:52:16  robertj
 * Changed PSemaphore to use Posix semaphores.
 *
 * Revision 1.26  1999/09/03 02:26:25  robertj
 * Changes to aid in breaking I/O locks on thread termination. Still needs more work esp in BSD!
 *
 * Revision 1.25  1999/09/02 11:56:35  robertj
 * Fixed problem with destroying PMutex that is already locked.
 *
 * Revision 1.24  1999/08/24 13:40:56  craigs
 * Fixed problem with condwait destorys failing on linux
 *
 * Revision 1.23  1999/08/23 05:33:45  robertj
 * Made last threading changes Linux only.
 *
 * Revision 1.22  1999/08/23 05:14:13  robertj
 * Removed blocking of interrupt signals as does not work in Linux threads.
 *
 * Revision 1.21  1999/07/30 00:40:32  robertj
 * Fixed problem with signal variable in non-Linux platforms
 *
 * Revision 1.20  1999/07/19 01:32:24  craigs
 * Changed signals used in pthreads code, is used by linux version.
 *
 * Revision 1.19  1999/07/15 13:10:55  craigs
 * Fixed problem with EINTR in nontimed sempahore waits
 *
 * Revision 1.18  1999/07/15 13:05:33  robertj
 * Fixed problem with getting EINTR in semaphore wait, is normal, not error.
 *
 * Revision 1.17  1999/07/11 13:42:13  craigs
 * pthreads support for Linux
 *
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

#ifdef P_LINUX

#define	SUSPEND_SIG	SIGALRM
#define	RESUME_SIG	SIGVTALRM

#else

#define	SUSPEND_SIG	SIGUSR1
#define	RESUME_SIG	SIGUSR2

#endif

#ifdef  P_PTHREADS
#define P_IO_BREAK_SIGNAL SIGPROF
#endif

PDECLARE_CLASS(HouseKeepingThread, PThread)
  public:
    HouseKeepingThread()
      : PThread(1000, NoAutoDeleteThread) { closing = FALSE; Resume(); }

    void Main();
    void SetClosing() { closing = TRUE; }

  protected:
    BOOL closing;
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

  // include the termination pipe into all blocking I/O functions
  int width = handle+1;
  FD_SET(termPipe[0], read_fds);
  width = PMAX(width, termPipe[0]+1);

  int retval = ::select(width, read_fds, write_fds, exception_fds, tptr);
  PProcess::Current().PXCheckSignals();

  if ((retval == 1) && FD_ISSET(termPipe[0], read_fds)) {
    BYTE ch;
    ::read(termPipe[0], &ch, 1);
    errno = EINTR;
    retval =  -1;
  }

  return retval;
}


static void sigSuspendHandler(int)
{
  // wait for a resume signal
  sigset_t waitSignals;
  sigemptyset(&waitSignals);
  sigaddset(&waitSignals, RESUME_SIG);
  sigaddset(&waitSignals, SIGINT);
  sigaddset(&waitSignals, SIGQUIT);
  sigaddset(&waitSignals, SIGTERM);

  for (;;) {
    int sig;
#if defined(P_LINUX) || defined(P_FREEBSD) || defined(P_OPENBSD)
    sigwait(&waitSignals, &sig);
#else
    sig = sigwait(&waitSignals);
#endif
    PProcess::Current().PXCheckSignals();
    if (sig == RESUME_SIG)
      return;
  }
}


void HouseKeepingThread::Main()
{
  PProcess & process = PProcess::Current();

  while (!closing) {
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
  sigaddset(&blockedSignals, RESUME_SIG);
  PAssertOS(pthread_sigmask(SIG_BLOCK, &blockedSignals, NULL) == 0);

  // set the file descriptor limit to something sensible
  struct rlimit rl;
  PAssertOS(getrlimit(RLIMIT_NOFILE, &rl) == 0);
  rl.rlim_cur = rl.rlim_max;
  PAssertOS(setrlimit(RLIMIT_NOFILE, &rl) == 0);

  // initialise the housekeeping thread
  housekeepingThread = NULL;

  CommonConstruct();
}


PProcess::~PProcess()
{
  if (housekeepingThread != NULL) {
    ((HouseKeepingThread *)housekeepingThread)->SetClosing();
    SignalTimerChange();
    housekeepingThread->WaitForTermination();
    delete housekeepingThread;
  }
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
  PX_threadId         = pthread_self();
  PX_suspendCount     = 0;
  ::pipe(termPipe);

#ifndef P_HAS_SEMAPHORES
  PX_waitingSemaphore = NULL;
  PAssertOS(pthread_mutex_init(&PX_WaitSemMutex, NULL) == 0);
#endif

  PAssertOS(pthread_mutex_init(&PX_suspendMutex, NULL) == 0);

  ((PProcess *)this)->activeThreads.DisallowDeleteObjects();
  ((PProcess *)this)->activeThreads.SetAt((unsigned)PX_threadId, this);
}


PThread::PThread(PINDEX stackSize,
                 AutoDeleteFlag deletion,
                 Priority /*priorityLevel*/,
                 const PString & name)
  : threadName(name)
{
  PAssert(stackSize > 0, PInvalidParameter);

  PX_origStackSize = stackSize;
  autoDelete       = (deletion == AutoDeleteThread);

#ifndef P_HAS_SEMAPHORES
  PX_waitingSemaphore = NULL;
  pthread_mutex_init(&PX_WaitSemMutex, NULL);
#endif

  PAssertOS(pthread_mutex_init(&PX_suspendMutex, NULL) == 0);

  ::pipe(termPipe);

  // throw the new thread
  PX_NewThread(TRUE);
}


PThread::~PThread()
{
  if (!IsTerminated())
    Terminate();

  ::close(termPipe[0]);
  ::close(termPipe[1]);

#ifndef P_HAS_SEMAPHORES
  //PAssertOS(pthread_mutex_destroy(&PX_WaitSemMutex) == 0);
  pthread_mutex_destroy(&PX_WaitSemMutex);
#endif

  //PAssertOS(pthread_mutex_destroy(&PX_suspendMutex) == 0);
  pthread_mutex_destroy(&PX_suspendMutex);
}


void PThread::PX_NewThread(BOOL startSuspended)
{
  // initialise suspend counter and create mutex
  PX_suspendCount = startSuspended ? 1 : 0;

  // throw the thread
//  pthread_attr_t threadAttr;
//  pthread_attr_init(&threadAttr);
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
  sigaddset(&blockedSignals, RESUME_SIG);
  //sigaddset(&blockedSignals, P_IO_BREAK_SIGNAL);
  PAssertOS(pthread_sigmask(SIG_BLOCK, &blockedSignals, NULL) == 0);

  // add thread to thread list
  process.threadMutex.Wait();
  process.activeThreads.SetAt((unsigned)threadId, thread);
  process.threadMutex.Signal();

  // make sure the cleanup routine is called when the thread exits
  pthread_cleanup_push(PThread::PX_ThreadEnd, arg);

  // if we are not supposed to start suspended, then don't wait
  // if we are supposed to start suspended, then wait for a resume

  //PAssertOS(pthread_mutex_lock(&thread->PX_suspendMutex) == 0);
  //if (thread->PX_suspendCount ==  0) 
  //  PAssertOS(pthread_mutex_unlock(&thread->PX_suspendMutex) == 0);
  //else {
  //  PAssertOS(pthread_mutex_unlock(&thread->PX_suspendMutex) == 0);

  if (thread->PX_suspendCount != 0) {
    sigset_t waitSignals;
    sigemptyset(&waitSignals);
    sigaddset(&waitSignals, RESUME_SIG);
#if defined(P_LINUX) || defined(P_FREEBSD) || defined(P_OPENBSD)
  int sig;
  sigwait(&waitSignals, &sig);
#else
    sigwait(&waitSignals);
#endif
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
  if (housekeepingThread == NULL)
    housekeepingThread = PNEW HouseKeepingThread;
  else
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
  
  thread->PX_threadId = 0;  // Prevent terminating terminated thread

  // delete the thread if required
  if (thread->autoDelete)
    delete thread;
}


unsigned PThread::PX_GetThreadId() const
{
  return (unsigned)PX_threadId;
}


void PThread::Restart()
{
  if (IsTerminated())
    return;

  PX_NewThread(FALSE);
}


void PThread::Terminate()
{
  if (PX_origStackSize <= 0)
    return;

  if (IsTerminated())
    return;

  if (Current() == this)
    pthread_exit(NULL);
  else {
#ifndef P_HAS_SEMAPHORES
    PAssertOS(pthread_mutex_lock(&PX_WaitSemMutex) == 0);
    if (PX_waitingSemaphore != NULL) {
      PAssertOS(pthread_mutex_lock(&PX_waitingSemaphore->mutex) == 0);
      PX_waitingSemaphore->queuedLocks--;
      PAssertOS(pthread_mutex_unlock(&PX_waitingSemaphore->mutex) == 0);
      PX_waitingSemaphore = NULL;
    }
    PAssertOS(pthread_mutex_unlock(&PX_WaitSemMutex) == 0);
#endif

#if defined(P_FREEBSD) || defined(P_OPENBSD)
    pthread_kill(PX_threadId, SIGKILL);
#else
    pthread_cancel(PX_threadId);
#endif
  }
}


void PThread::PXSetWaitingSemaphore(PSemaphore * sem)
{
#ifndef P_HAS_SEMAPHORES
  PAssertOS(pthread_mutex_lock(&PX_WaitSemMutex) == 0);
  PX_waitingSemaphore = sem;
  PAssertOS(pthread_mutex_unlock(&PX_WaitSemMutex) == 0);
#endif
}


BOOL PThread::IsTerminated() const
{
  if (PX_threadId == 0) 
    return TRUE;

  if (pthread_kill(PX_threadId, 0) != 0) 
    return TRUE;

  return FALSE;
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
#if defined(P_LINUX) || defined(P_FREEBSD) || defined(P_OPENBSD)
          int sig;
          sigwait(&waitSignals, &sig);
#else
          sigwait(&waitSignals);
#endif
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
  if (IsTerminated())
    return FALSE;

  PAssertOS(pthread_mutex_lock((pthread_mutex_t *)&PX_suspendMutex) == 0);
  BOOL suspended = PX_suspendCount > 0;
  PAssertOS(pthread_mutex_unlock((pthread_mutex_t *)&PX_suspendMutex) == 0);
  return suspended;
}


void PThread::SetPriority(Priority /*priorityLevel*/)
{
}


PThread::Priority PThread::GetPriority() const
{
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
  BYTE ch;
  ::write(termPipe[1], &ch, 1);

  while (!IsTerminated())
    Current()->Sleep(10);
}


BOOL PThread::WaitForTermination(const PTimeInterval & maxWait) const
{
  BYTE ch;
  ::write(termPipe[1], &ch, 1);

  PTimer timeout = maxWait;
  while (!IsTerminated()) {
    if (timeout == 0)
      return FALSE;
    Current()->Sleep(10);
  }
  return TRUE;
}


///////////////////////////////////////////////////////////////////////////////

PSemaphore::PSemaphore(unsigned initial, unsigned maxCount)
{
#ifdef P_HAS_SEMAPHORES
  PAssertOS(sem_init(&semId, 0, initial) == 0);
#else
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
#endif
}


PSemaphore::~PSemaphore()
{
#ifdef P_HAS_SEMAPHORES
  PAssertOS(sem_destroy(&semId) == 0);
#else
  PAssertOS(pthread_mutex_lock(&mutex) == 0);
  PAssert(queuedLocks == 0, "Semaphore destroyed with queued locks");
#if defined (P_LINUX) || (P_FREEBSD) || defined(P_OPENBSD)
  pthread_cond_destroy(&condVar);
  pthread_mutex_destroy(&mutex);
#else
  PAssertOS(pthread_cond_destroy(&condVar) == 0);
  PAssertOS(pthread_mutex_destroy(&mutex) == 0);
#endif
#endif
}


void PSemaphore::Wait()
{
#ifdef P_HAS_SEMAPHORES
  PAssertOS(sem_wait(&semId) == 0);
#else
  PAssertOS(pthread_mutex_lock(&mutex) == 0);

  queuedLocks++;
  PThread::Current()->PXSetWaitingSemaphore(this);

  while (currentCount == 0) {
    int err = pthread_cond_wait(&condVar, &mutex);
    PProcess::Current().PXCheckSignals();
    PAssert(err == 0 || err == EINTR, psprintf("wait error = %i", err));
  }

  PThread::Current()->PXSetWaitingSemaphore(NULL);
  queuedLocks--;

  currentCount--;

  PAssertOS(pthread_mutex_unlock(&mutex) == 0);
#endif
}


BOOL PSemaphore::Wait(const PTimeInterval & waitTime)
{
  if (waitTime == PMaxTimeInterval) {
    Wait();
    return TRUE;
  }

  // create absolute finish time 
  struct timeval finishTime;
  ::gettimeofday(&finishTime, NULL);
  finishTime.tv_sec += waitTime.GetSeconds();
  finishTime.tv_usec += waitTime.GetMilliSeconds() % 1000L;
  if (finishTime.tv_usec > 1000000) {
    finishTime.tv_usec -= 1000000;
    finishTime.tv_sec++;
  }

#ifdef P_HAS_SEMAPHORES

  // loop until timeout, or semaphore becomes available
  // don't use a PTimer, as this causes the housekeeping
  // thread to get very busy
  for (;;) {
    if (sem_trywait(&semId) == 0)
      return TRUE;

      PThread::Current()->Sleep(10);

      struct timeval now;
      ::gettimeofday(&now, NULL);
      if (now.tv_sec > finishTime.tv_sec) 
        return FALSE;
      else if ((now.tv_sec == finishTime.tv_sec) && (now.tv_usec >= finishTime.tv_usec))
        return FALSE;
  }
  return FALSE;

#else

  struct timespec absTime;
  absTime.tv_sec  = finishTime.tv_sec;
  absTime.tv_nsec = finishTime.tv_usec * 1000;

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
    else
      PAssert(err == 0 || err == EINTR, psprintf("timed wait error = %i", err));
  }

  PThread::Current()->PXSetWaitingSemaphore(NULL);
  queuedLocks--;

  if (ok)
    currentCount--;

  PAssertOS(pthread_mutex_unlock((pthread_mutex_t *)&mutex) == 0);

  return ok;
#endif
}


void PSemaphore::Signal()
{
#ifdef P_HAS_SEMAPHORES
  PAssertOS(sem_post(&semId) == 0);
#else
  PAssertOS(pthread_mutex_lock(&mutex) == 0);

  if (currentCount < maximumCount)
    currentCount++;

  if (queuedLocks > 0) 
    pthread_cond_signal(&condVar);

  PAssertOS(pthread_mutex_unlock(&mutex) == 0);
#endif
}


BOOL PSemaphore::WillBlock() const
{
#ifdef P_HAS_SEMAPHORES
  return sem_trywait((sem_t *)&semId) != 0;
#else
  return currentCount == 0;
#endif
}


PMutex::PMutex()
  : PSemaphore(1, 1)
{
#ifdef P_HAS_SEMAPHORES
  pthread_mutex_init(&mutex, NULL);
#endif
}


PMutex::~PMutex()
{
  pthread_mutex_unlock(&mutex);
#ifdef P_HAS_SEMAPHORES
  PAssertOS(pthread_mutex_destroy(&mutex) == 0);
#endif
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



