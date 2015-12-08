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
#include <ptclib/pxml.h>

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
#include <malloc.h>
#include <fstream>
#ifndef P_RTEMS
#include <sys/resource.h>
#endif
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


#if defined(P_LINUX)

static inline PTimeInterval jiffies_to_msecs(const uint64_t jiffies)
{
  static long sysconf_HZ = sysconf(_SC_CLK_TCK);
  return (jiffies * 1000) / sysconf_HZ;
}


static bool InternalGetTimes(const char * filename, PThread::Times & times)
{
  /* From the man page on the "stat" file
  Status information about the process. This is used by ps(1). It is defined in /usr/src/linux/fs/proc/array.c.
  The fields, in order, with their proper scanf(3) format specifiers, are:
  pid         %d   The process ID.
  comm        %s   The filename of the executable, in parentheses. This is visible
  whether or not the executable is swapped out.
  state       %c   One character from the string "RSDZTW" where R is running, S is
  sleeping in an interruptible wait, D is waiting in uninterruptible
  disk sleep, Z is zombie, T is traced or stopped (on a signal), and
  W is paging.
  ppid        %d   The PID of the parent.
  pgrp        %d   The process group ID of the process.
  session     %d   The session ID of the process.
  tty_nr      %d   The tty the process uses.
  tpgid       %d   The process group ID of the process which currently owns the tty
  that the process is connected to.
  flags       %lu  The kernel flags word of the process. For bit meanings, see the
  PF_* defines in <linux/sched.h>. Details depend on the kernel
  version.
  minflt      %lu  The number of minor faults the process has made which have not
  required loading a memory page from disk.
  cminflt     %lu  The number of minor faults that the process's waited-for children
  have made.
  majflt      %lu  The number of major faults the process has made which have required
  loading a memory page from disk.
  cmajflt     %lu  The number of major faults that the process's waited-for children
  have made.
  utime       %lu  The number of jiffies that this process has been scheduled in user
  mode.
  stime       %lu  The number of jiffies that this process has been scheduled in kernel
  mode.
  cutime      %ld  The number of jiffies that this process's waited-for children have
  been scheduled in user mode. (See also times(2).)
  cstime      %ld  The number of jiffies that this process's waited-for children have
  been scheduled in kernel mode.
  priority    %ld  The standard nice value, plus fifteen. The value is never negative
  in the kernel.
  nice        %ld  The nice value ranges from 19 (nicest) to -19 (not nice to others).
  num_threads %ld  Number of threads.
  itrealvalue %ld  The time in jiffies before the next SIGALRM is sent to the process
  due to an interval timer.
  starttime   %lu  The time in jiffies the process started after system boot.
  vsize       %lu  Virtual memory size in bytes.
  rss         %ld  Resident Set Size: number of pages the process has in real memory,
  minus 3 for administrative purposes. This is just the pages which
  count towards text, data, or stack space. This does not include
  pages which have not been demand-loaded in, or which are swapped out.
  rlim        %lu  Current limit in bytes on the rss of the process
  (usually 4294967295 on i386).
  startcode   %lu  The address above which program text can run.
  endcode     %lu  The address below which program text can run.
  startstack  %lu  The address of the start of the stack.
  kstkesp     %lu  The current value of esp (stack pointer), as found in the kernel
  stack page for the process.
  kstkeip     %lu  The current EIP (instruction pointer).
  signal      %lu  The bitmap of pending signals.
  blocked     %lu  The bitmap of blocked signals.
  sigignore   %lu  The bitmap of ignored signals.
  sigcatch    %lu  The bitmap of caught signals.
  wchan       %lu  This is the "channel" in which the process is waiting. It is the
  address of a system call, and can be looked up in a namelist if you
  need a textual name. (If you have an up-to-date /etc/psdatabase, then
  try ps -l to see the WCHAN field in action.)
  nswap       %lu  Number of pages swapped (not maintained).
  cnswap      %lu  Cumulative nswap for child processes (not maintained).
  exit_signal %d   Signal to be sent to parent when we die.
  processor   %d   CPU number last executed on.
  rt_priority %lu  (since kernel 2.5.19) Real-time scheduling priority (see sched_setscheduler(2)).
  policy      %lu  (since kernel 2.5.19) Scheduling policy (see sched_setscheduler(2)).
  delayacct_blkio_ticks %llu (since Linux 2.6.18) Aggregated block I/O delays, measured in
  clock ticks (centiseconds).
  */

  for (int retry = 0; retry < 3; ++retry) {
    std::ifstream statfile(filename);

    char line[1000];
    statfile.getline(line, sizeof(line));
    if (!statfile.good())
      continue;

    int pid;
    char comm[100];
    char state;
    int ppid, pgrp, session, tty_nr, tpgid;
    unsigned long flags, minflt, cminflt, majflt, cmajflt, utime, stime;
    long cutime, cstime, priority, nice, num_threads, itrealvalue;
    unsigned long starttime, vsize;
    long rss;
    unsigned long rlim, startcode, endcode, startstack, kstkesp, kstkeip, signal, blocked, sigignore, sigcatch, wchan, nswap, cnswap;
    int exit_signal, processor;
    unsigned long rt_priority, policy;
    unsigned long long delayacct_blkio_ticks;

    // 17698 (maxmcu) R 1 17033 8586 34833 17467 4202560 7
    // 0 0 0 0 0 0 0 -100 0 16
    // 0 55172504 258756608 6741 4294967295 134512640 137352760 3217892976 8185700 15991824
    // 0 0 4 201349635 0 0 0 -1 7 99
    // 2 0

    int count = sscanf(line,
                       "%d%s %c%d%d%d%d%d%lu%lu"
                       "%lu%lu%lu%lu%lu%ld%ld%ld%ld%ld"
                       "%ld%lu%lu%ld%lu%lu%lu%lu%lu%lu"
                       "%lu%lu%lu%lu%lu%lu%lu%d%d%lu"
                       "%lu%llu",
                       &pid, comm, &state, &ppid, &pgrp, &session, &tty_nr, &tpgid, &flags, &minflt,
                       &cminflt, &majflt, &cmajflt, &utime, &stime, &cutime, &cstime, &priority, &nice, &num_threads,
                       &itrealvalue, &starttime, &vsize, &rss, &rlim, &startcode, &endcode, &startstack, &kstkesp, &kstkeip,
                       &signal, &blocked, &sigignore, &sigcatch, &wchan, &nswap, &cnswap, &exit_signal, &processor, &rt_priority,
                       &policy, &delayacct_blkio_ticks);
    if (count != 42)
      continue;

    times.m_kernel = jiffies_to_msecs(stime);
    times.m_user = jiffies_to_msecs(utime);
    return true;
  }

  return false;
}


bool PThread::GetTimes(Times & times)
{
  // Do not use any PTLib functions in here as they could to a PTRACE, and this deadlock
  times.m_name = GetThreadName();
  times.m_threadId = m_threadId;
  times.m_uniqueId = GetUniqueIdentifier();
#if P_PTHREADS
  times.m_real = (PX_endTick != 0 ? PX_endTick : PTimer::Tick()) - PX_startTick;
#endif

  std::stringstream path;
  path << "/proc/self/task/" << times.m_uniqueId << "/stat";
  return InternalGetTimes(path.str().c_str(), times);
}


bool PProcess::GetProcessTimes(Times & times) const
{
  times.m_name = "Process Total";
  times.m_real = PTime() - GetStartTime();
  return InternalGetTimes("/proc/self/stat", times);
}


bool PProcess::GetSystemTimes(Times & times)
{
  times.m_name = "System Total";

  for (int retry = 0; retry < 3; ++retry) {
    std::ifstream statfile("/proc/stat");

    char dummy[10];
    uint64_t user, nice, system, idle, iowait, irq, softirq, steal;
    statfile >> dummy >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal;
    if (statfile.good()) {
      times.m_kernel = jiffies_to_msecs(system);
      times.m_user = jiffies_to_msecs(user);
      times.m_real = times.m_kernel + times.m_user + jiffies_to_msecs(idle);
      return true;
    }
  }
  return false;
}


void PProcess::GetMemoryUsage(MemoryUsage & usage)
{
  ifstream proc("/proc/self/statm");
  size_t virtPages, resPages;
  proc >> virtPages >> resPages;
  if (proc.good()) {
    usage.m_virtual = virtPages * 4096;
    usage.m_resident = resPages * 4096;
  }
  else
    usage.m_virtual = usage.m_resident = 0;

#if P_HAS_MALLOC_INFO
  char * buffer = NULL;
  size_t size;
  FILE * mem = open_memstream(&buffer, &size);
  bool ok = malloc_info(0, mem) == 0;
  fclose(mem);

  if (ok) {
    // Find last </heap> in XML
    size_t offset = 0;
    static PRegularExpression HeapRE("< */ *heap *>", PRegularExpression::Extended);
    PINDEX pos, len;
    while (HeapRE.Execute(buffer + offset, pos, len))
      offset += pos + len;

    // The <system type="xxx" size=yyy> after the <heap>s is total

    PStringArray substrings(2);

    static PRegularExpression MaxRE("< *system *type *= *\"max\" *size *= *\"([0-9]+)", PRegularExpression::Extended);
    if (MaxRE.Execute(buffer + offset, substrings)) {
      usage.m_max = (size_t)substrings[1].AsUnsigned64();
      ok = usage.m_max > 0;
    }

    static PRegularExpression CurrentRE("< *system *type *= *\"current\" *size *= *\"([0-9]+)", PRegularExpression::Extended);
    if (CurrentRE.Execute(buffer + offset, substrings))
      usage.m_current = (size_t)substrings[1].AsUnsigned64();
  }

  runtime_free(buffer);

  if (!ok)
#endif // P_HAS_MALLOC_INFO
  {
    struct mallinfo info = mallinfo();
    usage.m_max = info.usmblks > 0 ? info.usmblks : (info.uordblks + info.fordblks + info.fsmblks);
    usage.m_current = info.uordblks;
    usage.m_blocks = info.hblks;
  }
}

#else //P_LINUX

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

#endif // P_LINUX


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

#warning No thread support, practically nothing will work!


void PProcess::Construct()
{
  CommonConstruct();
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
  usleep(timeout.GetInterval() * 1000);
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

PTimedMutex::PTimedMutex(const char *, unsigned)
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

