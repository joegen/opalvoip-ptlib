//
//  Routines for pre-emptive threading system
//

#include <pthread.h>
#include <sys/resource.h>

#define	SUSPEND_SIG	SIGUSR1
#define	RESUME_SIG	SIGUSR2

PDECLARE_CLASS(HouseKeepingThread, PThread)
  public:
    HouseKeepingThread()
      : PThread(1000, AutoDeleteThread, StartImmediate) { }
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
    if (timeout.GetMilliSeconds() < 1000L*60L*60L*24L) {
      timeout_val.tv_usec = (timeout.GetMilliSeconds() % 1000) * 1000;
      timeout_val.tv_sec  = timeout.GetSeconds();
      tptr                = &timeout_val;
    }
  }

  return ::select(handle+1, read_fds, write_fds, exception_fds, tptr);
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
  sigaddset(&blockedSignals, RESUME_SIG);
  PAssert(pthread_sigmask(SIG_BLOCK, &blockedSignals, NULL) == 0, "pthread_sigmask failed");

  // set the file descriptor limit to something sensible
  struct rlimit rl;
  PAssert(getrlimit(RLIMIT_NOFILE, &rl) == 0, "getrlimit failed");
  rl.rlim_cur = rl.rlim_max;
  PAssert(setrlimit(RLIMIT_NOFILE, &rl) == 0, "setrlimit failed");

  // initialise the housekeeping thread
  housekeepingThread = NULL;
}

PProcess::~PProcess()
{
  if (housekeepingThread != NULL)
    delete housekeepingThread;
}

PThread::PThread()
{
  // see InitialiseProcessThread()
}

void PThread::InitialiseProcessThread()
{
  PX_origStackSize = 0;
  PX_autoDelete    = FALSE;
  PX_threadId      = pthread_self();

  ((PProcess *)this)->activeThreads.DisallowDeleteObjects();
  ((PProcess *)this)->activeThreads.SetAt(PX_GetThreadId(), this);
}

PThread::PThread(PINDEX stackSize,
                 AutoDeleteFlag deletion,
                 InitialSuspension start,
                 Priority /*priorityLevel*/)
  : PX_origStackSize(stackSize)
{
  PX_autoDelete  = (deletion == AutoDeleteThread);

  // throw the new thread
  PX_NewThread(start == StartSuspended);
}

PThread::~PThread()
{
//  printf("PThread destroyed\n");
}

void PThread::PX_NewThread(BOOL startSuspended)
{
  // initialise suspend counter and create mutex
  PX_suspendCount = startSuspended ? 1 : 0;
  PAssert(pthread_mutex_init(&PX_suspendMutex, NULL) == 0, "mutex create failed");

  // throw the thread
  pthread_attr_t threadAttr;
  pthread_attr_init(&threadAttr);
  PAssert(pthread_create(&PX_threadId, NULL, PX_ThreadStart, this) == 0, "thread create failed");
}

void * PThread::PX_ThreadStart(void * arg)
{ 
  // self-detach
  pthread_detach(pthread_self());

  PThread * thread = (PThread *)arg;

  PProcess & process = PProcess::Current();

  // block RESUME_SIG
  sigset_t blockedSignals;
  sigemptyset(&blockedSignals);
  sigaddset(&blockedSignals, RESUME_SIG);
  PAssert(pthread_sigmask(SIG_BLOCK, &blockedSignals, NULL) == 0, "pthread_sigmask failed");

  // add thread to thread list
  process.threadMutex.Wait();
  process.activeThreads.SetAt(thread->PX_GetThreadId(), thread);
  process.threadMutex.Signal();

  // make sure the cleanup routine is called when the thread exits
  pthread_cleanup_push(PThread::PX_ThreadEnd, arg);

  // if we are not supposed to start suspended, then don't wait
  // if we are supposed to start suspended, then wait for a resume
  PAssert(pthread_mutex_lock(&thread->PX_suspendMutex) == 0, "pthread_mutex_lock failed");
  if (thread->PX_suspendCount ==  0) 
    PAssert(pthread_mutex_unlock(&thread->PX_suspendMutex) == 0, "pthread_mutex_unlock failed");
  else {
    PAssert(pthread_mutex_unlock(&thread->PX_suspendMutex) == 0, "pthread_mutex_unlock failed");
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
  if (housekeepingThread == NULL)
    housekeepingThread = PNEW HouseKeepingThread;

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
  
  // destroy the suspend mutex
  PAssert(pthread_mutex_destroy(&thread->PX_suspendMutex) == 0, "pthread_mutex_destroy failed");

  // delete the thread if required
  if (thread->PX_autoDelete) {
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
  PAssert(!IsTerminated(), "Cannot terminate a thread which is already terminated");
  if (Current() == this) {
//    printf("self terminating thread\n");
    pthread_exit(NULL);
  } else {
//    printf("terminated thread\n");
    pthread_kill(PX_threadId, SIGKILL);
  }
}

BOOL PThread::IsTerminated() const
{
  return pthread_kill(PX_threadId, 0) != 0;
}

void PThread::Suspend(BOOL susp)
{
  PAssert(pthread_mutex_lock(&PX_suspendMutex) == 0, "pthread_mutex_lock failed");
  BOOL unlock = TRUE;

  if (pthread_kill(PX_threadId, 0) == 0) {

    // if suspending, then see if already suspended
    if (susp) {
      PX_suspendCount++;
      if (PX_suspendCount == 1) {
        if (PX_threadId != (unsigned)pthread_self()) 
          pthread_kill(PX_threadId, SUSPEND_SIG);
        else {
          unlock = FALSE;
          PAssert(pthread_mutex_unlock(&PX_suspendMutex) == 0, "pthread_mutex_unlock failed");
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
    PAssert(pthread_mutex_unlock(&PX_suspendMutex) == 0, "pthread_mutex_unlock failed");
}

void PThread::Resume()
{
  Suspend(FALSE);
}

BOOL PThread::IsSuspended() const
{
  PAssert(!IsTerminated(), "Operation on terminated thread");
  PAssert(pthread_mutex_lock((pthread_mutex_t *)&PX_suspendMutex) == 0, "pthread_mutex_lock failed");
  BOOL suspended = PX_suspendCount > 0;
  PAssert(pthread_mutex_unlock((pthread_mutex_t *)&PX_suspendMutex) == 0, "pthread_mutex_unlock failed");
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
  return thread;
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
    ;
}

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
  PAssert(pthread_cond_init(&condVar, NULL) == 0, "pthread_cond_init failed");
}

PSemaphore::~PSemaphore()
{
  PAssert(pthread_mutex_lock(&mutex) == 0, "pthread_mutex_lock failed");
  PAssert(queuedLocks == 0, "Semaphore destroyed with queued locks");
  PAssert(pthread_cond_destroy(&condVar) == 0, "pthread_cond_destroy failed");
  PAssert(pthread_mutex_destroy(&mutex) == 0, "pthread_mutex_destroy failed");
}


void PSemaphore::Wait()
{
  PAssert(pthread_mutex_lock(&mutex) == 0, "pthread_mutex_lock failed");

  queuedLocks++;
  while (currentCount == 0) 
    PAssert(pthread_cond_wait(&condVar, &mutex) == 0, "pthread_cond_wait failed");

  queuedLocks--;
  currentCount--;

  PAssert(pthread_mutex_unlock(&mutex) == 0, "pthread_mutex_unlock failed");
}

BOOL PSemaphore::Wait(const PTimeInterval & waitTime)
{
  if (waitTime == PMaxTimeInterval) {
    Wait();
    return TRUE;
  }

  struct timespec absTime;
  ::clock_gettime(CLOCK_REALTIME, &absTime);
  absTime.tv_sec += waitTime.GetMilliSeconds() / 1000L;

  PAssert(pthread_mutex_lock(&mutex) == 0, "pthread_mutex_lock failed");

  queuedLocks++;
  while (currentCount == 0) {
    int t = pthread_cond_timedwait(&condVar, &mutex, &absTime);
    if ((t == ETIME) || (t == ETIMEDOUT)) {
      queuedLocks--;
      pthread_mutex_unlock(&mutex);
      return FALSE;
    } else
      PAssert(t == 0, "pthread_cond_timedwait failed");
  }
  currentCount--;
  queuedLocks--;

  PAssert(pthread_mutex_unlock((pthread_mutex_t *)&mutex) == 0, "pthread_mutex_unlock failed");

  return TRUE;
}

void PSemaphore::Signal()
{
  PAssert(pthread_mutex_lock(&mutex) == 0, "pthread_mutex_lock failed");

  if (currentCount < maximumCount)
    currentCount++;

  if (queuedLocks > 0) 
    pthread_cond_signal(&condVar);

  PAssert(pthread_mutex_unlock(&mutex) == 0, "pthread_mutex_unlock failed");
}


BOOL PSemaphore::WillBlock() const
{
  return currentCount == 0;
}
