#pragma implementation "svcproc.h"

#include <ptlib.h>
#include <svcproc.h>

#include <syslog.h>
#include <stdarg.h>

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
  syslog(PwlibLogToUnixLog[level], buffer);
}

void PServiceProcess::SystemLog(SystemLogLevel level,
                                const PString & msg)
{
  SystemLog(level, "%s", (const char *)msg);
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

  // open the system logger for this program
  openlog((const char *)GetName(), LOG_PID, LOG_DAEMON);

  // call the main function
  if (OnStart())
    Main();

  // close the system log
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
