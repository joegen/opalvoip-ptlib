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

    ostream * out;
    if (systemLogFile == "-")
      out = &PError;
    else
      out = new ofstream(systemLogFile, ios::app);

    PTime now;
    *out << now.AsString("yyyy/MM/dd hh:mm:ss ");
    PString threadName = PThread::Current()->GetThreadName();
    if (!threadName)
      *out << setw(15) << threadName.Left(15);
    else
      *out << (void *)PThread::Current();
    *out << '\t'
         << PLevelName[level+1]
         << '\t'
         << cmsg << endl;

    if (out != &PError)
      delete out;

#ifdef P_PTHREADS
    pthread_mutex_unlock(&logMutex);
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

#ifdef _PATH_VARRUN
static PString get_pid_filename()
{
  return _PATH_VARRUN + PProcess::Current().GetFile().GetFileName() + ".pid";
}

static void killpidfile()
{
  PFile::Remove(get_pid_filename());
}

static pid_t get_daemon_pid(BOOL showError)
{
  PString pidfilename = get_pid_filename();
  ifstream pidfile(pidfilename);
  if (!pidfile.is_open()) {
    if (showError)
      PError << "Could not open pid file: " << pidfilename << endl;
    return 0;
  }

  pid_t pid;
  pidfile >> pid;
  if (pid == 0 && showError)
    PError << "Illegal format pid file: " << pidfilename << endl;

  return pid;
}
#endif

int PServiceProcess::_main(void *)
{
#if PMEMORY_CHECK
  PMemoryHeap::SetIgnoreAllocations(TRUE);
#endif
//  PSetErrorStream(new PSystemLog(PSystemLog::StdError));
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
             "p-pid-file."
             "k-kill."
             "t-terminate."
             "l-log-file:"
             "u-uid:"
             "g-gid:");

  // if only displaying version information, do it and finish
  if (args.HasOption('v')) {
    PError << "Product Name: " << productName << endl
           << "Manufacturer: " << manufacturer << endl
           << "Version     : " << GetVersion(TRUE) << endl
           << "System      : " << GetOSName() << '-'
                               << GetOSHardware() << ' '
                               << GetOSVersion() << endl;
    return 0;
  }

#ifdef _PATH_VARRUN
  if (args.HasOption('k') || args.HasOption('t')) {
    pid_t pid = get_daemon_pid(TRUE);
    if (pid != 0) {
      if (kill(pid, args.HasOption('t') ? SIGTERM : SIGKILL) == 0)
        return 0;
      PError << "Could not kill process " << pid << " - " << strerror(errno) << endl;
    }
    return 2;
  }
#endif

  BOOL helpAndExit = FALSE;

  // if displaying help, then do it
  if (args.HasOption('h')) 
    helpAndExit = TRUE;
  else if (!args.HasOption('d') && !args.HasOption('x')) {
    PError << "error: must specify one of -v, -h, "
#ifdef _PATH_VARRUN
              "-t, -k, "
#endif
              "-d or -x" << endl;
    helpAndExit = TRUE;
  }

  // set flag for console messages
  if (args.HasOption('c'))
    systemLogFile = '-';

  if (args.HasOption('l')) {
    systemLogFile = args.GetOptionString('l');
    if (systemLogFile.IsEmpty()) {
      PError << "error: must specify file name for -l" << endl;
      helpAndExit = TRUE;
    }
  }

  if (helpAndExit) {
    PError << "usage: [-c] -v|-d|-h|-x" << endl
           << "  -h --help           output this help message and exit" << endl
           << "  -v --version        display version information and exit" << endl
#ifndef BE_THREADS
           << "  -d --daemon         run as a daemon" << endl
#endif
           << "  -u --uid uid        set user id to run as" << endl
           << "  -g --gid gid        set group id to run as" << endl
#ifdef _PATH_VARRUN
           << "  -p --pid-file       do not write pid file" << endl
           << "  -t --terminate      terminate process in pid file" << endl
           << "  -k --kill           kill process in pid file" << endl
#endif
           << "  -c --console        output messages to stdout rather than syslog" << endl
           << "  -l --log-file file  output messages to file rather than syslog" << endl
           << "  -x --execute        execute as a normal program" << endl;
    return 0;
  }

  // open the system logger for this program
  if (systemLogFile.IsEmpty())
    openlog((char *)(const char *)GetName(), LOG_PID, LOG_DAEMON);
  else if (systemLogFile == "-")
    PError << "All output for " << GetName() << " is to console." << endl;

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
        PError << "Could not find user \"" << uidstr << '"' << endl;
        return 1;
      }
      uid = pw->pw_uid;
    }
    if (setuid(uid) != 0) {
      PError << "Could not set UID to \"" << uidstr << '"' << endl;
      return 1;
    }
  }

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
        PError << "Could not find group \"" << gidstr << '"' << endl;
        return 1;
      }
      gid = gr->gr_gid;
    }
    if (setgid(gid) != 0) {
      PError << "Could not set GID to \"" << gidstr << '"' << endl;
      return 1;
    }
  }

  // Run as a daemon, ie fork
#ifndef BE_THREADS
  if (args.HasOption('d')) {
#ifdef _PATH_VARRUN
    pid_t old_pid = get_daemon_pid(FALSE);
    if (old_pid != 0 && kill(old_pid, 0) == 0) {
      PError << "Already have daemon running with pid " << old_pid << endl;
      return 3;
    }
#endif

    // Need to get rid of the config write thread before fork, as on
    // pthreads platforms the forked process does not have the thread
    delete configFiles;
    CreateConfigFilesDictionary();

    pid_t pid = fork();
    switch (pid) {
      case -1 : // Failed
        PError << "Fork failed creating daemon process." << endl;
        return -1;

      case 0 : // The forked process
#ifdef _PATH_VARRUN
        atexit(killpidfile);
#endif
        // set the SIGINT and SIGQUIT to ignore so the child process doesn't
        // inherit them from the parent
        signal(SIGINT,  SIG_IGN);
        signal(SIGQUIT, SIG_IGN);

        // and set ourselves as out own process group so we don't get signals
        // from our parent's terminal (hopefully!)
        PSETPGRP();
        break;

      default :
#ifdef _PATH_VARRUN
        if (!args.HasOption('p')) {
          // Write out the child pid to magic file in /var/run (at least for linux)
          PString pidfilename = get_pid_filename();
          ofstream pidfile(pidfilename);
          if (pidfile.is_open())
            pidfile << pid;
          else
            PError << "Could not write pid to file: " << pidfilename << endl;
        }
#endif
        return 0;
    }
  }
#endif

  // call the main function
  if (OnStart())
    Main();

  // Avoid strange errors caused by threads (and the process itself!) being destoyed 
  // before they have EVER been scheduled
  Yield();

  // close the system log
  if (systemLogFile.IsEmpty())
    closelog();

  // return the return value
  return GetTerminationValue();
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


void PServiceProcess::PXOnAsyncSignal(int sig)
{
  static BOOL stoppingService = FALSE;
  if (!stoppingService)
    switch (sig) {
      case SIGINT :
      case SIGTERM :
        stoppingService = TRUE;
      case SIGHUP :
        return;
    }

  PProcess::PXOnAsyncSignal(sig);
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

