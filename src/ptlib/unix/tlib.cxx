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
 * Revision 1.7  1996/04/15 10:49:11  craigs
 * Last build prior to release of MibMaster v1.0
 *
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
#include <sys/wait.h>

#ifdef P_LINUX
#include <sys/cdefs.h>
#include <sys/utsname.h>
#include <sys/types.h>
#include <sys/mman.h>
#define VIRTUAL_STACK_SIZE (stackSize*5)
#endif

#include "uerror.h"

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

void PProcess::Construct()
{
  ioBlocks[0].AllowDeleteObjects(FALSE);
  ioBlocks[1].AllowDeleteObjects(FALSE);
  ioBlocks[2].AllowDeleteObjects(FALSE);
}

void PProcess::PXAbortIOBlock(int fd) 
{
  for (PINDEX i = 0; i < 3 ; i++) {
    POrdinalKey key(fd);
    PThread * thread = ioBlocks[i].GetAt(key);
    if (thread != NULL) {
      // ensure the thread is in the current list of active threads
      for (PThread * start = this; start != thread; start = start->link) 
        PAssert (start->link != this, "I/O abort for inactive thread");

      // check for terminated thread
      PAssert (thread->status != Terminated, "I/O abort for terminated thread");

      // zero out the thread entry
      ioBlocks[i].SetAt(key, NULL);
      thread->selectReturnVal = -1;
      thread->selectErrno     = EINTR;
      FD_ZERO(thread->read_fds);
      FD_ZERO(thread->write_fds);
      FD_ZERO(thread->exception_fds);
    }
  }
}

PString PProcess::GetOSClass()
{
  return PString("Unix");
}

PString PProcess::GetOSName()
{
#if defined(P_LINUX)
  return PString("Linux");
#elif defined(P_HPUX9)
  return PString("HP/UX");
#elif defined(P_SUN4)
  return PString("SUNOS");
#else
  return PString("Unknown");
#endif
}

PString PProcess::GetOSVersion()
{
#if defined(P_LINUX)
  struct utsname info;
  uname(&info);
  return PString(info.release);
#elif
  return PString("unknown");
#endif
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

void PProcess::PXShowSystemWarning(PINDEX num)
{
  PXShowSystemWarning(num, "");
}

void PProcess::PXShowSystemWarning(PINDEX num, const PString & str)
{
  PProcess::Current()->_PXShowSystemWarning(num, str);
}

void PProcess::_PXShowSystemWarning(PINDEX code, const PString & str)
{
  PError << "PWLib/Unix error #"
         << code
         << "-"
         << str
         << endl;
}

///////////////////////////////////////////////////////////////////////////////
//
// PThread

void PThread::AllocateStack(PINDEX stackSize)
{
#if defined(P_LINUX)
  PAssert((stackBase = mmap(0, VIRTUAL_STACK_SIZE, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0)) != (char *)-1,
          "Thread stack allocation failed");
  stackTop  = stackBase + VIRTUAL_STACK_SIZE-1;
#else
  stackBase = (char *)malloc(5*stackSize);
  stackTop  = stackBase + 5*stackSize;
#endif
}

PThread::PThread()
{
  hasIOTimer  = FALSE;
  handleWidth = 0;
  waitPid     = 0;
}

PThread::~PThread()
{
  // can never destruct ourselves!!!!
  PAssert(this == PProcess::Current() || this != PThread::Current(), "Thread attempted suicide!");

  // call the terminate function so overloads work properly
  Terminate();

  // now we can terminate
#if defined(P_LINUX)
  munmap(stackBase, stackTop-stackBase+1);
#else
  free(stackBase);
#endif
}

#if 0

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

#endif

void PThread::ClearBlock()
{
  if (waitPid > 0)
    waitPid = 0;
  else
    handleWidth = 0;
}

BOOL PThread::IsNoLongerBlocked()
{
  // if are blocked on a child process, see if that child is still going
  if (waitPid > 0) 
    return kill(waitPid, 0) != 0;

  // if the I/O block was terminated previously, perhaps by a close from
  // another thread, then return TRUE
  if (selectErrno != 0 || selectReturnVal != 0)
    return TRUE;

  // if we aren't blocked on a channel, assert
  PAssert (handleWidth > 0, "IsNoLongerBlocked called for thread not I/O blocked");

  // do a zero time select call to see if we would block if we did the read/write
  struct timeval timeout = {0, 0};

  fd_set rfds = *read_fds;
  fd_set wfds = *write_fds;
  fd_set efds = *exception_fds;

  if ((selectReturnVal = SELECT (handleWidth, read_fds, write_fds, exception_fds, &timeout)) != 0) {
    selectErrno  = errno;
    return TRUE;
  }

  *read_fds      = rfds;
  *write_fds     = wfds;
  *exception_fds = efds;

  // if the channel has timed out, return TRUE
  return hasIOTimer && (ioTimer == 0);
}


void PProcess::OperatingSystemYield()

{
  PThread * current = PProcess::Current();

  // setup file descriptor tables for select call
  fd_set rfds, wfds, efds;
  FD_ZERO (&rfds); FD_ZERO (&wfds); FD_ZERO (&efds);
  int width     = 0;
  int waitCount = 0;

  // collect the handles across all threads
  PThread * thread = current;
  do {
    if (thread->status == BlockedIO) {
      if (thread->waitPid > 0)
        waitCount++;
      else if (thread->handleWidth > 0) {
        unsigned long *srp = (unsigned long *)thread->read_fds;
        unsigned long *swp = (unsigned long *)thread->write_fds;
        unsigned long *sep = (unsigned long *)thread->exception_fds;

        unsigned long *drp = (unsigned long *)&rfds;
        unsigned long *dwp = (unsigned long *)&wfds;
        unsigned long *dep = (unsigned long *)&efds;

        int orSize = (thread->handleWidth + (8*sizeof (unsigned long)) - 1) / (8*sizeof (unsigned long));

        for (PINDEX i = 0; i < orSize; i++) {
          *drp++ |= *srp++;
          *dwp++ |= *swp++;
          *dep++ |= *sep++;
        }

        width  =  PMAX(width, thread->handleWidth);
      }
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

  // wait for something to happen on an I/O channel, or for a timeout, or a SIGCLD
  int childPid;
  if ((width > 0) || (timeout != NULL)) {
    SELECT (width, &rfds, &wfds, &efds, timeout);
    childPid = wait3(NULL, WNOHANG|WUNTRACED, NULL);
  } else if (waitCount > 0)
    childPid = wait3(NULL, WUNTRACED, NULL);
  else {
    PError << "OperatingSystemYield with no blocks! Sleeping....\n";
    ::sleep(1);
    childPid = 0;
  }

  // finish off any child processes that exited
  if (childPid >= 0) {
    int retVal;
    waitpid(childPid, &retVal, WNOHANG);
  }
 
  // process the timer list
  GetTimerList()->Process();
}

void PThread::PXSetOSHandleBlock(int fd, int type)
{
  POrdinalKey key(fd);
  PProcess * proc = PProcess::Current();
  if (type & 1) {
    PAssert(!proc->ioBlocks[0].Contains(key),
           "Attempt to read block on handle which already has a pending read operation");
    proc->ioBlocks[0].SetAt(key, this);
  }
  if (type & 2) {
    PAssert(!proc->ioBlocks[1].Contains(key),
           "Attempt to write block on handle which already has a pending write operation");
    proc->ioBlocks[1].SetAt(key, this);
  }
  if (type & 4) {
    PAssert(!proc->ioBlocks[2].Contains(key),
           "Attempt to exception block on handle which already has a pending except operation");
    proc->ioBlocks[2].SetAt(key, this);
  }
}

void PThread::PXClearOSHandleBlock(int fd, int type)
{
  POrdinalKey key(fd);
  PProcess * proc = PProcess::Current();
  if ((type & 1) && (proc->ioBlocks[0].GetAt(key) == this)) 
    proc->ioBlocks[0].SetAt(key, NULL);
  if ((type & 2) && (proc->ioBlocks[1].GetAt(key) == this)) 
    proc->ioBlocks[1].SetAt(key, NULL);
  if ((type & 4) && (proc->ioBlocks[2].GetAt(key) == this)) 
    proc->ioBlocks[2].SetAt(key, NULL);
}

////////////////////////////////////////////////////////////////////
//
// PThread::PXBlockOnIO
//   This function performs the specified IO block
//   The return value is the value that the select call returned
//   which caused the block to return. errno is also set to the 
//   value of errno at the time the select returned.
//

int PThread::PXBlockOnIO(int handle, int type, const PTimeInterval & timeout)
{
  // make sure this thread is running
  PAssert(status == Running, "Attempt to I/O block a thread which is not running");

  // make sure we flush the buffer before doing a write
  fd_set tmp_rfd, tmp_wfd, tmp_efd;
  read_fds      = &tmp_rfd;
  write_fds     = &tmp_wfd;
  exception_fds = &tmp_efd;

  FD_ZERO (read_fds);
  FD_ZERO (write_fds);
  FD_ZERO (exception_fds);

  int blockType;
  switch (type) {
    case PChannel::PXReadBlock:
    case PChannel::PXAcceptBlock:
      FD_SET (handle, read_fds);
      blockType = 1;
      break;
    case PChannel::PXWriteBlock:
      FD_SET (handle, write_fds);
      blockType = 2;
      break;
    case PChannel::PXConnectBlock:
      FD_SET (handle, write_fds);
      FD_SET (handle, exception_fds);
      blockType = 2+4;
      break;
    default:
      PAssertAlways(PLogicError);
      return 0;
  }

  // make sure no other thread is blocked on this os_handle
  PXSetOSHandleBlock(handle, blockType);

  hasIOTimer  = timeout != PMaxTimeInterval;
  ioTimer     = timeout;
  selectErrno = selectReturnVal = 0;
  handleWidth = handle+1;
  waitPid     = 0;

  status      = BlockedIO;
  Yield();

  PXClearOSHandleBlock(handle, blockType);

  ioTimer.Stop();
  handleWidth = 0;

  errno = selectErrno;
  return selectReturnVal;
}

int PThread::PXBlockOnIO(int maxHandles,
                    fd_set & readBits,
                    fd_set & writeBits,
                    fd_set & exceptionBits,
       const PTimeInterval & timeout,
           const PIntArray & osHandles)
{
  PAssert(status == Running, "Attempt to I/O block a thread which is not running");

  for (PINDEX i = 0; i < osHandles.GetSize(); i+=2) 
    PXSetOSHandleBlock(osHandles[i], osHandles[i+1]);

  read_fds      = &readBits;
  write_fds     = &writeBits;
  exception_fds = &exceptionBits;

  handleWidth   = maxHandles;

  hasIOTimer    = timeout != PMaxTimeInterval;
  ioTimer       = timeout;
  selectErrno   = selectReturnVal = 0;
  waitPid     = 0;

  status        = BlockedIO;
  Yield();

  for (i = 0; i < osHandles.GetSize(); i+=2)
    PXClearOSHandleBlock(osHandles[i], osHandles[i+1]);

  ioTimer.Stop();
  handleWidth = 0;

  errno = selectErrno;
  return selectReturnVal;
}

int PThread::PXBlockOnChildTerminate(int pid)
{
  waitPid  = pid;
  status   = BlockedIO;
  Yield();
  return 0;
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
