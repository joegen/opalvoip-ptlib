#include <ptlib.h>

#pragma implementation "svcproc.h"
#include <svcproc.h>

#include <syslog.h>
#include <stdarg.h>
#include <fstream.h>
#include <pthread.h>

#include "uerror.h"

#define	MAX_LOG_LINE_LEN	1024

static int PwlibLogToUnixLog[PSystemLog::NumLogLevels] = {
  LOG_CRIT,    // LogFatal,   
  LOG_ERR,     // LogError,   
  LOG_WARNING, // LogWarning, 
  LOG_INFO,    // LogInfo,    
};

#ifdef P_PTHREADS

static pthread_mutex_t logMutex = PTHREAD_MUTEX_INITIALIZER;

#endif

void PSystemLog::Output(Level level, const char * cmsg)
{
  if (PServiceProcess::Current().consoleMessages) {
#ifdef P_PTHREADS
    pthread_mutex_lock(&logMutex);
#endif
    PError << cmsg << endl;
#ifdef P_PTHREADS
    pthread_mutex_unlock(&logMutex);
#endif
  } else
    syslog(PwlibLogToUnixLog[level], "%s", cmsg);
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
void killpidfile()
{
  PString pidfilename = _PATH_VARRUN + PProcess::Current().GetFile().GetFileName() + ".pid";
  PFile::Remove(pidfilename);
}
#endif

int PServiceProcess::_main(int parmArgc,
                      char ** parmArgv,
                      char ** parmEnvp)
{
  // save the environment
  envp = parmEnvp;
  argc = parmArgc;
  argv = parmArgv;

  // setup the common process initialisation
  PXSetupProcess();

  // perform PWLib initialisation
  PreInitialise(argc, argv);

  // parse arguments so we can grab what we want
  PArgList args = GetArguments();

  args.Parse("vdchxp");

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

  BOOL helpAndExit = FALSE;

  // if displaying help, then do it
  if (args.HasOption('h')) 
    helpAndExit = TRUE;

  // if no arguments, then print help message
  if (!helpAndExit && !args.HasOption('d') && !args.HasOption('x')) {
    PError << "error: must specify one of -v, -d, -h or -x" << endl;
    helpAndExit = TRUE;
  }

  if (helpAndExit) {
    PError << "usage: [-c] -v|-d|-h|-x" << endl
           << "        -v    display version information and exit" << endl
           << "        -d    run as a daemon" << endl
#ifdef _PATH_VARRUN
           << "        -p    do not write pid file" << endl
#endif
           << "        -c    output messages to stdout rather than syslog" << endl
           << "        -h    output this help message and exit" << endl
           << "        -x    execute as a normal program" << endl;
    return 0;
  }

  // set flag for console messages
  consoleMessages = args.HasOption('c');

  // open the system logger for this program
  if (consoleMessages)
    PError << "All output for " << GetName() << " is to the console." << endl;
  else
    openlog((const char *)GetName(), LOG_PID, LOG_DAEMON);

  // Run as a daemon, ie fork
  if (args.HasOption('d')) {
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
          PString pidfilename = _PATH_VARRUN + GetFile().GetFileName() + ".pid";
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

  // close the system log
  if (!consoleMessages)
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


void PServiceProcess::PXOnSignal(int sig)
{
  if (sig == SIGINT)
    return;
  PProcess::PXOnSignal(sig);
}

void PServiceProcess::PXOnAsyncSignal(int sig)
{
  if (sig != SIGINT) 
    return;

  OnStop();
  exit(1);
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

