/*
 * $Id: svcproc.cxx,v 1.4 1996/06/10 09:54:08 robertj Exp $
 *
 * Portable Windows Library
 *
 * Operating System Classes Implementation
 *
 * Copyright 1993 Equivalence
 *
 * $Log: svcproc.cxx,v $
 * Revision 1.4  1996/06/10 09:54:08  robertj
 * Fixed Win95 service install bug (typo!)
 *
 * Revision 1.3  1996/05/30 11:49:10  robertj
 * Fixed crash on exit bug.
 *
 * Revision 1.2  1996/05/23 10:03:21  robertj
 * Windows 95 support.
 *
 * Revision 1.1  1996/05/15 21:11:51  robertj
 * Initial revision
 *
 */

#include <ptlib.h>
#include <svcproc.h>

#include <winuser.h>
#include <winnls.h>

#include <process.h>
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
    cin = *new ifstream(h2);

    h1 = GetStdHandle(STD_OUTPUT_HANDLE);
    h2 = _open_osfhandle((long)h1, _O_APPEND|_O_TEXT);
    cout = *new ofstream(h2);

    h1 = GetStdHandle(STD_ERROR_HANDLE);
    h2 = _open_osfhandle((long)h1, _O_APPEND|_O_TEXT);
    cerr = *new ofstream(h2);

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
    if (!debugMode) {
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
    if (!debugMode)
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



class ServiceManager
{
  public:
    ServiceManager()  { error = 0; }

    virtual BOOL Create(PServiceProcess * svc) = 0;
    virtual BOOL Delete(PServiceProcess * svc) = 0;
    virtual BOOL Start(PServiceProcess * svc) = 0;
    virtual BOOL Stop(PServiceProcess * svc) = 0;
    virtual BOOL Pause(PServiceProcess * svc) = 0;
    virtual BOOL Resume(PServiceProcess * svc) = 0;

    DWORD GetError() const { return error; }

  protected:
    DWORD error;
};


class Win95_ServiceManager : public ServiceManager
{
  public:
    virtual BOOL Create(PServiceProcess * svc);
    virtual BOOL Delete(PServiceProcess * svc);
    virtual BOOL Start(PServiceProcess * svc);
    virtual BOOL Stop(PServiceProcess * svc);
    virtual BOOL Pause(PServiceProcess * svc);
    virtual BOOL Resume(PServiceProcess * svc);
};


BOOL Win95_ServiceManager::Create(PServiceProcess * svc)
{
  HKEY key;
  if ((error = RegCreateKey(HKEY_LOCAL_MACHINE,
                           "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run",
                           &key)) != ERROR_SUCCESS)
    return FALSE;

  error = RegSetValueEx(key, svc->GetName(), 0, REG_SZ,
         (LPBYTE)(const char *)svc->GetFile(), svc->GetFile().GetLength() + 1);

  RegCloseKey(key);

  return error == ERROR_SUCCESS;
}


BOOL Win95_ServiceManager::Delete(PServiceProcess * svc)
{
  PString name =
        "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run\\" + svc->GetName();
  error = RegDeleteKey(HKEY_LOCAL_MACHINE, (char *)(const char *)name);

  return error == ERROR_SUCCESS;
}


BOOL Win95_ServiceManager::Start(PServiceProcess * svc)
{
  BOOL ok = _spawnl(_P_DETACH, svc->GetFile(), svc->GetFile(), NULL) >= 0;
  error = errno;
  return ok;
}


BOOL Win95_ServiceManager::Stop(PServiceProcess *)
{
  PError << "Cannot stop service under Windows 95" << endl;
  error = 1;
  return FALSE;
}


BOOL Win95_ServiceManager::Pause(PServiceProcess *)
{
  PError << "Cannot pause service under Windows 95" << endl;
  error = 1;
  return FALSE;
}


BOOL Win95_ServiceManager::Resume(PServiceProcess *)
{
  PError << "Cannot resume service under Windows 95" << endl;
  error = 1;
  return FALSE;
}



class NT_ServiceManager : public ServiceManager
{
  public:
    NT_ServiceManager()  { schSCManager = schService = NULL; }
    ~NT_ServiceManager();

    BOOL Create(PServiceProcess * svc);
    BOOL Delete(PServiceProcess * svc);
    BOOL Start(PServiceProcess * svc);
    BOOL Stop(PServiceProcess * svc)
      { return Control(svc, SERVICE_CONTROL_STOP); }
    BOOL Pause(PServiceProcess * svc)
      { return Control(svc, SERVICE_CONTROL_PAUSE); }
    BOOL Resume(PServiceProcess * svc)
      { return Control(svc, SERVICE_CONTROL_CONTINUE); }

    DWORD GetError() const { return error; }

  private:
    BOOL OpenManager();
    BOOL Open(PServiceProcess * svc);
    BOOL Control(PServiceProcess * svc, DWORD command);

    SC_HANDLE schSCManager, schService;
    DWORD error;
};


NT_ServiceManager::~NT_ServiceManager()
{
  if (schService != NULL)
    CloseServiceHandle(schService);
  if (schSCManager != NULL)
    CloseServiceHandle(schSCManager);
}


BOOL NT_ServiceManager::OpenManager()
{
  schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
  if (schSCManager != NULL)
    return TRUE;

  error = GetLastError();
  PError << "Could not open Service Manager." << endl;
  return FALSE;
}


BOOL NT_ServiceManager::Open(PServiceProcess * svc)
{
  if (!OpenManager())
    return FALSE;

  schService = OpenService(schSCManager, svc->GetName(), SERVICE_ALL_ACCESS);
  if (schService != NULL)
    return TRUE;

  error = GetLastError();
  PError << "Service is not installed." << endl;
  return FALSE;
}


BOOL NT_ServiceManager::Create(PServiceProcess * svc)
{
  if (!OpenManager())
    return FALSE;

  schService = OpenService(schSCManager, svc->GetName(), SERVICE_ALL_ACCESS);
  if (schService != NULL) {
    PError << "Service is already installed." << endl;
    return FALSE;
  }

  schService = CreateService(
                    schSCManager,                   // SCManager database
                    svc->GetName(),                 // name of service
                    svc->GetName(),                 // name to display
                    SERVICE_ALL_ACCESS,             // desired access
                    SERVICE_WIN32_OWN_PROCESS,      // service type
                    SERVICE_DEMAND_START,           // start type
                    SERVICE_ERROR_NORMAL,           // error control type
                    svc->GetFile(),                 // service's binary
                    NULL,                           // no load ordering group
                    NULL,                           // no tag identifier
                    svc->GetServiceDependencies(),  // no dependencies
                    NULL,                           // LocalSystem account
                    NULL);                          // no password
  if (schService == NULL) {
    error = GetLastError();
    return FALSE;
  }

  HKEY key;
  if ((error = RegCreateKey(HKEY_LOCAL_MACHINE,
             "SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application\\" +
                                       svc->GetName(), &key)) != ERROR_SUCCESS)
    return FALSE;

  PString fn = svc->GetFile();
  if ((error = RegSetValueEx(key, "EventMessageFile", 0, REG_EXPAND_SZ,
               (LPBYTE)(const char *)fn, fn.GetLength()+1)) == ERROR_SUCCESS) {
    DWORD dwData = EVENTLOG_ERROR_TYPE |
                             EVENTLOG_WARNING_TYPE | EVENTLOG_INFORMATION_TYPE;
    error = RegSetValueEx(key, "TypesSupported",
                                 0, REG_DWORD, (LPBYTE)&dwData, sizeof(DWORD));
  }

  RegCloseKey(key);

  return error == ERROR_SUCCESS;
}


BOOL NT_ServiceManager::Delete(PServiceProcess * svc)
{
  if (!Open(svc))
    return FALSE;

  PString name = "SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application\\"
                                                              + svc->GetName();
  error = RegDeleteKey(HKEY_LOCAL_MACHINE, (char *)(const char *)name);

  if (!DeleteService(schService))
    error = GetLastError();

  return error == ERROR_SUCCESS;
}


BOOL NT_ServiceManager::Start(PServiceProcess * svc)
{
  if (!Open(svc))
    return FALSE;

  BOOL ok = StartService(schService, 0, NULL);
  error = GetLastError();
  return ok;
}


BOOL NT_ServiceManager::Control(PServiceProcess * svc, DWORD command)
{
  if (!Open(svc))
    return FALSE;

  SERVICE_STATUS status;
  BOOL ok = ControlService(schService, command, &status);
  error = GetLastError();
  return ok;
}


PServiceProcess::ProcessCommandResult
                             PServiceProcess::ProcessCommand(const char * cmd)
{
  static const char * commandNames[] = {
    "debug", "version", "install", "remove", "start", "stop", "pause", "resume"
  };

  PINDEX cmdNum = 0;
  while (stricmp(cmd, commandNames[cmdNum]) != 0) {
    if (++cmdNum >= PARRAYSIZE(commandNames)) {
      if (*cmd != '\0')
        PError << "Unknown command \"" << cmd << "\".\n";
      PError << "usage: " << GetName() << " [ ";
      for (cmdNum = 0; cmdNum < PARRAYSIZE(commandNames)-1; cmdNum++)
        PError << commandNames[cmdNum] << " | ";
      PError << commandNames[cmdNum] << " ]" << endl;
      return ProcessCommandError;
    }
  }

  NT_ServiceManager nt;
  Win95_ServiceManager win95;
  ServiceManager * svcManager =
                    isWin95 ? (ServiceManager *)&win95 : (ServiceManager *)&nt;
  BOOL good = FALSE;
  switch (cmdNum) {
    case 0 : // Debug mode
      return DebugCommandMode;

    case 1 : // Version command
      PError << GetName() << ' '
             << GetOSClass() << '/' << GetOSName()
             << " Version " << GetVersion(TRUE) << endl;
      return ProcessCommandError;

    case 2 : // install
      good = svcManager->Create(this);
      break;

    case 3 : // remove
      good = svcManager->Delete(this);
      break;

    case 4 : // start
      good = svcManager->Start(this);
      break;

    case 5 : // stop
      good = svcManager->Stop(this);
      break;

    case 6 : // pause
      good = svcManager->Pause(this);
      break;

    case 7 : // resume
      good = svcManager->Resume(this);
      break;
  }

  PError << "Service command \"" << commandNames[cmdNum] << "\" ";
  if (good) {
    PError << "successful." << endl;
    return CommandProcessed;
  }
  else {
    PError << "failed - error code = " << svcManager->GetError() << endl;
    return ProcessCommandError;
  }
}


extern "C" int PASCAL WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
  return main(__argc, __argv, NULL);
}


// End Of File ///////////////////////////////////////////////////////////////
