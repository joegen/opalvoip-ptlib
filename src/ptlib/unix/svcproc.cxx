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
 * $Log: svcproc.cxx,v $
 * Revision 1.61  2001/09/20 05:35:47  robertj
 * Fixed crash (race condition) if shutdown service via signal and exit main.
 *
 * Revision 1.60  2001/08/11 15:38:43  rogerh
 * Add Mac OS Carbon changes from John Woods <jfw@jfwhome.funhouse.com>
 *
 * Revision 1.59  2001/08/07 03:05:54  robertj
 * Expanded thread name field width in system log.
 *
 * Revision 1.58  2001/07/09 04:26:08  yurik
 * Fixed lack of pthread_self function on BeOS
 *
 * Revision 1.57  2001/07/04 08:54:23  robertj
 * Added dump of thread in SEGV signal handler, this one seems to work.
 *
 * Revision 1.56  2001/07/03 04:41:25  yurik
 * Corrections to Jac's submission from 6/28
 *
 * Revision 1.55  2001/06/30 06:59:07  yurik
 * Jac Goudsmit from Be submit these changes 6/28. Implemented by Yuri Kiryanov
 *
 * Revision 1.54  2001/05/03 01:13:10  robertj
 * Closed stdin if in background, should never block in tty I/O if daemon!
 *
 * Revision 1.53  2001/04/20 05:41:35  craigs
 * Added ability to set core dump size from command line under Linux
 *
 * Revision 1.52  2001/04/20 05:08:42  robertj
 * Removed dump of thread in SEGV signal, it does not work.
 *
 * Revision 1.51  2001/04/17 03:13:21  robertj
 * Added dump of thread address in SEGV etc log output.
 *
 * Revision 1.50  2001/04/05 03:09:29  robertj
 * Changed so output to PError goes to system log. Useful for asserts.
 *
 * Revision 1.49  2001/03/29 03:25:03  robertj
 * Added dumping to log file of SEGV etc if running as daemon.
 *
 * Revision 1.48  2001/03/28 23:47:26  robertj
 * Added start log message and version numbers to start and stop log messages
 *
 * Revision 1.47  2001/03/28 05:36:14  robertj
 * Added milliseconds to system log time output.
 *
 * Revision 1.46  2001/03/24 00:49:02  robertj
 * Added status indication command for services
 *
 * Revision 1.45  2001/03/23 01:05:32  robertj
 * Added check that log file is writable after setuid but before fork.
 *
 * Revision 1.44  2001/03/22 22:48:25  robertj
 * Fixed errors in usage help text.
 *
 * Revision 1.43  2001/03/20 06:44:25  robertj
 * Lots of changes to fix the problems with terminating threads that are I/O
 *   blocked, especially when doing orderly shutdown of service via SIGTERM.
 *
 * Revision 1.42  2001/03/20 01:04:46  robertj
 * Fixed some difficulties with terminating a service process from signals or
 *   from simply dropping out of Main().
 *
 * Revision 1.41  2001/03/19 02:41:13  robertj
 * Extra trace output on exiting service.
 *
 * Revision 1.40  2001/03/19 00:20:55  robertj
 * Added test for if deamon actually stops
 *
 * Revision 1.39  2001/03/19 00:11:03  robertj
 * Added information message when killing service.
 *
 * Revision 1.38  2001/03/14 01:30:35  robertj
 * Do setgid before so setuid, ie when still root.
 *
 * Revision 1.37  2001/03/14 01:16:11  robertj
 * Fixed signals processing, now uses housekeeping thread to handle signals
 *   synchronously. This also fixes issues with stopping PServiceProcess.
 *
 * Revision 1.36  2001/03/13 03:47:18  robertj
 * Added ability to set pid file from command line.
 *
 * Revision 1.35  2001/03/09 06:31:22  robertj
 * Added ability to set default PConfig file or path to find it.
 *
 * Revision 1.34  2000/05/02 03:17:13  robertj
 * Added display of thread name in SystemLog, thanks Ashley Unitt.
 *
 * Revision 1.33  2000/04/03 18:36:50  robertj
 * Fix for BeOS support (stupid prototype in system header files).
 *
 * Revision 1.32  1999/09/14 13:02:53  robertj
 * Fixed PTRACE to PSYSTEMLOG conversion problem under Unix.
 *
 * Revision 1.31  1999/08/17 09:29:22  robertj
 * Added long name versions of parameters.
 *
 * Revision 1.30  1999/08/12 12:12:47  robertj
 * GCC 2.95 compatibility.
 *
 * Revision 1.29  1999/06/23 14:19:46  robertj
 * Fixed core dump problem with SIGINT/SIGTERM terminating process.
 *
 * Revision 1.28  1999/05/13 04:44:18  robertj
 * Added SIGHUP and SIGWINCH handlers to increase and decrease the log levels.
 *
 * Revision 1.27  1999/03/02 05:41:59  robertj
 * More BeOS changes
 *
 * Revision 1.26  1999/01/11 12:10:32  robertj
 * Improved operating system version display.
 *
 * Revision 1.26  1999/01/11 05:20:12  robertj
 * Added OS to the -v display text.
 *
 * Revision 1.25  1998/12/21 06:37:14  robertj
 * Fixed GNu warning on solaris x86
 *
 * Revision 1.24  1998/12/16 12:41:25  robertj
 * Fixed bug where .ini file is not written when service run as a daemon.
 *
 * Revision 1.23  1998/11/30 21:52:00  robertj
 * New directory structure.
 *
 * Revision 1.22  1998/11/06 03:44:55  robertj
 * Fixed bug in argument list parsing, not doing it to member variable.
 * Added check for daemon already running before starting a new daemon.
 *
 * Revision 1.21  1998/10/11 02:26:46  craigs
 * Added thread ID to output messages
 *
 * Revision 1.20  1998/09/24 04:12:20  robertj
 * Added open software license.
 *
 */

#include <ptlib.h>

#pragma implementation "svcproc.h"
#include <ptlib/svcproc.h>

#include <syslog.h>
#include <stdarg.h>
#include <fstream.h>
#include <pwd.h>
#include <grp.h>
#include <signal.h>

#include "uerror.h"

#ifdef P_LINUX
#include <sys/resource.h>
#endif

#define new PNEW


#define	MAX_LOG_LINE_LEN	1024

static int PwlibLogToUnixLog[PSystemLog::NumLogLevels] = {
  LOG_CRIT,    // LogFatal,   
  LOG_ERR,     // LogError,   
  LOG_WARNING, // LogWarning, 
  LOG_INFO,    // LogInfo,    
  LOG_DEBUG,   // LogDebug
  LOG_DEBUG,
  LOG_DEBUG
};

static const char * const PLevelName[PSystemLog::NumLogLevels+1] = {
  "Message",
  "Fatal error",
  "Error",
  "Warning",
  "Info",
  "Debug",
  "Debug2",
  "Debug3"
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
  PString systemLogFile = PServiceProcess::Current().systemLogFile;
  if (systemLogFile.IsEmpty())
    syslog(PwlibLogToUnixLog[level], "%s", cmsg);
  else {
#ifdef P_PTHREADS
    pthread_mutex_lock(&logMutex);
#endif
#ifdef P_MAC_MPTHREADS
    SetUpLogMutex();
    (void)MPWaitOnSemaphore(logMutex, kDurationForever);
#endif

    ostream * out;
    if (systemLogFile == "-")
      out = &cerr;
    else
      out = new ofstream(systemLogFile, ios::app);

    PTime now;
    *out << now.AsString("yyyy/MM/dd hh:mm:ss.uuu ");
    PString threadName = PThread::Current()->GetThreadName();
    if (threadName.GetLength() <= 23)
      *out << setw(23) << threadName;
    else
      *out << threadName.Left(10) << "..." << threadName.Right(10);
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
  isTerminating = FALSE;
}


PServiceProcess::~PServiceProcess()
{
  if (!pidFileToRemove)
    PFile::Remove(pidFileToRemove);

  // close the system log
  if (systemLogFile.IsEmpty())
    closelog();
}


PServiceProcess & PServiceProcess::Current()
{
  PProcess & process = PProcess::Current();
  PAssert(process.IsDescendant(PServiceProcess::Class()), "Not a service process!");
  return (PServiceProcess &)process;
}

void PServiceProcess::_PXShowSystemWarning(PINDEX code, const PString & str)
{
  PSYSTEMLOG(Warning, "PWLib/Unix error #"
                      << code
                      << "-"
                      << str
                      << endl);
}


int PServiceProcess::InitialiseService()
{
#if PMEMORY_CHECK
  PMemoryHeap::SetIgnoreAllocations(TRUE);
#endif
  PSetErrorStream(new PSystemLog(PSystemLog::StdError));
  PTrace::SetStream(new PSystemLog(PSystemLog::Debug3));
  PTrace::ClearOptions(PTrace::FileAndLine);
  PTrace::SetOptions(PTrace::SystemLogStream);
  PTrace::SetLevel(4);
#if PMEMORY_CHECK
  PMemoryHeap::SetIgnoreAllocations(FALSE);
#endif

  // parse arguments so we can grab what we want
  PArgList & args = GetArguments();

  args.Parse("v-version."
             "d-daemon."
             "c-console."
             "h-help."
             "x-execute."
             "p-pid-file:"
             "i-ini-file:"
             "k-kill."
             "t-terminate."
             "s-status."
             "l-log-file:"
             "u-uid:"
             "g-gid:"
             "C-core-size:");

  // if only displaying version information, do it and finish
  if (args.HasOption('v')) {
    cout << "Product Name: " << productName << endl
         << "Manufacturer: " << manufacturer << endl
         << "Version     : " << GetVersion(TRUE) << endl
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
    ifstream pidfile(pidfilename);
    if (!pidfile.is_open()) {
      cout << "Could not open pid file: \"" << pidfilename << "\""
              " - " << strerror(errno) << endl;
      return 1;
    }

    pid_t pid;
    pidfile >> pid;
    if (pid == 0) {
      cout << "Illegal format pid file \"" << pidfilename << '"' << endl;
      return 1;
    }

    if (args.HasOption('s')) {
      cout << "Process at " << pid << " is ";
      if (kill(pid, 0) != 0)
        cout << "not ";
      cout << "running." << endl;
      return 0;
    }

    int sig = args.HasOption('t') ? SIGTERM : SIGKILL;
    if (kill(pid, sig) == 0) {
      cout << "Sent SIG";
      if (sig == SIGTERM)
        cout << "TERM";
      else
        cout << "KILL";
      cout << " to daemon at pid " << pid << ' ' << flush;
      for (PINDEX retry = 1; retry <= 5; retry++) {
        Sleep(1000);
        if (kill(pid, 0) != 0) {
          cout << "\nDaemon stopped." << endl;
          return 0;
        }
        cout << '.' << flush;
      }
      cout << "\nDaemon has not stopped." << endl;
      return 2;
    }

    cout << "Could not stop process " << pid <<
            " - " << strerror(errno) << endl;
    return 1;
  }

  BOOL helpAndExit = FALSE;

  // if displaying help, then do it
  if (args.HasOption('h')) 
    helpAndExit = TRUE;
  else if (!args.HasOption('d') && !args.HasOption('x')) {
    cout << "error: must specify one of -v, -h, -t, -k, -d or -x" << endl;
    helpAndExit = TRUE;
  }

  // set flag for console messages
  if (args.HasOption('c'))
    systemLogFile = '-';

  if (args.HasOption('l')) {
    systemLogFile = args.GetOptionString('l');
    if (systemLogFile.IsEmpty()) {
      cout << "error: must specify file name for -l" << endl;
      helpAndExit = TRUE;
    }
    else if (PDirectory::Exists(systemLogFile))
      systemLogFile = PDirectory(systemLogFile) + PProcess::Current().GetFile().GetFileName() + ".log";
  }

  if (helpAndExit) {
    cout << "usage: [-c] -v|-d|-h|-x\n"
            "  -h --help           output this help message and exit\n"
            "  -v --version        display version information and exit\n"
#ifndef BE_THREADS
            "  -d --daemon         run as a daemon\n"
#endif
            "  -u --uid uid        set user id to run as\n"
            "  -g --gid gid        set group id to run as\n"
            "  -p --pid-file       name or directory for pid file\n"
            "  -t --terminate      orderly terminate process in pid file\n"
            "  -k --kill           preemptively kill process in pid file\n"
            "  -c --console        output messages to stdout rather than syslog\n"
            "  -l --log-file file  output messages to file or directory instead of syslog\n"
            "  -x --execute        execute as a normal program\n"
            "  -i --ini-file       set the ini file to use, may be explicit file or\n"
            "                      a ':' separated set of directories to search.\n"
#ifdef P_LINUX
            "  -C --core-size      set the maximum core file size\n"
#endif
         << endl;
    return 0;
  }

  if (args.HasOption('i'))
    SetConfigurationPath(args.GetOptionString('i'));

  // Set the gid we are running under
  if (args.HasOption('g')) {
    PString gidstr = args.GetOptionString('g');
    if (gidstr.IsEmpty())
      gidstr = "nobody";
    int gid;
    if (strspn(gidstr, "0123456789") == gidstr.GetLength())
      gid = gidstr.AsInteger();
    else {
      struct group * gr = getgrnam(gidstr);
      if (gr == NULL) {
        cout << "Could not find group \"" << gidstr << '"' << endl;
        return 1;
      }
      gid = gr->gr_gid;
    }
    if (setgid(gid) != 0) {
      cout << "Could not set GID to \"" << gidstr << "\" (" << gid << ")"
              " - " << strerror(errno) << endl;
      return 1;
    }
  }

  // Set the uid we are running under
  if (args.HasOption('u')) {
    PString uidstr = args.GetOptionString('u');
    if (uidstr.IsEmpty())
      uidstr = "nobody";
    int uid;
    if (strspn(uidstr, "0123456789") == uidstr.GetLength())
      uid = uidstr.AsInteger();
    else {
      struct passwd * pw = getpwnam(uidstr);
      if (pw == NULL) {
        cout << "Could not find user \"" << uidstr << '"' << endl;
        return 1;
      }
      uid = pw->pw_uid;
    }
    if (setuid(uid) != 0) {
      cout << "Could not set UID to \"" << uidstr << "\" (" << uid << ")"
              " - " << strerror(errno) << endl;
      return 1;
    }
  }

  // set the core file size
  if (args.HasOption('C')) {
#ifdef P_LINUX
    struct rlimit rlim;
    if (getrlimit(RLIMIT_CORE, &rlim) != 0) 
      cout << "Could not get current core file size : error = " << errno << endl;
    else {
      int v = args.GetOptionString('C').AsInteger();
      rlim.rlim_cur = v;
      if (setrlimit(RLIMIT_CORE, &rlim) != 0) 
        cout << "Could not set current core file size to " << v << " : error = " << errno << endl;
      else {
        getrlimit(RLIMIT_CORE, &rlim);
        cout << "Core file size set to " << rlim.rlim_cur << "/" << rlim.rlim_max << endl;
      }
    }
#endif
  }

  // open the system logger for this program
  if (systemLogFile.IsEmpty())
    openlog((char *)(const char *)GetName(), LOG_PID, LOG_DAEMON);
  else if (systemLogFile == "-")
    cout << "All output for " << GetName() << " is to console." << endl;
  else {
    ofstream logfile(systemLogFile, ios::app);
    if (!logfile.is_open()) {
      cout << "Could not open log file \"" << systemLogFile << "\""
              " - " << strerror(errno) << endl;
      return 1;
    }
  }

#ifndef BE_THREADS
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

#endif // BE_THREADS

  return -1;
}

int PServiceProcess::_main(void *)
{
  if ((terminationValue = InitialiseService()) < 0) {
    PSYSTEMLOG(Warning, "Starting service process \"" << GetName() << "\" v" << GetVersion(TRUE));

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


BOOL PServiceProcess::OnPause()
{
  return TRUE;
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

  isTerminating = TRUE;

  PSYSTEMLOG(Warning, "Stopping service process \"" << GetName() << "\" v" << GetVersion(TRUE));

  // Avoid strange errors caused by threads (and the process itself!) being destoyed 
  // before they have EVER been scheduled
  Yield();

  // Do the services stop code
  OnStop();

  // close the system log
  if (systemLogFile.IsEmpty())
    closelog();

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

  static BOOL inHandler = FALSE;
  if (inHandler)
    raise(SIGQUIT); // Dump core

  inHandler = TRUE;

#if P_MAC_MPTHREADS
  unsigned tid = (unsigned)MPCurrentTaskID();
#else
  unsigned tid = (unsigned) pthread_self();
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

  if (systemLogFile.IsEmpty()) {
    syslog(LOG_CRIT, 
#ifdef __BEOS__ // (Some?) BeOS versions of syslog.h have syslog() (wrongly) declared without const 
    (char *) 
#endif 
    msg); 
    closelog();
  }
  else {
    int fd = open(systemLogFile, O_WRONLY|O_APPEND);
    if (fd >= 0) {
      write(fd, msg, strlen(msg));
      close(fd);
    }
  }

  raise(SIGQUIT); // Dump core
}


void PServiceProcess::PXOnSignal(int sig)
{
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

    case SIGHUP :
      if (currentLogLevel < PSystemLog::NumLogLevels-1) {
        currentLogLevel = (PSystemLog::Level)(currentLogLevel+1);
        PSystemLog s(PSystemLog::StdError);
        s << "Log level increased to " << PLevelName[currentLogLevel+1];
      }
      break;

    case SIGWINCH :
      if (currentLogLevel > PSystemLog::Fatal) {
        currentLogLevel = (PSystemLog::Level)(currentLogLevel-1);
        PSystemLog s(PSystemLog::StdError);
        s << "Log level decreased to " << PLevelName[currentLogLevel+1];
      }
      break;
  }
}

