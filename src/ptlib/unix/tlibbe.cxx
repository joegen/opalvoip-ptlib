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
 * Contributor(s): Yuri Kiryanov, BeVox Group, yk@altavista.net.
 */

class PProcess;
class PSemaphore;

#include <ptlib.h>

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
  process.activeThreads.SetAt(thread->threadId, thread);
  process.activeThreadMutex.Signal();

  process.SignalTimerChange();

  thread->Main();

  return 0;
}

PThread::PThread()
{
}

PThread::PThread(PINDEX stackSize,
                 AutoDeleteFlag deletion,
                 Priority priorityLevel)
{
  PAssert(stackSize > 0, PInvalidParameter);
  autoDelete = deletion == AutoDeleteThread;
  originalStackSize = stackSize;

  priority = priorities[priorityLevel];

  threadId =  ::spawn_thread(ThreadFunction, // Function 
         "PWLT", // Name
         priority, // Priority 
         (void *) this); // Pass this as cookie

  PAssertOS(threadId != 0);

  if (autoDelete) {
    PProcess & process = PProcess::Current();
    process.deleteThreadMutex.Wait();
    //process.autoDeleteThreads.Append(this);
    process.deleteThreadMutex.Signal();
  }
}


PThread::~PThread()
{
  if (originalStackSize <= 0)
    return;

  PProcess & process = PProcess::Current();
  process.activeThreadMutex.Wait();
  process.activeThreads.SetAt(threadId, NULL);
  process.activeThreadMutex.Signal();
  
  if (!IsTerminated())
    Terminate();
}


void PThread::Restart()
{
  PAssert(IsTerminated(), "Cannot restart running thread");

  threadId =  ::spawn_thread(ThreadFunction, // Function 
         "PWLT", // Name
           priority, 
         (void *) this); // Pass this as cookie
         
  PAssertOS(threadId != 0);
}


void PThread::Terminate()
{
  PAssert(!IsTerminated(), "Operation on terminated thread");
  PAssert(originalStackSize > 0, PLogicError);

  if (Current() == this)
    ::exit_thread(0);
  else
    ::kill_thread(threadId);
}


BOOL PThread::IsTerminated() const
{
  thread_info info;
  return ::get_thread_info( threadId, &info ) == B_BAD_VALUE;
}


void PThread::WaitForTermination() const
{
  WaitForTermination(PMaxTimeInterval);
}


BOOL PThread::WaitForTermination(const PTimeInterval & /*maxWait*/) const // Fix timeout
{
  if (threadId == 0)
    return TRUE;

  status_t result, exit_value;
  PINDEX retries = 10;
  
  while ((result = ::wait_for_thread(threadId, &exit_value) != B_NO_ERROR)) {

    if ( result == B_INTERRUPTED ) { // thread was killed.
      return TRUE;
    }

    if (retries > 0)
      return TRUE;

    retries--;
  }

  return FALSE;
}


void PThread::Suspend(BOOL susp)
{
  PAssert(!IsTerminated(), "Operation on terminated thread");
  if (susp)
  {
    status_t result = ::suspend_thread(threadId);

    PAssert(result == B_OK, "Thread don't want to be suspended");
  }
  else
    Resume();
}


void PThread::Resume()
{
  PAssert(!IsTerminated(), "Operation on terminated thread");
  status_t result = ::resume_thread(threadId);

  PAssert(result == B_NO_ERROR, "Thread doesn't want to resume");
}


BOOL PThread::IsSuspended() const
{
  PAssert(!IsTerminated(), "Operation on terminated thread");

  thread_info info;
  status_t result = ::get_thread_info(threadId, &info);

  PAssert(result == B_OK && threadId == info.thread, "Thread info inaccessible");
  return info.state == B_THREAD_SUSPENDED;
}

void PThread::SetPriority(Priority priorityLevel)
{
  PAssert(!IsTerminated(), "Operation on terminated thread");

  priority = priorities[priorityLevel];
  status_t result = ::set_thread_priority(threadId, priority );

  PAssert(result == B_OK, "Thread priority change error");
}


PThread::Priority PThread::GetPriority() const
{
  PAssert(!IsTerminated(), "Operation on terminated thread");

  switch (priority) {
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
  bigtime_t microseconds = B_INFINITE_TIMEOUT;
  if( delay != PMaxTimeInterval )
  	 microseconds = delay.GetMilliSeconds() / 1000;
 
  status_t result = ::snooze( microseconds ) ; // delay in ms, snooze in microsec
  PAssert(result == B_OK, "Thread has insomnia");
}

void PThread::InitialiseProcessThread()
{
  originalStackSize = 0;
  autoDelete = FALSE;
  
  ((PProcess *)this)->activeThreads.DisallowDeleteObjects();
  ((PProcess *)this)->activeThreads.SetAt(threadId, this);
}


PThread * PThread::Current()
{
  PProcess & process = PProcess::Current();
  process.activeThreadMutex.Wait();
  
  PThread * thread = process.activeThreads.GetAt( ::find_thread(NULL) );

  process.activeThreadMutex.Signal();
  return thread;
}

int PThread::PXBlockOnChildTerminate(int pid, const PTimeInterval & /*timeout*/) // Fix timeout
{
  status_t result, exit_value;

  while ((result = ::wait_for_thread(pid, &exit_value) != B_NO_ERROR));
  return exit_value;
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
  PProcess::Current().PXCheckSignals();
  return retval;
}

int PThread::PXBlockOnIO(int maxHandles,
           fd_set & readBits,
           fd_set & writeBits,
           fd_set & exceptionBits,
           const PTimeInterval & timeout,
           const PIntArray & /*osHandles*/)
{
  // make sure we flush the buffer before doing a write
  fd_set * read_fds      = &readBits;
  fd_set * write_fds     = &writeBits;
  fd_set * exception_fds = &exceptionBits;

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
  PProcess::Current().PXCheckSignals();
  return retval;
}

///////////////////////////////////////////////////////////////////////////////
// PProcess::TimerThread

void PProcess::Construct()
{
  CommonConstruct();
}

PProcess::HouseKeepingThread::HouseKeepingThread()
  : PThread(1000, NoAutoDeleteThread, LowPriority)
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
			PThread & thread = process.autoDeleteThreads[i];
			if( thread.IsTerminated() )
			{
				process.autoDeleteThreads.RemoveAt(i--);
			}
		}
		
		process.deleteThreadMutex.Signal();
		
		PTimeInterval nextTimer = process.timers.Process();
		
		breakBlock.Wait( nextTimer );
		//Sleep( nextTimer );
	}
}

void PProcess::SignalTimerChange()
{
  if (houseKeeper == NULL)
     houseKeeper = new HouseKeepingThread;
  else
    houseKeeper->breakBlock.Signal();
	//houseKeeper->Resume();
}


///////////////////////////////////////////////////////////////////////////////
// PProcess

PProcess::~PProcess()
{
  Sleep(100);  // Give threads time to die a natural death

  // Get rid of the house keeper (majordomocide)
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

  CommonDestruct();
}

///////////////////////////////////////////////////////////////////////////////
// PSemaphore
//#define DEBUG_SEMAPHORES

PSemaphore::PSemaphore(sem_id anId)
{
  semId = anId;

  PAssertOS(semId != 0);
}

PSemaphore::PSemaphore(unsigned initial, unsigned /*maxCount*/)
	: semId(0)
{
  semId = ::create_sem(1, // the semaphore's thread count
  				"PWLS" // Name
  				);

  #ifdef DEBUG_SEMAPHORES
  PError << "::create_sem " << semId << endl;
  #endif 

  PAssertOS( semId != 0 );
}

PSemaphore::~PSemaphore()
{
	#ifdef DEBUG_SEMAPHORES
	int32 semCnt = 0;
	get_sem_count(semId, &semCnt);
    PError << "::delete_sem " << semId << ", count:" << semCnt << endl;
	#endif 

	if ( semId != 0 )
	{
		status_t result = ::delete_sem(semId);
		
		PAssertOS( result == B_NO_ERROR );
	}
}

void PSemaphore::Wait()
{
  #ifdef DEBUG_SEMAPHORES
  int32 semCnt = 0;
  get_sem_count(semId, &semCnt);
  PError << "::acquire_sem " << semId << ", count:" << semCnt << endl;
  #endif 

  status_t result = ::acquire_sem(semId) ;

  PAssertOS( result == B_NO_ERROR );
}


BOOL PSemaphore::Wait(const PTimeInterval & timeout)
{
  bigtime_t microseconds = B_INFINITE_TIMEOUT;
  if( timeout != PMaxTimeInterval )
  	 microseconds = timeout.GetMilliSeconds() / 1000;
 
  #ifdef DEBUG_SEMAPHORES
  sem_info info;
  ::get_sem_info(semId, &info);
  PError << "::acquire_sem_etc " << info.sem << ", count:" << info.count << ", delay:" << microseconds << endl;
  #endif 

  status_t result = ::acquire_sem_etc(semId, 1, B_TIMEOUT, microseconds ); 

  return result != B_TIMED_OUT;
}


void PSemaphore::Signal()
{
  #ifdef DEBUG_SEMAPHORES
  int32 semCnt = 0;
  get_sem_count(semId, &semCnt);
  PError << "::release_sem " << semId << ", count:" << semCnt << endl;
  #endif 

  status_t result = ::release_sem(semId);

  PAssertOS(result == B_NO_ERROR);
}


BOOL PSemaphore::WillBlock() const
{
  #ifdef DEBUG_SEMAPHORES
  int32 semCnt = 0;
  get_sem_count(semId, &semCnt);
  PError << "::acquire_sem_etc (WillBlock) " << semId << ", count:" << semCnt << endl;
  #endif 

  status_t result = ::acquire_sem_etc(semId, 1, B_TIMEOUT, 0); // Timeout 0 is the flag

  PAssertOS(result == B_NO_ERROR);

  return result == B_TIMED_OUT;
}

///////////////////////////////////////////////////////////////////////////////
// PMutex  
// Using benaphores

PMutex::PMutex() 
  : PSemaphore( ::create_sem(1, "PWLM" ) ), benaphoreCount(0)
{
  #ifdef DEBUG_SEMAPHORES
  PError << "::create_sem(PMutex) " << semId << endl;
  #endif 
}

void PMutex::Wait()
{
    if( atomic_add( &benaphoreCount, 1 ) > 0 ) {

		PSemaphore::Wait();
	
		atomic_add( &benaphoreCount, -1 );
		return;
    }
}

BOOL PMutex::Wait(const PTimeInterval & timeout)
{
    BOOL fRet = TRUE;
    
    if( atomic_add( &benaphoreCount, 1 ) > 0 ) {

		fRet = PSemaphore::Wait(timeout);
		if( fRet ) {
	
		    atomic_add( &benaphoreCount, -1 );
		}
    }
	
	return fRet;
}

void PMutex::Signal()
{
    if( atomic_add( &benaphoreCount, -1 ) > 1 ) {

		PSemaphore::Signal();
    }
}

BOOL PMutex::WillBlock() const {
	return PSemaphore::WillBlock();
}

///////////////////////////////////////////////////////////////////////////////
// PSyncPoint

PSyncPoint::PSyncPoint()
  : PSemaphore( ::create_sem(0, "PWLP" ) )
{
  #ifdef DEBUG_SEMAPHORES
  PError << "::create_sem(PSyncPoint) " << semId << endl;
  #endif 
}

// End Of File ///////////////////////////////////////////////////////////////
