#pragma implementation "svcproc.h"

#include <ptlib.h>
#include <svcproc.h>

#include <syslog.h>
#include <stdarg.h>

#include "uerror.h"

#define	MAX_LOG_LINE_LEN	1024

static int PwlibLogToUnixLog[PServiceProcess::NumLogLevels] = {
  LOG_CRIT,    // LogFatal,   
  LOG_ERR,     // LogError,   
  LOG_WARNING, // LogWarning, 
  LOG_INFO,    // LogInfo,    
};

PServiceProcess::PServiceProcess(const char * manuf,
                                 const char * name,
                                         WORD majorVersion,
                                         WORD minorVersion,
                                   CodeStatus status,
                                         WORD buildNumber)
  : PProcess(manuf, name, majorVersion, minorVersion, status, buildNumber)
{
}

PServiceProcess * PServiceProcess::Current()
{
  return (PServiceProcess *)PProcess::Current();
}

void PServiceProcess::SystemLog(SystemLogLevel level,
                                  const char * cmsg,
				  ...)
{
  va_list list;
  va_start(list, cmsg);
  char buffer[MAX_LOG_LINE_LEN];
  
  vsprintf(buffer, cmsg, list);
  if (consoleMessages)
    PError << buffer << endl;
  else
    syslog(PwlibLogToUnixLog[level], buffer);
}

void PServiceProcess::SystemLog(SystemLogLevel level,
                                const PString & msg)
{
  SystemLog(level, "%s", (const char *)msg);
}


void PServiceProcess::_PXShowSystemWarning(PINDEX code, const PString & str)
{
  PStringStream msg;
  msg << "PWLib/Unix error #"
      << code
      << "-"
      << str
      << endl;

  SystemLog(LogWarning, msg);
}

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
  args.Parse("vdchx");

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
           << "        -d    run as a daemon (NOT IMPLEMENTED)" << endl
           << "        -c    output messages to stdout rather than syslog" << endl
           << "        -h    output this help message and exit" << endl
           << "        -x    execute as a normal program" << endl;
    return 0;
  }

  // set flag for console messages
  consoleMessages = args.HasOption('c');

  // open the system logger for this program
  if (!consoleMessages)
    openlog((const char *)GetName(), LOG_PID, LOG_DAEMON);

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


void PServiceProcess::PXOnSigInt()
{
  OnStop();
  exit(1);
}

