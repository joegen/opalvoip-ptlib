/*
 * TLIB.CXX
 *
 * Portable Windows Library
 *
 * Operating System Classes Implementation
 *
 * Copyright 1993 by Robert Jongbloed and Craig Southeren
 */

#define _OSUTIL_CXX

#pragma implementation "args.h"
#pragma implementation "process.h"
#pragma implementation "thread.h"
#pragma implementation "dynalink.h"
#pragma implementation "semaphor.h"

#include "ptlib.h"

#include <sys/time.h>
#include <pwd.h>

#if defined(P_HPUX9)
#define	SELECT(p1,p2,p3,p4,p5)		select(p1,(int *)(p2),(int *)(p3),(int *)(p4),p5)
#else
#define	SELECT(p1,p2,p3,p4,p5)		select(p1,p2,p3,p4,p5)
#endif

#if defined(P_SUN4)
extern "C" void bzero(void *, int);
extern "C" int select(int width,
			fd_set *readfds,
			fd_set *writefds,
			fd_set *exceptfds,
			struct timeval *timeout);
#endif


///////////////////////////////////////////////////////////////////////////////
//
// PProcess

PProcess::PProcess()
{
  PProcessInstance = this;
}


///////////////////////////////////////////////////////////////////////////////
//
// PProcess::GetHomeDir
//

PString PProcess::GetHomeDir ()

{
  PString dest;
  char *ptr;
  struct passwd *pw = NULL;

  if ((ptr = getenv ("HOME")) != NULL) {
    dest = ptr;
  } else {
    pw = getpwuid (getuid ());
    if (pw != NULL) {
      dest = pw->pw_dir;
    } else {
      dest = ".";
    }
  }

  if (dest.GetLength() > 0 && dest[dest.GetLength()-1] != '/')
    dest += "/";

  return dest;
}

///////////////////////////////////////////////////////////////////////////////
//
// PProcess
//
// Return the effective user name of the process, eg "root" etc.

PString PProcess::GetUserName() const

{
  struct passwd * pw = getpwuid (getuid());
  return PString(pw->pw_name);
}



///////////////////////////////////////////////////////////////////////////////
//
// PThread

PThread::PThread()
{
  blockHandle = -1;
  hasIOTimer  = FALSE;
}

PThread::~PThread()
{
  Terminate();
  free(stackBase);
}

void PThread::SwitchContext(PThread * from)
{
  if (setjmp(from->context) != 0) // Are being reactivated from previous yield
    return;

  if (status == Starting) {
    if (setjmp(context) != 0) {
      status = Running;
      Main();
      Terminate(); // Never returns from here
    }
#if defined(P_LINUX)
    context[0].__sp = (__ptr_t)stackTop-16;  // Change the stack pointer in jmp_buf
#else
#warning No lightweight thread context switch mechanism defined
#endif
  }
  longjmp(context, TRUE);
  PAssertAlways("longjmp failed"); // Should never get here
}


BOOL PThread::IsNoLongerBlocked()
{
  // if we aren't blocked on a channel, return TRUE
  if (blockHandle < 0)
    return TRUE;

  // setup file descriptor tables for select call
  fd_set readfds, writefds;
  FD_ZERO (&readfds);
  FD_ZERO (&writefds);
  if (blockRead)
    FD_SET (blockHandle, &readfds);
  else
    FD_SET (blockHandle, &writefds);

  // do a zero time select call to see if we would block if we did the read/write
  struct timeval timeout = {0, 0};
  if (SELECT (blockHandle + 1, &readfds, &writefds, NULL, &timeout) == 1) {
    dataAvailable = TRUE;
    return TRUE;
  }

  // well, we definitly didn't get any data.
  dataAvailable = FALSE;

  // if the channel has timed out, return TRUE
  return hasIOTimer && (ioTimer == 0);
}


void PProcess::OperatingSystemYield()

{
  PProcess * process = PProcess::Current();
  PThread *  current = process->currentThread;
  PThread *  thread = current;

  // setup file descriptor tables for select call
  fd_set readfds, writefds;
  FD_ZERO (&readfds);
  FD_ZERO (&writefds);
  int width = 0;

  // collect the handles and timeouts across all threads
  do {
    if (thread->blockHandle >= 0) {
      if (thread->blockRead)
        FD_SET (thread->blockHandle, &readfds);
      else
        FD_SET (thread->blockHandle, &writefds);
      width = PMAX(width, thread->blockHandle+1);
    }
    thread = thread->link;
  } while (thread != current);

  //
  // find timeout until next timer event
  //
  struct timeval * timeout = NULL;
  struct timeval   timeout_val;
  PTimeInterval delay = GetTimerList()->Process();
  if (delay != PMaxTimeInterval) {
    timeout_val.tv_usec = (delay.GetMilliseconds() % 1000) * 1000;
    timeout_val.tv_sec  = delay.GetSeconds();
    timeout             = &timeout_val;
  }

  // wait for something to happen on an I/O channel, or for a timeout
  int selectCount;
  if ((width > 0) || (timeout != NULL)) {
    selectCount = SELECT (width, &readfds, &writefds, NULL, timeout);
  } else {
    PError << "OperatingSystemYield with no blocks! Sleeping....\n";
    ::sleep(1);
    selectCount = 0;
  }
 
  // process the timer list
  GetTimerList()->Process();

  // go through the list of threads again and update the status of threads
  // that are blocked on I/O
  thread = current;
  do {
    if (thread->blockHandle >= 0) {
      if (thread->blockRead)
        thread->dataAvailable = FD_ISSET (thread->blockHandle, &readfds);
      else
        thread->dataAvailable = FD_ISSET (thread->blockHandle, &writefds);
    }
    thread = thread->link;
  } while (thread != current);
}


BOOL PThread::PXBlockOnIO(int handle, BOOL isRead)

{
  blockHandle   = handle;
  blockRead     = isRead;
  hasIOTimer    = FALSE;
  dataAvailable = FALSE;

  status      = BlockedIO;
  Yield();

  return dataAvailable;
}

BOOL PThread::PXBlockOnIO(int handle, BOOL isRead, const PTimeInterval & timeout)

{
  blockHandle   = handle;
  blockRead     = isRead;
  hasIOTimer    = TRUE;
  ioTimer       = timeout;
  dataAvailable = FALSE;

  status        = BlockedIO;
  Yield();

  ioTimer.Stop();

  return dataAvailable;
}


///////////////////////////////////////////////////////////////////////////////
// PDynaLink

PDynaLink::PDynaLink()
{
  PAssertAlways(PUnimplementedFunction);
}


PDynaLink::PDynaLink(const PString &)
{
  PAssertAlways(PUnimplementedFunction);
}


PDynaLink::~PDynaLink()
{
}


BOOL PDynaLink::Open(const PString & name)
{
  PAssertAlways(PUnimplementedFunction);
  return FALSE;
}


void PDynaLink::Close()
{
}


BOOL PDynaLink::IsLoaded() const
{
  return FALSE;
}


BOOL PDynaLink::GetFunction(PINDEX index, Function & func)
{
  return FALSE;
}


BOOL PDynaLink::GetFunction(const PString & name, Function & func)
{
  return FALSE;
}



// End Of File ///////////////////////////////////////////////////////////////
