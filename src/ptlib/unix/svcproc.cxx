/*
 * svcproc.cxx
 *
 * Service process (daemon) implementation.
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

#include <ptlib.h>

#pragma implementation "svcproc.h"
#include <ptlib/svcproc.h>

#ifdef P_VXWORKS
#include <logLib.h>
#define LOG_EMERG      0
#define LOG_ALERT      1
#define LOG_CRIT      2
#define LOG_ERR        3
#define LOG_WARNING      4
#define  LOG_NOTICE      5
#define LOG_INFO      6
#define LOG_DEBUG      7
#else
#include <syslog.h>
#include <pwd.h>
#include <grp.h>
#endif

#include <stdarg.h>
#if (__GNUC__ >= 3)
#include <fstream>
#else
#include <fstream.h>
#endif
#include <signal.h>

#include "uerror.h"

#ifdef P_LINUX
#include <sys/resource.h>
#endif

#define new PNEW

extern void PXSignalHandler(int);

#define  MAX_LOG_LINE_LEN  1024

#ifndef P_VXWORKS
static int PwlibLogToUnixLog[PSystemLog::NumLogLevels] = {
  LOG_CRIT,    // LogFatal,   
  LOG_ERR,     // LogError,   
  LOG_WARNING, // LogWarning, 
  LOG_INFO,    // LogInfo,    
  LOG_DEBUG,   // LogDebug
  LOG_DEBUG,
  LOG_DEBUG,
  LOG_DEBUG,
  LOG_DEBUG,
  LOG_DEBUG
};
#endif // !P_VXWORKS

static const char * const PLevelName[PSystemLog::NumLogLevels+1] = {
  "Message",
  "Fatal error",
  "Error",
  "Warning",
  "Info",
  "Debug",
  "Debug2",
  "Debug3",
  "Debug4",
  "Debug5",
  "Debug6",
};

#ifdef P_MAC_MPTHREADS
// alas, this can't be statically initialized
// XXX This ought to be an MPCriticalRegionID, but they're broken in
// XXX Mac OS X 10.0.x!
static MPSemaphoreID logMutex;

// yuck.
static void SetUpLogMutex()
{
    if (logMutex == 0) {
        MPSemaphoreID tempCrit;
        long err = MPCreateSemaphore(1, 1, &tempCrit);
        PAssertOS(err == 0);
        if (!OTCompareAndSwap32(0, (UInt32)tempCrit, (UInt32*)&logMutex)) {
            // lost the race
            MPDeleteSemaphore(tempCrit);
        }
    }
}
#endif
#ifdef P_PTHREADS

static pthread_mutex_t logMutex = PTHREAD_MUTEX_INITIALIZER;

#endif

void PSystemLog::Output(Level level, const char * cmsg)
{
  PString systemLogFileName = PServiceProcess::Current().systemLogFileName;
  if (systemLogFileName.IsEmpty()) {
#ifdef P_VXWORKS
    printf("%s\n",cmsg);
    logMsg((char *)(const char)cmsg,0,0,0,0,0,0);
#else
    syslog(PwlibLogToUnixLog[level], "%s", cmsg);
#endif
  }
  else {
#ifdef P_PTHREADS
    pthread_mutex_lock(&logMutex);
#endif
#ifdef P_MAC_MPTHREADS
    SetUpLogMutex();
    (void)MPWaitOnSemaphore(logMutex, kDurationForever);
#endif

    ostream * out;
    if (systemLogFileName == "-")
      out = &cerr;
    else
      out = new ofstream(systemLogFileName, ios::app);

    PTime now;
    *out << now.AsString("yyyy/MM/dd hh:mm:ss.uuu\t");

    PThread * thread = PThread::Current();
    if (thread == NULL) {
#ifdef P_MAC_MPTHREADS
      unsigned tid = (unsigned)MPCurrentTaskID();
#elif defined(P_VXWORKS)
      unsigned tid = ::taskIdSelf();
#elif defined(BE_THREADS)
      thread_id tid = ::find_thread(NULL);
#else
      unsigned tid = (uintptr_t) pthread_self();
#endif
      *out << "ThreadID=0x"
           << setfill('0') << ::hex
           << setw(8) << tid
           << setfill(' ') << ::dec;
    } else {
      PString threadName = thread->GetThreadName();
      if (threadName.GetLength() <= 23)
        *out << setw(23) << threadName;
      else
        *out << threadName.Left(10) << "..." << threadName.Right(10);
    }

    *out << '\t'
         << PLevelName[level+1]
         << '\t'
         << cmsg << endl;

    if (out != &cerr)
      delete out;

#ifdef P_PTHREADS
    pthread_mutex_unlock(&logMutex);
#endif
#ifdef P_MAC_MPTHREADS
    MPSignalSemaphore(logMutex);
#endif
  }
}


int PSystemLog::Buffer::overflow(int c)
{
  if (pptr() >= epptr()) {
    int ppos = pptr() - pbase();
    char * newptr = string.GetPointer(string.GetSize() + 10);
    setp(newptr, newptr + string.GetSize() - 1);
    pbump(ppos);
  }
  if (c != EOF) {
    *pptr() = (char)c;
    pbump(1);
  }
  return 0;
}


int PSystemLog::Buffer::underflow()
{
  return EOF;
}


int PSystemLog::Buffer::sync()
{
  PSystemLog::Output(log->logLevel, string);

  string = PString();
  char * base = string.GetPointer(10);
  setp(base, base + string.GetSize() - 1);
  return 0;
}


PServiceProcess::PServiceProcess(const char * manuf,
                                 const char * name,
                                         WORD majorVersion,
                                         WORD minorVersion,
                                   CodeStatus status,
                                         WORD buildNumber)
  : PProcess(manuf, name, majorVersion, minorVersion, status, buildNumber)
{
  currentLogLevel = PSystemLog::Warning;
  isTerminating = PFalse;
}


PServiceProcess::~PServiceProcess()
{
  PSetErrorStream(NULL);
#if PTRACING
  PTrace::SetStream(NULL);
  PTrace::ClearOptions(PTrace::SystemLogStream);
#endif

  if (!pidFileToRemove)
    PFile::Remove(pidFileToRemove);

#ifndef P_VXWORKS
  // close the system log
  if (systemLogFileName.IsEmpty())
    closelog();
#endif // !P_VXWORKS
}


PServiceProcess & PServiceProcess::Current()
{
  PProcess & process = PProcess::Current();
  PAssert(PIsDescendant(&process, PServiceProcess), "Not a service process!");
  return (PServiceProcess &)process;
}


#ifndef P_VXWORKS
static int KillProcess(int pid, int sig)
{
  if (kill(pid, sig) != 0)
    return -1;

  cout << "Sent SIG";
  if (sig == SIGTERM)
    cout << "TERM";
  else
    cout << "KILL";
  cout << " to daemon at pid " << pid << ' ' << flush;

  for (PINDEX retry = 1; retry <= 10; retry++) {
    PThread::Sleep(1000);
    if (kill(pid, 0) != 0) {
      cout << "\nDaemon stopped." << endl;
      return 0;
    }
    cout << '.' << flush;
  }
  cout << "\nDaemon has not stopped." << endl;

  return 1;
}
#endif // !P_VXWORKS


void PServiceProcess::_PXShowSystemWarning(PINDEX code, const PString & str)
{
  PSYSTEMLOG(Warning, "PWLib\t" << GetOSClass() << " error #" << code << '-' << str);
}


int PServiceProcess::InitialiseService()
{
#ifndef P_VXWORKS
#if PMEMORY_CHECK
  PMemoryHeap::SetIgnoreAllocations(PTrue);
#endif
  PSetErrorStream(new PSystemLog(PSystemLog::StdError));
#if PTRACING
  PTrace::SetStream(new PSystemLog(PSystemLog::Debug3));
  PTrace::ClearOptions(PTrace::FileAndLine);
  PTrace::SetOptions(PTrace::SystemLogStream);
  PTrace::SetLevel(4);
#endif
#if PMEMORY_CHECK
  PMemoryHeap::SetIgnoreAllocations(PFalse);
#endif
  debugMode = PFalse;

  // parse arguments so we can grab what we want
  PArgList & args = GetArguments();

  args.Parse("v-version."
             "d-daemon."
             "c-console."
             "h-help."
             "x-execute."
             "p-pid-file:"
       "H-handlemax:"
             "i-ini-file:"
             "k-kill."
             "t-terminate."
             "s-status."
             "l-log-file:"
             "u-uid:"
             "g-gid:"
             "C-core-size:"
           , false);

  // if only displaying version information, do it and finish
  if (args.HasOption('v')) {
    cout << "Product Name: " << productName << endl
         << "Manufacturer: " << manufacturer << endl
         << "Version     : " << GetVersion(PTrue) << endl
         << "System      : " << GetOSName() << '-'
                             << GetOSHardware() << ' '
                             << GetOSVersion() << endl;
    return 0;
  }

  PString pidfilename;
  if (args.HasOption('p'))
    pidfilename = args.GetOptionString('p');
#ifdef _PATH_VARRUN
  else
    pidfilename =  _PATH_VARRUN;
#endif

  if (!pidfilename && PDirectory::Exists(pidfilename))
    pidfilename = PDirectory(pidfilename) + PProcess::Current().GetFile().GetFileName() + ".pid";

  if (args.HasOption('k') || args.HasOption('t') || args.HasOption('s')) {
    pid_t pid;

    {
      ifstream pidfile(pidfilename);
      if (!pidfile.is_open()) {
        cout << "Could not open pid file: \"" << pidfilename << "\""
                " - " << strerror(errno) << endl;
        return 1;
      }

      pidfile >> pid;
      if (pid == 0) {
        cout << "Illegal format pid file \"" << pidfilename << '"' << endl;
        return 1;
      }
    }

    if (args.HasOption('s')) {
      cout << "Process at " << pid << ' ';
      if (kill(pid, 0) == 0)
        cout << "is running.";
      else if (errno == ESRCH)
        cout << "does not exist.";
      else
        cout << " status could not be determined, error: " << strerror(errno);
      cout << endl;
      return 0;
    }

    int sig = args.HasOption('t') ? SIGTERM : SIGKILL;
    switch (KillProcess(pid, sig)) {
      case -1 :
        break;
      case 0 :
        PFile::Remove(pidfilename);
        return 0;
      case 1 :
        if (args.HasOption('t') && args.HasOption('k')) {
          switch (KillProcess(pid, SIGKILL)) {
            case -1 :
              break;
            case 0 :
              PFile::Remove(pidfilename);
              return 0;
            case 1 :
              return 2;
          }
        }
        else
          return 2;
    }

    cout << "Could not stop process " << pid <<
            " - " << strerror(errno) << endl;
    return 1;
  }

  // Set the gid we are running under
  if (args.HasOption('g')) {
    PString gidstr = args.GetOptionString('g');
    if (!SetGroupName(gidstr)) {
      cout << "Could not set GID to \"" << gidstr << "\" - " << strerror(errno) << endl;
      return 1;
    }
  }

  // Set the uid we are running under
  if (args.HasOption('u')) {
    PString uidstr = args.GetOptionString('u');
    if (!SetUserName(uidstr)) {
      cout << "Could not set UID to \"" << uidstr << "\" - " << strerror(errno) << endl;
      return 1;
    }
  }

  PBoolean helpAndExit = PFalse;

  // if displaying help, then do it
  if (args.HasOption('h')) 
    helpAndExit = PTrue;
  else if (!args.HasOption('d') && !args.HasOption('x')) {
    cout << "error: must specify one of -v, -h, -t, -k, -d or -x" << endl;
    helpAndExit = PTrue;
  }

  // set flag for console messages
  if (args.HasOption('c')) {
    systemLogFileName = '-';
    debugMode = PTrue;
  }

  if (args.HasOption('l')) {
    systemLogFileName = args.GetOptionString('l');
    if (systemLogFileName.IsEmpty()) {
      cout << "error: must specify file name for -l" << endl;
      helpAndExit = PTrue;
    }
    else if (PDirectory::Exists(systemLogFileName))
      systemLogFileName = PDirectory(systemLogFileName) + PProcess::Current().GetFile().GetFileName() + ".log";
  }

  if (helpAndExit) {
    cout << "usage: [-c] -v|-d|-h|-x\n"
            "  -h --help           output this help message and exit\n"
            "  -v --version        display version information and exit\n"
#if !defined(BE_THREADS) && !defined(P_RTEMS)
            "  -d --daemon         run as a daemon\n"
#endif
            "  -u --uid uid        set user id to run as\n"
            "  -g --gid gid        set group id to run as\n"
            "  -p --pid-file       name or directory for pid file\n"
            "  -t --terminate      orderly terminate process in pid file\n"
            "  -k --kill           preemptively kill process in pid file\n"
            "  -s --status         check to see if daemon is running\n"
            "  -c --console        output messages to stdout rather than syslog\n"
            "  -l --log-file file  output messages to file or directory instead of syslog\n"
            "  -x --execute        execute as a normal program\n"
            "  -i --ini-file       set the ini file to use, may be explicit file or\n"
            "                      a ':' separated set of directories to search.\n"
            "  -H --handlemax n    set maximum number of file handles (set before uid/gid)\n"
#ifdef P_LINUX
            "  -C --core-size      set the maximum core file size\n"
#endif
         << endl;
    return 0;
  }

  // open the system logger for this program
  if (systemLogFileName.IsEmpty())
    openlog((char *)(const char *)GetName(), LOG_PID, LOG_DAEMON);
  else if (systemLogFileName == "-")
    cout << "All output for " << GetName() << " is to console." << endl;
  else {
    ofstream logfile(systemLogFileName, ios::app);
    if (!logfile.is_open()) {
      cout << "Could not open log file \"" << systemLogFileName << "\""
              " - " << strerror(errno) << endl;
      return 1;
    }
  }
  PSYSTEMLOG(StdError, "Starting service process \"" << GetName() << "\" v" << GetVersion(PTrue));

  if (args.HasOption('i'))
    SetConfigurationPath(PFilePath(args.GetOptionString('i')));

  if (args.HasOption('H')) {
    int uid = geteuid();
    seteuid(getuid()); // Switch back to starting uid for next call
    SetMaxHandles(args.GetOptionString('H').AsInteger());
    seteuid(uid);
  }

  // set the core file size
  if (args.HasOption('C')) {
#ifdef P_LINUX
    struct rlimit rlim;
    if (getrlimit(RLIMIT_CORE, &rlim) != 0) 
      cout << "Could not get current core file size : error = " << errno << endl;
    else {
      int uid = geteuid();
      seteuid(getuid()); // Switch back to starting uid for next call
      int v = args.GetOptionString('C').AsInteger();
      rlim.rlim_cur = v;
      if (setrlimit(RLIMIT_CORE, &rlim) != 0) 
        cout << "Could not set current core file size to " << v << " : error = " << errno << endl;
      else {
        getrlimit(RLIMIT_CORE, &rlim);
        cout << "Core file size set to " << rlim.rlim_cur << "/" << rlim.rlim_max << endl;
      }
      seteuid(uid);
    }
#endif
  }

#if !defined(BE_THREADS) && !defined(P_RTEMS)
  if (!args.HasOption('d'))
    return -1;

  // Run as a daemon, ie fork

  if (!pidfilename) {
    ifstream pidfile(pidfilename);
    if (pidfile.is_open()) {
      pid_t pid;
      pidfile >> pid;
      if (pid != 0 && kill(pid, 0) == 0) {
        cout << "Already have daemon running with pid " << pid << endl;
        return 2;
      }
    }
  }

  // Need to get rid of the config write thread before fork, as on
  // pthreads platforms the forked process does not have the thread
  CommonDestruct();

  pid_t pid = fork();
  switch (pid) {
    case 0 : // The forked process
      break;

    case -1 : // Failed
      cout << "Fork failed creating daemon process." << endl;
      return 1;

    default : // Parent process
      cout << "Daemon started with pid " << pid << endl;
      if (!pidfilename) {
        // Write out the child pid to magic file in /var/run (at least for linux)
        ofstream pidfile(pidfilename);
        if (pidfile.is_open())
          pidfile << pid;
        else
          cout << "Could not write pid to file \"" << pidfilename << "\""
                  " - " << strerror(errno) << endl;
      }
      return 0;
  }

  // Set ourselves as out own process group so we don't get signals
  // from our parent's terminal (hopefully!)
  PSETPGRP();

  CommonConstruct();

  pidFileToRemove = pidfilename;

  // Only if we are running in the background as a daemon, we intercept
  // the core dumping signals so get a message in the log file.
  signal(SIGSEGV, PXSignalHandler);
  signal(SIGFPE, PXSignalHandler);
  signal(SIGBUS, PXSignalHandler);

  // Also if in background, don't want to get blocked on input from stdin
  ::close(STDIN_FILENO);

#endif // !BE_THREADS && !P_RTEMS
#endif // !P_VXWORKS
  return -1;
}

int PServiceProcess::InternalMain(void *)
{
  if ((terminationValue = InitialiseService()) < 0) {
    // Make sure housekeeping thread is going so signals are handled.
    SignalTimerChange();

    terminationValue = 1;
    if (OnStart()) {
      terminationValue = 0;
      Main();
      Terminate();
    }
  }

  return terminationValue;
}


PBoolean PServiceProcess::OnPause()
{
  return PTrue;
}

void PServiceProcess::OnContinue()
{
}

void PServiceProcess::OnStop()
{
}


void PServiceProcess::Terminate()
{
  if (isTerminating) {
    // If we are the process itself and another thread is terminating us,
    // just stop and wait forever for us to go away
    if (PThread::Current() == this)
      Sleep(PMaxTimeInterval);
    PSYSTEMLOG(Error, "Nested call to process termination!");
    return;
  }

  isTerminating = PTrue;

  PSYSTEMLOG(Warning, "Stopping service process \"" << GetName() << "\" v" << GetVersion(PTrue));

  // Avoid strange errors caused by threads (and the process itself!) being destoyed 
  // before they have EVER been scheduled
  Yield();

  // Do the services stop code
  OnStop();

#ifndef P_VXWORKS
  // close the system log
  if (systemLogFileName.IsEmpty())
    closelog();
#endif // !P_VXWORKS

  // Now end the program
  exit(terminationValue);
}

void PServiceProcess::PXOnAsyncSignal(int sig)
{
  const char * sigmsg;

  // Override the default behavious for these signals as that just
  // summarily exits the program. Allow PXOnSignal() to do orderly exit.

  switch (sig) {
    case SIGINT :
    case SIGTERM :
    case SIGHUP :
      return;

    case SIGSEGV :
      sigmsg = "segmentation fault (SIGSEGV)";
      break;

    case SIGFPE :
      sigmsg = "floating point exception (SIGFPE)";
      break;

#ifndef __BEOS__ // In BeOS, SIGBUS is the same value as SIGSEGV
    case SIGBUS :
      sigmsg = "bus error (SIGBUS)";
      break;
#endif
    default :
      PProcess::PXOnAsyncSignal(sig);
      return;
  }

  signal(SIGSEGV, SIG_DFL);
  signal(SIGFPE, SIG_DFL);
  signal(SIGBUS, SIG_DFL);

  static PBoolean inHandler = PFalse;
  if (inHandler) {
    raise(SIGQUIT); // Dump core
    _exit(-1); // Fail safe if raise() didn't dump core and exit
  }

  inHandler = PTrue;

#ifdef P_MAC_MPTHREADS
  unsigned tid = (unsigned)MPCurrentTaskID();
#elif defined(P_VXWORKS)
  unsigned tid = ::taskIdSelf();
#elif defined(BE_THREADS)
  thread_id tid = ::find_thread(NULL);
#else
  unsigned tid = (uintptr_t) pthread_self();
#endif
  PThread * thread_ptr = activeThreads.GetAt(tid);

  char msg[200];
  sprintf(msg, "\nCaught %s, thread_id=%u", sigmsg, tid);

  if (thread_ptr != NULL) {
    PString thread_name = thread_ptr->GetThreadName();
    if (thread_name.IsEmpty())
      sprintf(&msg[strlen(msg)], " obj_ptr=%p", thread_ptr);
    else {
      strcat(msg, " name=");
      strcat(msg, thread_name);
    }
  }

  strcat(msg, ", aborting.\n");

  if (systemLogFileName.IsEmpty()) {
#ifdef P_VXWORKS
  logMsg((char *)msg,0,0,0,0,0,0);
#else
    syslog(LOG_CRIT, msg); 
    closelog();
#endif // !P_VXWORKS
  }
  else {
#ifdef P_VXWORKS
    int fd = open(systemLogFileName, O_WRONLY|O_APPEND, FWRITE|FAPPEND);
#else
    int fd = open(systemLogFileName, O_WRONLY|O_APPEND);
#endif // !P_VXWORKS
    if (fd >= 0) {
      write(fd, msg, strlen(msg));
      close(fd);
    }
  }

  raise(SIGQUIT); // Dump core
  _exit(-1); // Fail safe if raise() didn't dump core and exit
}


void PServiceProcess::PXOnSignal(int sig)
{
  PProcess::PXOnSignal(sig);
  switch (sig) {
    case SIGINT :
    case SIGTERM :
      Terminate();
      break;

    case SIGUSR1 :
      OnPause();
      break;

    case SIGUSR2 :
      OnContinue();
      break;

#if 0
    case SIGHUP :
      if (currentLogLevel < PSystemLog::NumLogLevels-1) {
        currentLogLevel = (PSystemLog::Level)(currentLogLevel+1);
        PSystemLog s(PSystemLog::StdError);
        s << "Log level increased to " << PLevelName[currentLogLevel+1];
      }
      break;

#ifdef SIGWINCH
    case SIGWINCH :
      if (currentLogLevel > PSystemLog::Fatal) {
        currentLogLevel = (PSystemLog::Level)(currentLogLevel-1);
        PSystemLog s(PSystemLog::StdError);
        s << "Log level decreased to " << PLevelName[currentLogLevel+1];
      }
      break;
#endif
#endif
  }
}

