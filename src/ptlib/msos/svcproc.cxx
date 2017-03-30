/*
 * svcproc.cxx
 *
 * Service process implementation for Win95 and WinNT
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

#ifndef _WIN32_WCE
#include <winuser.h>
#include <winnls.h>
#include <shellapi.h>
#include <commdlg.h>

#include <process.h>
#include <signal.h>
#include <fcntl.h>
#include <io.h>

#ifdef _MSC_VER
#pragma comment(lib,"advapi32.lib")
#pragma comment(lib,"user32.lib")
#pragma comment(lib,"comdlg32.lib")
#endif

#include <fstream>

#include <ptlib/svcproc.h>


#define new PNEW


#define UWM_SYSTRAY (WM_USER + 1)
#define ICON_RESID 1
#define SYSTRAY_ICON_ID 1

static HINSTANCE hInstance;

#define DATE_WIDTH    72
#define THREAD_WIDTH  80
#define LEVEL_WIDTH   32
#define PROTO_WIDTH   40
#define ACTION_WIDTH  48

enum {
  SvcCmdDefault,
  SvcCmdTray,
  SvcCmdNoTray,
  SvcCmdVersion,
  SvcCmdInstall,
  SvcCmdRemove,
  SvcCmdStart,
  SvcCmdStop,
  SvcCmdPause,
  SvcCmdResume,
  SvcCmdDeinstall,
  SvcCmdAutoRestart,
  SvcCmdNoRestart,
  SvcCmdNoWindow,
  SvcCmdHelp,
  NumSvcCmds
};

static const char * const ServiceCommandNames[NumSvcCmds] = {
  "Default",
  "Tray",
  "NoTray",
  "Version",
  "Install",
  "Remove",
  "Start",
  "Stop",
  "Pause",
  "Resume",
  "Deinstall",
  "AutoRestart",
  "NoRestart",
  "NoWin",
  "Help"
};


static const char WindowLogOutput[] = "Window Log Output";
static const char DebuggerLogOutput[] = "Debugger Log Output";

static char emptyString[] = "";
static char crlfString[] = "\r\n";


class PNotifyIconData : public NOTIFYICONDATA {
  public:
    PNotifyIconData(HWND hWnd, UINT flags, const char * tip = NULL);
    void Add()    { Shell_NotifyIcon(NIM_ADD,    this); }
    void Delete() { Shell_NotifyIcon(NIM_DELETE, this); }
    void Modify() { Shell_NotifyIcon(NIM_MODIFY, this); }
};


PNotifyIconData::PNotifyIconData(HWND window, UINT flags, const char * tip)
{
  cbSize = sizeof(NOTIFYICONDATA);
  hWnd   = window;
  uID    = SYSTRAY_ICON_ID;
  uFlags = flags;
  if (tip != NULL) {
    strncpy(szTip, tip, sizeof(szTip)-1);
    szTip[sizeof(szTip)-1] = '\0';
    uFlags |= NIF_TIP;
  }
}


enum TrayIconRegistryCommand {
  AddTrayIcon,
  DelTrayIcon,
  CheckTrayIcon
};

static bool TrayIconRegistry(PServiceProcess * svc, TrayIconRegistryCommand cmd)
{
  HKEY key;
  if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                   "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run",
                   0, KEY_ALL_ACCESS, &key) != ERROR_SUCCESS)
    return false;

  DWORD err = 1;
  DWORD type;
  DWORD len;
  PString str;
  switch (cmd) {
    case CheckTrayIcon :
      err = RegQueryValueEx(key, svc->GetName(), 0, &type, NULL, &len);
      break;

    case AddTrayIcon :
      str = "\"" + svc->GetFile() + "\" Tray";
      err = RegSetValueEx(key, svc->GetName(), 0, REG_SZ,
                         (LPBYTE)(const char *)str, str.GetLength() + 1);
      break;

    case DelTrayIcon :
      err = RegDeleteValue(key, (char *)(const char *)svc->GetName());
  }

  RegCloseKey(key);
  return err == ERROR_SUCCESS;
}



///////////////////////////////////////////////////////////////////////////////
// Logging

void PServiceProcess::LogToWindow::Output(PSystemLog::Level level, const char * msg)
{
  if (msg == NULL || *msg == '\0' || !PProcess::IsInitialised())
    return;

  PServiceProcess & process = PServiceProcess::Current();
  if (process.m_controlWindow != NULL) {
    static HANDLE mutex = CreateMutex(NULL, false, NULL);
    WaitForSingleObject(mutex, INFINITE);

    PStringStream str;
    OutputToStream(str, level, msg);
    process.DebugOutput(str);

    ReleaseMutex(mutex);
    SetLastError(0);
  }
}


class PSystemLogToEvent : public PSystemLogTarget
{
  virtual void Output(PSystemLog::Level level, const char * msg)
  {
    if (!PProcess::IsInitialised())
      return;

    DWORD err = GetLastError();

    // Use event logging to log the error.
    HANDLE hEventSource = RegisterEventSource(NULL, PProcess::Current().GetName());
    if (hEventSource == NULL)
      return;

    char errbuf[25];
    if (level > PSystemLog::StdError && level < PSystemLog::Info && err != 0)
      ::sprintf(errbuf, "Error code = %lu", err);
    else
      errbuf[0] = '\0';

    LPCTSTR strings[4];
    PString threadName = PThread::GetCurrentThreadName();
    strings[0] = threadName.GetPointer();
    strings[1] = msg;
    strings[2] = errbuf;
    strings[3] = level != PSystemLog::Fatal ? "" : " Program aborted.";

    static const WORD levelType[PSystemLog::Info+1] = {
      EVENTLOG_INFORMATION_TYPE,
      EVENTLOG_ERROR_TYPE,
      EVENTLOG_ERROR_TYPE,
      EVENTLOG_WARNING_TYPE
    };
    ReportEvent(hEventSource, // handle of event source
      (WORD)(level < PSystemLog::Info ? levelType[level+1] : EVENTLOG_INFORMATION_TYPE), // event type
                (WORD)(level+1),      // event category
                0x1000,               // event ID
                NULL,                 // current user's SID
                PARRAYSIZE(strings),  // number of strings
                0,                    // no bytes of raw data
                strings,              // array of error strings
                NULL);                // no raw data
    DeregisterEventSource(hEventSource);
  }
};


///////////////////////////////////////////////////////////////////////////////
// PServiceProcess

PServiceProcess::PServiceProcess(const char * manuf, const char * name,
                                 unsigned major, unsigned minor, CodeStatus stat, unsigned build)
  : PProcess(manuf, name, major, minor, stat, build, false, true)
  , m_controlWindow(NULL)
  , m_debugWindow(NULL)
{
}


PServiceProcess & PServiceProcess::Current()
{
  PServiceProcess & process = (PServiceProcess &)PProcess::Current();
  PAssert(PIsDescendant(&process, PServiceProcess), "Not a service!");
  return process;
}


void PServiceProcess::SetLogLevel(PSystemLog::Level level)
{
  PSystemLog::GetTarget().SetThresholdLevel(level);

#if PTRACING
  if ((PTrace::GetOptions()&PTrace::SystemLogStream) != 0)
    PTrace::SetLevel(level);
#endif
}


const char * PServiceProcess::GetServiceDependencies() const
{
  return "EventLog\0";
}


PBoolean PServiceProcess::IsServiceProcess() const
{
  return !m_debugMode;
}


static bool IsServiceRunning(PServiceProcess * svc)
{
  HANDLE hEvent = OpenEvent(EVENT_MODIFY_STATE, false, svc->GetName());
  if (hEvent == NULL) {
    DWORD error = ::GetLastError();
    return error == ERROR_ACCESS_DENIED;
  }

  CloseHandle(hEvent);
  return true;
}


int PServiceProcess::InternalMain(void * arg)
{
  {
    PMEMORY_IGNORE_ALLOCATIONS_FOR_SCOPE;

    PSetErrorStream(new PSystemLog(PSystemLog::StdError));
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

  hInstance = (HINSTANCE)arg;

  
  if (!PProcess::IsOSVersion(5,1,0)) {
    PError << "Unsupported Win32 platform type!" << endl;
    return 1;
  }

  m_debugMode = arguments.GetCount() > 0 && 
	              (strcasecmp(arguments[0], "Debug") == 0 || strcasecmp(arguments[0], "foreground") == 0);
  m_debugHidden = arguments.GetCount() > 0 && strcasecmp(arguments[0], "DebugHidden") == 0;
  SetLogLevel(m_debugMode ? PSystemLog::Info : PSystemLog::Warning);

  if (!m_debugMode && arguments.GetCount() > 0) {
    for (PINDEX a = 0; a < arguments.GetCount(); a++) {
      if (!ProcessCommand(arguments[a]))
        return 1;
    }

    if (m_controlWindow == NULL || m_controlWindow == (HWND)-1)
      return GetTerminationValue();

    if (m_debugWindow != NULL && m_debugWindow != (HWND)-1) {
      ::SetLastError(0);
      PError << "Close window or select another command from the Control menu.\n" << endl;
    }

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) != 0) {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }

    return GetTerminationValue();
  }

  if (!m_debugMode) {
    SERVICE_TABLE_ENTRY dispatchTable[] = {
      { (char *)(const char *)GetName(), PServiceProcess::StaticMainEntry },
      { NULL, NULL }
    };

    PSystemLog::SetTarget(new PSystemLogToEvent());

    if (StartServiceCtrlDispatcher(dispatchTable))
      return GetTerminationValue();

    if (GetLastError() != ERROR_FAILED_SERVICE_CONTROLLER_CONNECT) {
      PSYSTEMLOG(Fatal, "StartServiceCtrlDispatcher failed: error=" << GetLastError());
    }
    ProcessCommand(ServiceCommandNames[SvcCmdDefault]);
    return 1;
  }

  // Skip past the first argument
  arguments.Shift(1);

  if (!CreateControlWindow(m_debugMode))
    return 1;

  if (IsServiceRunning(this)) {
    MessageBox(NULL, "Service already running", GetName(), MB_TASKMODAL);
    return 3;
  }

  if (m_debugMode) {
    ::SetLastError(0);
    DebugOutput(PSTRSTRM("Service simulation started for \"" << GetName()
                << "\" version " << GetVersion(true)
                << " by " << GetManufacturer()
                << " on " << GetOSClass() << ' ' << GetOSName()
                << " (" << GetOSVersion() << '-' << GetOSHardware()
                << ") with PTLib (v" << GetLibVersion()
                << ") at " << PTime().AsString("yyyy/M/d h:mm:ss.uuu")
                << "\nClose window to terminate.\n"));
  }

  m_terminationEvent = CreateEvent(NULL, true, false, GetName());
  PAssertOS(m_terminationEvent != NULL);

  m_threadHandle.Detach();
  m_threadHandle = (HANDLE)_beginthread(StaticThreadEntry, 0, this);
  PAssertOS(m_threadHandle.IsValid());

  SetTerminationValue(0);

  MSG msg;
  msg.message = WM_QUIT+1; //Want somethingthat is not WM_QUIT
  do {
    switch (MsgWaitForMultipleObjects(1, &m_terminationEvent,
                                      false, INFINITE, QS_ALLINPUT)) {
      case WAIT_OBJECT_0+1 :
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
          if (msg.message != WM_QUIT) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
          }
        }
        break;

      default :
        // This is a work around for '95 coming up with an erroneous error
        if (::GetLastError() == ERROR_INVALID_HANDLE &&
                          WaitForSingleObject(m_terminationEvent, 0) == WAIT_TIMEOUT)
          break;
        // Else fall into next case

      case WAIT_OBJECT_0 :
        if (!m_debugMode || m_controlWindow == NULL)
          msg.message = WM_QUIT;
        else {
          DebugOutput("\nService simulation stopped.\n\n"
                      "Close window to terminate.");
          ResetEvent(m_terminationEvent);
        }
    }
  } while (msg.message != WM_QUIT);

  if (m_controlWindow != NULL && m_controlWindow != (HWND)-1)
    DestroyWindow(m_controlWindow);

  // Set thread ID for process to this thread
  m_threadMutex.Wait();
  m_activeThreads.erase(m_threadId);
  m_threadId = GetCurrentThreadId();
  m_threadHandle.Detach();
  m_threadHandle = GetCurrentThread();
  m_activeThreads[m_threadId] = this;
  m_threadMutex.Signal();
  OnStop();

  m_controlWindow = NULL; // This stops the logging

  return GetTerminationValue();
}


enum {
  ExitMenuID = 100,
  HideMenuID,
  ControlMenuID,
  CopyMenuID,
  CutMenuID,
  DeleteMenuID,
  SelectAllMenuID,
#if PMEMORY_HEAP
  MarkMenuID,
  DumpMenuID,
  StatsMenuID,
  ValidateMenuID,
#endif
  OutputToMenuID,
  WindowOutputMenuID,
  DebuggerOutputMenuID,
  SvcCmdBaseMenuID = 1000,
  LogLevelBaseMenuID = 2000
};

static const char ServiceSimulationSectionName[] = "Service Simulation Parameters";
static const char WindowLeftKey[] = "Window Left";
static const char WindowTopKey[] = "Window Top";
static const char WindowRightKey[] = "Window Right";
static const char WindowBottomKey[] = "Window Bottom";
static const char SystemLogFileNameKey[] = "System Log File Name";


PBoolean PServiceProcess::CreateControlWindow(PBoolean createDebugWindow)
{
  if (m_controlWindow != NULL)
    return true;

  WNDCLASS wclass;
  wclass.style = CS_HREDRAW|CS_VREDRAW;
  wclass.lpfnWndProc = (WNDPROC)StaticWndProc;
  wclass.cbClsExtra = 0;
  wclass.cbWndExtra = 0;
  wclass.hInstance = hInstance;
  wclass.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(ICON_RESID));
  wclass.hCursor = NULL;
  wclass.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
  wclass.lpszMenuName = NULL;
  wclass.lpszClassName = GetName();
  if (RegisterClass(&wclass) == 0)
    return false;

  HMENU menubar = CreateMenu();
  HMENU menu = CreatePopupMenu();
  AppendMenu(menu, MF_STRING, OutputToMenuID, "&Output To...");
  AppendMenu(menu, MF_STRING, WindowOutputMenuID, "Output To &Window");
  AppendMenu(menu, MF_STRING, DebuggerOutputMenuID, "Output To &Debugger");
  AppendMenu(menu, MF_SEPARATOR, 0, NULL);
  AppendMenu(menu, MF_STRING, ControlMenuID, "&Control");
  AppendMenu(menu, MF_STRING, HideMenuID, "&Hide");
  AppendMenu(menu, MF_STRING, SvcCmdBaseMenuID+SvcCmdVersion, "&Version");
  AppendMenu(menu, MF_SEPARATOR, 0, NULL);
#if PMEMORY_HEAP
  AppendMenu(menu, MF_STRING, MarkMenuID, "&Mark Memory");
  AppendMenu(menu, MF_STRING, DumpMenuID, "&Dump Memory");
  AppendMenu(menu, MF_STRING, StatsMenuID, "&Statistics");
  AppendMenu(menu, MF_STRING, ValidateMenuID, "&Validate Heap");
  AppendMenu(menu, MF_SEPARATOR, 0, NULL);
#endif
  AppendMenu(menu, MF_STRING, ExitMenuID, "E&xit");
  AppendMenu(menubar, MF_POPUP, (UINT_PTR)menu, "&File");

  menu = CreatePopupMenu();
  AppendMenu(menu, MF_STRING, CopyMenuID, "&Copy");
  AppendMenu(menu, MF_STRING, CutMenuID, "C&ut");
  AppendMenu(menu, MF_STRING, DeleteMenuID, "&Delete");
  AppendMenu(menu, MF_SEPARATOR, 0, NULL);
  AppendMenu(menu, MF_STRING, SelectAllMenuID, "&Select All");
  AppendMenu(menubar, MF_POPUP, (UINT_PTR)menu, "&Edit");

  menu = CreatePopupMenu();
  AppendMenu(menu, MF_STRING, SvcCmdBaseMenuID+SvcCmdInstall, "&Install");
  AppendMenu(menu, MF_STRING, SvcCmdBaseMenuID+SvcCmdRemove, "&Remove");
  AppendMenu(menu, MF_STRING, SvcCmdBaseMenuID+SvcCmdDeinstall, "&Deinstall");
  AppendMenu(menu, MF_STRING, SvcCmdBaseMenuID+SvcCmdStart, "&Start");
  AppendMenu(menu, MF_STRING, SvcCmdBaseMenuID+SvcCmdStop, "S&top");
  AppendMenu(menu, MF_STRING, SvcCmdBaseMenuID+SvcCmdPause, "&Pause");
  AppendMenu(menu, MF_STRING, SvcCmdBaseMenuID+SvcCmdResume, "R&esume");
  AppendMenu(menubar, MF_POPUP, (UINT_PTR)menu, "&Control");

  menu = CreatePopupMenu();
  AppendMenu(menu, MF_STRING, LogLevelBaseMenuID+PSystemLog::Fatal,   "&Fatal Error");
  AppendMenu(menu, MF_STRING, LogLevelBaseMenuID+PSystemLog::Error,   "&Error");
  AppendMenu(menu, MF_STRING, LogLevelBaseMenuID+PSystemLog::Warning, "&Warning");
  AppendMenu(menu, MF_STRING, LogLevelBaseMenuID+PSystemLog::Info,    "&Information");
  AppendMenu(menu, MF_STRING, LogLevelBaseMenuID+PSystemLog::Debug,   "&Debug");
  AppendMenu(menu, MF_STRING, LogLevelBaseMenuID+PSystemLog::Debug2,  "Debug &2");
  AppendMenu(menu, MF_STRING, LogLevelBaseMenuID+PSystemLog::Debug3,  "Debug &3");
  AppendMenu(menubar, MF_POPUP, (UINT_PTR)menu, "&Log Level");

  if (CreateWindow(GetName(),
                   GetName(),
                   WS_OVERLAPPEDWINDOW,
                   CW_USEDEFAULT, CW_USEDEFAULT,
                   CW_USEDEFAULT, CW_USEDEFAULT, 
                   NULL,
                   menubar,
                   hInstance,
                   NULL) == NULL)
    return false;

  if (createDebugWindow && m_debugWindow == NULL) {
#if P_CONFIG_FILE
    PConfig cfg(ServiceSimulationSectionName);
    int l = cfg.GetInteger(WindowLeftKey, -1);
    int t = cfg.GetInteger(WindowTopKey, -1);
    int r = cfg.GetInteger(WindowRightKey, -1);
    int b = cfg.GetInteger(WindowBottomKey, -1);
    if (l > 0 && t > 0 && r > 0 && b > 0)
      SetWindowPos(m_controlWindow, NULL, l, t, r-l, b-t, 0);
#endif // P_CONFIG_FILE

    m_debugWindow = CreateWindow("edit",
                               "",
                               WS_CHILD|WS_HSCROLL|WS_VSCROLL|WS_VISIBLE|WS_BORDER|
                                      ES_MULTILINE|ES_READONLY,
                               0, 0, 0, 0,
                               m_controlWindow,
                               (HMENU)10,
                               hInstance,
                               NULL);
    SendMessage(m_debugWindow, EM_SETLIMITTEXT, 128000, 0);
    DWORD TabStops[] = {
      DATE_WIDTH,
      DATE_WIDTH+THREAD_WIDTH,
      DATE_WIDTH+THREAD_WIDTH+LEVEL_WIDTH,
      DATE_WIDTH+THREAD_WIDTH+LEVEL_WIDTH+PROTO_WIDTH,
      DATE_WIDTH+THREAD_WIDTH+LEVEL_WIDTH+PROTO_WIDTH+ACTION_WIDTH,
      DATE_WIDTH+THREAD_WIDTH+LEVEL_WIDTH+PROTO_WIDTH+ACTION_WIDTH+32  // Standard tab width
    };
    SendMessage(m_debugWindow, EM_SETTABSTOPS, PARRAYSIZE(TabStops), (LPARAM)(LPDWORD)TabStops);

    PString systemLogFileName;
#if P_CONFIG_FILE
    systemLogFileName = cfg.GetString(SystemLogFileNameKey);
#endif // P_CONFIG_FILE
    if (systemLogFileName.IsEmpty() || systemLogFileName == WindowLogOutput)
      PSystemLog::SetTarget(new LogToWindow());
    else {
      if (systemLogFileName == DebuggerLogOutput)
        PSystemLog::SetTarget(new PSystemLogToDebug());
      else {
        PFile::Remove(systemLogFileName);
        PSystemLog::SetTarget(new PSystemLogToFile(systemLogFileName));
      }
      DebugOutput("Sending all system log output to \"" + systemLogFileName + "\".\n");
    }
  }

  return true;
}


LPARAM WINAPI PServiceProcess::StaticWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  return Current().WndProc(hWnd, msg, wParam, lParam);
}


#if P_CONFIG_FILE
static void SaveWindowPosition(HWND hWnd)
{
  RECT r;
  GetWindowRect(hWnd, &r);
  PConfig cfg(ServiceSimulationSectionName);
  cfg.SetInteger(WindowLeftKey, r.left);
  cfg.SetInteger(WindowTopKey, r.top);
  cfg.SetInteger(WindowRightKey, r.right);
  cfg.SetInteger(WindowBottomKey, r.bottom);
}
#endif // P_CONFIG_FILE


LPARAM PServiceProcess::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
#if PMEMORY_HEAP
  static PMemoryHeap::State memoryState;
#endif

  switch (msg) {
    case WM_CREATE :
      m_controlWindow = hWnd;
      break;

    case WM_DESTROY :
      if (m_debugWindow == (HWND)-1) {
        PNotifyIconData nid(hWnd, NIF_TIP);
        nid.Delete(); // This removes the systray icon
      }

      m_controlWindow = m_debugWindow = (HWND)-1;

      PostQuitMessage(0);
      break;

    case WM_ENDSESSION :
      if (wParam && (m_debugMode || lParam != (LPARAM)ENDSESSION_LOGOFF) && m_debugWindow != (HWND)-1)
        OnStop();
      return 0;

#if P_CONFIG_FILE
    case WM_MOVE :
      if (m_debugWindow != NULL)
        SaveWindowPosition(hWnd);
      break;
#endif // P_CONFIG_FILE

    case WM_SIZE :
      if (m_debugWindow != NULL && m_debugWindow != (HWND)-1) {
#if P_CONFIG_FILE
        SaveWindowPosition(hWnd);
#endif // P_CONFIG_FILE
        MoveWindow(m_debugWindow, 0, 0, LOWORD(lParam), HIWORD(lParam), true);
      }
      break;

    case WM_INITMENUPOPUP :
    {
      int enableItems = MF_BYCOMMAND|(m_debugMode ? MF_ENABLED : MF_GRAYED);
      for (int i = PSystemLog::Fatal; i < PSystemLog::NumLogLevels; i++) {
        CheckMenuItem((HMENU)wParam, LogLevelBaseMenuID+i, MF_BYCOMMAND|MF_UNCHECKED);
        EnableMenuItem((HMENU)wParam, LogLevelBaseMenuID+i, enableItems);
      }
      CheckMenuItem((HMENU)wParam, LogLevelBaseMenuID+GetLogLevel(), MF_BYCOMMAND|MF_CHECKED);

      enableItems = MF_BYCOMMAND|(m_debugMode ? MF_GRAYED : MF_ENABLED);
      EnableMenuItem((HMENU)wParam, SvcCmdBaseMenuID+SvcCmdStart, enableItems);
      EnableMenuItem((HMENU)wParam, SvcCmdBaseMenuID+SvcCmdStop, enableItems);
      EnableMenuItem((HMENU)wParam, SvcCmdBaseMenuID+SvcCmdPause, enableItems);
      EnableMenuItem((HMENU)wParam, SvcCmdBaseMenuID+SvcCmdResume, enableItems);

      DWORD start, finish;
      if (m_debugWindow != NULL && m_debugWindow != (HWND)-1)
        SendMessage(m_debugWindow, EM_GETSEL, (WPARAM)&start, (LPARAM)&finish);
      else
        start = finish = 0;
      enableItems = MF_BYCOMMAND|(start == finish ? MF_GRAYED : MF_ENABLED);
      EnableMenuItem((HMENU)wParam, CopyMenuID, enableItems);
      EnableMenuItem((HMENU)wParam, CutMenuID, enableItems);
      EnableMenuItem((HMENU)wParam, DeleteMenuID, enableItems);

      enableItems = MF_BYCOMMAND|(IsServiceRunning(this) ? MF_ENABLED : MF_GRAYED);
      EnableMenuItem((HMENU)wParam, ControlMenuID, enableItems);
      break;
    }

    case WM_COMMAND :
      switch (wParam) {
        case ExitMenuID :
          DestroyWindow(hWnd);
          break;

        case ControlMenuID :
          if (IsServiceRunning(this))
            OnControl();
          break;

        case HideMenuID :
          ShowWindow(hWnd, SW_HIDE);
          break;

#if PMEMORY_HEAP
        case MarkMenuID :
          PMemoryHeap::GetState(memoryState);
          break;

        case DumpMenuID :
          PMemoryHeap::DumpObjectsSince(memoryState);
          break;

        case StatsMenuID :
          PMemoryHeap::DumpStatistics();
          break;
        case ValidateMenuID :
          PMemoryHeap::ValidateHeap();
          break;
#endif

        case CopyMenuID :
          if (m_debugWindow != NULL && m_debugWindow != (HWND)-1)
            SendMessage(m_debugWindow, WM_COPY, 0, 0);
          break;

        case CutMenuID :
          if (m_debugWindow != NULL && m_debugWindow != (HWND)-1)
            SendMessage(m_debugWindow, WM_CUT, 0, 0);
          break;

        case DeleteMenuID :
          if (m_debugWindow != NULL && m_debugWindow != (HWND)-1)
            SendMessage(m_debugWindow, EM_REPLACESEL, false, (LPARAM)emptyString);
          break;

        case SelectAllMenuID :
          if (m_debugWindow != NULL && m_debugWindow != (HWND)-1)
            SendMessage(m_debugWindow, EM_SETSEL, 0, -1);
          break;

        case OutputToMenuID :
          if (m_debugWindow != NULL && m_debugWindow != (HWND)-1) {
            char fileBuffer[_MAX_PATH];
            OPENFILENAME fileDlgInfo;
            memset(&fileDlgInfo, 0, sizeof(fileDlgInfo));
            fileDlgInfo.lStructSize = sizeof(fileDlgInfo);
            fileDlgInfo.hwndOwner = hWnd;
            fileDlgInfo.hInstance = hInstance;
            fileBuffer[0] = '\0';
            fileDlgInfo.lpstrFile = fileBuffer;
            char customFilter[100];
            strcpy(customFilter, "All Files");
            memcpy(&customFilter[strlen(customFilter)+1], "*.*\0", 5);
            fileDlgInfo.lpstrCustomFilter = customFilter;
            fileDlgInfo.nMaxCustFilter = sizeof(customFilter);
            fileDlgInfo.nMaxFile = sizeof(fileBuffer);
            fileDlgInfo.Flags = OFN_ENABLEHOOK|OFN_HIDEREADONLY|OFN_NOVALIDATE|OFN_EXPLORER|OFN_CREATEPROMPT;
            //fileDlgInfo.lCustData = (DWORD)this;
            if (GetSaveFileName(&fileDlgInfo)) {
              PFilePath newLogFile = fileBuffer;
              if (!PIsDescendant(&PSystemLog::GetTarget(), PSystemLogToFile) ||
                    ((PSystemLogToFile &)PSystemLog::GetTarget()).GetFilePath() != newLogFile) {
                PSystemLog::SetTarget(new PSystemLogToFile(newLogFile));
#if P_CONFIG_FILE
                PConfig cfg(ServiceSimulationSectionName);
                cfg.SetString(SystemLogFileNameKey, newLogFile);
#endif // P_CONFIG_FILE
                DebugOutput("Sending all system log output to \"" + newLogFile + "\".\n");
                PError << "Logging started for \"" << GetName() << "\" version " << GetVersion(true) << endl;
              }
            }
          }
          break;

        case WindowOutputMenuID :
          if (!PIsDescendant(&PSystemLog::GetTarget(), LogToWindow)) {
            PError << "Logging stopped." << endl;
            PSystemLog::SetTarget(new LogToWindow());
#if P_CONFIG_FILE
            PConfig cfg(ServiceSimulationSectionName);
            cfg.SetString(SystemLogFileNameKey, WindowLogOutput);
#endif // P_CONFIG_FILE
          }
          break;

        case DebuggerOutputMenuID :
          if (!PIsDescendant(&PSystemLog::GetTarget(), PSystemLogToDebug)) {
            PError << "Logging stopped." << endl;
            PSystemLog::SetTarget(new PSystemLogToDebug());
#if P_CONFIG_FILE
            PConfig cfg(ServiceSimulationSectionName);
            cfg.SetString(SystemLogFileNameKey, DebuggerLogOutput);
#endif // P_CONFIG_FILE
          }
          break;

        default :
          if (wParam >= LogLevelBaseMenuID+PSystemLog::Fatal && wParam < LogLevelBaseMenuID+PSystemLog::NumLogLevels)
            SetLogLevel((PSystemLog::Level)(wParam-LogLevelBaseMenuID));
          else if (wParam >= SvcCmdBaseMenuID && wParam < SvcCmdBaseMenuID+NumSvcCmds) {
            const char * cmdname = ServiceCommandNames[wParam-SvcCmdBaseMenuID];
            if (wParam == SvcCmdBaseMenuID+SvcCmdVersion ||
                MessageBox(hWnd, cmdname & GetName() & "?", GetName(),
                           MB_ICONQUESTION|MB_YESNO) == IDYES)
              ProcessCommand(cmdname);
          }
      }
      break;

    // Notification of event over sysTray icon
    case UWM_SYSTRAY :
      switch (lParam) {
        case WM_MOUSEMOVE :
          // update status of process for tool tips if no buttons down
          if (wParam == SYSTRAY_ICON_ID) {
            PNotifyIconData nid(hWnd, NIF_TIP,
                          GetName() & (IsServiceRunning(this) ? "is" : "not") & "running.");
            nid.Modify(); // Modify tooltip
          }
          break;

        // Click on icon - display message
        case WM_LBUTTONDBLCLK :
          if (IsServiceRunning(this))
            OnControl();
          else {
            SetForegroundWindow(hWnd); // Our MessageBox pops up in front
            MessageBox(hWnd, "Service is not running!", GetName(), MB_TASKMODAL);
          }
          break;

        // Popup menu
        case WM_RBUTTONUP :
          POINT pt;
          GetCursorPos(&pt);

          HMENU menu = CreatePopupMenu();
          AppendMenu(menu, MF_STRING, ControlMenuID, "&Open Properties");
          AppendMenu(menu, MF_SEPARATOR, 0, NULL);
          AppendMenu(menu, MF_STRING, SvcCmdBaseMenuID+SvcCmdVersion, "&Version");
          if (IsServiceRunning(this)) {
            MENUITEMINFO inf;
            inf.cbSize = sizeof(inf);
            inf.fMask = MIIM_STATE;
            inf.fState = MFS_DEFAULT;
            SetMenuItemInfo(menu, ControlMenuID, false, &inf);
            AppendMenu(menu, MF_STRING, SvcCmdBaseMenuID+SvcCmdStop, "&Stop Service");
          }
          else {
            EnableMenuItem(menu, ControlMenuID, MF_GRAYED);
            AppendMenu(menu, MF_STRING, SvcCmdBaseMenuID+SvcCmdStart, "&Start Service");
          }
          AppendMenu(menu, MF_STRING, SvcCmdBaseMenuID+SvcCmdNoTray, "&Tray Icon");
          CheckMenuItem(menu, SvcCmdBaseMenuID+SvcCmdNoTray,
                        TrayIconRegistry(this, CheckTrayIcon) ? MF_CHECKED : MF_UNCHECKED);
          AppendMenu(menu, MF_SEPARATOR, 0, NULL);
          AppendMenu(menu, MF_STRING, ExitMenuID, "&Close");

          /* SetForegroundWindow and the ensuing null PostMessage is a
             workaround for a Windows 95 bug (see MSKB article Q135788,
             http://www.microsoft.com/kb/articles/q135/7/88.htm, I think).
             In typical Microsoft style this bug is listed as "by design".
             SetForegroundWindow also causes our MessageBox to pop up in front
             of any other application's windows. */
          SetForegroundWindow(hWnd);

          /* We specifiy TPM_RETURNCMD, so TrackPopupMenu returns the menu
             selection instead of returning immediately and our getting a
             WM_COMMAND with the selection. You don't have to do it this way.
          */
          WndProc(hWnd, WM_COMMAND, TrackPopupMenu(menu,            // Popup menu to track
                                                   TPM_RETURNCMD |  // Return menu code
                                                   TPM_RIGHTBUTTON, // Track right mouse button?
                                                   pt.x, pt.y,      // screen coordinates
                                                   0,               // reserved
                                                   hWnd,            // owner
                                                   NULL),           // LPRECT user can click in without dismissing menu
                                                   0);
          PostMessage(hWnd, 0, 0, 0); // see above
          DestroyMenu(menu); // Delete loaded menu and reclaim its resources
          break;
      }
  }

  return DefWindowProc(hWnd, msg, wParam, lParam);
}

void PServiceProcess::DebugOutput(const char * out)
{
  if (m_controlWindow == NULL)
    return;

  if (m_debugWindow == NULL || m_debugWindow == (HWND)-1) {
    for (PINDEX i = 0; i < 3; i++) {
      const char * tab = strchr(out, '\t');
      if (tab == NULL)
        break;
      out = tab+1;
    }
    MessageBox(m_controlWindow, out, GetName(), MB_TASKMODAL);
    return;
  }


  if (!IsWindowVisible(m_controlWindow) && !m_debugHidden)
    ShowWindow(m_controlWindow, SW_SHOWDEFAULT);

  int len = strlen(out);
  while (GetWindowTextLength(m_debugWindow)+len >= 128000) {
    SendMessage(m_debugWindow, WM_SETREDRAW, false, 0);
    DWORD start, finish;
    SendMessage(m_debugWindow, EM_GETSEL, (WPARAM)&start, (LPARAM)&finish);
    SendMessage(m_debugWindow, EM_SETSEL, 0,
                SendMessage(m_debugWindow, EM_LINEINDEX, 1, 0));
    SendMessage(m_debugWindow, EM_REPLACESEL, false, (LPARAM)emptyString);
    SendMessage(m_debugWindow, EM_SETSEL, start, finish);
    SendMessage(m_debugWindow, WM_SETREDRAW, true, 0);
  }

  SendMessage(m_debugWindow, EM_SETSEL, 128000, 128000);
  const char * lf;
  const char * prev = out;
  while ((lf = strchr(prev, '\n')) != NULL) {
    if (lf == out || *(lf-1) != '\r') {
      size_t len = lf - out;
      char * line = (char *)alloca(len+3);
      memcpy(line, out, len);
      strcpy(line+len, crlfString);
      SendMessage(m_debugWindow, EM_REPLACESEL, false, (LPARAM)line);
      out = lf+1;
    }
    prev = lf+1;
  }
  if (*out != '\0')
    SendMessage(m_debugWindow, EM_REPLACESEL, false, (LPARAM)out);
}


void PServiceProcess::StaticMainEntry(DWORD argc, LPTSTR * argv)
{
  Current().MainEntry(argc, argv);
}


void PServiceProcess::MainEntry(DWORD argc, LPTSTR * argv)
{
  // SERVICE_STATUS members that don't change
  m_status.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
  m_status.dwServiceSpecificExitCode = 0;

  // register our service control handler:
  m_statusHandle = RegisterServiceCtrlHandler(GetName(), StaticControlEntry);
  if (m_statusHandle == FALSE)
    return;

  // report the status to Service Control Manager.
  if (!ReportStatus(SERVICE_START_PENDING, NO_ERROR, 1, 20000))
    return;

  // create the stop event object. The control handler function signals
  // this event when it receives the "stop" control code.
  m_terminationEvent = CreateEvent(NULL, true, false, GetName());
  if (m_terminationEvent == NULL)
    return;

  m_startedEvent = CreateEvent(NULL, true, false, NULL);
  if (m_startedEvent == NULL)
    return;

  GetArguments().SetArgs(argc, argv);

  // start the thread that performs the work of the service.
  m_threadHandle.Detach();
  m_threadHandle = (HANDLE)_beginthread(StaticThreadEntry, 0, this);
  if (m_threadHandle.IsValid()) {
    while (WaitForSingleObject(m_startedEvent, 10000) == WAIT_TIMEOUT) {
      if (!ReportStatus(SERVICE_START_PENDING, NO_ERROR, 1, 20000))
        return;
    }
    // Wait here for the end
    WaitForSingleObject(m_terminationEvent, INFINITE);
  }

  CloseHandle(m_startedEvent);
  CloseHandle(m_terminationEvent);
  ReportStatus(SERVICE_STOPPED, terminationValue);
}


void PServiceProcess::StaticThreadEntry(void * arg)
{
  ((PServiceProcess *)arg)->ThreadEntry();
}


void PServiceProcess::ThreadEntry()
{
  m_threadMutex.Wait();
  m_threadId = ::GetCurrentThreadId();
  m_activeThreads[m_threadId] = this;
  m_threadMutex.Signal();

  Startup();

  SetTerminationValue(1);
  bool ok = OnStart();

  // signal the above function to stop reporting the "start pending" status
  // and start waiting for the termination event
  if (!m_debugMode)
    SetEvent(m_startedEvent);

  // if the OnStart handler reported success, enter the main loop
  if (ok) {
    ReportStatus(SERVICE_RUNNING);
    SetTerminationValue(0);
    Main();
  }
  else {
    if (m_debugMode)
      DebugOutput("Failed to initialise service.\n");
  }

  ReportStatus(SERVICE_STOP_PENDING, terminationValue, 1, 30000);
  SetEvent(m_terminationEvent);
}


void PServiceProcess::StaticControlEntry(DWORD code)
{
  Current().ControlEntry(code);
}


void PServiceProcess::ControlEntry(DWORD code)
{
  switch (code) {
    case SERVICE_CONTROL_PAUSE : // Pause the service if it is running.
      if (m_status.dwCurrentState != SERVICE_RUNNING)
        ReportStatus(m_status.dwCurrentState);
      else {
        if (OnPause())
          ReportStatus(SERVICE_PAUSED);
      }
      break;

    case SERVICE_CONTROL_CONTINUE : // Resume the paused service.
      if (m_status.dwCurrentState == SERVICE_PAUSED)
        OnContinue();
      ReportStatus(m_status.dwCurrentState);
      break;

    case SERVICE_CONTROL_STOP : // Stop the service.
      // Report the status, specifying the checkpoint and waithint, before
      // setting the termination event.
      ReportStatus(SERVICE_STOP_PENDING, NO_ERROR, 1, 30000);
      OnStop();
      SetEvent(m_terminationEvent);
      break;

    case SERVICE_CONTROL_INTERROGATE : // Update the service status.
    default :
      ReportStatus(m_status.dwCurrentState);
  }
}


PBoolean PServiceProcess::ReportStatus(DWORD dwCurrentState,
                                   DWORD dwWin32ExitCode,
                                   DWORD dwCheckPoint,
                                   DWORD dwWaitHint)
{
  // Disable control requests until the service is started.
  if (dwCurrentState == SERVICE_START_PENDING)
    m_status.dwControlsAccepted = 0;
  else
    m_status.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_PAUSE_CONTINUE;

  // These SERVICE_STATUS members are set from parameters.
  m_status.dwCurrentState = dwCurrentState;
  m_status.dwWin32ExitCode = dwWin32ExitCode;
  m_status.dwCheckPoint = dwCheckPoint;
  m_status.dwWaitHint = dwWaitHint;

  if (m_debugMode)
    return true;

  // Report the status of the service to the service control manager.
  if (SetServiceStatus(m_statusHandle, &m_status))
    return true;

  // If an error occurs, stop the service.
  PSYSTEMLOG(Error, "SetServiceStatus failed");
  return false;
}


void PServiceProcess::Main()
{
  m_exitMain.Wait();
}


void PServiceProcess::OnStop()
{
  m_exitMain.Signal();
  PSYSTEMLOG(Warning, GetName() << " stopped.");
}


PBoolean PServiceProcess::OnPause()
{
  SuspendThread(m_threadHandle);
  return true;
}


void PServiceProcess::OnContinue()
{
  ResumeThread(m_threadHandle);
}


void PServiceProcess::OnControl()
{
}



class NT_ServiceManager
{
  public:
    NT_ServiceManager()  { schSCManager = schService = NULL; error = 0; }
    ~NT_ServiceManager();

    bool Create(PServiceProcess * svc);
    bool Remove(PServiceProcess * svc);
    bool Start(PServiceProcess * svc);
    bool Stop(PServiceProcess * svc)   { return Control(svc, SERVICE_CONTROL_STOP); }
    bool Pause(PServiceProcess * svc)  { return Control(svc, SERVICE_CONTROL_PAUSE); }
    bool Resume(PServiceProcess * svc) { return Control(svc, SERVICE_CONTROL_CONTINUE); }
    bool SetConfig(PServiceProcess * svc, SC_ACTION_TYPE action);
    bool IsInstalled(PServiceProcess * svc);
    bool IsRunning(PServiceProcess * svc);

    DWORD GetError() const { return error; }

  private:
    bool OpenManager();
    bool Open(PServiceProcess * svc);
    bool Control(PServiceProcess * svc, DWORD command);

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


bool NT_ServiceManager::OpenManager()
{
  schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
  if (schSCManager != NULL)
    return true;

  error = ::GetLastError();
  PError << "Could not open Service Manager, error=" << error << endl;
  return false;
}


bool NT_ServiceManager::Open(PServiceProcess * svc)
{
  if (!OpenManager())
    return false;

  schService = OpenService(schSCManager, svc->GetName(), SERVICE_ALL_ACCESS);
  if (schService != NULL)
    return true;

  error = ::GetLastError();
  PError << "Could not open service, error=" << error << endl;
  return false;
}


bool NT_ServiceManager::Create(PServiceProcess * svc)
{
  if (!OpenManager())
    return false;

  char binaryFilename[_MAX_PATH];
  PINDEX fnlen = GetShortPathName(svc->GetFile(), binaryFilename, sizeof(binaryFilename));
  schService = CreateService(
                    schSCManager,                   // SCManager database
                    svc->GetName(),                 // name of service
                    svc->GetName(),                 // name to display
                    SERVICE_ALL_ACCESS,             // desired access
                    SERVICE_WIN32_OWN_PROCESS,      // service type
                    SERVICE_AUTO_START,             // start type
                    SERVICE_ERROR_NORMAL,           // error control type
                    binaryFilename,                 // service's binary
                    NULL,                           // no load ordering group
                    NULL,                           // no tag identifier
                    svc->GetServiceDependencies(),  // no dependencies
                    NULL,                           // LocalSystem account
                    NULL);                          // no password
  if (schService == NULL) {
    error = ::GetLastError();
    PError << "Could not create service, error=" << error << endl;
    return false;
  }

  PString description = svc->GetDescription();
  if (!description.IsEmpty()) {
    SERVICE_DESCRIPTION sd;
    sd.lpDescription = (LPTSTR)description.GetPointer();
    if (!ChangeServiceConfig2(schService, SERVICE_CONFIG_DESCRIPTION, &sd)) {
      error = ::GetLastError();
      PError << "Could not set service description, error=" << error << endl;
    }
  }

  HKEY key;
  if ((error = RegCreateKey(HKEY_LOCAL_MACHINE,
             "SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application\\" +
                                       svc->GetName(), &key)) != ERROR_SUCCESS)
    return false;

  LPBYTE fn = (LPBYTE)binaryFilename;
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


bool NT_ServiceManager::Remove(PServiceProcess * svc)
{
  if (!Open(svc))
    return false;

  PString name = "SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application\\" + svc->GetName();
  error = ::RegDeleteKey(HKEY_LOCAL_MACHINE, (char *)(const char *)name);

  if (::DeleteService(schService))
    return true;

  error = ::GetLastError();
  PError << "Could not delete service, error=" << error << endl;
  return false;
}


bool NT_ServiceManager::Start(PServiceProcess * svc)
{
  if (!Open(svc))
    return false;

  if (::StartService(schService, 0, NULL)) {
    if (IsRunning(svc))
      return true;
    error = ERROR_SERVICE_NEVER_STARTED;
  }
  else {
    error = ::GetLastError();
    PError << "Could not start service, error=" << error << endl;
  }
  return false;
}


bool NT_ServiceManager::Control(PServiceProcess * svc, DWORD command)
{
  if (!Open(svc))
    return false;

  SERVICE_STATUS status;
  bool ok = ::ControlService(schService, command, &status);
  error = ::GetLastError();
  return ok;
}

bool NT_ServiceManager::SetConfig(PServiceProcess * svc, SC_ACTION_TYPE action)
{
  if (!Open(svc))
    return false;

  SC_ACTION scAction[4];
  PINDEX count;
  for (count = 0; count < (PINDEX)(sizeof(scAction)/sizeof(scAction[0])-1); ++count) {
    scAction[count].Type = action;
    scAction[count].Delay = 1000;
  }
  scAction[count].Type = SC_ACTION_NONE;
  scAction[count].Delay = 1000;

  SERVICE_FAILURE_ACTIONS sfActions;
  sfActions.dwResetPeriod = 300; // 5 minutes
  sfActions.lpRebootMsg = emptyString;
  sfActions.lpCommand = emptyString;
  sfActions.cActions = ++count;
  sfActions.lpsaActions = scAction;

  return ChangeServiceConfig2(schService, SERVICE_CONFIG_FAILURE_ACTIONS, &sfActions);
}


bool NT_ServiceManager::IsInstalled(PServiceProcess * svc)
{
  return Open(svc);
}


bool NT_ServiceManager::IsRunning(PServiceProcess * svc)
{
  if (!Open(svc))
    return false;

  SERVICE_STATUS serviceStatus;

  // query the service status
  if (!QueryServiceStatus(schService, &serviceStatus)) {
    error = ::GetLastError();
    PError << "QueryServiceStatus failed, error=" << error << endl;
    return false;
  }

  // if pending periodicaly re-query the status
  while (serviceStatus.dwCurrentState == SERVICE_START_PENDING) {
    DWORD waitTime = std::min(serviceStatus.dwWaitHint / 10, 10000UL);
    Sleep(waitTime);

    if (!QueryServiceStatus(schService, &serviceStatus)) {
      error = ::GetLastError();
      PError << "QueryServiceStatus failed, error=" << error << endl;
      return false;
    }
  }

  if (serviceStatus.dwCurrentState == SERVICE_RUNNING)
    return true;

  PError << "Service nor running." << endl;
  error = serviceStatus.dwWin32ExitCode;
  return false;
}



bool PServiceProcess::ProcessCommand(const char * cmd)
{
  if (*cmd == '/')
    ++cmd;
  else if (cmd[0] == '-' && cmd[1] == '-')
    cmd += 2;

  PINDEX cmdNum = 0;
  while (strcasecmp(cmd, ServiceCommandNames[cmdNum]) != 0) {
    if (++cmdNum >= NumSvcCmds)
      break;
  }

  NT_ServiceManager svcManager;

  PStringStream msg;
  bool good = false;

  switch (cmdNum) {
    case SvcCmdNoWindow :
      if (m_controlWindow == NULL)
        m_controlWindow = (HWND)-1;
      return true;

    case SvcCmdTray :
      if (CreateControlWindow(false)) {
        PNotifyIconData nid(m_controlWindow, NIF_MESSAGE|NIF_ICON, GetName());
        nid.hIcon = (HICON)LoadImage(hInstance, MAKEINTRESOURCE(ICON_RESID), IMAGE_ICON, // 16x16 icon
                               GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0);
        nid.uCallbackMessage = UWM_SYSTRAY; // message sent to nid.hWnd
        nid.Add();    // This adds the icon
        m_debugWindow = (HWND)-1;
        return true;
      }
      return false;

    case SvcCmdNoTray :
      if (TrayIconRegistry(this, CheckTrayIcon)) {
        TrayIconRegistry(this, DelTrayIcon);
        PError << "Tray icon removed.";
      }
      else {
        TrayIconRegistry(this, AddTrayIcon);
        PError << "Tray icon installed.";
      }
      return true;

    case SvcCmdVersion : // Version command
      msg << GetName() << " Version " << GetVersion(true)
          << " by " << GetManufacturer()
          << " on " << GetOSClass()   << ' ' + GetOSName()
          << " ("   << GetOSVersion() << '-' + GetOSHardware() << ')';
      break;

    case SvcCmdDefault : // run app with no params.
      if (svcManager.IsInstalled(this)) {
        if (svcManager.IsRunning(this)) {
          if (MessageBox(NULL, "Do you wish to stop the service?", GetName(), MB_YESNO|MB_TASKMODAL) != IDYES)
            return false;

          good = svcManager.Stop(this);
        }
        else {
          switch (MessageBox(NULL,
                             "Do you wish to start (Yes) or deinstall (No) the service?",
                             GetName(),
                             MB_YESNOCANCEL|MB_TASKMODAL)) {
            case IDYES :
              good = svcManager.Start(this);
              break;

            case IDNO :
              good = svcManager.Remove(this);
              TrayIconRegistry(this, DelTrayIcon);
              break;

            case IDCANCEL :
              return false;
          }
        }
      }
      else if (svcManager.GetError() != ERROR_ACCESS_DENIED) {
        if (MessageBox(NULL, "Do you wish to install & start the service?", GetName(), MB_YESNO|MB_TASKMODAL) != IDYES)
          return false;

        good = svcManager.Create(this);
        if (good) {
          TrayIconRegistry(this, AddTrayIcon);
          good = svcManager.Start(this);
        }
      }
      else {
        MessageBox(NULL, "Access denied, please run as Administrator!", GetName(), MB_TASKMODAL);
        return false;
      }
      break;

    case SvcCmdInstall : // install
      good = svcManager.Create(this);
      if (good)
        TrayIconRegistry(this, AddTrayIcon);
      break;

    case SvcCmdRemove : // remove
      good = svcManager.Remove(this);
      TrayIconRegistry(this, DelTrayIcon);
      break;

    case SvcCmdStart : // start
      good = svcManager.Start(this);
      break;

    case SvcCmdStop : // stop
      good = svcManager.Stop(this);
      break;

    case SvcCmdPause : // pause
      good = svcManager.Pause(this);
      break;

    case SvcCmdResume : // resume
      good = svcManager.Resume(this);
      break;

    case SvcCmdAutoRestart :
      good = svcManager.SetConfig(this, SC_ACTION_RESTART);
      break;

    case SvcCmdNoRestart :
      good = svcManager.SetConfig(this, SC_ACTION_NONE);
      break;

    case SvcCmdDeinstall : // deinstall
      svcManager.Remove(this);
      TrayIconRegistry(this, DelTrayIcon);
#if P_CONFIG_FILE
      {
        PConfig cfg;
        PStringArray sections = cfg.GetSections();
        PINDEX i;
        for (i = 0; i < sections.GetSize(); i++)
          cfg.DeleteSection(sections[i]);
      }
#endif // P_CONFIG_FILE
      good = true;
      break;

    default :
      if (cmdNum != SvcCmdHelp)
        msg << "Unknown command \"" << cmd << "\".\r\n\r\n";
      msg << "usage: " << GetFile().GetTitle() << " [ ";
      for (cmdNum = 0; cmdNum < NumSvcCmds-1; cmdNum++)
        msg << ServiceCommandNames[cmdNum] << " | ";
      msg << ServiceCommandNames[cmdNum] << " ]" << endl;
  }

  SetLastError(0);

  if (msg.IsEmpty()) {
    msg << "Service command ";
    if (cmdNum != SvcCmdDefault)
      msg << '"' << ServiceCommandNames[cmdNum] << "\" ";
    if (good)
      msg << "successful.";
    else {
      msg << "failed - ";
      switch (svcManager.GetError()) {
        case ERROR_SUCCESS :
        case ERROR_ACCESS_DENIED :
          msg << "Access denied";
          break;
        case ERROR_SERVICE_EXISTS :
          msg << "Already installed.";
          break;
        default :
          msg << "Error code = " << svcManager.GetError();
      }
    }
  }

  if (m_controlWindow == NULL)
    MessageBox(NULL, msg, GetName(), MB_TASKMODAL);
  else
    PError << msg << endl;

  return good;
}

void PServiceProcess::SetDescription(const PString & description)
{
  m_description = description;
}

const PString & PServiceProcess::GetDescription() const
{
  return m_description;
}

#endif // !_WIN32_WCE

// End Of File ///////////////////////////////////////////////////////////////
