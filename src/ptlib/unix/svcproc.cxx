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
#include <svcproc.h>

#include <syslog.h>
#include <stdarg.h>
#include <fstream.h>
#include <pwd.h>
#include <grp.h>

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

#ifdef P_PTHREADS

static pthread_mutex_t logMutex = {{ PTHREAD_MUTEX_INITIALIZER }};

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

    static const char * const levelName[NumLogLevels+1] = {
      "Message",
      "Fatal error",
      "Error",
      "Warning",
      "Info",
      "Debug",
      "Debug2",
      "Debug3"
    };

    PTime now;
    *out << now.AsString("yyyy/MM/dd hh:mm:ss ")
         << (void *)PThread::Current()
         << ' '
         << levelName[level+1]
         << '\t'
         << cmsg << endl;

    if (out != &PError)
      delete out;

#ifdef P_PTHREADS
    pthread_mutex_unlock(&logMutex);
#endif
  }
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
  // parse arguments so we can grab what we want
  PArgList & args = GetArguments();

  args.Parse("vdchxpktl:u:g:");

  const char * statusToStr[NumCodeStatuses] = { "Alpha", "Beta", "Release" };

  // if only displaying version information, do it and finish
  if (args.HasOption('v')) {
    PError << "Product Name : " << productName << endl
           << "Manufacturer : " << manufacturer << endl
           << "Major version: " << majorVersion << endl
           << "Minor version: " << minorVersion << endl
           << "Code status  : " << statusToStr[status] << endl
           << "Build number : " << buildNumber << endl;
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
           << "        -h       output this help message and exit" << endl
           << "        -v       display version information and exit" << endl
           << "        -d       run as a daemon" << endl
           << "        -u uid   set user id to run as" << endl
           << "        -g gid   set group id to run as" << endl
#ifdef _PATH_VARRUN
           << "        -p       do not write pid file" << endl
           << "        -t       terminate process in pid file" << endl
           << "        -k       kill process in pid file" << endl
#endif
           << "        -c       output messages to stdout rather than syslog" << endl
           << "        -l file  output messages to file rather than syslog" << endl
           << "        -x       execute as a normal program" << endl;
    return 0;
  }

  // open the system logger for this program
  if (systemLogFile.IsEmpty())
    openlog((const char *)GetName(), LOG_PID, LOG_DAEMON);
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
  if (args.HasOption('d')) {
#ifdef _PATH_VARRUN
    pid_t old_pid = get_daemon_pid(FALSE);
    if (old_pid != 0 && kill(old_pid, 0) == 0) {
      PError << "Already have daemon running with pid " << old_pid << endl;
      return 3;
    }
#endif
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


static BOOL stoppingService = FALSE;

void PServiceProcess::PXOnAsyncSignal(int sig)
{
  if (stoppingService || (sig != SIGINT && sig != SIGTERM))
    PProcess::PXOnAsyncSignal(sig);
}


void PServiceProcess::PXOnSignal(int sig)
{
  switch (sig) {
    case SIGINT :
    case SIGTERM :
      stoppingService = TRUE;
      OnStop();
      exit(1);

    case SIGUSR1 :
      OnPause();
      break;

    case SIGUSR2 :
      OnContinue();
      break;
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

  string.SetSize(10);
  char * base = string.GetPointer();
  *base = '\0';
  setp(base, base + string.GetSize() - 1);
  return 0;
}

