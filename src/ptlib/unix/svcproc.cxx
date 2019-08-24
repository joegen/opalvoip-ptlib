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
 */

#include <ptlib.h>

#pragma implementation "svcproc.h"
#include <ptlib/svcproc.h>
#include <ptlib/pluginmgr.h>


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
#define _PATH_VARRUN "/var/run"
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
                                     unsigned majorVersion,
                                     unsigned minorVersion,
                                   CodeStatus status,
                                     unsigned patchVersion,
                                     unsigned oemVersion)
  : PProcess(manuf, name, majorVersion, minorVersion, status, patchVersion, false, true, oemVersion)
{
  isTerminating = false;
}


PServiceProcess::~PServiceProcess()
{
  if (!pidFileToRemove.IsEmpty())
    PFile::Remove(pidFileToRemove);
}


PServiceProcess & PServiceProcess::Current()
{
  PProcess & process = PProcess::Current();
  PAssert(PIsDescendant(&process, PServiceProcess), "Not a service process!");
  return (PServiceProcess &)process;
}


void PServiceProcess::SetLogLevel(PSystemLog::Level level)
{
  PSystemLog::GetTarget().SetThresholdLevel(level);

#if PTRACING
  if ((PTrace::GetOptions()&PTrace::SystemLogStream) != 0)
    PTrace::SetLevel(level);
#endif
}


#ifndef P_VXWORKS
static int KillProcess(int pid, unsigned timeout, int sig)
{
  if (kill(pid, sig) != 0) {
    cout << "Could not stop process " << pid << " - " << strerror(errno) << endl;
    return -1;
  }

  cout << "Sent SIG";
  if (sig == SIGTERM)
    cout << "TERM";
  else
    cout << "KILL";
  cout << " to daemon at pid " << pid << ' ' << flush;

  for (unsigned retry = 1; retry <= timeout; ++retry) {
    cout << '.' << flush;
    usleep(1000000);
    if (kill(pid, 0) != 0) {
      cout << "\nDaemon stopped." << endl;
      return 0;
    }
  }

  cout << "\nDaemon has not stopped." << endl;
  return 1;
}
#endif // !P_VXWORKS


void PServiceProcess::_PXShowSystemWarning(PINDEX code, const PString & str)
{
  PSYSTEMLOG(Warning, "PTLib\t" << GetOSClass() << " error #" << code << '-' << str);
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
  {
    PMEMORY_IGNORE_ALLOCATIONS_FOR_SCOPE;

#if PTRACING
    PTrace::SetStream(new PSystemLog(PSystemLog::Debug3));
#if _DEBUG
    PTrace::ClearOptions(PTrace::Timestamp);
    PTrace::SetOptions(PTrace::FileAndLine | PTrace::ContextIdentifier | PTrace::SystemLogStream);
#else
    PTrace::ClearOptions(PTrace::Timestamp | PTrace::FileAndLine);
    PTrace::SetOptions(PTrace::ContextIdentifier | PTrace::SystemLogStream);
#endif
    PTrace::SetLevel(GetLogLevel());
#endif
  }

  m_debugMode = false;

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
             "P-plugin-dir:       set plugin directory, overrides environment variable\n"
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
             "T-timeout:          timeout for terminate/kill (default 30 seconds)\n"
             "U-trace-up.         increase the trace log level\n"
             "D-trace-down.       reduce the trace log level\n"
             , false);

  // if only displaying version information, do it and finish
  if (args.HasOption('v')) {
    cout << "Product Name: " << GetName() << "\n"
            "Manufacturer: " << GetManufacturer() << "\n"
            "Version     : " << GetVersion(true) << "\n"
            "System      : " << PProcess::GetOSClass() << ' '
                             << GetOSName() << ' '
                             << GetOSVersion() << '('
                             << GetOSHardware() << ")\n"
            "PTLib       : " << PProcess::GetLibVersion() << endl;
    return 0;
  }

    // Validate the various command line options
  const char RunOptionSet[] = "dx";
  const char CtrlOptionSet[] = "ktsUD";
  const char LogOptionSet[] = "clr";
  int runOptionsCount = CountOptionSet(args, RunOptionSet);
  int ctrlOptionsCount = CountOptionSet(args, CtrlOptionSet);
  int logOptionsCount = CountOptionSet(args, LogOptionSet);
  if (  args.HasOption('h') ||
        !(
            (runOptionsCount == 0 && ctrlOptionsCount == 1) ||
            (runOptionsCount == 1 && ctrlOptionsCount == 0)
         ) ||
         logOptionsCount > 1)
  {
    if (runOptionsCount > 1)
      cerr << "error: must specify exactly one of " << ExpandOptionSet(RunOptionSet) << "\n\n";
    else if (runOptionsCount > 0 && ctrlOptionsCount > 0)
      cerr << "error: cannot specify " << ExpandOptionSet(RunOptionSet) << " with " << ExpandOptionSet(CtrlOptionSet) << "\n\n";
    else if (ctrlOptionsCount > 1)
      cerr << "error: must specify at most one of " << ExpandOptionSet(CtrlOptionSet) << "\n\n";
    else if (logOptionsCount > 1)
      cerr << "error: must specify at most one of " << ExpandOptionSet(LogOptionSet) << "\n\n";

    cerr << "usage: " << progName << " [ options ]\n";
    args.Usage(cerr);
    return 2;
  }

  PString pidfilename;
  if (args.HasOption('p'))
    pidfilename = args.GetOptionString('p');
  else
    pidfilename =  _PATH_VARRUN;

  if (!pidfilename.IsEmpty() && PDirectory::Exists(pidfilename))
    pidfilename = PDirectory(pidfilename) + progName + ".pid";

  if (ctrlOptionsCount > 0) {
    pid_t pid;

    {
      ifstream pidfile((const char *)pidfilename);
      if (!pidfile.is_open()) {
        if (args.HasOption('s'))
          cout << "Process has not been started." << endl;
        else
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

    switch (KillProcess(pid, args.GetOptionString('T', "30").AsUnsigned(), SIGTERM)) {
      case -1 :
        return 1;

      case 1 :
        if (!args.HasOption('k'))
          return 2;

        switch (KillProcess(pid, 5, SIGKILL)) {
          case -1 :
            return 1;

          case 1 :
            return 3;
        }
    }

    PFile::Remove(pidfilename);
    return 0;
  }


  // Set UID/GID before we open log file so has right ownership.

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

  // set flag for console messages
  if (args.HasOption('c')) {
#if PTRACING
    PSetErrorStream(PTrace::GetStream());
#endif
    PSystemLog::SetTarget(new PSystemLogToStderr());
    m_debugMode = true;
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
  PSYSTEMLOG(StdError, "Starting service process \"" << GetName() << "\" v" << GetVersion(true));

  if (args.HasOption('i'))
    SetConfigurationPath(args.GetOptionString('i'));

  if (args.HasOption('H')) {
    int uid = geteuid();
    PAssertOS(seteuid(getuid()) == 0); // Switch back to starting uid for next call
    unsigned maxHandles = args.GetOptionString('H').AsUnsigned();
    if (SetMaxHandles(maxHandles))
      PSYSTEMLOG(StdError, "Maximum handles set to " << maxHandles);
    else
      PSYSTEMLOG(StdError, "Could not set maximum handles to " << maxHandles);
    PAssertOS(seteuid(uid) == 0);
  }

#ifdef P_LINUX
  // set the core file size
  if (args.HasOption('C')) {
    struct rlimit rlim;
    if (getrlimit(RLIMIT_CORE, &rlim) != 0) 
      PSYSTEMLOG(StdError, "Could not get current core file size : error = " << errno);
    else {
      int uid = geteuid();
      PAssertOS(seteuid(getuid()) == 0); // Switch back to starting uid for next call
      int v = args.GetOptionString('C').AsInteger();
      rlim.rlim_cur = v;
      if (setrlimit(RLIMIT_CORE, &rlim) != 0) 
        PSYSTEMLOG(StdError, "Could not set current core file size to " << v << " : error = " << errno);
      else {
        getrlimit(RLIMIT_CORE, &rlim);
        PSYSTEMLOG(StdError, "Core file size set to " << rlim.rlim_cur << "/" << rlim.rlim_max);
      }
      PAssertOS(seteuid(uid) == 0);
    }
  }
#endif

  if (args.HasOption('P'))
    PPluginManager::GetPluginManager().SetDirectories(args.GetOptionString('P').Lines());

  bool daemon = args.HasOption('d');

  // Remove the service arguments
  PStringArray params = args.GetParameters();
  args.SetArgs("");
  args.SetArgs(params);

 // We are a service, don't want to get blocked on input from stdin during asserts
  if (!m_debugMode)
    ::close(STDIN_FILENO);

  if (!daemon) {
    Startup();
    return -1;
  }

  PSetErrorStream(new PSystemLog(PSystemLog::StdError));

#if !defined(BE_THREADS) && !defined(P_RTEMS)

  // Run as a daemon, ie fork
  if (!pidfilename.IsEmpty()) {
    ifstream pidfile((const char *)pidfilename);
    if (pidfile.is_open()) {
      pid_t pid;
      pidfile >> pid;
      if (pid != 0 && kill(pid, 0) == 0) {
        cout << "Already have daemon running with pid " << pid << endl;
        return 2;
      }
    }
  }

  // Need to put signals back before fork
  RemoveRunTimeSignalHandlers();

  pid_t pid = fork();
  switch (pid) {
    case 0 : // The forked process
      break;

    case -1 : // Failed
      cout << "Fork failed creating daemon process." << endl;
      return 1;

    default : // Parent process
      cout << "Daemon started with pid " << pid << endl;
      if (!pidfilename.IsEmpty()) {
        // Write out the child pid to magic file in /var/run (at least for linux)
        ofstream pidfile((const char *)pidfilename);
        if (pidfile.is_open())
          pidfile << pid;
        else
          cout << "Could not write pid to file \"" << pidfilename << "\""
                  " - " << strerror(errno) << endl;
      }
      return 0;
  }

  PSYSTEMLOG(StdError, "Forked to PID " << getpid());

  // Set ourselves as out own process group so we don't get signals
  // from our parent's terminal (hopefully!)
  PSETPGRP();

  // As new process, need to re-initialise some things
  PlatformConstruct();
  Startup();

  pidFileToRemove = pidfilename;

#endif // !BE_THREADS && !P_RTEMS
#endif // !P_VXWORKS
  return -1;
}

int PServiceProcess::InternalMain(void *)
{
  SetTerminationValue(InitialiseService());
  if (GetTerminationValue() < 0) {
    // Make sure housekeeping thread is going so signals are handled.
    SignalTimerChange();

    SetTerminationValue(1);
    bool started;
#if P_EXCEPTIONS
    try {
      started = OnStart();
    }
    catch (const std::exception & e) {
      PAssertAlways(PSTRSTRM("Exception (" << typeid(e).name() << " \"" << e.what() << "\") caught in service process start, terminating"));
      std::terminate();
    }
    catch (...) {
      PAssertAlways(PSTRSTRM("Exception caught in service process start, terminating"));
      std::terminate();
    }
#else
    started = OnStart();
#endif

    if (started) {
      SetTerminationValue(0);
      PProcess::InternalMain(NULL);
    }
  }

  return GetTerminationValue();
}


void PServiceProcess::Main()
{
  m_exitMain.Wait();
}


void PServiceProcess::OnStop()
{
  m_exitMain.Signal();
}


PBoolean PServiceProcess::OnPause()
{
  return true;
}


void PServiceProcess::OnContinue()
{
}


void PServiceProcess::OnControl()
{
}


bool PServiceProcess::IsServiceProcess() const
{
  return true;
}


void PServiceProcess::AddRunTimeSignalHandlers(const int * signals)
{
  static int const ExtraSignals[] = { SIGILL, SIGSEGV, SIGFPE, SIGBUS, 0 };
  PProcess::AddRunTimeSignalHandlers(ExtraSignals);
  PProcess::AddRunTimeSignalHandlers(signals);
}


static atomic<bool> InSignalHandler(false);

void PServiceProcess::AsynchronousRunTimeSignal(int signal, PProcessIdentifier source)
{
  const char * sigmsg;

  // Override the default behavious for these signals as that just
  // summarily exits the program. Allow PXOnSignal() to do orderly exit.

  switch (signal) {
    case SIGHUP :
      InternalPostRunTimeSignal(signal, source);
      return; // Don't call base class function, as it terminates app

    case SIGILL :
      sigmsg = "illegal instruction";
      break;

    case SIGSEGV :
      sigmsg = "segmentation fault";
      break;

    case SIGFPE :
      sigmsg = "floating point exception";
      break;

#ifndef __BEOS__ // In BeOS, SIGBUS is the same value as SIGSEGV
    case SIGBUS :
      sigmsg = "bus error";
      break;
#endif

    default :
      PProcess::AsynchronousRunTimeSignal(signal, source);
      return;
  }

  RemoveRunTimeSignalHandlers();

  if (!InSignalHandler.exchange(true)) {
    PThreadIdentifier tid = GetCurrentThreadId();
    PUniqueThreadIdentifier uid = PThread::GetCurrentUniqueIdentifier();

    const char * couldNotWrite = ", stderr write failed";
    {
      /* We get basic crash information using purely static memory (we hope) as,
         if the heap is trashed, all the PTRACE/PSystemLog stuff is very likely
         to also crash, and we never see this info. */
      char buffer[1000];
      int length = snprintf(buffer, sizeof(buffer),
                            "\nCaught %s (%s), in thread " P_THREAD_ID_FMT " (" P_UNIQUE_THREAD_ID_FMT ")\n",
                            sigmsg, GetRunTimeSignalName(signal), tid, uid);
      if (length > 0 && write(STDERR_FILENO, buffer, length) >= length)
        couldNotWrite = "";
    }

    PSystemLog log(PSystemLog::Fatal);
    log << "Caught " << sigmsg << " (" << GetRunTimeSignalName(signal) << "),"
           " thread-id=" << PThread::GetIdentifiersAsString(tid, uid) << ","
           " name=\"" << PThread::GetThreadName(tid) << '"' << couldNotWrite;

#if PTRACING
    log << ", stack:";
    PTrace::WalkStack(log);
#endif
  }

  abort(); // Dump core
  _exit(signal+100); // Fail safe if raise() didn't dump core and exit
}


void PServiceProcess::HandleRunTimeSignal(int signal)
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

  switch (signal) {
    case SIGHUP :
      PTRACE(2, "PTLib", "Received SIGHUP, executing PServiceProcess::OnControl()");
      OnControl();
      return;

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

  PProcess::HandleRunTimeSignal(signal);
}

