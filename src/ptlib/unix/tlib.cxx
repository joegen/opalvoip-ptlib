/*
 * TLIB.CXX
 *
 * Portable Windows Library
 *
 * Operating System Classes Implementation
 *
 * Copyright 1993 by Robert Jongbloed and Craig Southeren
 *
 * $Log: tlib.cxx,v $
 * Revision 1.6  1996/01/26 11:09:42  craigs
 * Added signal handlers
 *
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
#include <signal.h>

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

PProcess * PProcessInstance;
ostream  * PErrorStream = &cerr;

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
  // if we aren't blocked on a channel, assert
  PAssert (blockHandle >= 0, "IsNoLongerBlocked called for thread not I/O blocked");

  // setup file descriptor tables for select call
  fd_set readfds, writefds, exceptionfds;
  FD_ZERO (&readfds);
  FD_ZERO (&writefds);
  FD_ZERO (&exceptionfds);

  switch (blockType) {
    case PChannel::PXReadBlock:
    case PChannel::PXAcceptBlock:
      FD_SET (blockHandle, &readfds);
      break;
    case PChannel::PXWriteBlock:
      FD_SET (blockHandle, &writefds);
      break;
    case PChannel::PXOtherBlock:
      FD_SET (blockHandle, &exceptionfds);
      break;
  }

  // do a zero time select call to see if we would block if we did the read/write
  struct timeval timeout = {0, 0};
  if (SELECT (blockHandle + 1, &readfds, &writefds, &exceptionfds, &timeout) == 1) {
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
  fd_set readfds, writefds, exceptionfds;
  FD_ZERO (&readfds);
  FD_ZERO (&writefds);
  FD_ZERO (&exceptionfds);
  int width = 0;

  // collect the handles and timeouts across all threads
  do {
    if (thread->blockHandle >= 0) {
      switch (thread->blockType) {
        case PChannel::PXReadBlock:
        case PChannel::PXAcceptBlock:
          FD_SET (thread->blockHandle, &readfds);
          break;
        case PChannel::PXWriteBlock:
          FD_SET (thread->blockHandle, &writefds);
          break;
        case PChannel::PXOtherBlock:
          FD_SET (thread->blockHandle, &exceptionfds);
          break;
      }
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
    selectCount = SELECT (width, &readfds, &writefds, &exceptionfds, timeout);
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
        thread->dataAvailable = FALSE;
//      if (thread->blockRead)
//        thread->dataAvailable = FD_ISSET (thread->blockHandle, &readfds);
//      else
//        thread->dataAvailable = FD_ISSET (thread->blockHandle, &writefds);
    }
    thread = thread->link;
  } while (thread != current);
}


BOOL PThread::PXBlockOnIO(int handle, int type)

{
  PAssert(status == Running, "Attempt to I/O block a thread which is not running");

  blockHandle   = handle;
  blockType     = type;
  hasIOTimer    = FALSE;
  dataAvailable = FALSE;

  status      = BlockedIO;
  Yield();

  return dataAvailable;
}

BOOL PThread::PXBlockOnIO(int handle, int type, const PTimeInterval & timeout)

{
  PAssert(status == Running, "Attempt to I/O block a thread which is not running");

  blockHandle   = handle;
  blockType     = type;
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

void PXSignalHandler(int sig)
{
  signal(sig, PXSignalHandler);

  PProcess * process = PProcess::Current();
  switch (sig) {
#ifdef SIGHUP
    case SIGHUP:
      process->PXOnSigHup();
      break;
#endif
#ifdef SIGINT
    case SIGINT:
      process->PXOnSigInt();
      break;
#endif
#ifdef SIGQUIT
    case SIGQUIT:
      process->PXOnSigQuit();
      break;
#endif
#ifdef SIGUSR1
    case SIGUSR1:
      process->PXOnSigUsr1();
      break;
#endif
#ifdef SIGUSR2
    case SIGUSR2:
      process->PXOnSigUsr1();
      break;
#endif
#ifdef SIGPIPE
    case SIGPIPE:
      process->PXOnSigPipe();
      break;
#endif
#ifdef SIGTERM
    case SIGTERM:
      process->PXOnSigTerm();
      break;
#endif
#ifdef SIGCHLD		
    case SIGCHLD:
      process->PXOnSigChld();
      break;
#endif
  }
}

void PProcess::PXSetupProcess()
{
  // Setup signal handlers
#ifdef SIGHUP
  signal(SIGHUP, &PXSignalHandler);
#endif
#ifdef SIGINT
  signal(SIGINT, &PXSignalHandler);
#endif
#ifdef SIGQUIT
  signal(SIGQUIT, &PXSignalHandler);
#endif
#ifdef SIGUSR1
  signal(SIGUSR1, &PXSignalHandler);
#endif
#ifdef SIGUSR2
  signal(SIGUSR2, &PXSignalHandler);
#endif
#ifdef SIGPIPE
  signal(SIGPIPE, &PXSignalHandler);
#endif
#ifdef SIGTERM
  signal(SIGTERM, &PXSignalHandler);
#endif
#ifdef SIGCHLD		
  signal(SIGCHLD, &PXSignalHandler);
#endif

  // initialise the timezone information
  tzset();
}

int PProcess::_main (int parmArgc, char *parmArgv[], char *parmEnvp[])
{
  // save the environment
  envp = parmEnvp;
  argc = parmArgc;
  argv = parmArgv;

  // perform process initialisation
  PXSetupProcess();

  // perform PWLib initialisation
  PreInitialise(argc, argv);

  // call the main program
  Main();

  return terminationValue;
}

void PProcess::PXOnSigHup()
{
  raise(SIGKILL);
}

void PProcess::PXOnSigInt()
{
  raise(SIGKILL);
}

void PProcess::PXOnSigQuit()
{
  raise(SIGKILL);
}

void PProcess::PXOnSigUsr1()
{
}

void PProcess::PXOnSigUsr2()
{
}

void PProcess::PXOnSigPipe()
{
}

void PProcess::PXOnSigTerm()
{
  raise(SIGKILL);
}

void PProcess::PXOnSigChld()
{
}
