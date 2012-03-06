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
#define LOG_EMERG    0
#define LOG_ALERT    1
#define LOG_CRIT     2
#define LOG_ERR      3
#define LOG_WARNING  4
#define  LOG_NOTICE  5
#define LOG_INFO     6
#define LOG_DEBUG    7
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


// Doesn't everybody have this?
#ifndef _PATH_VARRUN
#define _PATH_VARRUN /var/run
#endif


#define new PNEW


extern void PXSignalHandler(int);

#define  MAX_LOG_LINE_LEN  1024

static const unsigned TraceUpSignal = SIGUSR1;
static const unsigned TraceDownSignal = SIGUSR2;


///////////////////////////////////////////////////////////////////////
//
PServiceProcess::PServiceProcess(const char * manuf,
                                 const char * name,
                                         WORD majorVersion,
                                         WORD minorVersion,
                                   CodeStatus status,
                                         WORD buildNumber)
  : PProcess(manuf, name, majorVersion, minorVersion, status, buildNumber)
{
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


static unsigned CountOptionSet(const PArgList & args, const char * options)
{
  unsigned count = 0;
  while (*options != '\0') {
    if (args.HasOption(*options))
      count++;
    ++options;
  }
  return count;
}


static PString ExpandOptionSet(const char * options)
{
  PStringStream strm;
  bool comma = false;
  while (options[1] != '\0') {
    if (comma)
      strm << ", ";
    else
      comma = true;
    strm << '-' << *options++;
  }
  strm << " or " << '-' << *options;
  return strm;
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
  PTrace::SetLevel(GetLogLevel());
#endif
#if PMEMORY_CHECK
  PMemoryHeap::SetIgnoreAllocations(PFalse);
#endif
  debugMode = PFalse;

  // parse arguments so we can grab what we want
  PArgList & args = GetArguments();

  PString progName = GetFile().GetTitle();

  args.Parse("[Execution:]"
#if !defined(BE_THREADS) && !defined(P_RTEMS)
             "d-daemon.           run as a daemon\n"
#endif
             "x-execute.          execute as a normal program\n"
             "v-version.          display version information and exit\n"
             "h-help.             output this help message and exit\n"
             "[Options:]"
             "p-pid-file:         file path or directory for pid file (default " _PATH_VARRUN ")\r"
                                 "if directory, then file is <dir>/" + progName + ".pid\n"
             "i-ini-file:         set the ini file to use, may be explicit file path or\r"
                                 "if directory, then file is <dir>/" + progName + ".ini\n"
             "u-uid:              set user id to run as\n"
             "g-gid:              set group id to run as\n"
             "c-console.          output messages to stdout instead of syslog\n"
             "l-log-file:         output messages to file or directory instead of syslog\r"
                                 "if directory then file is <dir>/" + progName + ".log\n"
             "r-remote-log:       output messages to remote syslog server\n"
             "H-handle-max:       set maximum number of file handles, this is set\r"
                                 "before the uid/gid, so can initially be superuser\n"
#ifdef P_LINUX
             "C-core-size:        set the maximum core file size\n"
#endif
             "[Control:]"
             "s-status.           check to see if daemon is running\n"
             "t-terminate.        orderly terminate process in pid file (SIGTERM)\n"
             "k-kill.             preemptively kill process in pid file (SIGKILL)\n"
             "U-trace-up.         increase the trace log level\n"
             "D-trace-down.       reduce the trace log level\n"
             , false);

  // if only displaying version information, do it and finish
  if (args.HasOption('v')) {
    cout << "Product Name: " << productName << "\n"
            "Manufacturer: " << manufacturer << "\n"
            "Version     : " << GetVersion(true) << "\n"
            "System      : " << PProcess::GetOSClass() << ' '
                             << GetOSName() << ' '
                             << GetOSVersion() << '('
                             << GetOSHardware() << ")\n"
            "PTLib       : " << PProcess::GetLibVersion() << endl;
    return 0;
  }

    // Validate the various command line options
  const char Set1[] = "dx";
  const char Set2[] = "ktsR";
  const char Set3[] = "clr";
  int set1 = CountOptionSet(args, Set1);
  int set2 = CountOptionSet(args, Set2);
  int set3 = CountOptionSet(args, Set3);
  if (args.HasOption('h') || !(set1 == 0 && set2 == 1) || !(set1 == 1 && set2 == 0) || set3 > 1) {
    if (set1 > 1)
      cerr << "error: must specify exactly one of " << ExpandOptionSet(Set1) << "\n\n";
    else if (set1 > 0 && set2 > 0)
      cerr << "error: cannot specify " << ExpandOptionSet(Set1) << " with " << ExpandOptionSet(Set2) << "\n\n";
    else if (set2 > 1)
      cerr << "error: must specify at most one of " << ExpandOptionSet(Set2) << "\n\n";
    else if (set3 > 1)
      cerr << "error: must specify at most one of " << ExpandOptionSet(Set3) << "\n\n";

    cerr << "usage: " << progName << " [ options ]\n";
    args.Usage(cerr);
    return 2;
  }

  PString pidfilename;
  if (args.HasOption('p'))
    pidfilename = args.GetOptionString('p');
  else
    pidfilename =  _PATH_VARRUN;

  if (!pidfilename && PDirectory::Exists(pidfilename))
    pidfilename = PDirectory(pidfilename) + progName + ".pid";

  if (set2 > 0) {
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

    if (args.HasOption('U') || args.HasOption('D')) {
      if (kill(pid, args.HasOption('D') ? TraceDownSignal : TraceUpSignal) != 0) {
        cout << "Process at " << pid << ' ';
        if (errno == ESRCH)
          cout << "does not exist.";
        else
          cout << " status could not be determined, error: " << strerror(errno);
        cout << endl;
      }
      return 0;
    }

    switch (KillProcess(pid, SIGTERM)) {
      case -1 :
        break;
      case 0 :
        PFile::Remove(pidfilename);
        return 0;
      case 1 :
        if (args.HasOption('k')) {
          switch (KillProcess(pid, SIGKILL)) {
            case -1 :
              break;
            case 0 :
              PFile::Remove(pidfilename);
              return 0;
          }
        }
        cout << "Process " << pid << " did not stop.\n";
        return 2;
    }

    cout << "Could not stop process " << pid << " - " << strerror(errno) << endl;
    return 1;
  }

  // set flag for console messages
  if (args.HasOption('c')) {
    PSystemLog::SetTarget(new PSystemLogToStderr());
    debugMode = true;
  }
  else if (args.HasOption('l')) {
    PFilePath fileName = args.GetOptionString('l');
    if (fileName.IsEmpty()) {
      cout << "error: must specify file name for -l" << endl;
      return 1;
    }
    else if (PDirectory::Exists(fileName))
      fileName = PDirectory(fileName) + progName + ".log";
    PSystemLog::SetTarget(new PSystemLogToFile(fileName));
  }
  else if (args.HasOption('r'))
    PSystemLog::SetTarget(new PSystemLogToNetwork(args.GetOptionString('r')));
  else
    PSystemLog::SetTarget(new PSystemLogToSyslog());

  // open the system logger for this program
  PSYSTEMLOG(StdError, "Starting service process \"" << GetName() << "\" v" << GetVersion(PTrue));

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

  if (args.HasOption('i'))
    SetConfigurationPath(args.GetOptionString('i'));

  if (args.HasOption('H')) {
    int uid = geteuid();
    seteuid(getuid()); // Switch back to starting uid for next call
    SetMaxHandles(args.GetOptionString('H').AsInteger());
    seteuid(uid);
  }

#ifdef P_LINUX
  // set the core file size
  if (args.HasOption('C')) {
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
  }
#endif

  // We are a service, don't want to get blocked on input from stdin during asserts
  if (!debugMode)
    ::close(STDIN_FILENO);

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

  PSystemLog::SetTarget(NULL);

  // Now end the program
  _exit(terminationValue);
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

  PThreadIdentifier tid = GetCurrentThreadId();
  ThreadMap::iterator thread = m_activeThreads.find(tid);

  char msg[200];
  sprintf(msg, "\nCaught %s, thread_id=%08lx", sigmsg, (long)tid);

  if (thread != m_activeThreads.end()) {
    PString thread_name = thread->second->GetThreadName();
    if (thread_name.IsEmpty())
      sprintf(&msg[strlen(msg)], " obj_ptr=%p", thread->second);
    else {
      strcat(msg, " name=");
      strcat(msg, thread_name);
    }
  }

  strcat(msg, ", aborting.\n");

  PSYSTEMLOG(Fatal, msg);

  raise(SIGQUIT); // Dump core
  _exit(-1); // Fail safe if raise() didn't dump core and exit
}


void PServiceProcess::PXOnSignal(int sig)
{
  static const char * const LevelName[PSystemLog::NumLogLevels] = {
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

  PProcess::PXOnSignal(sig);
  switch (sig) {
    case SIGINT :
    case SIGTERM :
      Terminate();
      break;

    case TraceUpSignal :
      if (GetLogLevel() < PSystemLog::NumLogLevels-1) {
        SetLogLevel((PSystemLog::Level)(GetLogLevel()+1));
        PSystemLog s(PSystemLog::StdError);
        s << "Log level increased to " << LevelName[GetLogLevel()];
      }
      break;

    case TraceDownSignal :
      if (GetLogLevel() > PSystemLog::Fatal) {
        SetLogLevel((PSystemLog::Level)(GetLogLevel()-1));
        PSystemLog s(PSystemLog::StdError);
        s << "Log level decreased to " << LevelName[GetLogLevel()];
      }
      break;
  }
}

