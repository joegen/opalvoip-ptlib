/*
 * $Id: svcproc.cxx,v 1.32 1998/02/03 06:16:31 robertj Exp $
 *
 * Portable Windows Library
 *
 * Operating System Classes Implementation
 *
 * Copyright 1993 Equivalence
 *
 * $Log: svcproc.cxx,v $
 * Revision 1.32  1998/02/03 06:16:31  robertj
 * Added extra log levels.
 * Fixed bug where window disappears after debug service termination.
 *
 * Revision 1.31  1998/01/26 00:56:11  robertj
 * Changed ServiceProcess to exclusively use named event to detect running process.
 *
 * Revision 1.30  1997/12/18 05:05:45  robertj
 * Added Edit menu.
 *
 * Revision 1.29  1997/11/04 06:01:45  robertj
 * Fix of "service hung at startup" message for NT service.
 *
 * Revision 1.28  1997/10/30 10:17:10  robertj
 * Fixed bug in detection of running service.
 *
 * Revision 1.27  1997/10/03 15:14:17  robertj
 * Fixed crash on exit.
 *
 * Revision 1.26  1997/08/28 12:50:32  robertj
 * Fixed race condition in cleaning up threads on application termination.
 *
 * Revision 1.25  1997/07/17 12:43:29  robertj
 * Fixed bug for auto-start of service under '95.
 *
 * Revision 1.24  1997/07/14 11:47:20  robertj
 * Added "const" to numerous variables.
 *
 * Revision 1.23  1997/07/08 13:00:30  robertj
 * DLL support.
 * Fixed '95 support so service runs without logging in.
 *
 * Revision 1.22  1997/04/27 05:50:27  robertj
 * DLL support.
 *
 * Revision 1.21  1997/03/18 21:23:27  robertj
 * Fix service manager falsely accusing app of crashing if OnStart() is slow.
 *
 * Revision 1.20  1997/02/05 11:50:40  robertj
 * Changed current process function to return reference and validate objects descendancy.
 * Changed log file name calculation to occur only once.
 * Added some MSVC memory debugging functions.
 *
 * Revision 1.19  1996/12/05 11:53:49  craigs
 * Fixed failure to output PError to debug window if CRLF pairs used
 *
 * Revision 1.18  1996/11/30 12:07:19  robertj
 * Changed service creation for NT so is auto-start,
 *
 * Revision 1.17  1996/11/18 11:32:04  robertj
 * Fixed bug in doing a "stop" command closing ALL instances of service.
 *
 * Revision 1.16  1996/11/12 10:15:16  robertj
 * Fixed bug in NT 3.51 locking up when needs to output to window.
 *
 * Revision 1.15  1996/11/10 21:04:32  robertj
 * Added category names to event log.
 * Fixed menu enables for debug and command modes.
 *
 * Revision 1.14  1996/11/04 03:39:13  robertj
 * Improved detection of running service so debug mode cannot run.
 *
 * Revision 1.13  1996/10/31 12:54:01  robertj
 * Fixed bug in window not being displayed when command line used.
 *
 * Revision 1.12  1996/10/18 11:22:14  robertj
 * Fixed problems with window not being shown under NT.
 *
 * Revision 1.11  1996/10/14 03:09:58  robertj
 * Fixed major bug in debug outpuit locking up (infinite loop)
 * Changed menus so cannot start service if in debug mode
 *
 * Revision 1.10  1996/10/08 13:04:43  robertj
 * Rewrite to use standard window isntead of console window.
 *
 * Revision 1.9  1996/09/16 12:56:27  robertj
 * DLL support
 *
 * Revision 1.8  1996/09/14 12:34:23  robertj
 * Fixed problem with spontaneous exit from app under Win95.
 *
 * Revision 1.7  1996/08/19 13:36:03  robertj
 * Added "Debug" level to system log.
 *
 * Revision 1.6  1996/07/30 12:23:32  robertj
 * Added better service running test.
 * Changed SIGINTR handler to just set termination event.
 *
 * Revision 1.5  1996/07/27 04:07:57  robertj
 * Changed thread creation to use C library function instead of direct WIN32.
 * Changed SystemLog to be stream based rather than printf based.
 * Fixed Win95 support for service start/stop and prevent multiple starts.
 *
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
#include <crtdbg.h>


static HINSTANCE hInstance;

enum {
  SvcCmdDebug,
  SvcCmdVersion,
  SvcCmdInstall,
  SvcCmdRemove,
  SvcCmdStart,
  SvcCmdStop,
  SvcCmdPause,
  SvcCmdResume,
  SvcCmdDeinstall,
  NumSvcCmds
};

static const char * const ServiceCommandNames[NumSvcCmds] = {
  "debug", "version", "install", "remove", "start", "stop", "pause", "resume", "deinstall"
};


///////////////////////////////////////////////////////////////////////////////
// PSystemLog

static PString CreateLogFileName(const PString & processName)
{
  PString dir;
  GetWindowsDirectory(dir.GetPointer(256), 255);
  return dir + "\\" + processName + " Log.TXT";
}

void PSystemLog::Output(Level level, const char * msg)
{
  PServiceProcess & process = PServiceProcess::Current();
  if (level > process.GetLogLevel())
    return;

  DWORD err = GetLastError();

  if (process.isWin95 || process.debugWindow != NULL) {
    static HANDLE mutex = CreateMutex(NULL, FALSE, NULL);
    WaitForSingleObject(mutex, INFINITE);

    ostream * out;
    if (process.debugWindow != NULL)
      out = new PStringStream;
    else {
      static PString logFileName = CreateLogFileName(process.GetName());
      out = new ofstream(logFileName, ios::app);
    }

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
    *out << now.AsString("yy/MM/dd hh:mm:ss ") << levelName[level+1];
    if (msg[0] != '\0')
      *out << ": " << msg;
    if (level < Info && err != 0)
      *out << " - error = " << err << endl;
    else if (msg[0] == '\0' || msg[strlen(msg)-1] != '\n')
      *out << endl;

    if (process.debugWindow != NULL)
      process.DebugOutput(*(PStringStream*)out);

    delete out;
    ReleaseMutex(mutex);
    SetLastError(0);
  }
  else {
    // Use event logging to log the error.
    HANDLE hEventSource = RegisterEventSource(NULL, process.GetName());
    if (hEventSource == NULL)
      return;

    char errbuf[25];
    if (level < Info && err != 0)
      ::sprintf(errbuf, "\nError code = %d", err);
    else
      errbuf[0] = '\0';

    LPCTSTR strings[3];
    strings[0] = msg;
    strings[1] = errbuf;
    strings[2] = level != Fatal ? "" : " Program aborted.";

    static const WORD levelType[NumLogLevels+1] = {
      EVENTLOG_INFORMATION_TYPE,
      EVENTLOG_ERROR_TYPE,
      EVENTLOG_ERROR_TYPE,
      EVENTLOG_WARNING_TYPE,
      EVENTLOG_INFORMATION_TYPE,
      EVENTLOG_INFORMATION_TYPE,
      EVENTLOG_INFORMATION_TYPE,
      EVENTLOG_INFORMATION_TYPE
    };
    ReportEvent(hEventSource, // handle of event source
                levelType[level+1],     // event type
                (WORD)(level+1),      // event category
                0x1000,               // event ID
                NULL,                 // current user's SID
                PARRAYSIZE(strings),  // number of strings
                0,                    // no bytes of raw data
                strings,              // array of error strings
                NULL);                // no raw data
    DeregisterEventSource(hEventSource);
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


///////////////////////////////////////////////////////////////////////////////
// PServiceProcess

PServiceProcess::PServiceProcess(const char * manuf, const char * name,
                           WORD major, WORD minor, CodeStatus stat, WORD build)
  : PProcess(manuf, name, major, minor, stat, build)
{
  controlWindow = debugWindow = NULL;
  currentLogLevel = PSystemLog::Warning;
}


PServiceProcess & PServiceProcess::Current()
{
  PServiceProcess & process = (PServiceProcess &)PProcess::Current();
  PAssert(process.IsDescendant(PServiceProcess::Class()), "Not a service!");
  return process;
}


int PServiceProcess::_main(int argc, char ** argv, char **)
{
  _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_DEBUG);
  _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF|_CRTDBG_LEAK_CHECK_DF);

  PSetErrorStream(new PSystemLog(PSystemLog::StdError));
  PreInitialise(1, argv);

  debugMode = FALSE;
  isWin95 = GetOSName() == "95";

  BOOL processedCommand = FALSE;
  while (--argc > 0) {
    if (!CreateControlWindow(TRUE))
      return 1;
    if (ProcessCommand(*++argv))
      processedCommand = TRUE;
  }

  if (processedCommand) {
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) != 0)
      DispatchMessage(&msg);
    return GetTerminationValue();
  }

  currentLogLevel = debugMode ? PSystemLog::Info : PSystemLog::Warning;

  if (!processedCommand && !debugMode && !isWin95) {
    static SERVICE_TABLE_ENTRY dispatchTable[] = {
      { "", PServiceProcess::StaticMainEntry },
      { NULL, NULL }
    };
    dispatchTable[0].lpServiceName = (char *)(const char *)GetName();

    if (StartServiceCtrlDispatcher(dispatchTable))
      return GetTerminationValue();

    PSystemLog::Output(PSystemLog::Fatal, "StartServiceCtrlDispatcher failed.");
    MessageBox(NULL, "Not run as a service!", GetName(), MB_OK);
    return 1;
  }

  if (!CreateControlWindow(debugMode))
    return 1;

  HANDLE hEvent = OpenEvent(EVENT_MODIFY_STATE, FALSE, GetName());
  if (hEvent != NULL || ::GetLastError() == ERROR_ACCESS_DENIED) {
    if (hEvent != NULL)
      CloseHandle(hEvent);
    MessageBox(NULL, "Service already running", GetName(), MB_OK);
    return 3;
  }

  if (debugMode) {
    ::SetLastError(0);
    PError << "Service simulation started for \"" << GetName() << "\".\n"
              "Close window to terminate.\n" << endl;
  }

  terminationEvent = CreateEvent(NULL, TRUE, FALSE, GetName());
  PAssertOS(terminationEvent != NULL);

  threadHandle = (HANDLE)_beginthread(StaticThreadEntry, 0, this);
  PAssertOS(threadHandle != (HANDLE)-1);

  SetTerminationValue(0);

  MSG msg;
  do {
    switch (MsgWaitForMultipleObjects(1, &terminationEvent,
                                      FALSE, INFINITE, QS_ALLINPUT)) {
      case WAIT_OBJECT_0+1 :
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
          if (msg.message != WM_QUIT)
            DispatchMessage(&msg);
        }
        break;

      default :
        // This is a work around for '95 coming up with an erroneous error
        if (GetLastError() == ERROR_INVALID_HANDLE &&
                          WaitForSingleObject(terminationEvent, 0) == WAIT_TIMEOUT)
          break;
        // Else fall into next case

      case WAIT_OBJECT_0 :
        if (!debugMode || controlWindow == NULL)
          msg.message = WM_QUIT;
        else {
          PError << "nService simulation stopped for \"" << GetName() << "\".\n\n"
                    "Close window to terminate.\n" << endl;
          ResetEvent(terminationEvent);
        }
    }
  } while (msg.message != WM_QUIT);

  if (controlWindow != NULL)
    DestroyWindow(controlWindow);

  // Set thread ID for process to this thread
  threadMutex.Wait();
  activeThreads.SetAt(threadId, NULL);
  threadId = GetCurrentThreadId();
  threadHandle = GetCurrentThread();
  activeThreads.SetAt(threadId, this);
  threadMutex.Signal();
  OnStop();

  return GetTerminationValue();
}


enum {
  ExitMenuID = 100,
  HideMenuID,
  CopyMenuID,
  CutMenuID,
  DeleteMenuID,
  SelectAllMenuID,
  SvcCmdBaseMenuID = 1000,
  LogLevelBaseMenuID = 2000
};

BOOL PServiceProcess::CreateControlWindow(BOOL createDebugWindow)
{
  if (controlWindow == NULL) {
    WNDCLASS wclass;
    wclass.style = CS_HREDRAW|CS_VREDRAW;
    wclass.lpfnWndProc = (WNDPROC)StaticWndProc;
    wclass.cbClsExtra = 0;
    wclass.cbWndExtra = 0;
    wclass.hInstance = hInstance;
    wclass.hIcon = NULL;
    wclass.hCursor = NULL;
    wclass.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
    wclass.lpszMenuName = NULL;
    wclass.lpszClassName = GetName();
    if (RegisterClass(&wclass) == 0)
      return FALSE;

    HMENU menubar = CreateMenu();
    HMENU menu = CreatePopupMenu();
    AppendMenu(menu, MF_STRING, HideMenuID, "&Hide");
    AppendMenu(menu, MF_STRING, SvcCmdBaseMenuID+SvcCmdVersion, "&Version");
    AppendMenu(menu, MF_SEPARATOR, 0, NULL);
    AppendMenu(menu, MF_STRING, ExitMenuID, "E&xit");
    AppendMenu(menubar, MF_POPUP, (UINT)menu, "&File");

    menu = CreatePopupMenu();
    AppendMenu(menu, MF_STRING, CopyMenuID, "&Copy");
    AppendMenu(menu, MF_STRING, CutMenuID, "C&ut");
    AppendMenu(menu, MF_STRING, DeleteMenuID, "&Delete");
    AppendMenu(menu, MF_SEPARATOR, 0, NULL);
    AppendMenu(menu, MF_STRING, SelectAllMenuID, "&Select All");
    AppendMenu(menubar, MF_POPUP, (UINT)menu, "&Edit");

    menu = CreatePopupMenu();
    AppendMenu(menu, MF_STRING, SvcCmdBaseMenuID+SvcCmdInstall, "&Install");
    AppendMenu(menu, MF_STRING, SvcCmdBaseMenuID+SvcCmdRemove, "&Remove");
    AppendMenu(menu, MF_STRING, SvcCmdBaseMenuID+SvcCmdDeinstall, "&Deinstall");
    AppendMenu(menu, MF_STRING, SvcCmdBaseMenuID+SvcCmdStart, "&Start");
    AppendMenu(menu, MF_STRING, SvcCmdBaseMenuID+SvcCmdStop, "S&top");
    AppendMenu(menu, MF_STRING, SvcCmdBaseMenuID+SvcCmdPause, "&Pause");
    AppendMenu(menu, MF_STRING, SvcCmdBaseMenuID+SvcCmdResume, "R&esume");
    AppendMenu(menubar, MF_POPUP, (UINT)menu, "&Control");

    menu = CreatePopupMenu();
    AppendMenu(menu, MF_STRING, LogLevelBaseMenuID+PSystemLog::Fatal,   "&Fatal Error");
    AppendMenu(menu, MF_STRING, LogLevelBaseMenuID+PSystemLog::Error,   "&Error");
    AppendMenu(menu, MF_STRING, LogLevelBaseMenuID+PSystemLog::Warning, "&Warning");
    AppendMenu(menu, MF_STRING, LogLevelBaseMenuID+PSystemLog::Info,    "&Information");
    AppendMenu(menu, MF_STRING, LogLevelBaseMenuID+PSystemLog::Debug,   "&Debug");
    AppendMenu(menu, MF_STRING, LogLevelBaseMenuID+PSystemLog::Debug2,  "Debug &2");
    AppendMenu(menu, MF_STRING, LogLevelBaseMenuID+PSystemLog::Debug3,  "Debug &3");
    AppendMenu(menubar, MF_POPUP, (UINT)menu, "&Log Level");

    if (CreateWindow(GetName(),
                     GetName(),
                     WS_OVERLAPPEDWINDOW,
                     CW_USEDEFAULT, CW_USEDEFAULT,
                     CW_USEDEFAULT, CW_USEDEFAULT, 
                     NULL,
                     menubar,
                     hInstance,
                     NULL) == NULL)
      return FALSE;
  }

  if (createDebugWindow && debugWindow == NULL) {
    debugWindow = CreateWindow("edit",
                               "",
                               WS_CHILD|WS_HSCROLL|WS_VSCROLL|WS_VISIBLE|WS_BORDER|
                                      ES_MULTILINE|ES_READONLY,
                               0, 0, 0, 0,
                               controlWindow,
                               (HMENU)10,
                               hInstance,
                               NULL);
    SendMessage(debugWindow, EM_SETLIMITTEXT, isWin95 ? 32000 : 128000, 0);
  }

  return TRUE;
}


LPARAM WINAPI PServiceProcess::StaticWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  return Current().WndProc(hWnd, msg, wParam, lParam);
}


LPARAM PServiceProcess::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  switch (msg) {
    case WM_CREATE :
      controlWindow = hWnd;
      break;

    case WM_DESTROY :
      controlWindow = debugWindow = NULL;
      PostQuitMessage(0);
      break;

    case WM_SIZE :
      if (debugWindow != NULL)
        MoveWindow(debugWindow, 0, 0, LOWORD(lParam), HIWORD(lParam), TRUE);
      break;

    case WM_INITMENUPOPUP :
    {
      int enableItems = MF_BYCOMMAND|(debugMode ? MF_ENABLED : MF_GRAYED);
      for (int i = PSystemLog::Fatal; i < PSystemLog::NumLogLevels; i++) {
        CheckMenuItem((HMENU)wParam, LogLevelBaseMenuID+i, MF_BYCOMMAND|MF_UNCHECKED);
        EnableMenuItem((HMENU)wParam, LogLevelBaseMenuID+i, enableItems);
      }
      CheckMenuItem((HMENU)wParam, LogLevelBaseMenuID+GetLogLevel(), MF_BYCOMMAND|MF_CHECKED);

      enableItems = MF_BYCOMMAND|(debugMode ? MF_GRAYED : MF_ENABLED);
      EnableMenuItem((HMENU)wParam, SvcCmdBaseMenuID+SvcCmdStart, enableItems);
      EnableMenuItem((HMENU)wParam, SvcCmdBaseMenuID+SvcCmdStop, enableItems);
      EnableMenuItem((HMENU)wParam, SvcCmdBaseMenuID+SvcCmdPause, enableItems);
      EnableMenuItem((HMENU)wParam, SvcCmdBaseMenuID+SvcCmdResume, enableItems);

      DWORD start, finish;
      if (debugWindow != NULL)
        SendMessage(debugWindow, EM_GETSEL, (WPARAM)&start, (LPARAM)&finish);
      else
        start = finish = 0;
      enableItems = MF_BYCOMMAND|(start == finish ? MF_GRAYED : MF_ENABLED);
      EnableMenuItem((HMENU)wParam, CopyMenuID, enableItems);
      EnableMenuItem((HMENU)wParam, CutMenuID, enableItems);
      EnableMenuItem((HMENU)wParam, DeleteMenuID, enableItems);
      break;
    }

    case WM_COMMAND :
      switch (wParam) {
        case ExitMenuID :
          DestroyWindow(hWnd);
          break;

        case HideMenuID :
          ShowWindow(hWnd, SW_HIDE);
          break;

        case CopyMenuID :
          if (debugWindow != NULL)
            SendMessage(debugWindow, WM_COPY, 0, 0);
          break;

        case CutMenuID :
          if (debugWindow != NULL)
            SendMessage(debugWindow, WM_CUT, 0, 0);
          break;

        case DeleteMenuID :
          if (debugWindow != NULL)
            SendMessage(debugWindow, WM_CLEAR, 0, 0);
          break;

        case SelectAllMenuID :
          if (debugWindow != NULL)
            SendMessage(debugWindow, EM_SETSEL, 0, -1);
          break;

        default :
          if (wParam >= SvcCmdBaseMenuID && wParam < SvcCmdBaseMenuID+NumSvcCmds)
            ProcessCommand(ServiceCommandNames[wParam-SvcCmdBaseMenuID]);
          if (wParam >= LogLevelBaseMenuID+PSystemLog::Fatal && wParam < LogLevelBaseMenuID+PSystemLog::NumLogLevels)
            SetLogLevel((PSystemLog::Level)(wParam-LogLevelBaseMenuID));
      }
      break;

    case WM_ENDSESSION :
      if (wParam)
        PostQuitMessage(0);
      return 0;
  }

  return DefWindowProc(hWnd, msg, wParam, lParam);
}


void PServiceProcess::DebugOutput(const char * out)
{
  if (controlWindow == NULL || debugWindow == NULL)
    return;

  if (!IsWindowVisible(controlWindow))
    ShowWindow(controlWindow, SW_SHOWDEFAULT);

  int len = strlen(out);
  int max = isWin95 ? 32000 : 128000;
  while (GetWindowTextLength(debugWindow)+len >= max) {
    SendMessage(debugWindow, WM_SETREDRAW, FALSE, 0);
    DWORD start, finish;
    SendMessage(debugWindow, EM_GETSEL, (WPARAM)&start, (LPARAM)&finish);
    SendMessage(debugWindow, EM_SETSEL, 0,
                SendMessage(debugWindow, EM_LINEINDEX, 1, 0));
    SendMessage(debugWindow, EM_REPLACESEL, FALSE, (DWORD)"");
    SendMessage(debugWindow, EM_SETSEL, start, finish);
    SendMessage(debugWindow, WM_SETREDRAW, TRUE, 0);
  }

  SendMessage(debugWindow, EM_SETSEL, max, max);
  char * lf;
  const char * prev = out;
  while ((lf = strchr(prev, '\n')) != NULL) {
    if (*(lf-1) == '\r')
      prev = lf+1;
    else {
      *lf++ = '\0';
      SendMessage(debugWindow, EM_REPLACESEL, FALSE, (DWORD)out);
      SendMessage(debugWindow, EM_REPLACESEL, FALSE, (DWORD)"\r\n");
      prev = out = lf;
    }
  }
  SendMessage(debugWindow, EM_REPLACESEL, FALSE, (DWORD)out);
}


void PServiceProcess::StaticMainEntry(DWORD argc, LPTSTR * argv)
{
  Current().MainEntry(argc, argv);
}


void PServiceProcess::MainEntry(DWORD argc, LPTSTR * argv)
{
  // SERVICE_STATUS members that don't change
  status.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
  status.dwServiceSpecificExitCode = 0;

  // register our service control handler:
  statusHandle = RegisterServiceCtrlHandler(GetName(), StaticControlEntry);
  if (statusHandle == NULL)
    return;

  // report the status to Service Control Manager.
  if (!ReportStatus(SERVICE_START_PENDING, NO_ERROR, 1, 20000))
    return;

  // create the stop event object. The control handler function signals
  // this event when it receives the "stop" control code.
  terminationEvent = CreateEvent(NULL, TRUE, FALSE, (const char *)GetName());
  if (terminationEvent == NULL)
    return;

  startedEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
  if (startedEvent == NULL)
    return;

  GetArguments().SetArgs(argc, argv);

  // start the thread that performs the work of the service.
  threadHandle = (HANDLE)_beginthread(StaticThreadEntry, 0, this);
  if (threadHandle != (HANDLE)-1) {
    while (WaitForSingleObject(startedEvent, 10000) == WAIT_TIMEOUT) {
      if (!ReportStatus(SERVICE_START_PENDING, NO_ERROR, 1, 20000))
        return;
    }
    // Wait here for the end
    WaitForSingleObject(terminationEvent, INFINITE);
  }

  CloseHandle(startedEvent);
  CloseHandle(terminationEvent);
  ReportStatus(SERVICE_STOPPED, 0);
}


void PServiceProcess::StaticThreadEntry(void * arg)
{
  ((PServiceProcess *)arg)->ThreadEntry();
}


void PServiceProcess::ThreadEntry()
{
  threadMutex.Wait();
  threadId = GetCurrentThreadId();
  threadHandle = GetCurrentThread();
  activeThreads.SetAt(threadId, this);
  threadMutex.Signal();

  SetTerminationValue(1);
  if (OnStart()) {

    SetEvent(startedEvent);
    ReportStatus(SERVICE_RUNNING);
    SetTerminationValue(0);

    Main();

    ReportStatus(SERVICE_STOP_PENDING, NO_ERROR, 1, 30000);
  }

  SetEvent(terminationEvent);
}


void PServiceProcess::StaticControlEntry(DWORD code)
{
  Current().ControlEntry(code);
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
      ReportStatus(SERVICE_STOP_PENDING, NO_ERROR, 1, 30000);
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

  if (debugMode || isWin95)
    return TRUE;

  // Report the status of the service to the service control manager.
  if (SetServiceStatus(statusHandle, &status))
    return TRUE;

  // If an error occurs, stop the service.
  PSystemLog::Output(PSystemLog::Error, "SetServiceStatus failed");
  return FALSE;
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
  if (RegCreateKey(HKEY_LOCAL_MACHINE,
                   "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run",
                   &key) == ERROR_SUCCESS) {
    RegDeleteValue(key, (char *)(const char *)svc->GetName());
    RegCloseKey(key);
  }

  if ((error = RegCreateKey(HKEY_LOCAL_MACHINE,
                            "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\RunServices",
                            &key)) != ERROR_SUCCESS)
    return FALSE;

  error = RegSetValueEx(key, svc->GetName(), 0, REG_SZ,
         (LPBYTE)(const char *)svc->GetFile(), svc->GetFile().GetLength() + 1);

  RegCloseKey(key);

  return error == ERROR_SUCCESS;
}


BOOL Win95_ServiceManager::Delete(PServiceProcess * svc)
{
  HKEY key;
  if (RegCreateKey(HKEY_LOCAL_MACHINE,
                   "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run",
                   &key) == ERROR_SUCCESS) {
    RegDeleteValue(key, (char *)(const char *)svc->GetName());
    RegCloseKey(key);
  }

  if ((error = RegCreateKey(HKEY_LOCAL_MACHINE,
                            "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\RunServices",
                            &key)) != ERROR_SUCCESS)
    return FALSE;

  error = RegDeleteValue(key, (char *)(const char *)svc->GetName());

  RegCloseKey(key);

  return error == ERROR_SUCCESS;
}


BOOL Win95_ServiceManager::Start(PServiceProcess * svc)
{
  HANDLE hEvent = OpenEvent(EVENT_MODIFY_STATE, FALSE, svc->GetName());
  if (hEvent != NULL) {
    CloseHandle(hEvent);
    PError << "Service already running" << endl;
    error = 1;
    return FALSE;
  }

  BOOL ok = _spawnl(_P_DETACH, svc->GetFile(), svc->GetFile(), NULL) >= 0;
  error = errno;
  return ok;
}


BOOL Win95_ServiceManager::Stop(PServiceProcess * service)
{
  HANDLE hEvent = OpenEvent(EVENT_MODIFY_STATE, FALSE, service->GetName());
  if (hEvent == NULL) {
    error = GetLastError();
    PError << "Service is not running" << endl;
    return FALSE;
  }

  SetEvent(hEvent);
  CloseHandle(hEvent);
  return TRUE;
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
                    SERVICE_AUTO_START,             // start type
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

  LPBYTE fn = (LPBYTE)(const char *)svc->GetFile();
  PINDEX fnlen = svc->GetFile().GetLength()+1;
  if ((error = RegSetValueEx(key, "EventMessageFile",
                             0, REG_EXPAND_SZ, fn, fnlen)) == ERROR_SUCCESS &&
      (error = RegSetValueEx(key, "CategoryMessageFile",
                             0, REG_EXPAND_SZ, fn, fnlen)) == ERROR_SUCCESS) {
    DWORD dwData = EVENTLOG_ERROR_TYPE | EVENTLOG_WARNING_TYPE | EVENTLOG_INFORMATION_TYPE;
    if ((error = RegSetValueEx(key, "TypesSupported",
                               0, REG_DWORD, (LPBYTE)&dwData, sizeof(DWORD))) == ERROR_SUCCESS) {
      dwData = PSystemLog::NumLogLevels;
      error = RegSetValueEx(key, "CategoryCount", 0, REG_DWORD, (LPBYTE)&dwData, sizeof(DWORD));
    }
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


BOOL PServiceProcess::ProcessCommand(const char * cmd)
{
  PINDEX cmdNum = 0;
  while (stricmp(cmd, ServiceCommandNames[cmdNum]) != 0) {
    if (++cmdNum >= NumSvcCmds) {
      if (*cmd != '\0')
        PError << "Unknown command \"" << cmd << "\".\n";
      else
        PError << "Could not start service.\n";
      PError << "usage: " << GetName() << " [ ";
      for (cmdNum = 0; cmdNum < NumSvcCmds-1; cmdNum++)
        PError << ServiceCommandNames[cmdNum] << " | ";
      PError << ServiceCommandNames[cmdNum] << " ]" << endl;
      return TRUE;
    }
  }

  NT_ServiceManager nt;
  Win95_ServiceManager win95;
  ServiceManager * svcManager =
                    isWin95 ? (ServiceManager *)&win95 : (ServiceManager *)&nt;
  BOOL good = FALSE;
  switch (cmdNum) {
    case SvcCmdDebug : // Debug mode
      debugMode = TRUE;
      return FALSE;

    case SvcCmdVersion : // Version command
      ::SetLastError(0);
      PError << GetName() << ' '
             << GetOSClass() << '/' << GetOSName()
             << " Version " << GetVersion(TRUE) << endl;
      return TRUE;

    case SvcCmdInstall : // install
      good = svcManager->Create(this);
      break;

    case SvcCmdRemove : // remove
      good = svcManager->Delete(this);
      break;

    case SvcCmdStart : // start
      good = svcManager->Start(this);
      break;

    case SvcCmdStop : // stop
      good = svcManager->Stop(this);
      break;

    case SvcCmdPause : // pause
      good = svcManager->Pause(this);
      break;

    case SvcCmdResume : // resume
      good = svcManager->Resume(this);
      break;

    case SvcCmdDeinstall : // deinstall
      svcManager->Delete(this);
      PConfig cfg;
      PStringList sections = cfg.GetSections();
      PINDEX i;
      for (i = 0; i < sections.GetSize(); i++)
        cfg.DeleteSection(sections[i]);
      good = TRUE;
      break;
  }

  SetLastError(svcManager->GetError());

  PError << "Service command \"" << ServiceCommandNames[cmdNum] << "\" ";
  if (good)
    PError << "successful." << endl;
  else
    PError << "failed - error code = " << svcManager->GetError() << endl;
  return TRUE;
}


// End Of File ///////////////////////////////////////////////////////////////
