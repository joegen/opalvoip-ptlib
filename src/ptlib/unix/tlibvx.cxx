/*
 * tlibvx.cxx
 *
 * Thread library implementation for VxWorks
*/


class PProcess;
class PSemaphore;

#include <ptlib.h>

#define VX_LOWEST_PRIORITY          250
#define VX_LOW_PRIORITY             200
#define VX_NORMAL_PRIORITY          150
#define VX_DISPLAY_PRIORITY         100
#define VX_URGENT_DISPLAY_PRIORITY  50  

///////////////////////////////////////////////////////////////////////////////
// Threads
static int const priorities[] = {
  VX_LOWEST_PRIORITY,
  VX_LOW_PRIORITY,
  VX_NORMAL_PRIORITY,
  VX_DISPLAY_PRIORITY,
  VX_URGENT_DISPLAY_PRIORITY
};

int PThread::ThreadFunction(void *threadPtr)
{
	PAssertNULL(threadPtr);

  PThread * thread = (PThread *)threadPtr;

  PProcess & process = PProcess::Current();
  process.activeThreadMutex.Wait();
  process.activeThreads.SetAt(thread->PX_threadId, thread);
  process.activeThreadMutex.Signal();

  process.SignalTimerChange();

  thread->Main();

  return 0;
}


PThread::PThread()
 : PX_threadId(ERROR),
   priority(VX_NORMAL_PRIORITY),
   originalStackSize(0)
{
}

PThread::PThread(PINDEX stackSize,
                 AutoDeleteFlag deletion,
                 Priority priorityLevel,
     		   const PString & name
)
{
  PAssert(stackSize > 0, PInvalidParameter);
  autoDelete = deletion == AutoDeleteThread;
  originalStackSize = stackSize;

  priority = priorities[priorityLevel];

  PX_threadId =  ::taskSpawn(
					name,                         // Name
					priority,                     // Priority 
					0,                            // options	
					stackSize,                    // stacksize
					(FUNCPTR)ThreadFunction,      // entrypoint
					(int)this,0,0,0,0,0,0,0,0,0); // arg 1 --- arg 10


  PAssertOS(PX_threadId != ERROR);
  // threads are created suspended
  Suspend();
  if (autoDelete) {
    PProcess & process = PProcess::Current();
    process.deleteThreadMutex.Wait();
    process.autoDeleteThreads.Append(this);
    process.deleteThreadMutex.Signal();
  }
}


PThread::~PThread()
{
  if (originalStackSize <= 0)
    return;

  PProcess & process = PProcess::Current();
  process.activeThreadMutex.Wait();
  process.activeThreads.SetAt(PX_threadId, NULL);
  process.activeThreadMutex.Signal();
  
  if (!IsTerminated())
    Terminate();
}


void PThread::Restart()
{
  char *name = ::taskName(PX_threadId);
  PAssert(IsTerminated(), "Cannot restart running thread");

  PX_threadId =  ::taskSpawn(
					name, 										    // Name
					priority, 										// Priority 
					0,														// options	
					originalStackSize,            // stacksize
					(FUNCPTR)ThreadFunction,			// entrypoint
					(int)this,0,0,0,0,0,0,0,0,0);	// arg 1 --- arg 10
         
  PAssertOS(PX_threadId != ERROR);
}


void PThread::Terminate()
{
	PAssert(!IsTerminated(), "Cannot terminate a thread which is already terminated");
	PAssert(originalStackSize > 0, PLogicError);
	
	::taskDelete(PX_threadId);
}


BOOL PThread::IsTerminated() const
{
		STATUS stat = taskIdVerify(PX_threadId);
		return stat == ERROR;
}


void PThread::WaitForTermination() const
{
  while (!IsTerminated()) {
    Current()->Sleep(100);
	}
}


BOOL PThread::WaitForTermination(const PTimeInterval & maxWait) const
{
  if (PX_threadId == 0)
    return TRUE;

  PTimer timeout = maxWait;
  while (!IsTerminated()) {
    if (timeout == 0)	{
      return FALSE;
		}
    Current()->Sleep(100);
  }
 return TRUE;
}


void PThread::Suspend(BOOL susp)
{
  PAssert(!IsTerminated(), "Operation on terminated thread");
  if (susp)
  {
    STATUS result = ::taskSuspend(PX_threadId);

    PAssert(result != ERROR, "Thread don't want to be suspended");
  }
  else {
    Resume();
	}
}


void PThread::Resume()
{
  PAssert(!IsTerminated(), "Operation on terminated thread");
	if (!IsTerminated()) {
	  STATUS result = ::taskResume(PX_threadId);
	  PAssert(result != ERROR, "Thread doesn't want to resume");
	}
}


BOOL PThread::IsSuspended() const
{
  struct TASK_DESC info;
  STATUS result = ::taskInfoGet(PX_threadId, &info);

  PAssert(result != ERROR || PX_threadId != info.td_id, "Thread info inaccessible");
  return taskIsSuspended(PX_threadId);
}

void PThread::SetAutoDelete(AutoDeleteFlag deletion)
{
  PAssert(deletion != AutoDeleteThread || this != &PProcess::Current(), PLogicError);
  autoDelete = deletion == AutoDeleteThread;
}

void PThread::SetPriority(Priority priorityLevel)
{
  PAssert(!IsTerminated(), "Operation on terminated thread");

  priority = priorities[priorityLevel];
  STATUS result = ::taskPrioritySet(PX_threadId, priority );

  PAssert(result != ERROR, "Thread priority change error");
}


PThread::Priority PThread::GetPriority() const
{
  PAssert(!IsTerminated(), "Operation on terminated thread");

	int prio;
	taskPriorityGet(PX_threadId, &prio);

  switch (prio) {
    case VX_LOWEST_PRIORITY :
      return LowestPriority;
    case VX_LOW_PRIORITY :
      return LowPriority;
    case VX_NORMAL_PRIORITY :
      return NormalPriority;
    case VX_DISPLAY_PRIORITY :
      return HighPriority;
    case VX_URGENT_DISPLAY_PRIORITY :
      return HighestPriority;
  }
  PAssertAlways(POperatingSystemError);
  return LowestPriority;
}

void PThread::Yield()
{
  taskDelay(NO_WAIT);
}

void PThread::Sleep( const PTimeInterval & delay ) // Time interval to sleep for in microsec.
{
  taskDelay(delay.GetInterval()*sysClkRateGet()/1000);
}

void PThread::InitialiseProcessThread()
{
  originalStackSize = 0;
  autoDelete = FALSE;

  PX_threadId = ::taskIdSelf();
  PAssertOS(PX_threadId >= OK);

  ((PProcess *)this)->activeThreads.DisallowDeleteObjects();
  ((PProcess *)this)->activeThreads.SetAt(PX_threadId, this);
}


PThread * PThread::Current()
{
  PProcess & process = PProcess::Current();
  process.activeThreadMutex.Wait();
  
  PThread * thread = process.activeThreads.GetAt( taskIdSelf() );

  process.activeThreadMutex.Signal();
  return thread;
}

int PThread::PXBlockOnChildTerminate(int pid, const PTimeInterval & /*timeout*/) // Fix timeout
{

  while (!IsTerminated()) {
    Current()->Sleep(100);
  }
  return TRUE;
}

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
  if (timeout != PMaxTimeInterval) { // Clean up for infinite timeout
    static const PTimeInterval oneDay(0, 0, 0, 0, 1);

    if (timeout < oneDay) {

      timeout_val.tv_usec = (timeout.GetMilliSeconds() % 1000) * 1000;
      timeout_val.tv_sec  = timeout.GetSeconds();
      tptr                = &timeout_val;
    }
  }
  int retval = ::select(handle+1, read_fds, write_fds, exception_fds, tptr);

  return retval;
}

int PThread::PXBlockOnIO(int maxHandles,
           fd_set * readBits,
           fd_set * writeBits,
           fd_set * exceptionBits,
           const PTimeInterval & timeout,
           const PIntArray & /*osHandles*/)
{
  // make sure we flush the buffer before doing a write
  fd_set * read_fds      = readBits;
  fd_set * write_fds     = writeBits;
  fd_set * exception_fds = exceptionBits;

  struct timeval * tptr = NULL;
  struct timeval   timeout_val;
  if (timeout != PMaxTimeInterval) { // Clean up for infinite timeout
    static const PTimeInterval oneDay(0, 0, 0, 0, 1);
    if (timeout < oneDay) {
      timeout_val.tv_usec = (timeout.GetMilliSeconds() % 1000) * 1000;
      timeout_val.tv_sec  = timeout.GetSeconds();
      tptr                = &timeout_val;
    }
  }
  int retval = ::select(maxHandles, read_fds, write_fds, exception_fds, tptr);

  return retval;
}

void PThread::PXAbortBlock() const
{
}

///////////////////////////////////////////////////////////////////////////////
// PProcess::HouseKeepingThread

void PProcess::Construct()
{
  // hard coded value, change this to handle more sockets at once with the select call
  maxHandles = 1024; 
  houseKeeper=NULL;
  CommonConstruct();
}

PProcess::HouseKeepingThread::HouseKeepingThread()
  : PThread(5000, NoAutoDeleteThread, LowPriority)
{
  Resume();
}

void PProcess::HouseKeepingThread::Main()
{
	PProcess & process = PProcess::Current();

	while(1) {
		process.deleteThreadMutex.Wait();
		for (PINDEX i = 0; i < process.autoDeleteThreads.GetSize(); i++)
		{
			PThread * pThread = (PThread *) process.autoDeleteThreads.GetAt(i);
			if( pThread->IsTerminated() )
			{
				process.autoDeleteThreads.RemoveAt(i--);
			}
		}
		process.deleteThreadMutex.Signal();
		PTimeInterval nextTimer = process.timers.Process();
    if (nextTimer != PMaxTimeInterval)
		{
			if ( nextTimer.GetInterval() > 10000 )
			{
				nextTimer = 10000;
			}
		}
		breakBlock.Wait( nextTimer );
	}
}

void PProcess::SignalTimerChange()
{
  if (houseKeeper == NULL)
     houseKeeper = new HouseKeepingThread;  
  else
    houseKeeper->breakBlock.Signal();
}


///////////////////////////////////////////////////////////////////////////////
// PProcess

PProcess::~PProcess()
{
  Sleep(100);  // Give threads time to die a natural death

  delete houseKeeper;

  // OK, if there are any left we get really insistent...
  activeThreadMutex.Wait();
  for (PINDEX i = 0; i < activeThreads.GetSize(); i++) {
    PThread & thread = activeThreads.GetDataAt(i);
    if (this != &thread && !thread.IsTerminated())
      thread.Terminate();  // With extreme prejudice
  }
  activeThreadMutex.Signal();

  deleteThreadMutex.Wait();
  autoDeleteThreads.RemoveAll();
  deleteThreadMutex.Signal();

  delete configFiles;
}

///////////////////////////////////////////////////////////////////////////////
// PSemaphore

PSemaphore::PSemaphore(SEM_ID anId)
{
  semId = anId;
  PAssertOS(semId != 0);
}

PSemaphore::PSemaphore(unsigned initial, unsigned maxCount)
{
  semId = semCCreate(SEM_Q_FIFO, initial);
  PAssertOS(semId != NULL);
}

PSemaphore::~PSemaphore()
{
	if ( semId != 0 )
	{
		::semDelete(semId);
	}
}

void PSemaphore::Wait()
{
	STATUS result = semTake(semId, WAIT_FOREVER);
	PAssertOS(result != ERROR);
}


BOOL PSemaphore::Wait(const PTimeInterval & timeout)
{
  long wait;
	if (timeout == PMaxTimeInterval) {
		wait = WAIT_FOREVER;
	}
	else {
	 int ticks = sysClkRateGet();
	 wait = (timeout.GetInterval() * ticks);
 	 wait = wait / 1000;
	}

	STATUS result = semTake(semId, wait);

  return result == ERROR;
}

void PSemaphore::Signal()
{
	semGive(semId);
}

BOOL PSemaphore::WillBlock() const
{
	STATUS result = semTake(semId, WAIT_FOREVER);
	PAssertOS(result != ERROR);
  return result == ERROR;
}

///////////////////////////////////////////////////////////////////////////////
// PMutex  
PMutex::PMutex() 
: PSemaphore( ::semMCreate(SEM_Q_FIFO) )
{
}

PMutex::~PMutex()
{
	PAssertOS(::semDelete(semId) == 0);
}

void PMutex::Wait()
{
  STATUS result = semTake(semId, WAIT_FOREVER);	// wait forever
	PAssertOS(result == OK);
}

BOOL PMutex::Wait(const PTimeInterval & timeout)
{
  long wait;
	if (timeout == PMaxTimeInterval) {
		wait = WAIT_FOREVER;
	}
	else {
	 int ticks = sysClkRateGet();
	 wait = (timeout.GetMilliSeconds() * ticks);
	 wait = wait / 1000;
	}
	STATUS result = semTake(semId, wait);
  return result == ERROR;
}

void PMutex::Signal()
{
	::semGive(semId);
}

BOOL PMutex::WillBlock() const 
{
    STATUS result = semTake(semId, WAIT_FOREVER);	// wait forever
		PAssertOS(result == OK);
		return result == ERROR;
}

///////////////////////////////////////////////////////////////////////////////
// PSyncPoint

PSyncPoint::PSyncPoint()
  : PSemaphore(0, 1)
{
}

// End Of File ///////////////////////////////////////////////////////////////
