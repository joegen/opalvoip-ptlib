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

#include "ptlib.h"

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
  priority = B_NORMAL_PRIORITY;

  threadId =  ::spawn_thread(ThreadFunction, // Function 
         "PWLibThread", // Name
         priority, // Normal priority 
         (void *) this); // Pass this as cookie
  //PError << "::spawn_thread(PThread) " << threadId << endl;
  PAssertOS(threadId != 0);
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
         "PWLibThread", // Name
         priority, // Priority 
         (void *) this); // Pass this as cookie

  //PError << "::spawn_thread(PThread(...)) " << threadId << endl;
         
  PAssertOS(threadId != 0);

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
  process.activeThreads.SetAt(threadId, NULL);
  process.activeThreadMutex.Signal();
  
  if (!IsTerminated())
    Terminate();
}


void PThread::Restart()
{
  PAssert(IsTerminated(), "Cannot restart running thread");

  threadId =  ::spawn_thread(ThreadFunction, // Function 
         "PWLibThread", // Name
           priority, 
         (void *) this); // Pass this as cookie
         
  //PError << "::spawn_thread(Restart) " << threadId << endl;
  PAssertOS(threadId != 0);
}


void PThread::Terminate()
{
  PAssert(!IsTerminated(), "Operation on terminated thread");

  if (Current() == this)
  {
    ::exit_thread(0);
  	//PError << "::exit_thread(0) " << threadId << endl;
  }
  else
  {
    ::kill_thread(threadId);
    //PError << "::kill_thread " << threadId << endl;
   }
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
  //PError << "::wait_for_thread " << threadId << endl;
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
    //PError << "::suspend_thread " << threadId << endl;
    PAssert(result == B_OK, "Thread don't want to be suspended");
  }
  else
    Resume();
}


void PThread::Resume()
{
  PAssert(!IsTerminated(), "Operation on terminated thread");
  status_t result = ::resume_thread(threadId);
  //PError << "::resume_thread " << threadId << endl;
  PAssert(result == B_NO_ERROR, "Thread doesn't want to resume");
}


BOOL PThread::IsSuspended() const
{
  PAssert(!IsTerminated(), "Operation on terminated thread");

  thread_info info;
  status_t result = ::get_thread_info(threadId, &info);
  //PError << "::get_thread_info " << threadId << endl;

  PAssert(result == B_OK && threadId == info.thread, "Thread info inaccessible");
  return info.state == B_THREAD_SUSPENDED;
}

void PThread::SetPriority(Priority priorityLevel)
{
  PAssert(!IsTerminated(), "Operation on terminated thread");

  priority = priorities[priorityLevel];
  status_t result = ::set_thread_priority(threadId, priority );
  //PError << "::set_thread_priority " << threadId << endl;
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
  //PError << "::snooze " << endl;
  ::snooze(100);
}

void PThread::Sleep( const PTimeInterval & delay ) // Time interval to sleep for.
{
  status_t result = ::snooze( delay.GetMilliSeconds() / 1000 ) ; // delay in ms, snooze in microsec
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
  //PError << "::find_thread(NULL) " << endl;
  process.activeThreadMutex.Signal();
  return thread;
}

int PThread::PXBlockOnChildTerminate(int pid, const PTimeInterval & /*timeout*/) // Fix timeout
{
  status_t result, exit_value;
  //PError << "::wait_for_thread " << threadId << endl;
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
		//Sleep(nextTimer);
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
}

///////////////////////////////////////////////////////////////////////////////
// PSemaphore

PSemaphore::PSemaphore(sem_id anId)
{
  semId = anId;
  PAssertOS(semId != 0);
}

PSemaphore::PSemaphore(unsigned initial, unsigned /*maxCount*/)
{
  semId = ::create_sem(initial, // the semaphore's thread count
  				"PWLibSemaphore" // Name
  				);
  //PError << "::create_sem " << semId << endl;
  PAssertOS( semId != 0 );
}

PSemaphore::~PSemaphore()
{
  if ( semId != 0 )
  {
	status_t result = ::delete_sem(semId);
    //PError << "::delete_sem " << semId << endl;
    PAssertOS( result == B_NO_ERROR );
  }
}

void PSemaphore::Wait()
{
  status_t result = ::acquire_sem(semId) ;
  //PError << "::acquire_sem " << semId << endl;
  PAssertOS( result == B_NO_ERROR );
}


BOOL PSemaphore::Wait(const PTimeInterval & timeout)
{
  status_t result = ::acquire_sem_etc(semId, 1, B_TIMEOUT, 
  	timeout == PMaxTimeInterval ? B_INFINITE_TIMEOUT : timeout.GetMilliSeconds() / 1000); 
  //PError << "::acquire_sem_etc " << semId << endl;
  PAssertOS( result == B_NO_ERROR );
  return result != B_TIMED_OUT;
}


void PSemaphore::Signal()
{
  status_t result = ::release_sem(semId);
  //PError << "::release_sem " << semId << endl;
  PAssertOS(result == B_NO_ERROR);
}


BOOL PSemaphore::WillBlock() const
{
  status_t result = ::acquire_sem_etc(semId, 1, B_TIMEOUT, 0); 
  //PError << "::acquire_sem_etc (WillBlock) " << semId << endl;
  PAssertOS(result == B_NO_ERROR);
  return result == B_TIMED_OUT;
}

///////////////////////////////////////////////////////////////////////////////
// PMutex - need benaphores

PMutex::PMutex() 
  : PSemaphore( ::create_sem(1, "PWLibMutex" ) )
{
  //PError << "::create_sem(PMutex) " << semId << endl;
}

///////////////////////////////////////////////////////////////////////////////
// PSyncPoint

PSyncPoint::PSyncPoint()
  : PSemaphore( ::create_sem(1, "PWLibSyncPoint" ) )
{
  //PError << "::create_sem(PSyncPoint) " << semId << endl;
}

// End Of File ///////////////////////////////////////////////////////////////
