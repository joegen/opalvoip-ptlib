 /*
 * tlibbe.cxx
 *
 * Thread library implementation for BeOS
 *
 * Portable Windows Library
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
 * Portions are Copyright (c) 1993-1998 Equivalence Pty. Ltd.
 *
 * Portions are Copyright (C) 1993 Free Software Foundation, Inc.
 * All Rights Reserved.
 *
 * Contributor(s): Yuri Kiryanov, ykiryanov at users.sourceforge.net
 *
 * $Log: tlibbe.cxx,v $
 * Revision 1.22  2004/04/18 00:23:40  ykiryanov
 * Rearranged code to be more reliable. We nearly there
 *
 * Revision 1.21  2004/04/02 03:17:19  ykiryanov
 * New version, improved
 *
 * Revision 1.20  2004/02/23 23:40:42  ykiryanov
 * Added missing constructor for PMutex
 *
 * Revision 1.19  2004/02/23 21:23:09  ykiryanov
 * Removed assert line to enable semaphore constructor
 *
 * Revision 1.18  2004/02/23 20:37:17  ykiryanov
 * Changed function definition PXBlockIO to prototype one
 *
 * Revision 1.17  2004/02/23 18:10:39  ykiryanov
 * Added a parameter to semaphore constructor to avoid ambiguity
 *
 * Revision 1.16  2004/02/23 00:02:20  ykiryanov
 * Changed my e-mail to ykiryanov at users.sourceforge.net. Just in case someone wants to collaborate
 *
 * Revision 1.15  2004/02/22 23:59:28  ykiryanov
 * Added missing functions: PProcess::SetMaxHandles(), PThread::GetCurrentThreadId(), 
 * PThread::PXAbortBlock(), PSyncPoint::Signal(), ::Wait(), ::Wait(timeout), ::WillBlock()
 *
 * Revision 1.14  2004/02/22 04:35:04  ykiryanov
 * Removed PMutex desctructor
 *
 * Revision 1.13  2003/02/26 01:13:18  robertj
 * Fixed race condition where thread can terminatebefore an IsSuspeded() call
 *   occurs and cause an assert, thanks Sebastian Meyer
 *
 * Revision 1.12  2001/06/30 06:59:07  yurik
 * Jac Goudsmit from Be submit these changes 6/28. Implemented by Yuri Kiryanov
 *
 * Revision 1.11  2001/03/07 06:57:32  yurik
 * Changed email to current one
 *
 * Revision 1.10  2001/01/16 12:32:06  rogerh
 * Remove duplicate SetAutoDelete() function. Submitted by
 * Jac Goudsmit <jac_goudsmit@yahoo.com>
 *
 *
 */

class PProcess;
class PSemaphore;

#include <ptlib.h>

//#define DEBUG_THREADS

///////////////////////////////////////////////////////////////////////////////
// Threads

static int const priorities[] = {
  1, // Lowest priority is 1. 0 is not
  B_LOW_PRIORITY,
  B_NORMAL_PRIORITY,
  B_DISPLAY_PRIORITY,
  B_URGENT_DISPLAY_PRIORITY,
};

int32 PThread::ThreadFunction(void * threadPtr)
{
  PThread * thread = (PThread *)PAssertNULL(threadPtr);

  PProcess & process = PProcess::Current();

  process.activeThreadMutex.Wait();
  process.activeThreads.SetAt((unsigned) thread->mId, thread);
  process.activeThreadMutex.Signal();

  process.SignalTimerChange();

  thread->Main();

  return 0;
}

PThread::PThread()
 : autoDelete(TRUE),
   mId(B_BAD_THREAD_ID),
   mPriority(B_NORMAL_PRIORITY),
   mStackSize(0),
   mSuspendCount(0)
{
}

PThread::PThread(PINDEX stackSize,
                 AutoDeleteFlag deletion,
                 Priority priorityLevel,
                 const PString & name)
 : mId(B_BAD_THREAD_ID),
   mPriority(B_NORMAL_PRIORITY),
   mStackSize(0),
   mSuspendCount(0)
{
  PAssert(stackSize > 0, PInvalidParameter);
  autoDelete = deletion == AutoDeleteThread;
 
  mId =  ::spawn_thread(ThreadFunction, // Function 
         (const char*) name, // Name
         priorities[priorityLevel], // Priority 
         (void *) this); // Pass this as cookie

  PAssertOS(mId >= B_NO_ERROR);
    
  mSuspendCount = 1;
  mStackSize = stackSize;
  mPriority = priorities[priorityLevel];

  threadName.sprintf(name, mId);
  ::rename_thread(mId, (const char*) threadName); // real, unique name - with id

  #ifdef DEBUG_THREADS
  PError << ">>> Spawned thread " << (const char*) threadName << ", id: " << mId << ", priority: " << mPriority << endl;
  #endif
  
  ::pipe(mUnblockPipe);
}

PThread::~PThread()
{
  if (!IsTerminated())
    Terminate();

//  /*/ remove this thread from the active thread list
  PProcess & process = PProcess::Current();
  process.activeThreadMutex.Wait();
  process.activeThreads.SetAt((unsigned) mId, NULL);
  process.activeThreadMutex.Signal();
//  */

  ::close(mUnblockPipe[0]);
  ::close(mUnblockPipe[1]);
}


void PThread::Restart()
{
  if(!IsTerminated())
    return;

  mId =  ::spawn_thread(ThreadFunction, // Function 
         "PWLT", // Name
          mPriority, 
          (void *) this); // Pass this as cookie

  PAssertOS(mId >= B_NO_ERROR);

  threadName.sprintf("PWLib Thread %d", mId);
  ::rename_thread(mId, (const char*) threadName); // real, unique name - with id

  #ifdef DEBUG_THREADS
  PError << ">>> Spawned (restarted) thread " << (const char*) threadName << ", id: " << mId << ", priority: " << mPriority << endl;
  #endif
}


void PThread::Terminate()
{
  PAssert(!IsTerminated(), "Operation on terminated thread");
  PAssert(mStackSize > 0, PLogicError);
	
  if (Current() == this)
  {
    sem_id semId = ::create_sem( 1, "Current PThread terminate semaphore" );
    if (::acquire_sem(semId) == B_NO_ERROR)
    {
      // Invalidate the thread
      mId = B_BAD_THREAD_ID;
      ::release_sem(semId);
      ::delete_sem(semId);
		
      #ifdef DEBUG_THREADS
      PError << ">>> Exiting thread, id:" << mId << endl;
      #endif
      ::exit_thread(0);
    }
  }
  else 
  {
    sem_id semId = ::create_sem( 1, "Non-current PThread terminate semaphore" );
    if (::acquire_sem(semId) == B_NO_ERROR)
    {
      thread_id idToKill;
      idToKill = mId;

      // Invalidate the thread
      mId = B_BAD_THREAD_ID;
			
      // Kill it
      if (idToKill != B_BAD_THREAD_ID)
      { 
        ::release_sem(semId);
	::delete_sem(semId);

        #ifdef DEBUG_THREADS
        PError << ">>> Killing thread, id: " << idToKill << endl;
        #endif
	
        ::kill_thread(idToKill);
      }
    }
  }
  PAssert(mId == B_BAD_THREAD_ID, "Can't acquire semaphore to terminate thread");
}


BOOL PThread::IsTerminated() const
{
  return mId == B_BAD_THREAD_ID;
}


void PThread::WaitForTermination() const
{
  WaitForTermination(PMaxTimeInterval);
}


BOOL PThread::WaitForTermination(const PTimeInterval & /*maxWait*/) const // Fix timeout
{
  status_t result = B_NO_ERROR;
  status_t exit_value = B_NO_ERROR;

  #ifdef DEBUG_THREADS
  PError << "::wait_for_thread(" << mId << "), result:";
  #endif

  result = ::wait_for_thread(mId, &exit_value);
  if ( result == B_INTERRUPTED ) { // thread was killed.
    #ifdef DEBUG_THREADS
    PError << "B_INTERRUPTED" << endl;
    #endif
    return TRUE;
  }

  if ( result == B_OK ) { // thread is dead
    #ifdef DEBUG_THREADS
    PError << "B_OK" << endl;
    #endif
    return TRUE;
  }

  if ( result == B_BAD_THREAD_ID ) { // thread has invalid id
    #ifdef DEBUG_THREADS
    PError << "B_BAD_THREAD_ID" << endl;
    #endif
    return TRUE;
  }

  return FALSE;
}


void PThread::Suspend(BOOL susp)
{
  //debugger("Suspend");

  PAssert(!IsTerminated(), "Operation on terminated thread");
  if (susp)
  {
    #ifdef DEBUG_THREADS
    PError << "Suspending thread " << (const char*) threadName << ", id: " << mId << ", priority: " << mPriority << endl;
    #endif

    status_t result = ::suspend_thread(mId);
    if(B_OK == result)
	::atomic_add(&mSuspendCount, 1);

    PAssert(result == B_OK, "Thread don't want to be suspended");
  }
  else
    Resume();
}


void PThread::Resume()
{
  PAssert(!IsTerminated(), "Operation on terminated thread");
  status_t result = ::resume_thread(mId);
  if(B_OK == result)
    ::atomic_add(&mSuspendCount, -1);

  PAssert(result == B_NO_ERROR, "Thread doesn't want to resume");
}


BOOL PThread::IsSuspended() const
{
  #ifdef DEBUG_THREADS
  PError << "Checking if thread suspended " << (const char*) threadName << ", id: " << mId << ", suspend count: " << mSuspendCount << endl;
  #endif

  return (mSuspendCount > 0);
}

void PThread::SetAutoDelete(AutoDeleteFlag deletion)
{
  PAssert(deletion != AutoDeleteThread || this != &PProcess::Current(), PLogicError);
  autoDelete = deletion == AutoDeleteThread;
}

void PThread::SetPriority(Priority priorityLevel)
{
  PAssert(!IsTerminated(), "Operation on terminated thread");

  mPriority = priorities[priorityLevel];
  status_t result = ::set_thread_priority(mId, mPriority );

  PAssert(result == B_OK, "Thread priority change error");
}


PThread::Priority PThread::GetPriority() const
{
  PAssert(!IsTerminated(), "Operation on terminated thread");

  switch (mPriority) {
    case 0 :
      return LowestPriority;
    case B_LOW_PRIORITY :
      return LowPriority;
    case B_NORMAL_PRIORITY :
      return NormalPriority;
    case B_DISPLAY_PRIORITY :
      return HighPriority;
    case B_URGENT_DISPLAY_PRIORITY :
      return HighestPriority;
  }
  PAssertAlways(POperatingSystemError);
  return LowestPriority;
}

void PThread::Yield()
{
  // we just sleep for long enough to cause a reschedule (100 microsec)
  ::snooze(100);
}

void PThread::Sleep( const PTimeInterval & delay ) // Time interval to sleep for.
{
  bigtime_t microseconds = 
		delay == PMaxTimeInterval ? B_INFINITE_TIMEOUT : (delay.GetMilliSeconds() * 1000 );
 
  status_t result = ::snooze( microseconds ) ; // delay in ms, snooze in microsec
  PAssert(result == B_OK, "Thread has insomnia");
}

void PThread::InitialiseProcessThread()
{
  mStackSize = 0;

  autoDelete = FALSE;
  
  ::pipe(mUnblockPipe);

  ((PProcess *)this)->activeThreads.DisallowDeleteObjects();
  ((PProcess *)this)->activeThreads.SetAt(mId, this);
}


PThread * PThread::Current()
{
  PProcess & process = PProcess::Current();
  thread_id currentId = GetCurrentThreadId();
  PAssertOS(currentId >= B_NO_ERROR);

  process.activeThreadMutex.Wait();
  PThread * thread = process.activeThreads.GetAt((unsigned) currentId);
  process.activeThreadMutex.Signal();

  return thread;
}

int PThread::PXBlockOnChildTerminate(int pid, const PTimeInterval & /*timeout*/) // Fix timeout
{
  status_t result = B_NO_ERROR;
  status_t exit_value = B_NO_ERROR;

  #ifdef DEBUG_THREADS
  PError << "::wait_for_thread(" << pid << "), result:";
  #endif

  result = ::wait_for_thread(pid, &exit_value);
  if ( result == B_INTERRUPTED ) 
  { 
    // thread was killed.
    #ifdef DEBUG_THREADS
    PError << "B_INTERRUPTED" << endl;
    #endif
    return 1;
  }

  if ( result == B_OK ) 
  { 
    // thread is dead
    #ifdef DEBUG_THREADS
    PError << "B_OK" << endl;
    #endif
     return 1;
  }

  if ( result == B_BAD_THREAD_ID ) 
  { 
    // thread has invalid id
    #ifdef DEBUG_THREADS
    PError << "B_BAD_THREAD_ID" << endl;
    #endif
  
    return 1;
  }

  return 0; // ???
}

PThreadIdentifier PThread::GetCurrentThreadId(void)
{
  return ::find_thread(NULL);
}

int PThread::PXBlockOnIO(int handle, int type, const PTimeInterval & timeout)
{
  int retval = 0;

  // make sure we flush the buffer before doing a write
  fd_set tmp_rfd, tmp_wfd, tmp_efd;
  fd_set * read_fds      = &tmp_rfd;
  fd_set * write_fds     = &tmp_wfd;
  fd_set * exception_fds = &tmp_efd;

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

  #ifdef DEBUG_THREADS
  PError << ">> PXBlockOnIO Timeval, sec " << timeout_val.tv_sec << ", usec" << timeout_val.tv_usec << endl;
  #endif

  for (;;) 
  { 
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
	  
	  } // switch(type)
	
    // include the termination pipe into all blocking I/O functions
    int width = handle+1;
    FD_SET(mUnblockPipe[0], read_fds);
    width = PMAX(width, mUnblockPipe[0]+1);
  
    retval = ::select(width, read_fds, write_fds, exception_fds, tptr);

    if ((retval >= 0) || (errno != EINTR))
      break;
  } // end for (;;)

  if ((retval == 1) && FD_ISSET(mUnblockPipe[0], read_fds)) 
  {
    BYTE ch;
    ::read(mUnblockPipe[0], &ch, 1);
    errno = EINTR;
    retval =  -1;
    #ifdef DEBUG_THREADS
    PError << ">>> Unblocked I/O" << endl;
    #endif
  }

  return retval;
}

int PThread::PXBlockOnIO(int maxHandles,
           fd_set * readBits,
           fd_set * writeBits,
           fd_set * exceptionBits,
           const PTimeInterval & timeout,
           const PIntArray & /*osHandles*/)
{
  int retval = 0;

  struct timeval * tptr = NULL;
  struct timeval   timeout_val;
  if (timeout != PMaxTimeInterval) 
  { 
    // Clean up for infinite timeout
    static const PTimeInterval oneDay(0, 0, 0, 0, 1);
    if (timeout < oneDay) {
      timeout_val.tv_usec = (timeout.GetMilliSeconds() % 1000) * 1000;
      timeout_val.tv_sec  = timeout.GetSeconds();
      tptr                = &timeout_val;
    }
  }

   #ifdef DEBUG_THREADS
  PError << ">> PXBlockOnIO Timeval, sec " << timeout_val.tv_sec << ", usec" << timeout_val.tv_usec << endl;
  #endif

  retval = ::select(maxHandles, readBits, writeBits, exceptionBits, tptr);
  PProcess::Current().PXCheckSignals();

  return retval;
}

void PThread::PXAbortBlock(void) const
{
  BYTE ch;
  ::write(mUnblockPipe[1], &ch, 1);
}

///////////////////////////////////////////////////////////////////////////////
// PProcess
PDECLARE_CLASS(PHouseKeepingThread, PThread)
  public:
    PHouseKeepingThread()
      : PThread(1000, NoAutoDeleteThread, NormalPriority, "PWLib Housekeeper")
      { closing = FALSE; Resume(); }

    void Main();
    void SetClosing() { closing = TRUE; }

  protected:
    BOOL closing;
};

void PProcess::Construct()
{
  ::pipe(timerChangePipe);

  // initialise the housekeeping thread
  housekeepingThread = NULL;

  CommonConstruct();
}

void PHouseKeepingThread::Main()
{
  PProcess & process = PProcess::Current();

  while (!closing) {
    PTimeInterval delay = process.timers.Process();

    int fd = process.timerChangePipe[0];
    
    fd_set tmp_rfd;
    fd_set * read_fds = &tmp_rfd;

    FD_ZERO(read_fds);
    FD_SET(fd, read_fds);
    
    static const PTimeInterval oneDay(0, 0, 0, 0, 1);
    struct timeval * tptr = NULL;
    timeval tval;
    if (delay < oneDay) {
      tval.tv_usec = (delay.GetMilliSeconds() % 1000) * 1000;
      tval.tv_sec  = delay.GetSeconds();
      tptr                = &tval;
    }
    if (::select(fd+1, read_fds, NULL, NULL, tptr) == 1) {
      BYTE ch;
      ::read(fd, &ch, 1);
    }

    process.PXCheckSignals();
  }
}

void PProcess::SignalTimerChange()
{
  if (housekeepingThread == NULL) {
    housekeepingThread = new PHouseKeepingThread;
  }

  BYTE ch;
  write(timerChangePipe[1], &ch, 1);
}

BOOL PProcess::SetMaxHandles(int)
{
  return TRUE;
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

///////////////////////////////////////////////////////////////////////////////
// PSemaphore

PSemaphore::PSemaphore(unsigned initial, unsigned maxCount)
  : mOwner(::find_thread(NULL)), semId(::create_sem(initial, "PWLS")), mCount(initial)
{
  PAssertOS(semId >= B_NO_ERROR);
  PAssertOS(mOwner != B_BAD_THREAD_ID);

  #ifdef DEBUG_SEMAPHORES
  sem_info info;
  get_sem_info(semId, &info);
  PError << "::create_sem (PSemaphore(i,m)), id: " << semId << ", this: " << this << ", name: " << info.name << ", count:" << info.count << endl;
  #endif 
}
 

PSemaphore::~PSemaphore()
{
  status_t result = B_NO_ERROR;
  PAssertOS(semId >= B_NO_ERROR);
  
  // Transmit ownership of the semaphore to our thread
  thread_id curThread = ::find_thread(NULL);
  if(mOwner != curThread)
  {
     thread_info tinfo;
     ::get_thread_info(curThread, &tinfo);
     ::set_sem_owner(semId, tinfo.team);
      mOwner = curThread; 
  } 
 
  #ifdef DEBUG_SEMAPHORES
  sem_info info;
  get_sem_info(semId, &info);
  PError << "::delete_sem, id: " << semId << ", this: " << this << ", name: " << info.name << ", count:" << info.count;
  #endif 

  // Deleting the semaphore id
  result = ::delete_sem(semId);

  #ifdef DEBUG_SEMAPHORES
  if( result != B_NO_ERROR )
    PError << "...delete_sem failed, error: " << strerror(result) << endl;
  #endif 
}

void PSemaphore::Wait()
{
  PAssertOS(semId >= B_NO_ERROR);
 
  status_t result = B_NO_ERROR;

  #ifdef DEBUG_SEMAPHORES
  sem_info info;
  get_sem_info(semId, &info);
  PError << "::acquire_sem_etc, id: " << semId << ", this: " << this << ", name: " << info.name << ", count:" << info.count;
  #endif 

  while ((B_BAD_THREAD_ID != mOwner) 
    && ((result = ::acquire_sem(semId)) == B_INTERRUPTED))
  {
  }

  #ifdef DEBUG_SEMAPHORES
  if( result != B_NO_ERROR )
    PError << "... failed, error: " << strerror(result);

  PError << endl;
  #endif 
}

BOOL PSemaphore::Wait(const PTimeInterval & timeout)
{
  PAssertOS(semId >= B_NO_ERROR);
 
  status_t result = B_NO_ERROR;
   
  PInt64 ms = timeout.GetMilliSeconds();
  bigtime_t microseconds = 
   ms? timeout == PMaxTimeInterval ? B_INFINITE_TIMEOUT : ( ms * 1000 ) : 0;
 
  #ifdef DEBUG_SEMAPHORES
  sem_info info;
  get_sem_info(semId, &info);
  PError << "::acquire_sem_etc " << semId << ",this: " << this << ", name: " << info.name << ", count:" << info.count << ", timeout:";

  if( microseconds == B_INFINITE_TIMEOUT ) 
    PError << "infinite";
  else
    PError << microseconds << " ms";
  #endif 
	
  while((B_BAD_THREAD_ID != mOwner) 
    && ((result = ::acquire_sem_etc(semId, 1, 
      B_RELATIVE_TIMEOUT, microseconds)) == B_INTERRUPTED))
  {
  }

  #ifdef DEBUG_SEMAPHORES
  if( result != B_NO_ERROR ) 
   PError << " ... failed! error: " << strerror(result);	  

  PError << " " << endl;
  #endif
	
  return result == B_TIMED_OUT;
}

void PSemaphore::Signal()
{
  PAssertOS(semId >= B_NO_ERROR);
 
  #ifdef DEBUG_SEMAPHORES
  sem_info info;
  get_sem_info(semId, &info);
  PError << "::release_sem " << semId << ", this: " << this << ", name: " << info.name << ", count:" << info.count;

  status_t result = 
  #endif 
    ::release_sem(semId);
		
  #ifdef DEBUG_SEMAPHORES
  if( result != B_NO_ERROR ) 
    PError << "... failed, error: " << strerror(result);

  PError << endl;
  #endif 
}

BOOL PSemaphore::WillBlock() const
{
  PAssertOS(semId >= B_NO_ERROR);
 
  status_t result = B_NO_ERROR;

  #ifdef DEBUG_SEMAPHORES
  sem_info info;
  get_sem_info(semId, &info);
  PError << "::acquire_sem_etc (WillBlock) " << semId << ", this: " << this << ", name: " << info.name << ", count:" << info.count;
  #endif
	
  result = ::acquire_sem_etc(semId, 0, B_RELATIVE_TIMEOUT, 0);
	
  #ifdef DEBUG_SEMAPHORES
  if( result != B_NO_ERROR ) 
    PError << "... failed, error: " << strerror(result);

  PError << endl;
  #endif 

  return result == B_WOULD_BLOCK;
}

///////////////////////////////////////////////////////////////////////////////
// PMutex  

PMutex::PMutex() 
  : PSemaphore(1, 1)
{
  PAssertOS(semId >= B_NO_ERROR);
  #ifdef DEBUG_SEMAPHORES
  sem_info info;
  get_sem_info(semId, &info);
  PError << "::create_sem (PMutex()), id: " << semId << " " << ", this: " << this << ", " << info.name << ", count:" << info.count << endl;
  #endif  
}

PMutex::PMutex(const PMutex& m) 
  : PSemaphore(1, 1)
{
  PAssertOS(semId >= B_NO_ERROR);
  #ifdef DEBUG_SEMAPHORES
  sem_info info;
  get_sem_info(semId, &info);
  PError << "::create_sem (PMutex(PMutex)), id: " << semId << " " << ", this: " << this << ", " << info.name << ", count:" << info.count << endl;
  #endif 
} 

void PMutex::Wait()
{
  PSemaphore::Wait();
}

BOOL PMutex::Wait(const PTimeInterval & timeout)
{
  return PSemaphore::Wait(timeout);
}

void PMutex::Signal()
{
  PSemaphore::Signal();
}

BOOL PMutex::WillBlock() const 
{
  return PSemaphore::WillBlock();
}

///////////////////////////////////////////////////////////////////////////////
// PSyncPoint

PSyncPoint::PSyncPoint()
  : PSemaphore(0, 1)
{
  PAssertOS(semId >= B_NO_ERROR);
  #ifdef DEBUG_SEMAPHORES
  sem_info info;
  get_sem_info(semId, &info);
  PError << "::create_sem (PSyncPoint()), id: " << semId << " " << ", this: " << this << info.name << ", count:" << info.count << endl;
  #endif 
}

void PSyncPoint::Signal()
{
  PSemaphore::Signal();
}
                                                                                                      
void PSyncPoint::Wait()
{
  PSemaphore::Wait();
}
                                                                                                      
BOOL PSyncPoint::Wait(const PTimeInterval & timeout)
{
  return PSemaphore::Wait(timeout);
}
                                                                                                      
BOOL PSyncPoint::WillBlock() const
{
  return PSemaphore::WillBlock();
}

//////////////////////////////////////////////////////////////////////////////
// Extra functionality not found in BeOS

int seteuid(uid_t uid) { return 0; }
int setegid(gid_t gid) { return 0; }

// End Of File ///////////////////////////////////////////////////////////////
