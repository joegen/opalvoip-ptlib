/*
 * tlib.cxx
 *
 * Miscelaneous class implementation
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
 * $Revision$
 * $Author$
 * $Date$
 */

#define _OSUTIL_CXX

#define SIGNALS_DEBUG(fmt,...) //fprintf(stderr, fmt, ##__VA_ARGS__)


#pragma implementation "args.h"
#pragma implementation "pprocess.h"
#pragma implementation "thread.h"
#pragma implementation "semaphor.h"
#pragma implementation "mutex.h"
#pragma implementation "critsec.h"
#pragma implementation "psync.h"
#pragma implementation "syncpoint.h"
#pragma implementation "syncthrd.h"

#include "ptlib.h"
#include <ptlib/pprocess.h>

#ifdef P_VXWORKS
#include <sys/times.h>
#include <time.h>
#include <hostLib.h>
#include <remLib.h>
#include <taskLib.h>
#include <intLib.h>
#else
#include <sys/time.h>
#include <pwd.h>
#include <grp.h>
#endif // P_VXWORKS
#include <signal.h>
#include <sys/wait.h>
#include <errno.h>

#if defined(P_LINUX) || defined(P_GNU_HURD)
#include <sys/cdefs.h>
#include <sys/types.h>
#include <sys/mman.h>
#endif

#if defined(P_LINUX) || defined(P_SUN4) || defined(P_SOLARIS) || defined(P_FREEBSD) || defined(P_OPENBSD) || defined(P_NETBSD) || defined(P_MACOSX) || defined(P_IOS) || defined (P_AIX) || defined(P_BEOS) || defined(P_IRIX) || defined(P_QNX) || defined(P_GNU_HURD) || defined(P_ANDROID)
#include <sys/utsname.h>
#define  HAS_UNAME
#elif defined(P_RTEMS)
extern "C" {
#include <sys/utsname.h>
}
#define  HAS_UNAME
#endif

#include "uerror.h"

#if defined(P_HPUX9)
#define  SELECT(p1,p2,p3,p4,p5)    select(p1,(int *)(p2),(int *)(p3),(int *)(p4),p5)
#else
#define  SELECT(p1,p2,p3,p4,p5)    select(p1,p2,p3,p4,p5)
#endif

#if defined(P_SUN4)
extern "C" void bzero(void *, int);
extern "C" int select(int width,
      fd_set *readfds,
      fd_set *writefds,
      fd_set *exceptfds,
      struct timeval *timeout);
#endif

#ifdef P_BEOS
#include "OS.h"
#endif


PString PProcess::GetOSClass()
{
#ifndef P_BEOS
  return PString("Unix");
#elif defined P_VXWORKS
  return PString("VxWorks");
#else
  return PString("Be Inc.");
#endif
}

PString PProcess::GetOSName()
{
#if defined(HAS_UNAME)
  struct utsname info;
  uname(&info);
#ifdef P_SOLARIS
  return PString(info.sysname) & info.release;
#else
  return PString(info.sysname);
#endif
#elif defined(P_VXWORKS)
  return PString::Empty();
#else
#warning No GetOSName specified
  return PString("Unknown");
#endif
}

PString PProcess::GetOSHardware()
{
#if defined(HAS_UNAME)
  struct utsname info;
  uname(&info);
  return PString(info.machine);
#elif defined(P_VXWORKS)
  return PString(sysModel());
#else
#warning No GetOSHardware specified
  return PString("unknown");
#endif
}

PString PProcess::GetOSVersion()
{
#if defined(HAS_UNAME)
  struct utsname info;
  uname(&info);
#ifdef P_SOLARIS
  return PString(info.version);
#else
  return PString(info.release);
#endif
#elif defined(P_VXWORKS)
  return PString(sysBspRev());
#else
#warning No GetOSVersion specified
  return PString("?.?");
#endif
}

bool PProcess::IsOSVersion(unsigned major, unsigned minor, unsigned build)
{
#if defined(HAS_UNAME)
  struct utsname info;
  uname(&info);
  unsigned maj, min, bld;
  sscanf(
#ifdef P_SOLARIS
         info.version
#else
         info.release
#endif
         , "%u.%u.%u", &maj, &min, &bld);
  if (maj < major)
    return false;
  if (maj > major)
    return true;

  if (min < minor)
    return false;
  if (min > minor)
    return true;

  return bld >= build;
#elif defined(P_VXWORKS)
  return sysBspRev() >= major;
#else
  return true;
#endif
}


PDirectory PProcess::GetOSConfigDir()
{
#ifdef P_VXWORKS
  return "./";
#else
  return "/etc";
#endif // P_VXWORKS
}


#if defined(P_VXWORKS)
  #undef GETPWUID
#elif defined(P_GETPWUID_R5)
  #define GETPWUID(pw) \
    struct passwd pwd; \
    char buffer[1024]; \
    if (::getpwuid_r(geteuid(), &pwd, buffer, 1024, &pw) < 0) \
      pw = NULL
#elif defined(P_GETPWUID_R4)
  #define GETPWUID(pw) \
    struct passwd pwd; \
    char buffer[1024]; \
    pw = ::getpwuid_r(geteuid(), &pwd, buffer, 1024);
#else
  #define GETPWUID(pw) \
    pw = ::getpwuid(geteuid());
#endif


PString PProcess::GetUserName() const
{
#ifdef GETPWUID

  struct passwd * pw;
  GETPWUID(pw);

  if (pw != NULL && pw->pw_name != NULL)
    return pw->pw_name;

  const char * user;
  if ((user = getenv("USER")) != NULL)
    return user;

#endif // GETPWUID

  return GetName();
}


PBoolean PProcess::SetUserName(const PString & username, PBoolean permanent)
{
#ifdef P_VXWORKS
  PAssertAlways("PProcess::SetUserName - not implemented for VxWorks");
  return false;
#else
  if (username.IsEmpty())
    return seteuid(getuid()) != -1;

  int uid = -1;

  if (username[0] == '#') {
    PString s = username.Mid(1);
    if (s.FindSpan("1234567890") == P_MAX_INDEX)
      uid = s.AsInteger();
  }
  else {
    struct passwd * pw;
#if defined(P_GETPWNAM_R5)
    struct passwd pwd;
    char buffer[1024];
    if (::getpwnam_r(username, &pwd, buffer, 1024, &pw) < 0)
      pw = NULL;
#elif defined(P_GETPWNAM_R4)
    struct passwd pwd;
    char buffer[1024];
    pw = ::getpwnam_r(username, &pwd, buffer, 1024);
#else
    pw = ::getpwnam(username);
#endif

    if (pw != NULL)
      uid = pw->pw_uid;
    else {
      if (username.FindSpan("1234567890") == P_MAX_INDEX)
       uid = username.AsInteger();
    }
  }

  if (uid < 0)
    return false;

  if (permanent)
    return setuid(uid) != -1;
    
  return seteuid(uid) != -1;
#endif // P_VXWORKS
}


PDirectory PProcess::GetHomeDirectory() const
{
#ifdef GETPWUID

  const char * home = getenv("HOME");
  if (home != NULL)
    return home;

  struct passwd * pw;
  GETPWUID(pw);

  if (pw != NULL && pw->pw_dir != NULL)
    return pw->pw_dir;

#endif // GETPWUID

  return ".";
}


///////////////////////////////////////////////////////////////////////////////
//
// PProcess
//
// Return the effective group name of the process, eg "wheel" etc.

PString PProcess::GetGroupName() const

{
#ifdef P_VXWORKS

  return PString("VxWorks");

#else

#if defined(P_PTHREADS) && !defined(P_THREAD_SAFE_LIBC)
  struct group grp;
  char buffer[1024];
  struct group * gr = NULL;
#if defined(P_GETGRGID_R5)
  ::getgrgid_r(getegid(), &grp, buffer, 1024, &gr);
#elif defined(P_GETGRGID_R4)
  gr = ::getgrgid_r(getegid(), &grp, buffer, 1024);
#else
  #error "Cannot identify getgrgid_r"
#endif
#else
  struct group * gr = ::getgrgid(getegid());
#endif

  char * ptr;
  if (gr != NULL && gr->gr_name != NULL)
    return PString(gr->gr_name);
  else if ((ptr = getenv("GROUP")) != NULL)
    return PString(ptr);
  else
    return PString("group");
#endif // P_VXWORKS
}


PBoolean PProcess::SetGroupName(const PString & groupname, PBoolean permanent)
{
#ifdef P_VXWORKS
  PAssertAlways("PProcess::SetGroupName - not implemented for VxWorks");
  return false;
#else
  if (groupname.IsEmpty())
    return setegid(getgid()) != -1;

  int gid = -1;

  if (groupname[0] == '#') {
    PString s = groupname.Mid(1);
    if (s.FindSpan("1234567890") == P_MAX_INDEX)
      gid = s.AsInteger();
  }
  else {
#if defined(P_PTHREADS) && !defined(P_THREAD_SAFE_LIBC)
    struct group grp;
    char buffer[1024];
    struct group * gr = NULL;
#if defined (P_GETGRNAM_R5)
    ::getgrnam_r(groupname, &grp, buffer, 1024, &gr);
#elif defined(P_GETGRNAM_R4)
    gr = ::getgrnam_r(groupname, &grp, buffer, 1024);
#else
    #error "Cannot identify getgrnam_r"
#endif
#else
    struct group * gr = ::getgrnam(groupname);
#endif

    if (gr != NULL && gr->gr_name != NULL)
      gid = gr->gr_gid;
    else {
      if (groupname.FindSpan("1234567890") == P_MAX_INDEX)
       gid = groupname.AsInteger();
    }
  }

  if (gid < 0)
    return false;

  if (permanent)
    return setgid(gid) != -1;
    
  return setegid(gid) != -1;
#endif // P_VXWORKS
}


void PProcess::HostSystemURLHandlerInfo::SetIcon(const PString & _icon)
{
}

PString PProcess::HostSystemURLHandlerInfo::GetIcon() const 
{
  return PString();
}

void PProcess::HostSystemURLHandlerInfo::SetCommand(const PString & key, const PString & _cmd)
{
}

PString PProcess::HostSystemURLHandlerInfo::GetCommand(const PString & key) const
{
  return PString();
}

bool PProcess::HostSystemURLHandlerInfo::GetFromSystem()
{
  return false;
}

bool PProcess::HostSystemURLHandlerInfo::CheckIfRegistered()
{
  return false;
}

bool PProcess::HostSystemURLHandlerInfo::Register()
{
  return false;
}



void PProcess::PXShowSystemWarning(PINDEX num)
{
  PXShowSystemWarning(num, "");
}

void PProcess::PXShowSystemWarning(PINDEX num, const PString & str)
{
  PProcess::Current()._PXShowSystemWarning(num, str);
}

void PProcess::_PXShowSystemWarning(PINDEX code, const PString & str)
{
  PError << "PWLib " << GetOSClass() << " error #" << code << '-' << str << endl;
}

void PXSignalHandler(int sig)
{
  SIGNALS_DEBUG("\nSIGNAL<%u>\n",sig);
  PProcess::Current().PXOnAsyncSignal(sig);
  signal(sig, PXSignalHandler);
}

void PProcess::PXCheckSignals()
{
  if (m_pxSignals == 0)
    return;

  PTRACE(3, "PTLib", "Checking signals: 0x" << hex << m_pxSignals << dec);

  for (int sig = 0; sig < 32; ++sig) {
    int bit = 1 << sig;
    if (m_pxSignals&bit) {
      m_pxSignals &= ~bit;
      PXOnSignal(sig);
    }
  }
}


static void SetSignals(void (*handler)(int))
{
  SIGNALS_DEBUG("\nSETSIG<%p>\n",handler);

  if (handler == NULL)
    handler = SIG_DFL;

#ifdef SIGHUP
  signal(SIGHUP, handler);
#endif
#ifdef SIGINT
  signal(SIGINT, handler);
#endif
#ifdef SIGUSR1
  signal(SIGUSR1, handler);
#endif
#ifdef SIGUSR2
  signal(SIGUSR2, handler);
#endif
#ifdef SIGPIPE
  signal(SIGPIPE, handler);
#endif
#ifdef SIGTERM
  signal(SIGTERM, handler);
#endif
#ifdef SIGWINCH
  signal(SIGWINCH, handler);
#endif
#ifdef SIGPROF
  signal(SIGPROF, handler);
#endif
#ifdef SIGTRAP
  signal(SIGTRAP, handler);
#endif
}


void PProcess::PXOnAsyncSignal(int sig)
{
  SIGNALS_DEBUG("\nASYNCSIG<%u>\n",sig);

  switch (sig) {
    case SIGINT:
    case SIGHUP:
    case SIGTERM:
      if (OnInterrupt(sig == SIGTERM))
        return;

#if P_HAS_BACKTRACE && PTRACING
    case WalkStackSignal:
      InternalWalkStackSignaled();
      break;
#endif
  }

  m_pxSignals |= 1 << sig;
  SignalTimerChange(); // Inform house keeping thread we have a signal to be processed
}

void PProcess::PXOnSignal(int sig)
{
  SIGNALS_DEBUG("\nSYNCSIG<%u>\n",sig);
  PTRACE(3, "PTLib", "Handling signal " << sig);

  switch (sig) {
    case SIGINT:
    case SIGHUP:
    case SIGTERM:
      raise(SIGKILL);

#ifdef _DEBUG
    case 28 :
      #if PMEMORY_CHECK
        PMEMORY_IGNORE_ALLOCATIONS_FOR_SCOPE;
        static PMemoryHeap::State state;
        PMemoryHeap::GetState(state);
      #endif // PMEMORY_CHECK
      PStringStream strm;
      m_threadMutex.Wait();
      strm << "===============\n"
           << m_activeThreads.size() << " active threads\n";
      for (ThreadMap::iterator it = m_activeThreads.begin(); it != m_activeThreads.end(); ++it)
        strm << "  " << *it->second << "\n";
      #if PMEMORY_CHECK
        strm << "---------------\n";
        PMemoryHeap::DumpObjectsSince(state, strm);
        PMemoryHeap::GetState(state);
      #endif // PMEMORY_CHECK
      strm << "===============\n";
      m_threadMutex.Signal();
      fprintf(stderr, "%s", (const char *)strm);
#endif // _DEBUG
  }
}

void PProcess::CommonConstruct()
{
  // Setup signal handlers
  m_pxSignals = 0;

  if (!m_library)
    SetSignals(&PXSignalHandler);

#if !defined(P_VXWORKS) && !defined(P_RTEMS)
  // initialise the timezone information
  tzset();
#endif

#ifndef P_RTEMS
  // get the file descriptor limit
  struct rlimit rl;
  PAssertOS(getrlimit(RLIMIT_NOFILE, &rl) == 0);
  maxHandles = rl.rlim_cur;
  PTRACE(4, "PTLib\tMaximum per-process file handles is " << maxHandles);
#else
  maxHandles = 500; // arbitrary value
#endif
}

void PProcess::CommonDestruct()
{
  if (!m_library)
    SetSignals(NULL);

  m_keepingHouse = false;
}

//////////////////////////////////////////////////////////////////////////////
// P_fd_set

void P_fd_set::Construct()
{
  max_fd = PProcess::Current().GetMaxHandles();
  // Use an array for FORTIFY_SOURCE
  set = (fd_set *)malloc(((max_fd + FD_SETSIZE - 1) / FD_SETSIZE)*sizeof(fd_set));
}


void P_fd_set::Zero()
{
  if (PAssertNULL(set) != NULL)
    memset(set, 0, ((max_fd + FD_SETSIZE - 1) / FD_SETSIZE)*sizeof(fd_set));
}


P_fd_set::P_fd_set()
{
  Construct();
  Zero();
}

PBoolean P_fd_set::IsPresent(intptr_t fd) const
{
  const int fd_num = fd / FD_SETSIZE;
  const int fd_off = fd % FD_SETSIZE;
  return FD_ISSET(fd_off, set + fd_num);
}

P_fd_set::P_fd_set(intptr_t fd)
{
  Construct();
  Zero();
  const int fd_num = fd / FD_SETSIZE;
  const int fd_off = fd % FD_SETSIZE;
  FD_SET(fd_off, set + fd_num);
}


P_fd_set & P_fd_set::operator=(intptr_t fd)
{
  PAssert(fd < max_fd, PInvalidParameter);
  Zero();
  const int fd_num = fd / FD_SETSIZE;
  const int fd_off = fd % FD_SETSIZE;
  FD_SET(fd_off, set + fd_num);
  return *this;
}


P_fd_set & P_fd_set::operator+=(intptr_t fd)
{
  PAssert(fd < max_fd, PInvalidParameter);
  const int fd_num = fd / FD_SETSIZE;
  const int fd_off = fd % FD_SETSIZE;
  FD_SET(fd_off, set + fd_num);
  return *this;
}


P_fd_set & P_fd_set::operator-=(intptr_t fd)
{
  PAssert(fd < max_fd, PInvalidParameter);
  const int fd_num = fd / FD_SETSIZE;
  const int fd_off = fd % FD_SETSIZE;
  FD_CLR(fd_off, set + fd_num);
  return *this;
}


///////////////////////////////////////////////////////////////////////////////

#if defined(P_PTHREADS)
#include "tlibthrd.cxx"
#elif defined(BE_THREADS)
#include "tlibbe.cxx"
#elif defined(VX_TASKS)
#include "tlibvx.cxx"
#elif defined(P_RTEMS)
#include "tlibrtems.cxx"
#else

void PProcess::HouseKeeping()
{
  while (m_keepingHouse) {
    PTimeInterval delay = m_timerList->Process();
    if (delay > 10000)
      delay = 10000;

    m_signalHouseKeeper.Wait(delay);

    InternalCleanAutoDeleteThreads();

    PXCheckSignals();
  }
}


void PProcess::Construct()
{
  CommonConstruct();
}


PBoolean PProcess::SetMaxHandles(int newMax)
{
#ifndef P_RTEMS
  // get the current process limit
  struct rlimit rl;
  PAssertOS(getrlimit(RLIMIT_NOFILE, &rl) == 0);

  // set the new current limit
  rl.rlim_cur = newMax;
  if (setrlimit(RLIMIT_NOFILE, &rl) == 0) {
    PAssertOS(getrlimit(RLIMIT_NOFILE, &rl) == 0);
    maxHandles = rl.rlim_cur;
    if (maxHandles == newMax) {
      PTRACE(2, "PTLib\tNew maximum per-process file handles set to " << maxHandles);
      return true;
    }
  }
#endif // !P_RTEMS

  PTRACE(1, "PTLib\tCannot set per-process file handle limit to "
         << newMax << " (is " << maxHandles << ") - check permissions");
  return false;
}


PProcess::~PProcess()
{
  PreShutdown();

  CommonDestruct();

  PostShutdown();
}


PThread::PThread(bool isProcess)
  : m_type(isProcess ? e_IsProcess : e_IsExternal)
  , m_originalStackSize(0)
{
}


PThread::PThread(PINDEX stackSize,
                 AutoDeleteFlag deletion,
                 Priority priorityLevel,
                 const PString & name)
  : m_type(deletion == AutoDeleteThread ? e_IsAutoDelete : e_IsManualDelete)
  , m_threadName(name)
{
}


void PThread::InternalDestroy()
{
}


void PThread::PXAbortBlock() const
{
}


void PThread::Restart()
{
}


void PThread::Suspend(PBoolean susp)
{
}


void PThread::Resume()
{
}


PBoolean PThread::IsSuspended() const
{
  return true;
}


void PThread::SetPriority(Priority priorityLevel)
{
}


PThread::Priority PThread::GetPriority() const
{
  return NormalPriority;
}


void PThread::Sleep(const PTimeInterval & timeout)
{
}


void PThread::Yield()
{
}


void PThread::Terminate()
{
}


PBoolean PThread::IsTerminated() const
{
  return false;
}


void PThread::WaitForTermination() const
{
  WaitForTermination(PMaxTimeInterval);
}


PBoolean PThread::WaitForTermination(const PTimeInterval & maxWait) const
{
  return true;
}


PUniqueThreadIdentifier PThread::GetUniqueIdentifier() const
{
  return GetThreadId();
}


PUniqueThreadIdentifier PThread::GetCurrentUniqueIdentifier()
{
  return GetCurrentThreadId();
}


void PProcess::GetMemoryUsage(MemoryUsage & usage)
{
}


bool PThread::GetTimes(Times & times)
{
  return false;
}


bool PProcess::GetProcessTimes(Times & times) const
{
  return false;
}


bool PProcess::GetSystemTimes(Times & times)
{
  return false;
}


int PThread::PXBlockOnIO(int handle, int type, const PTimeInterval & timeout)
{
  return false;
}


PSemaphore::~PSemaphore()
{
}


void PSemaphore::Reset(unsigned initial, unsigned maximum)
{
}


void PSemaphore::Wait()
{
}


PBoolean PSemaphore::Wait(const PTimeInterval & waitTime)
{
  return false;
}


void PSemaphore::Signal()
{
}


///////////////////////////////////////////////////////////////////////////////

PTimedMutex::PTimedMutex()
{
}

PTimedMutex::PTimedMutex(const PTimedMutex &)
{
}


void PTimedMutex::Wait()
{
}


PBoolean PTimedMutex::Wait(const PTimeInterval & waitTime)
{
  return true;
}


void PTimedMutex::Signal()
{
}


PSyncPoint::PSyncPoint()
{
}

PSyncPoint::PSyncPoint(const PSyncPoint &)
{
}

void PSyncPoint::Wait()
{
}


PBoolean PSyncPoint::Wait(const PTimeInterval & waitTime)
{
  return false;
}


void PSyncPoint::Signal()
{
}


#endif

