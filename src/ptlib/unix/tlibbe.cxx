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

///////////////////////////////////////////////////////////////////////////////
// Threads
//#define DEBUG_THREADS

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
 : threadId(B_BAD_THREAD_ID),
   priority(B_NORMAL_PRIORITY),
   originalStackSize(0)
{
}

PThread::PThread(PINDEX stackSize,
                 AutoDeleteFlag deletion,
                 Priority priorityLevel,
                 const PString & name)
  : threadName(name)
{
  PAssert(stackSize > 0, PInvalidParameter);
  autoDelete = deletion == AutoDeleteThread;
  originalStackSize = stackSize;

  priority = priorities[priorityLevel];

  PString str("PWLT ");
  str += threadName;
  #ifdef DEBUG_THREADS
  PError << "::spawn_thread(" << str << "), priority:" << priority << endl;
  #endif

  threadId =  ::spawn_thread(ThreadFunction, // Function 
         (const char*) str, // Name
         priority, // Priority 
         (void *) this); // Pass this as cookie

  PAssertOS(threadId >= B_NO_ERROR);

  #ifdef DEBUG_THREADS
  PError << ", id: " << threadId << endl;
  #endif

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

  PString str("PWLT");
  str += threadName;
  #ifdef DEBUG_THREADS
  PError << "::spawn_thread(" << str << "), priority:" << priority << endl;
  #endif

  threadId =  ::spawn_thread(ThreadFunction, // Function 
         (const char*) str, // Name
         priority, 
         (void *) this); // Pass this as cookie
         
  #ifdef DEBUG_THREADS
  PError << ", id: " << threadId << endl;
  #endif

  PAssertOS(threadId >= B_NO_ERROR);
}


void PThread::Terminate()
{
	PAssert(!IsTerminated(), "Operation on terminated thread");
	PAssert(originalStackSize > 0, PLogicError);
	
	if (Current() == this)
	{
		sem_id semId = ::create_sem( 1, "PWST" );
		if ( ::acquire_sem(semId) == B_NO_ERROR )
		{
			// Invalidate the thread
			threadId = B_BAD_THREAD_ID;
			::release_sem(semId);
			::delete_sem(semId);
			
	  	    #ifdef DEBUG_THREADS
	  	    PError << "::exit_thread(0), id:" << threadId << endl;
	  	    #endif
			::exit_thread(0);
		}
	}
	else 
	{
		sem_id semId = ::create_sem( 1, "PWTS" );
		if ( ::acquire_sem(semId) == B_NO_ERROR )
		{
			thread_id idToKill;
			idToKill = threadId;

			// Invalidate the thread
			threadId = B_BAD_THREAD_ID;
			
			// Kill it
			if (idToKill != B_BAD_THREAD_ID)
			{
				::release_sem(semId);
				::delete_sem(semId);

		  	    #ifdef DEBUG_THREADS
		  	    PError << "::kill_thread(" << idToKill << ")" << endl;
		  	    #endif
				::kill_thread(idToKill);
			}
			
		}
	}
	PAssert(threadId == B_BAD_THREAD_ID, "Can't acquire semaphore to terminate thread");
}


BOOL PThread::IsTerminated() const
{
  return threadId == B_BAD_THREAD_ID;
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
  PError << "::wait_for_thread(" << threadId << "), result:";
  #endif

  result = ::wait_for_thread(threadId, &exit_value);
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
  thread_info info;
  status_t result = ::get_thread_info(threadId, &info);

  PAssert(result == B_OK && threadId == info.thread, "Thread info inaccessible");
  return info.state == B_THREAD_SUSPENDED;
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
  bigtime_t microseconds = 
		delay == PMaxTimeInterval ? B_INFINITE_TIMEOUT : (delay.GetMilliSeconds() * 1000 );
 
  status_t result = ::snooze( microseconds ) ; // delay in ms, snooze in microsec
  PAssert(result == B_OK, "Thread has insomnia");
}

void PThread::InitialiseProcessThread()
{
  originalStackSize = 0;
  autoDelete = FALSE;
  
  threadId = ::find_thread(NULL);
  PAssertOS(threadId >= B_NO_ERROR);

  ((PProcess *)this)->activeThreads.DisallowDeleteObjects();
  ((PProcess *)this)->activeThreads.SetAt(threadId, this);
}


PThread * PThread::Current()
{
  PProcess & process = PProcess::Current();
  process.activeThreadMutex.Wait();
  
  thread_id tId = ::find_thread(NULL);
  PAssertOS(tId >= B_NO_ERROR);

  PThread * thread = process.activeThreads.GetAt( tId );

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
  if ( result == B_INTERRUPTED ) { // thread was killed.
	   #ifdef DEBUG_THREADS
	   PError << "B_INTERRUPTED" << endl;
	   #endif
     return 1;
  }

  if ( result == B_OK ) { // thread is dead
	   #ifdef DEBUG_THREADS
	   PError << "B_OK" << endl;
	   #endif
     return 1;
  }

  if ( result == B_BAD_THREAD_ID ) { // thread has invalid id
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
           fd_set * readBits,
           fd_set * writeBits,
           fd_set * exceptionBits,
           const PTimeInterval & timeout,
           const PIntArray & /*osHandles*/)
{
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

  int retval = ::select(maxHandles, readBits, writeBits, exceptionBits, tptr);
  PProcess::Current().PXCheckSignals();
  return retval;
}

void PThread::PXAbortBlock(void) const
{
}

///////////////////////////////////////////////////////////////////////////////
// PProcess

void PProcess::Construct()
{
  houseKeeper=NULL;

  CreateConfigFilesDictionary();
  
  CommonConstruct();
}

PProcess::HouseKeepingThread::HouseKeepingThread()
  : PThread(256*1024 , NoAutoDeleteThread, LowPriority, "HouseKeepingThread")
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
			PThread * pThread = (PThread *) 
				process.autoDeleteThreads.GetAt(i);
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
  if ( !houseKeeper )
     houseKeeper = new HouseKeepingThread;
  else
    houseKeeper->breakBlock.Signal();
}

BOOL PProcess::SetMaxHandles(int)
{
  return TRUE;
}

PProcess::~PProcess()
{
  Sleep(100);  // Give threads time to die a natural death

  if( houseKeeper )
  	delete houseKeeper;

  // OK, if there are any left we get really insistent...
  activeThreadMutex.Wait();
  for (PINDEX i = 0; i < activeThreads.GetSize(); i++) {
    PThread* pThread = activeThreads.GetAt(i);
    if (pThread && (this != pThread) && !pThread->IsTerminated())
      pThread->Terminate();  // With extreme prejudice
  }
  activeThreadMutex.Signal();

  deleteThreadMutex.Wait();
  autoDeleteThreads.RemoveAll();
  deleteThreadMutex.Signal();

  delete configFiles;
}

///////////////////////////////////////////////////////////////////////////////
// PSemaphore
//#define DEBUG_SEMAPHORES
#define USE_BENAPHORES // Comment this line if you don't want benaphores

PSemaphore::PSemaphore(sem_id anId, int32 initialBenaphore, int32 param)
  : semId( anId ), benaphoreCount(initialBenaphore)
{
  #ifdef DEBUG_SEMAPHORES
  PAssertOS(semId != 0);
  #endif 
}

PSemaphore::PSemaphore(unsigned initial, unsigned maxCount)
	: semId(0), benaphoreCount(0)
{
   PAssertOS(FALSE); // This constructor is never called
}

PSemaphore::~PSemaphore()
{
	status_t result = B_NO_ERROR;

	#ifdef DEBUG_SEMAPHORES
    PAssertOS( semId >= B_NO_ERROR );
	#endif 

	#ifdef DEBUG_SEMAPHORES
	int32 semCnt = 0;
	get_sem_count(semId, &semCnt);
    PError << "::delete_sem " << semId << ", count:" << semCnt << endl;
	#endif 

	if ( semId != 0 )
	{
		result = ::delete_sem(semId);
	}

	#ifdef DEBUG_SEMAPHORES
	if( result != B_NO_ERROR )
		PError << "::Error: " << strerror(result) << endl;
	#endif 
}

void PSemaphore::Wait()
{
	status_t result = B_NO_ERROR;

	#ifdef DEBUG_SEMAPHORES
    PAssertOS( semId >= B_NO_ERROR );
	#endif 

	#ifdef DEBUG_SEMAPHORES
	PError << "PSemaphore::Wait, benaphore: " << benaphoreCount << endl;
	#endif 

    #ifdef USE_BENAPHORES
    if( atomic_add( &benaphoreCount, 1 ) > 0 ) {
    #endif // USE_BENAPHORES

	#ifdef DEBUG_SEMAPHORES
	sem_info info;
	get_sem_info(semId, &info);
	PError << "::acquire_sem_etc, id: " << semId << " (" << info.name << "), count:" << info.count << endl;
	#endif 

	while ((result = ::acquire_sem(semId)) == B_INTERRUPTED)
	{
		#ifdef DEBUG_SEMAPHORES
		PError << "::acquire_sem_etc " << semId << ", interrupted!" << endl;
		#endif 
	}

	#ifdef DEBUG_SEMAPHORES
	if( result != B_NO_ERROR )
		PError << "::Error: " << strerror(result) << endl;
	#endif 

    #ifdef USE_BENAPHORES
  		atomic_add(&benaphoreCount, -1);
    }
    #endif // USE_BENAPHORES
}


BOOL PSemaphore::Wait(const PTimeInterval & timeout)
{
	status_t result = B_NO_ERROR;

	#ifdef DEBUG_SEMAPHORES
    PAssertOS( semId >= B_NO_ERROR );
	#endif 

	#ifdef DEBUG_SEMAPHORES
	PError << "PSemaphore::Wait(timeout), benaphore: " << benaphoreCount << endl;
	#endif 

    #ifdef USE_BENAPHORES
    if( atomic_add( &benaphoreCount, 1 ) > 0 ) {
    #endif // USE_BENAPHORES

	PInt64 ms = timeout.GetMilliSeconds();
	bigtime_t microseconds = 
		ms? timeout == PMaxTimeInterval ? B_INFINITE_TIMEOUT : ( ms * 1000 ) : 0;
 
	#ifdef DEBUG_SEMAPHORES
	int32 semCnt = 0;
	get_sem_count(semId, &semCnt);
	PError << "::acquire_sem_etc " << semId << ", count:" << semCnt << ", timeout:";
	
	if( microseconds == B_INFINITE_TIMEOUT ) 
		PError << "infinite" << endl;
	else
		PError << microseconds << endl;
	#endif 
	
	result = ::acquire_sem_etc(semId, 1, 
							B_RELATIVE_TIMEOUT, microseconds);

	#ifdef DEBUG_SEMAPHORES
	if( result != B_NO_ERROR ) 
	{
		PError << "::acquire_sem_etc " << semId << " with ";
		if( microseconds == B_INFINITE_TIMEOUT ) 
			PError << "infinite";
		else
			PError << microseconds;
		PError << " timeout failed, Error: " << strerror(result) << endl; 
	}
	#endif
	
    #ifdef USE_BENAPHORES
  		atomic_add(&benaphoreCount, -1);
    }
    #endif // USE_BENAPHORES

  return result == B_TIMED_OUT;
}


void PSemaphore::Signal()
{
    #ifdef DEBUG_SEMAPHORES
    PAssertOS( semId >= B_NO_ERROR );
    #endif 

	#ifdef DEBUG_SEMAPHORES
	PError << "PSemaphore::Wait(timeout), benaphore: " << benaphoreCount << endl;
	#endif 

    #ifdef USE_BENAPHORES
    if( atomic_add( &benaphoreCount, -1 ) > 1 ) 
    {
    #endif // USE_BENAPHORES

		#ifdef DEBUG_SEMAPHORES
  		status_t result = 
		#endif 
		::release_sem_etc(semId, 1, 0);
		
		#ifdef DEBUG_SEMAPHORES
		if( result != B_NO_ERROR ) 
			PError << "::Error:" << strerror(result) << endl;
		#endif 

    #ifdef USE_BENAPHORES
    }
    #endif // USE_BENAPHORES
}


BOOL PSemaphore::WillBlock() const
{
	status_t result = B_NO_ERROR;

	#ifdef DEBUG_SEMAPHORES
	PAssertOS( semId >= B_NO_ERROR );
	#endif 
	
    #ifdef USE_BENAPHORES
    if( atomic_add( (volatile int32*) &benaphoreCount, -1 ) > 1 ) 
    {
    #endif // USE_BENAPHORES

	#ifdef DEBUG_SEMAPHORES
	int32 semCnt = 0;
	get_sem_count(semId, &semCnt);
	PError << "::acquire_sem_etc (WillBlock) " << semId << ", count:" << semCnt << endl;
	#endif 
	
	result = ::acquire_sem_etc(semId, 0, B_RELATIVE_TIMEOUT, 0);
	
	#ifdef DEBUG_SEMAPHORES
	if( result != B_NO_ERROR ) 
		PError << "::Error:" << strerror(result) << endl;
	#endif 

    #ifdef USE_BENAPHORES
    }
    #endif // USE_BENAPHORES

	return result == B_WOULD_BLOCK;
}

///////////////////////////////////////////////////////////////////////////////
// PMutex  

PMutex::PMutex() 
  : PSemaphore( ::create_sem(1, "PWLM" ) , 0 )
{
  #ifdef DEBUG_SEMAPHORES
  PError << "::create_sem(PMutex) " << semId << endl;
  PAssertOS( semId >= B_NO_ERROR );
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
  : PSemaphore( ::create_sem(0, "PWLP" ), 1 )
{
  #ifdef DEBUG_SEMAPHORES
  PError << "::create_sem(PSyncPoint) " << semId << endl;
  #endif 
  
  PAssertOS( semId >= B_NO_ERROR );
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
