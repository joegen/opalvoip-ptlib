/*
 * $Id: svcproc.cxx,v 1.1 1996/05/15 21:11:51 robertj Exp $
 *
 * Portable Windows Library
 *
 * Operating System Classes Implementation
 *
 * Copyright 1993 Equivalence
 *
 * $Log: svcproc.cxx,v $
 * Revision 1.1  1996/05/15 21:11:51  robertj
 * Initial revision
 *
 */

#include <ptlib.h>
#include <svcproc.h>

#include <winuser.h>
#include <winnls.h>

#include <fstream.h>
#include <signal.h>
#include <fcntl.h>
#include <io.h>


///////////////////////////////////////////////////////////////////////////////
// PServiceProcess

PServiceProcess::PServiceProcess(const char * manuf, const char * name,
                           WORD major, WORD minor, CodeStatus stat, WORD build)
  : PProcess(manuf, name, major, minor, stat, build)
{
}


static void Control_C(int)
{
  PServiceProcess::Current()->OnStop();
  exit(1);
}


int PServiceProcess::_main(int argc, char ** argv, char **)
{
  PErrorStream = &cerr;
  PreInitialise(1, argv);

  debugMode = FALSE;
  isWin95 = GetOSName() == "95";

  if (argc > 1) {
    PAssertOS(AllocConsole());

    HANDLE h1 = GetStdHandle(STD_INPUT_HANDLE);
    int h2 = _open_osfhandle((long)h1, _O_RDONLY|_O_TEXT);
    static ifstream newcin(h2);
    cin = newcin;

    h1 = GetStdHandle(STD_OUTPUT_HANDLE);
    h2 = _open_osfhandle((long)h1, _O_APPEND|_O_TEXT);
    static ofstream newcout(h2);
    cout = newcout;

    h1 = GetStdHandle(STD_ERROR_HANDLE);
    h2 = _open_osfhandle((long)h1, _O_APPEND|_O_TEXT);
    static ofstream newcerr(h2);
    cerr = newcerr;

    switch (ProcessCommand(argv[1])) {
      case CommandProcessed :
        return 0;
      case ProcessCommandError :
        return 1;
      case DebugCommandMode :
        PError << "Service simulation started for \"" << GetName() << "\".\n"
                  "Press Ctrl-C to terminate.\n" << endl;
        debugMode = TRUE;
    }
  }

  currentLogLevel = debugMode ? LogInfo : LogError;

  if (debugMode || isWin95) {
    signal(SIGINT, Control_C);
    SetTerminationValue(1);
    if (OnStart()) {
      SetTerminationValue(0);
      Main();
    }
    OnStop();
    return GetTerminationValue();
  }

  // SERVICE_STATUS members that don't change
  status.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
  status.dwServiceSpecificExitCode = 0;

  static SERVICE_TABLE_ENTRY dispatchTable[] = {
    { "", PServiceProcess::StaticMainEntry },
    { NULL, NULL }
  };

  dispatchTable[0].lpServiceName = (char *)(const char *)GetName();

  if (StartServiceCtrlDispatcher(dispatchTable))
    return GetTerminationValue();

  SystemLog(LogFatal, "StartServiceCtrlDispatcher failed.");
  PError << "Could not start service.\n";
  ProcessCommand("");
  return 1;
}


void PServiceProcess::StaticMainEntry(DWORD argc, LPTSTR * argv)
{
  Current()->MainEntry(argc, argv);
}


void PServiceProcess::MainEntry(DWORD argc, LPTSTR * argv)
{
  // register our service control handler:
  statusHandle = RegisterServiceCtrlHandler(GetName(), StaticControlEntry);
  if (statusHandle == NULL)
    return;

  // report the status to Service Control Manager.
  if (!ReportStatus(SERVICE_START_PENDING, NO_ERROR, 1, 3000))
    return;

  // create the event object. The control handler function signals
  // this event when it receives the "stop" control code.
  terminationEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
  if (terminationEvent == NULL)
    return;

  GetArguments().SetArgs(argc, argv);

  // start the thread that performs the work of the service.
  threadHandle = CreateThread(NULL,4096,StaticThreadEntry,NULL,0,&threadId);
  if (threadHandle != NULL)
    WaitForSingleObject(terminationEvent, INFINITE);  // Wait here for the end

  TerminateThread(threadHandle, 1);
  CloseHandle(terminationEvent);
  ReportStatus(SERVICE_STOPPED, 0);
}


DWORD EXPORTED PServiceProcess::StaticThreadEntry(LPVOID)
{
  Current()->ThreadEntry();
  return 0;
}


void PServiceProcess::ThreadEntry()
{
  if (OnStart()) {
    ReportStatus(SERVICE_RUNNING);
    Main();
    ReportStatus(SERVICE_STOP_PENDING, NO_ERROR, 1, 3000);
  }
  SetEvent(terminationEvent);
}


void PServiceProcess::StaticControlEntry(DWORD code)
{
  Current()->ControlEntry(code);
}


void PServiceProcess::ControlEntry(DWORD code)
{
  switch (code) {
    case SERVICE_CONTROL_PAUSE : // Pause the service if it is running.
      if (status.dwCurrentState != SERVICE_RUNNING)
        ReportStatus(status.dwCurrentState);
      else {
        if (OnPause())
          ReportStatus(SERVICE_PAUSED);
      }
      break;

    case SERVICE_CONTROL_CONTINUE : // Resume the paused service.
      if (status.dwCurrentState == SERVICE_PAUSED)
        OnContinue();
      ReportStatus(status.dwCurrentState);
      break;

    case SERVICE_CONTROL_STOP : // Stop the service.
      // Report the status, specifying the checkpoint and waithint, before
      // setting the termination event.
      ReportStatus(SERVICE_STOP_PENDING, NO_ERROR, 1, 3000);
      OnStop();
      SetEvent(terminationEvent);
      break;

    case SERVICE_CONTROL_INTERROGATE : // Update the service status.
    default :
      ReportStatus(status.dwCurrentState);
  }
}


BOOL PServiceProcess::ReportStatus(DWORD dwCurrentState,
                                   DWORD dwWin32ExitCode,
                                   DWORD dwCheckPoint,
                                   DWORD dwWaitHint)
{
  // Disable control requests until the service is started.
  if (dwCurrentState == SERVICE_START_PENDING)
    status.dwControlsAccepted = 0;
  else
    status.dwControlsAccepted =
                           SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_PAUSE_CONTINUE;

  // These SERVICE_STATUS members are set from parameters.
  status.dwCurrentState = dwCurrentState;
  status.dwWin32ExitCode = dwWin32ExitCode;
  status.dwCheckPoint = dwCheckPoint;
  status.dwWaitHint = dwWaitHint;

  if (debugMode)
    return TRUE;

  // Report the status of the service to the service control manager.
  if (SetServiceStatus(statusHandle, &status))
    return TRUE;

  // If an error occurs, stop the service.
  SystemLog(LogError, "SetServiceStatus failed");
  return FALSE;
}


void PServiceProcess::SystemLog(SystemLogLevel level,const PString & msg)
{
  SystemLog(level, "%s", (const char *)msg);
}


void PServiceProcess::SystemLog(SystemLogLevel level, const char * fmt, ...)
{
  if (level > currentLogLevel)
    return;

  DWORD err = GetLastError();

  char msg[1000];
  va_list args;
  va_start(args, fmt);
  vsprintf(msg, fmt, args);

  if (debugMode || isWin95) {
    static HANDLE mutex = CreateSemaphore(NULL, 1, 1, NULL);
    WaitForSingleObject(mutex, INFINITE);
    static const char * levelName[NumLogLevels] = {
      "Fatal error",
      "Error",
      "Warning",
      "Information"
    };
    ostream * out = PErrorStream;
    if (isWin95) {
      PString dir;
      GetWindowsDirectory(dir.GetPointer(256), 255);
      out = new ofstream(dir + "\\FireDoorLog.TXT", ios::app);
    }
    PTime now;
    *out << now.AsString("yy/MM/dd hh:mm:ss ") << levelName[level];
    if (msg[0] != '\0')
      *out << ": " << msg;
    if (level != LogInfo && err != 0)
      *out << " - error = " << err;
    *out << endl;
    if (isWin95)
      delete out;
    ReleaseSemaphore(mutex, 1, NULL);
  }
  else {
    // Use event logging to log the error.
    HANDLE hEventSource = RegisterEventSource(NULL, GetName());
    if (hEventSource == NULL)
      return;

    if (level != LogInfo && err != 0)
      sprintf(&msg[strlen(msg)], "\nError code = %d", err);

    LPCTSTR strings[2];
    strings[0] = msg;
    strings[1] = level != LogFatal ? "" : " Program aborted.";

    static const WORD levelType[NumLogLevels] = {
      EVENTLOG_ERROR_TYPE,
      EVENTLOG_ERROR_TYPE,
      EVENTLOG_WARNING_TYPE,
      EVENTLOG_INFORMATION_TYPE
    };
    ReportEvent(hEventSource, // handle of event source
                levelType[level],     // event type
                0,                    // event category
                1,                    // event ID
                NULL,                 // current user's SID
                PARRAYSIZE(strings),  // number of strings
                0,                    // no bytes of raw data
                strings,              // array of error strings
                NULL);                // no raw data
    DeregisterEventSource(hEventSource);
  }
}


void PServiceProcess::OnStop()
{
}


BOOL PServiceProcess::OnPause()
{
  SuspendThread(threadHandle);
  return TRUE;
}


void PServiceProcess::OnContinue()
{
  ResumeThread(threadHandle);
}


class P_SC_HANDLE
{
  public:
    P_SC_HANDLE()
        { h = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS); }
    P_SC_HANDLE(const P_SC_HANDLE & manager, PServiceProcess * svc)
        { h = OpenService(manager, svc->GetName(), SERVICE_ALL_ACCESS); }
    ~P_SC_HANDLE()
        { if (h != NULL) CloseServiceHandle(h); }
    operator SC_HANDLE() const
        { return h; }
    BOOL IsNULL()
        { return h == NULL; }
  private:
    SC_HANDLE h;
};


PServiceProcess::ProcessCommandResult
                             PServiceProcess::ProcessCommand(const char * cmd)
{
  static const char * commandNames[] = {
    "debug", "version",
    "install", "remove", "start", "stop", "pause", "resume"
  };
#define FIRST_NTSVC_COMMAND (PARRAYSIZE(commandNames)-6)

  PINDEX numCmds = isWin95 ? FIRST_NTSVC_COMMAND : PARRAYSIZE(commandNames);
  PINDEX cmdNum = 0;
  while (stricmp(cmd, commandNames[cmdNum]) != 0) {
    if (++cmdNum >= numCmds) {
      if (*cmd != '\0')
        PError << "Unknown command \"" << cmd << "\".\n";
      PError << "usage: " << GetName() << " [ ";
      for (cmdNum = 0; cmdNum < numCmds-1; cmdNum++)
        PError << commandNames[cmdNum] << " | ";
      PError << commandNames[cmdNum] << " ]" << endl;
      return ProcessCommandError;
    }
  }

  switch (cmdNum) {
    case 0 : // Debug mode
      return DebugCommandMode;

    case  1 : // Version command
      PError << GetName() << ' '
             << GetOSClass() << '/' << GetOSName()
             << " Version " << GetVersion(TRUE) << endl;
      return ProcessCommandError;
  }

  P_SC_HANDLE schSCManager;
  if (schSCManager.IsNULL()) {
    PError << "Could not open Service Manager." << endl;
    return ProcessCommandError;
  }

  P_SC_HANDLE schService(schSCManager, this);
  if (cmdNum != FIRST_NTSVC_COMMAND && schService.IsNULL()) {
    PError << "Service is not installed." << endl;
    return ProcessCommandError;
  }

  SERVICE_STATUS status;

  BOOL good = FALSE;
  switch (cmdNum) {
    case FIRST_NTSVC_COMMAND : // install
      if (!schService.IsNULL()) {
        PError << "Service is already installed." << endl;
        return ProcessCommandError;
      }
      else {
        SC_HANDLE newService = CreateService(
                          schSCManager,               // SCManager database
                          GetName(),                  // name of service
                          GetName(),                  // name to display
                          SERVICE_ALL_ACCESS,         // desired access
                          SERVICE_WIN32_OWN_PROCESS,  // service type
                          SERVICE_DEMAND_START,       // start type
                          SERVICE_ERROR_NORMAL,       // error control type
                          GetFile(),                  // service's binary
                          NULL,                       // no load ordering group
                          NULL,                       // no tag identifier
                          GetServiceDependencies(),   // no dependencies
                          NULL,                       // LocalSystem account
                          NULL);                      // no password
        good = newService != NULL;
        if (good) {
          CloseServiceHandle(newService);
          HKEY key;
          if (RegCreateKey(HKEY_LOCAL_MACHINE,
              "SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application\\" +
                                           GetName(), &key) == ERROR_SUCCESS) {
            RegSetValueEx(key, "EventMessageFile", 0, REG_EXPAND_SZ,
                   (LPBYTE)(const char *)GetFile(), GetFile().GetLength() + 1);
            DWORD dwData = EVENTLOG_ERROR_TYPE |
                             EVENTLOG_WARNING_TYPE | EVENTLOG_INFORMATION_TYPE;
            RegSetValueEx(key, "TypesSupported",
                                 0, REG_DWORD, (LPBYTE)&dwData, sizeof(DWORD));
            RegCloseKey(key);
          }
        }
      }
      break;

    case FIRST_NTSVC_COMMAND+1 : // remove
      good = DeleteService(schService);
      if (good) {
        PString name = "SYSTEM\\CurrentControlSet\\Services\\"
                                         "EventLog\\Application\\" + GetName();
        RegDeleteKey(HKEY_LOCAL_MACHINE, (char *)(const char *)name);
      }
      break;

    case FIRST_NTSVC_COMMAND+2 : // start
      good = StartService(schService, 0, NULL);
      break;

    case FIRST_NTSVC_COMMAND+3 : // stop
      good = ControlService(schService, SERVICE_CONTROL_STOP, &status);
      break;

    case FIRST_NTSVC_COMMAND+4 : // pause
      good = ControlService(schService, SERVICE_CONTROL_PAUSE, &status);
      break;

    case FIRST_NTSVC_COMMAND+5 : // resume
      good = ControlService(schService, SERVICE_CONTROL_CONTINUE, &status);
      break;
  }

  DWORD err = GetLastError();
  PError << "Service command \"" << commandNames[cmdNum] << "\" ";
  if (good) {
    PError << "successful." << endl;
    return CommandProcessed;
  }
  else {
    PError << "failed - error code = " << err << endl;
    return ProcessCommandError;
  }
}


extern "C" int PASCAL WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
  return main(__argc, __argv, NULL);
}


// End Of File ///////////////////////////////////////////////////////////////
