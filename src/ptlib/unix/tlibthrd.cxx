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
 * Revision 1.73  2001/09/19 17:37:47  craigs
 * Added support for nested mutexes under Linux
 *
 * Revision 1.72  2001/09/18 06:53:35  robertj
 * Made sure suspend can't exit early if get spurious signal
 *
 * Revision 1.71  2001/09/18 05:56:03  robertj
 * Fixed numerous problems with thread suspend/resume and signals handling.
 *
 * Revision 1.70  2001/09/10 03:03:02  robertj
 * Major change to fix problem with error codes being corrupted in a
 *   PChannel when have simultaneous reads and writes in threads.
 * Changed threading so does not actually start thread until Resume(), makes
 *   the logic of start up much simpler and more portable.
 * Quite a bit of tidyin up of the pthreads code.
 *
 * Revision 1.69  2001/08/30 08:57:40  robertj
 * Changed calls to usleep to be PThread::Yield(), normalising single
 *   timeslice process swap out.
 *
 * Revision 1.68  2001/08/20 06:55:45  robertj
 * Fixed ability to have PMutex::Wait() with times less than one second.
 * Fixed small error in return value of block on I/O function, not critical.
 *
 * Revision 1.67  2001/08/07 02:50:03  craigs
 * Fixed potential race condition in IO blocking
 *
 * Revision 1.66  2001/07/09 13:23:37  rogerh
 * Fix a subtle bug in semaphore wait which showed up on FreeBSD
 *
 * Revision 1.65  2001/05/29 00:49:18  robertj
 * Added ability to put in a printf %x in thread name to get thread object
 *   address into user settable thread name.
 *
 * Revision 1.64  2001/05/23 00:18:55  robertj
 * Added support for real time threads, thanks Erland Lewin.
 *
 * Revision 1.63  2001/04/20 09:27:25  robertj
 * Fixed previous change for auto delete threads, must have thread ID zeroed.
 *
 * Revision 1.62  2001/04/20 09:09:05  craigs
 * Removed possible race condition whilst shutting down threads
 *
 * Revision 1.61  2001/03/20 06:44:25  robertj
 * Lots of changes to fix the problems with terminating threads that are I/O
 *   blocked, especially when doing orderly shutdown of service via SIGTERM.
 *
 * Revision 1.60  2001/03/14 01:16:11  robertj
 * Fixed signals processing, now uses housekeeping thread to handle signals
 *   synchronously. This also fixes issues with stopping PServiceProcess.
 *
 * Revision 1.59  2001/02/25 19:39:42  rogerh
 * Use a Semaphore on Mac OS X to support threads which are started as 'suspended'
 *
 * Revision 1.58  2001/02/24 14:49:22  rogerh
 * Add missing bracket
 *
 * Revision 1.57  2001/02/24 13:29:34  rogerh
 * Mac OS X change to avoid Assertion
 *
 * Revision 1.56  2001/02/24 13:24:24  rogerh
 * Add PThread support for Mac OS X and Darwin. There is one major issue. This
 * OS does not suport pthread_kill() and sigwait() so we cannot support the
 * Suspend() and Resume() functions to start and stop threads and we cannot
 * create new threads in 'suspended' mode.
 * Calling Suspend() raises an assertion. Calling Resume() does nothing.
 * Threads started in 'suspended' mode start immediatly.
 *
 * Revision 1.55  2001/02/21 22:48:42  robertj
 * Fixed incorrect test in PSemaphore::WillBlock() just added, thank Artis Kugevics.
 *
 * Revision 1.54  2001/02/20 00:21:14  robertj
 * Fixed major bug in PSemapahore::WillBlock(), thanks Tomas Heran.
 *
 * Revision 1.53  2000/12/21 12:36:32  craigs
 * Removed potential to stop threads twice
 *
 * Revision 1.52  2000/12/05 08:24:50  craigs
 * Fixed problem with EINTR causing havoc
 *
 * Revision 1.51  2000/11/16 11:06:38  rogerh
 * Add a better fix for the "user signal 2" aborts seen on FreeBSD 4.2 and above.
 * We need to sched_yeild() after the pthread_create() to make sure the new thread
 * actually has a chance to execute. The abort problem was caused when the
 * resume signal was issued before the thread was ready for it.
 *
 * Revision 1.50  2000/11/12 23:30:02  craigs
 * Added extra WaitForTermination to assist bug location
 *
 * Revision 1.49  2000/11/12 08:16:07  rogerh
 * This change and the previous change, make pthreads work on FreeBSD 4.2.
 * FreeBSD has improved its thread signal handling and now correctly generates a
 * SIGUSR2 signal on a thread (the Resume Signal).  However there was no handler
 * for this signal and applications would abort with "User signal 2".
 * So, a dummy sigResumeHandler has been added.
 *
 * Revision 1.48  2000/11/12 07:57:45  rogerh
 * *** empty log message ***
 *
 * Revision 1.47  2000/10/31 08:09:51  rogerh
 * Change return type of PX_GetThreadId() to save unnecessary typecasting
 *
 * Revision 1.46  2000/10/31 07:52:06  rogerh
 * Add type casts to allow the code to compile on FreeBSD 4.1.1
 *
 * Revision 1.45  2000/10/30 05:48:33  robertj
 * Added assert when get nested mutex.
 *
 * Revision 1.44  2000/10/24 03:32:40  robertj
 * Fixed problem where thread that uses PThread::Current() in dtor crashes.
 *
 * Revision 1.43  2000/10/20 06:11:48  robertj
 * Added function to change auto delete flag on a thread.
 *
 * Revision 1.42  2000/09/20 04:24:09  craigs
 * Added extra tracing, and removed segv on exit when using tracing
 *
 * Revision 1.41  2000/06/21 01:01:22  robertj
 * AIX port, thanks Wolfgang Platzer (wolfgang.platzer@infonova.at).
 *
 * Revision 1.40  2000/04/13 07:21:10  rogerh
 * Fix typo in #defined
 *
 * Revision 1.39  2000/04/11 11:38:49  rogerh
 * More NetBSD Pthread changes
 *
 * Revision 1.38  2000/04/10 11:47:02  rogerh
 * Add initial NetBSD pthreads support
 *
 * Revision 1.37  2000/04/06 12:19:49  rogerh
 * Add Mac OS X support submitted by Kevin Packard
 *
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


#define	SUSPEND_SIG SIGVTALRM


PDECLARE_CLASS(PHouseKeepingThread, PThread)
  public:
    PHouseKeepingThread()
      : PThread(1000, NoAutoDeleteThread, NormalPriority, "Housekeeper")
      { closing = FALSE; Resume(); }

    void Main();
    void SetClosing() { closing = TRUE; }

  protected:
    BOOL closing;
};


#define new PNEW


void PHouseKeepingThread::Main()
{
  PProcess & process = PProcess::Current();

  while (!closing) {
    PTimeInterval delay = process.timers.Process();

    int fd = process.timerChangePipe[0];

    fd_set read_fds;
    FD_ZERO(&read_fds);
    FD_SET(fd, &read_fds);

    struct timeval tval;
    if (::select(fd+1, &read_fds, NULL, NULL, delay.AsTimeVal(tval)) == 1) {
      BYTE ch;
      ::read(fd, &ch, 1);
    }

    process.PXCheckSignals();
  }
}


void PProcess::SignalTimerChange()
{
  if (housekeepingThread == NULL) {
#if PMEMORY_CHECK
  PMemoryHeap::SetIgnoreAllocations(TRUE);
#endif
    housekeepingThread = new PHouseKeepingThread;
#if PMEMORY_CHECK
  PMemoryHeap::SetIgnoreAllocations(FALSE);
#endif
  }

  BYTE ch;
  write(timerChangePipe[1], &ch, 1);
}


void PProcess::Construct()
{
  // set the file descriptor limit to something sensible
  struct rlimit rl;
  PAssertOS(getrlimit(RLIMIT_NOFILE, &rl) == 0);
  rl.rlim_cur = rl.rlim_max;
  PAssertOS(setrlimit(RLIMIT_NOFILE, &rl) == 0);

  ::pipe(timerChangePipe);

  // initialise the housekeeping thread
  housekeepingThread = NULL;

  CommonConstruct();
}


PProcess::~PProcess()
{
  // Don't wait for housekeeper to stop if Terminate() is called from it.
  if (housekeepingThread != NULL && PThread::Current() != housekeepingThread) {
    housekeepingThread->SetClosing();
    SignalTimerChange();
    housekeepingThread->WaitForTermination();
    delete housekeepingThread;
  }
  CommonDestruct();
}


//////////////////////////////////////////////////////////////////////////////

PThread::PThread()
{
  // see InitialiseProcessThread()
}


void PThread::InitialiseProcessThread()
{
  autoDelete          = FALSE;

  PX_origStackSize    = 0;
  PX_threadId         = pthread_self();
  PX_priority         = NormalPriority;
  PX_suspendCount     = 0;

#ifndef P_HAS_SEMAPHORES
  PX_waitingSemaphore = NULL;
  PAssertOS(pthread_mutex_init(&PX_WaitSemMutex, NULL) == 0);
#endif

  PAssertOS(pthread_mutex_init(&PX_suspendMutex, NULL) == 0);

  PAssertOS(::pipe(unblockPipe) == 0);

  ((PProcess *)this)->activeThreads.DisallowDeleteObjects();
  ((PProcess *)this)->activeThreads.SetAt((unsigned)PX_threadId, this);
}


PThread::PThread(PINDEX stackSize,
                 AutoDeleteFlag deletion,
                 Priority priorityLevel,
                 const PString & name)
  : threadName(name)
{
  autoDelete = (deletion == AutoDeleteThread);

  PAssert(stackSize > 0, PInvalidParameter);
  PX_origStackSize = stackSize;
  PX_threadId = 0;
  PX_priority = priorityLevel;
  PX_suspendCount = 1;

#ifndef P_HAS_SEMAPHORES
  PX_waitingSemaphore = NULL;
  pthread_mutex_init(&PX_WaitSemMutex, NULL);
#endif

  PAssertOS(pthread_mutex_init(&PX_suspendMutex, NULL) == 0);

  PAssertOS(::pipe(unblockPipe) == 0);

  // new thread is actually started the first time Resume() is called.
  PX_firstTimeStart = TRUE;
}


PThread::~PThread()
{
  if (!IsTerminated()) 
    Terminate();

  ::close(unblockPipe[0]);
  ::close(unblockPipe[1]);

#ifndef P_HAS_SEMAPHORES
  pthread_mutex_destroy(&PX_WaitSemMutex);
#endif

  pthread_mutex_destroy(&PX_suspendMutex);
}


pthread_t PThread::PX_GetThreadId() const
{
  return PX_threadId;
}


void PThread::Restart()
{
  if (!IsTerminated())
    return;

  pthread_attr_t threadAttr;
  pthread_attr_init(&threadAttr);

#ifdef P_LINUX
  /*
    Set realtime scheduling if our effective user id is root (only then is this
    allowed) AND our priority is Highest.
      As far as I can see, we could use either SCHED_FIFO or SCHED_RR here, it
    doesn't matter.
      I don't know if other UNIX OSs have SCHED_FIFO and SCHED_RR as well.

    WARNING: a misbehaving thread (one that never blocks) started with Highest
    priority can hang the entire machine. That is why root permission is 
    neccessary.
  */
  if ((geteuid() == 0) && (PX_priority == HighestPriority))
    PAssertOS(pthread_attr_setschedpolicy(&threadAttr, SCHED_FIFO) == 0);
#endif

  PAssertOS(pthread_create(&PX_threadId, &threadAttr, PX_ThreadStart, this) == 0);
}


void PX_SuspendSignalHandler(int)
{
  PThread * thread = PThread::Current();
  if (thread == NULL)
    return;

  BOOL notResumed = TRUE;
  while (notResumed) {
    BYTE ch;
    notResumed = ::read(thread->unblockPipe[0], &ch, 1) < 0 && errno == EINTR;
    pthread_testcancel();
  }
}


void PThread::Suspend(BOOL susp)
{
  PAssertOS(pthread_mutex_lock(&PX_suspendMutex) == 0);

  // Check for start up condition, first time Resume() is called
  if (PX_firstTimeStart) {
    if (susp)
      PX_suspendCount++;
    else {
      if (PX_suspendCount > 0)
        PX_suspendCount--;
      if (PX_suspendCount == 0) {
        PX_firstTimeStart = FALSE;
        Restart();
      }
    }

    PAssertOS(pthread_mutex_unlock(&PX_suspendMutex) == 0);
    return;
  }

#ifdef P_MACOSX
  // Suspend - warn the user with an Assertion
  PAssertAlways("Cannot suspend threads on Mac OS X due to lack of pthread_kill()");
#else
  if (pthread_kill(PX_threadId, 0) == 0) {

    // if suspending, then see if already suspended
    if (susp) {
      PX_suspendCount++;
      if (PX_suspendCount == 1) {
        if (PX_threadId != pthread_self()) {
          signal(SUSPEND_SIG, PX_SuspendSignalHandler);
          pthread_kill(PX_threadId, SUSPEND_SIG);
        }
        else {
          PAssertOS(pthread_mutex_unlock(&PX_suspendMutex) == 0);
          PX_SuspendSignalHandler(SUSPEND_SIG);
          return;  // Mutex already unlocked
        }
      }
    }

    // if resuming, then see if to really resume
    else if (PX_suspendCount > 0) {
      PX_suspendCount--;
      if (PX_suspendCount == 0) 
        PXAbortBlock();
    }
  }

  PAssertOS(pthread_mutex_unlock(&PX_suspendMutex) == 0);
#endif // P_MACOSX
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
  BOOL suspended = PX_suspendCount != 0;
  PAssertOS(pthread_mutex_unlock((pthread_mutex_t *)&PX_suspendMutex) == 0);
  return suspended;
}


void PThread::SetAutoDelete(AutoDeleteFlag deletion)
{
  PAssert(deletion != AutoDeleteThread || this != &PProcess::Current(), PLogicError);
  autoDelete = deletion == AutoDeleteThread;
}


void PThread::SetPriority(Priority priorityLevel)
{
  PX_priority = priorityLevel;

#ifdef P_LINUX
  if (IsTerminated())
    return;

  struct sched_param sched_param;
  
  if ((priorityLevel == HighestPriority) && (geteuid() == 0) ) {
    sched_param.sched_priority = sched_get_priority_min( SCHED_FIFO );
    
    PAssertOS(pthread_setschedparam(PX_threadId, SCHED_FIFO, &sched_param) == 0);
  }
  else if (priorityLevel != HighestPriority) {
    /* priority 0 is the only permitted value for the SCHED_OTHER scheduler */ 
    sched_param.sched_priority = 0;
    
    PAssertOS(pthread_setschedparam(PX_threadId, SCHED_OTHER, &sched_param) == 0);
  }
#endif
}


PThread::Priority PThread::GetPriority() const
{
#ifdef LINUX
  int schedulingPolicy;
  struct sched_param schedParams;
  
  PAssertOS( pthread_getschedparam( PX_threadId, &schedulingPolicy, 
                                    &schedParams ) );
  
  switch( schedulingPolicy )
  {
    case SCHED_OTHER:
      break;
      
    case SCHED_FIFO:
    case SCHED_RR:
      return HighestPriority;
      
    default:
      /* Unknown scheduler. We don't know what priority this thread has. */
      PTRACE( 1, "PWLib\tPThread::GetPriority: unknown scheduling policy #" 
              << schedulingPolicy );
  }
#endif

  return NormalPriority; /* as good a guess as any */
}


#ifndef P_HAS_SEMAPHORES
void PThread::PXSetWaitingSemaphore(PSemaphore * sem)
{
  PAssertOS(pthread_mutex_lock(&PX_WaitSemMutex) == 0);
  PX_waitingSemaphore = sem;
  PAssertOS(pthread_mutex_unlock(&PX_WaitSemMutex) == 0);
}
#endif


void PThread::Sleep(const PTimeInterval & timeout)
{
  PTime lastTime;
  PTime targetTime = lastTime + timeout;
  do {
    PTimeInterval delay = targetTime - lastTime;
    struct timeval tval;
    if (select(0, NULL, NULL, NULL, delay.AsTimeVal(tval)) < 0 && errno != EINTR)
      break;

    pthread_testcancel();

    lastTime = PTime();
  } while (lastTime < targetTime);
}


void PThread::Yield()
{
  sched_yield();
}


PThread * PThread::Current()
{
  PProcess & process = PProcess::Current();
  process.threadMutex.Wait();
  PThread * thread = process.activeThreads.GetAt((unsigned)pthread_self());
  process.threadMutex.Signal();
  return PAssertNULL(thread);
}


void PThread::Terminate()
{
  if (PX_origStackSize <= 0)
    return;

  if (IsTerminated())
    return;

  PTRACE(1, "PWLib\tForcing termination of thread " << (void *)this);

  if (Current() == this)
    pthread_exit(NULL);
  else {
    PXAbortBlock();
    WaitForTermination(20);

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

#if defined(P_FREEBSD) || defined(P_OPENBSD) || defined(P_NETBSD) || defined (P_AIX)
    pthread_kill(PX_threadId, SIGKILL);
#else
    pthread_cancel(PX_threadId);
#endif
  }
}


BOOL PThread::IsTerminated() const
{
  if (PX_threadId == 0) {
    //PTRACE(1, "PWLib\tIsTerminated(" << (void *)this << ") = 0");
    return TRUE;
  }

#ifndef P_MACOSX
  // MacOS X does not support pthread_kill so we cannot use it
  // to test the validity of the thread
  if (pthread_kill(PX_threadId, 0) != 0)  {
    //PTRACE(1, "PWLib\tIsTerminated(" << (void *)this << ") terminated");
    return TRUE;
  }
#endif

  //PTRACE(1, "PWLib\tIsTerminated(" << (void *)this << ") not dead yet");
  return FALSE;
}


void PThread::WaitForTermination() const
{
  PAssert(Current() != this, "Waiting for self termination!");
  
  while (!IsTerminated())
    Yield(); // One time slice
}


BOOL PThread::WaitForTermination(const PTimeInterval & maxWait) const
{
  PAssert(Current() != this, "Waiting for self termination!");
  
  //PTRACE(1, "PWLib\tWaitForTermination(" << maxWait << ')');

  PTimer timeout = maxWait;
  while (!IsTerminated()) {
    if (timeout == 0)
      return FALSE;
    Yield(); // One time slice
  }
  return TRUE;
}


void * PThread::PX_ThreadStart(void * arg)
{ 
  pthread_t threadId = pthread_self();

  // self-detach
  pthread_detach(threadId);

  PThread * thread = (PThread *)arg;
  thread->SetThreadName(thread->GetThreadName());

  PProcess & process = PProcess::Current();

  // add thread to thread list
  process.threadMutex.Wait();
  process.activeThreads.SetAt((unsigned)threadId, thread);
  process.threadMutex.Signal();

  // make sure the cleanup routine is called when the thread exits
  pthread_cleanup_push(PThread::PX_ThreadEnd, arg);

  // now call the the thread main routine
  //PTRACE(1, "PWLib\tAbout to call Main");
  thread->Main();

  // execute the cleanup routine
  pthread_cleanup_pop(1);

  return NULL;
}


void PThread::PX_ThreadEnd(void * arg)
{
  PThread * thread = (PThread *)arg;
  PProcess & process = PProcess::Current();
  
  pthread_t id = thread->PX_GetThreadId();
  if (id != 0) {

    // remove this thread from the active thread list
    process.threadMutex.Wait();
    process.activeThreads.SetAt((unsigned)id, NULL);
    process.threadMutex.Signal();
  }

  // delete the thread if required, note this is done this way to avoid
  // a race condition, the thread ID cannot be zeroed before the if!
  if (thread->autoDelete) {
    thread->PX_threadId = 0;  // Prevent terminating terminated thread
    delete thread;
  }
  else
    thread->PX_threadId = 0;
}


int PThread::PXBlockOnIO(int handle, int type, const PTimeInterval & timeout)
{
  //PTRACE(1,"PWLib\tPThread::PXBlockOnIO(" << handle << ',' << type << ')');

  if ((handle < 0) || (handle >= FD_SETSIZE)) {
    errno = EBADF;
    return -1;
  }

  // make sure we flush the buffer before doing a write
  fd_set tmp_rfd, tmp_wfd, tmp_efd;
  fd_set * read_fds      = &tmp_rfd;
  fd_set * write_fds     = &tmp_wfd;
  fd_set * exception_fds = &tmp_efd;

  int retval;
  do {

    FD_ZERO(read_fds);
    FD_ZERO(write_fds);
    FD_ZERO(exception_fds);

    switch (type) {
      case PChannel::PXReadBlock:
      case PChannel::PXAcceptBlock:
        FD_SET(handle, read_fds);
        break;
      case PChannel::PXWriteBlock:
        FD_SET(handle, write_fds);
        break;
      case PChannel::PXConnectBlock:
        FD_SET(handle, write_fds);
        FD_SET(handle, exception_fds);
        break;
      default:
        PAssertAlways(PLogicError);
        return 0;
    }

    // include the termination pipe into all blocking I/O functions
    FD_SET(unblockPipe[0], read_fds);

    struct timeval tval;
    retval = ::select(PMAX(handle, unblockPipe[0])+1,
                      read_fds, write_fds, exception_fds,
                      timeout.AsTimeVal(tval));
  } while (retval < 0 && errno == EINTR);

  if ((retval == 1) && FD_ISSET(unblockPipe[0], read_fds)) {
    BYTE ch;
    ::read(unblockPipe[0], &ch, 1);
    errno = EINTR;
    retval =  -1;
    PTRACE(4, "PWLib\tUnblocked I/O");
  }

  return retval;
}

void PThread::PXAbortBlock() const
{
  BYTE ch;
  ::write(unblockPipe[1], &ch, 1);
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
  pthread_cond_destroy(&condVar);
  pthread_mutex_destroy(&mutex);
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
  PTime finishTime;
  finishTime += waitTime;

#ifdef P_HAS_SEMAPHORES

  // loop until timeout, or semaphore becomes available
  // don't use a PTimer, as this causes the housekeeping
  // thread to get very busy
  do {
    if (sem_trywait(&semId) == 0)
      return TRUE;

    PThread::Yield(); // One time slice
  } while (PTime() < finishTime);

  return FALSE;

#else

  struct timespec absTime;
  absTime.tv_sec  = finishTime.GetTimeInSeconds();
  absTime.tv_nsec = finishTime.GetMicrosecond() * 1000;

  PAssertOS(pthread_mutex_lock(&mutex) == 0);

  PThread * thread = PThread::Current();
  thread->PXSetWaitingSemaphore(this);
  queuedLocks++;

  BOOL ok = TRUE;
  while (currentCount == 0) {
    int err = pthread_cond_timedwait(&condVar, &mutex, &absTime);
    if (err == ETIMEDOUT) {
      ok = FALSE;
      break;
    }
    else
      PAssert(err == 0 || err == EINTR, psprintf("timed wait error = %i", err));
  }

  thread->PXSetWaitingSemaphore(NULL);
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
  if (sem_trywait((sem_t *)&semId) != 0) {
    PAssertOS(errno == EAGAIN);
    return TRUE;
  }
  PAssertOS(sem_post((sem_t *)&semId) == 0);
  return FALSE;
#else
  return currentCount == 0;
#endif
}


PMutex::PMutex()
  : PSemaphore(1, 1)
{
#ifdef P_HAS_RECURSIVE_MUTEX
  pthread_mutexattr_t attr;
  pthread_mutexattr_init(&attr);
  pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE_NP);
  pthread_mutex_init(&mutex, &attr);
#else
  ownerThreadId = UINT_MAX;
  lockCount = 0;
#ifdef P_HAS_SEMAPHORES
  pthread_mutex_init(&mutex, NULL);
#endif
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
#ifndef P_HAS_RECURSIVE_MUTEX
  pthread_t currentThreadId = pthread_self();

  // if the mutex is already acquired by this thread,
  // then just increment the lock count
  if (pthread_equal(ownerThreadId, currentThreadId)) {
    lockCount++;
    return;
  }
#endif

  // acquire the lock for real
  PAssertOS(pthread_mutex_lock(&mutex) == 0);

#ifndef P_HAS_RECURSIVE_MUTEX
  // set flags
  PAssert((ownerThreadId == UINT_MAX) && (lockCount == 0), "PMutex acquired whilst locked by another thread");
  ownerThreadId = currentThreadId;
#endif
}


BOOL PMutex::Wait(const PTimeInterval & waitTime)
{
  // if waiting indefinitely, then do so
  if (waitTime == PMaxTimeInterval) {
    Wait();
    return TRUE;
  }

#ifndef P_HAS_RECURSIVE_MUTEX
  // get the current thread ID
  pthread_t currentThreadId = pthread_self();

  // if we already have the mutex, return immediately
  if (pthread_equal(ownerThreadId, currentThreadId)) {
    lockCount++;
    return TRUE;
  }
#endif

  // create absolute finish time 
  PTime finishTime;
  finishTime += waitTime;

  do {
    if (pthread_mutex_trylock(&mutex) == 0) {
#ifndef P_HAS_RECURSIVE_MUTEX
      PAssert((ownerThreadId == UINT_MAX) && (lockCount == 0), "PMutex acquired whilst locked by another thread");
      ownerThreadId = currentThreadId;
#endif
      return TRUE;
    }

    PThread::Current()->Sleep(10); // sleep for 10ms
  } while (PTime() < finishTime);

  return FALSE;
}


void PMutex::Signal()
{
#ifndef P_HAS_RECURSIVE_MUTEX
  PAssert(pthread_equal(ownerThreadId, pthread_self()), "PMutex signal failed - no matching wait or signal by wrong thread");

  // if lock was recursively acquired, then decrement the counter
  if (lockCount > 0) {
    lockCount--;
    return;
  }

  // otherwise mark mutex as available
  ownerThreadId = UINT_MAX;
#endif

  // and unlock for sure
  PAssert(pthread_mutex_unlock(&mutex) == 0, "PMutex signal failed - no matching wait or signal by wrong thread");
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
