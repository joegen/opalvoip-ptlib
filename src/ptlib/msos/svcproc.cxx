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
 *
 * $Revision$
 * $Author$
 * $Date$
 */

#include <ptlib.h>

#include <winuser.h>
#include <winnls.h>
#include <shellapi.h>
#include <commdlg.h>

#include <process.h>
#include <signal.h>
#include <fcntl.h>
#include <io.h>

#pragma comment(lib,"advapi32.lib")
#pragma comment(lib,"user32.lib")
#pragma comment(lib,"comdlg32.lib")

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
  SvcCmdNoWindow,
  NumSvcCmds
};

static const char * const ServiceCommandNames[NumSvcCmds] = {
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
  "NoWin"
};


static const char WindowLogOutput[] = "Window Log Output";
static const char DebuggerLogOutput[] = "Debugger Log Output";


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

static PBoolean TrayIconRegistry(PServiceProcess * svc, TrayIconRegistryCommand cmd)
{
  HKEY key;
  if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                   "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run",
                   0, KEY_ALL_ACCESS, &key) != ERROR_SUCCESS)
    return PFalse;

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
// PSystemLog

void PSystemLog::Output(Level level, const char * msg)
{
  PServiceProcess & process = PServiceProcess::Current();
  if (level > process.GetLogLevel())
    return;

  DWORD err = ::GetLastError();

  if (process.isWin95 || process.controlWindow != NULL) {
    static HANDLE mutex = CreateMutex(NULL, PFalse, NULL);
    WaitForSingleObject(mutex, INFINITE);

    ostream * out;
    if (process.systemLogFileName == WindowLogOutput || process.systemLogFileName == DebuggerLogOutput)
      out = new PStringStream;
    else
      out = new ofstream(process.systemLogFileName, ios::app);

    PTime now;
    *out << now.AsString("yyyy/MM/dd hh:mm:ss.uuu\t");
    PThread * thread = PThread::Current();
    PString threadName;
    if (thread == NULL)
      threadName.sprintf("ThreadID" PTHREAD_ID_FMT, GetCurrentThreadId());
    else
      threadName = thread->GetThreadName();
    if (threadName.GetLength() <= 23)
      *out << setw(23) << threadName;
    else
      *out << threadName.Left(10) << "..." << threadName.Right(10);

    *out << '\t';
    if (level < 0)
      *out << "Message";
    else {
      static const char * const levelName[4] = {
        "Fatal error",
        "Error",
        "Warning",
        "Info"
      };
      if (level < PARRAYSIZE(levelName))
        *out << levelName[level];
      else
        *out << "Debug" << (level-Info);
    }

    *out << '\t' << msg;
    if (level < Info && err != 0)
      *out << " - error = " << err << endl;
    else if (msg[0] == '\0' || msg[strlen(msg)-1] != '\n')
      *out << endl;

    if (process.systemLogFileName == WindowLogOutput)
      process.DebugOutput(*(PStringStream*)out);
    else if (process.systemLogFileName == DebuggerLogOutput)
      OutputDebugStringA(*(PStringStream*)out);

    delete out;
    ReleaseMutex(mutex);
    SetLastError(0);
  }
  else {
    // Use event logging to log the error.
    HANDLE hEventSource = RegisterEventSource(NULL, process.GetName());
    if (hEventSource == NULL)
      return;

    PString threadName;
    PThread * thread = PThread::Current();
    if (thread != NULL)
      threadName = thread->GetThreadName();
    else
      threadName.sprintf("%u", GetCurrentThreadId());

    char thrdbuf[16];
    if (threadName.IsEmpty())
      sprintf(thrdbuf, "0x%08X", thread);
    else {
      strncpy(thrdbuf, threadName, sizeof(thrdbuf)-1);
      thrdbuf[sizeof(thrdbuf)-1] = '\0';
    }

    char errbuf[25];
    if (level > StdError && level < Info && err != 0)
      ::sprintf(errbuf, "Error code = %d", err);
    else
      errbuf[0] = '\0';

    LPCTSTR strings[4];
    strings[0] = thrdbuf;
    strings[1] = msg;
    strings[2] = errbuf;
    strings[3] = level != Fatal ? "" : " Program aborted.";

    static const WORD levelType[Info+1] = {
      EVENTLOG_INFORMATION_TYPE,
      EVENTLOG_ERROR_TYPE,
      EVENTLOG_ERROR_TYPE,
      EVENTLOG_WARNING_TYPE
    };
    ReportEvent(hEventSource, // handle of event source
                (WORD)(level < Info ? levelType[level+1]
                                    : EVENTLOG_INFORMATION_TYPE), // event type
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
    PMEMORY_IGNORE_ALLOCATIONS_FOR_SCOPE;

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
  Level logLevel = log->logLevel;

#if PTRACING
  if (log->width() != 0 &&(PTrace::GetOptions()&PTrace::SystemLogStream) != 0) {
    // Trace system sets the ios stream width as the last thing it does before
    // doing a flush, which gets us here. SO now we can get a PTRACE looking
    // exactly like a PSYSTEMLOG of appropriate level.
    unsigned traceLevel = log->width() -1 + PSystemLog::Warning;
    log->width(0);
    if (traceLevel >= PSystemLog::NumLogLevels)
      traceLevel = PSystemLog::NumLogLevels-1;
    logLevel = (Level)traceLevel;
  }
#endif

  PSystemLog::Output(logLevel, string);

  PMEMORY_IGNORE_ALLOCATIONS_FOR_SCOPE;

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
  : PProcess(manuf, name, major, minor, stat, build),
    systemLogFileName(GetFile().GetDirectory() + GetName() + " Log.TXT")
{
  controlWindow = debugWindow = NULL;
  currentLogLevel = PSystemLog::Warning;
}


PServiceProcess & PServiceProcess::Current()
{
  PServiceProcess & process = (PServiceProcess &)PProcess::Current();
  PAssert(PIsDescendant(&process, PServiceProcess), "Not a service!");
  return process;
}


const char * PServiceProcess::GetServiceDependencies() const
{
  return "EventLog\0";
}


PBoolean PServiceProcess::IsServiceProcess() const
{
  return PTrue;
}


static PBoolean IsServiceRunning(PServiceProcess * svc)
{
  HANDLE hEvent = OpenEvent(EVENT_MODIFY_STATE, PFalse, svc->GetName());
  if (hEvent == NULL)
    return ::GetLastError() == ERROR_ACCESS_DENIED;

  CloseHandle(hEvent);
  return PTrue;
}


int PServiceProcess::_main(void * arg)
{
  {
    PMEMORY_IGNORE_ALLOCATIONS_FOR_SCOPE;

    PSetErrorStream(new PSystemLog(PSystemLog::StdError));
#if PTRACING
    PTrace::SetStream(new PSystemLog(PSystemLog::Debug3));
    PTrace::ClearOptions(PTrace::FileAndLine);
    PTrace::SetOptions(PTrace::SystemLogStream);
    PTrace::SetLevel(4);
#endif
  }

  hInstance = (HINSTANCE)arg;

  OSVERSIONINFO verinfo;
  verinfo.dwOSVersionInfoSize = sizeof(verinfo);
  GetVersionEx(&verinfo);
  switch (verinfo.dwPlatformId) {
    case VER_PLATFORM_WIN32_NT :
      isWin95 = PFalse;
      break;
    case VER_PLATFORM_WIN32_WINDOWS :
      isWin95 = PTrue;
      break;
    default :
      PError << "Unsupported Win32 platform type!" << endl;
      return 1;
  }

  debugMode = arguments.GetCount() > 0 && 
	              (strcasecmp(arguments[0], "Debug") == 0 || strcasecmp(arguments[0], "foreground") == 0);
  debugHidden = arguments.GetCount() > 0 && strcasecmp(arguments[0], "DebugHidden") == 0;
  if (debugHidden)
    debugMode=TRUE;
  currentLogLevel = debugMode ? PSystemLog::Info : PSystemLog::Warning;

  if (!debugMode && arguments.GetCount() > 0) {
    for (PINDEX a = 0; a < arguments.GetCount(); a++)
      ProcessCommand(arguments[a]);

    if (controlWindow == NULL || controlWindow == (HWND)-1)
      return GetTerminationValue();

    if (debugWindow != NULL && debugWindow != (HWND)-1) {
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

  if (!debugMode && !isWin95) {
    static SERVICE_TABLE_ENTRY dispatchTable[] = {
      { (char*) "", PServiceProcess::StaticMainEntry },
      { NULL, NULL }
    };
    dispatchTable[0].lpServiceName = (char *)(const char *)GetName();

    if (StartServiceCtrlDispatcher(dispatchTable))
      return GetTerminationValue();

    PSystemLog::Output(PSystemLog::Fatal, "StartServiceCtrlDispatcher failed.");
    MessageBox(NULL, "Not run as a service!", GetName(), MB_TASKMODAL);
    return 1;
  }

  if (!CreateControlWindow(debugMode))
    return 1;

  if (IsServiceRunning(this)) {
    MessageBox(NULL, "Service already running", GetName(), MB_TASKMODAL);
    return 3;
  }

  if (debugMode) {
    ::SetLastError(0);
    PError << "Service simulation started for \"" << GetName() << "\" version " << GetVersion(PTrue) << "\n"
              "Close window to terminate.\n" << endl;
  }

  terminationEvent = CreateEvent(NULL, PTrue, PFalse, GetName());
  PAssertOS(terminationEvent != NULL);

  threadHandle = (HANDLE)_beginthread(StaticThreadEntry, 0, this);
  PAssertOS(threadHandle != (HANDLE)-1);

  SetTerminationValue(0);

  MSG msg;
  msg.message = WM_QUIT+1; //Want somethingthat is not WM_QUIT
  do {
    switch (MsgWaitForMultipleObjects(1, &terminationEvent,
                                      PFalse, INFINITE, QS_ALLINPUT)) {
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
  activeThreadMutex.Wait();
  activeThreads.SetAt(threadId, NULL);
  threadId = GetCurrentThreadId();
  threadHandle = GetCurrentThread();
  activeThreads.SetAt(threadId, this);
  activeThreadMutex.Signal();
  OnStop();

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
  if (controlWindow != NULL)
    return PTrue;

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
    return PFalse;

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
    return PFalse;

  if (createDebugWindow && debugWindow == NULL) {
#if P_CONFIG_FILE
    PConfig cfg(ServiceSimulationSectionName);
    int l = cfg.GetInteger(WindowLeftKey, -1);
    int t = cfg.GetInteger(WindowTopKey, -1);
    int r = cfg.GetInteger(WindowRightKey, -1);
    int b = cfg.GetInteger(WindowBottomKey, -1);
    if (l > 0 && t > 0 && r > 0 && b > 0)
      SetWindowPos(controlWindow, NULL, l, t, r-l, b-t, 0);
#endif // P_CONFIG_FILE

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
    DWORD TabStops[] = {
      DATE_WIDTH,
      DATE_WIDTH+THREAD_WIDTH,
      DATE_WIDTH+THREAD_WIDTH+LEVEL_WIDTH,
      DATE_WIDTH+THREAD_WIDTH+LEVEL_WIDTH+PROTO_WIDTH,
      DATE_WIDTH+THREAD_WIDTH+LEVEL_WIDTH+PROTO_WIDTH+ACTION_WIDTH,
      DATE_WIDTH+THREAD_WIDTH+LEVEL_WIDTH+PROTO_WIDTH+ACTION_WIDTH+32  // Standard tab width
    };
    SendMessage(debugWindow, EM_SETTABSTOPS, PARRAYSIZE(TabStops), (LPARAM)(LPDWORD)TabStops);

#if P_CONFIG_FILE
    systemLogFileName = cfg.GetString(SystemLogFileNameKey);
#endif // P_CONFIG_FILE
    if (systemLogFileName.IsEmpty())
      systemLogFileName = WindowLogOutput;
    if (systemLogFileName != WindowLogOutput) {
      if (systemLogFileName != DebuggerLogOutput)
        PFile::Remove(systemLogFileName);
      DebugOutput("Sending all system log output to \"" + systemLogFileName + "\".\n");
    }
  }

  return PTrue;
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
      controlWindow = hWnd;
      break;

    case WM_DESTROY :
      if (debugWindow == (HWND)-1) {
        PNotifyIconData nid(hWnd, NIF_TIP);
        nid.Delete(); // This removes the systray icon
      }

      controlWindow = debugWindow = NULL;

      PostQuitMessage(0);
      break;

    case WM_ENDSESSION :
      if (wParam && (debugMode || lParam != ENDSESSION_LOGOFF) && debugWindow != (HWND)-1)
        OnStop();
      return 0;

#if P_CONFIG_FILE
    case WM_MOVE :
      if (debugWindow != NULL)
        SaveWindowPosition(hWnd);
      break;
#endif // P_CONFIG_FILE

    case WM_SIZE :
      if (debugWindow != NULL && debugWindow != (HWND)-1) {
#if P_CONFIG_FILE
        SaveWindowPosition(hWnd);
#endif // P_CONFIG_FILE
        MoveWindow(debugWindow, 0, 0, LOWORD(lParam), HIWORD(lParam), PTrue);
      }
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
      if (debugWindow != NULL && debugWindow != (HWND)-1)
        SendMessage(debugWindow, EM_GETSEL, (WPARAM)&start, (LPARAM)&finish);
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
          if (debugWindow != NULL && debugWindow != (HWND)-1)
            SendMessage(debugWindow, WM_COPY, 0, 0);
          break;

        case CutMenuID :
          if (debugWindow != NULL && debugWindow != (HWND)-1)
            SendMessage(debugWindow, WM_CUT, 0, 0);
          break;

        case DeleteMenuID :
          if (debugWindow != NULL && debugWindow != (HWND)-1)
            SendMessage(debugWindow, WM_CLEAR, 0, 0);
          break;

        case SelectAllMenuID :
          if (debugWindow != NULL && debugWindow != (HWND)-1)
            SendMessage(debugWindow, EM_SETSEL, 0, -1);
          break;

        case OutputToMenuID :
          if (debugWindow != NULL && debugWindow != (HWND)-1) {
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
            fileDlgInfo.lCustData = (DWORD)this;
            if (GetSaveFileName(&fileDlgInfo)) {
              if (systemLogFileName != fileBuffer) {
                systemLogFileName = fileBuffer;
                PFile::Remove(systemLogFileName);
#if P_CONFIG_FILE
                PConfig cfg(ServiceSimulationSectionName);
                cfg.SetString(SystemLogFileNameKey, systemLogFileName);
#endif // P_CONFIG_FILE
                DebugOutput("Sending all system log output to \"" + systemLogFileName + "\".\n");
                PError << "Logging started for \"" << GetName() << "\" version " << GetVersion(PTrue) << endl;
              }
            }
          }
          break;

        case WindowOutputMenuID :
          if (systemLogFileName != WindowLogOutput) {
            PError << "Logging stopped." << endl;
            DebugOutput("System log output to \"" + systemLogFileName + "\" stopped.\n");
            systemLogFileName = WindowLogOutput;
#if P_CONFIG_FILE
            PConfig cfg(ServiceSimulationSectionName);
            cfg.SetString(SystemLogFileNameKey, systemLogFileName);
#endif // P_CONFIG_FILE
          }
          break;

        case DebuggerOutputMenuID :
          if (systemLogFileName != DebuggerLogOutput) {
            PError << "Logging stopped." << endl;
            DebugOutput("System log output to \"" + systemLogFileName + "\" stopped.\n");
            systemLogFileName = DebuggerLogOutput;
#if P_CONFIG_FILE
            PConfig cfg(ServiceSimulationSectionName);
            cfg.SetString(SystemLogFileNameKey, systemLogFileName);
#endif // P_CONFIG_FILE
          }
          break;

        default :
          if (wParam >= LogLevelBaseMenuID+PSystemLog::Fatal && wParam < LogLevelBaseMenuID+PSystemLog::NumLogLevels) {
            SetLogLevel((PSystemLog::Level)(wParam-LogLevelBaseMenuID));
#if PTRACING
            PTrace::SetLevel(wParam-LogLevelBaseMenuID-PSystemLog::Warning);
#endif
          }
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
            SetMenuItemInfo(menu, ControlMenuID, PFalse, &inf);
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
  if (controlWindow == NULL)
    return;

  if (debugWindow == NULL || debugWindow == (HWND)-1) {
    for (PINDEX i = 0; i < 3; i++) {
      const char * tab = strchr(out, '\t');
      if (tab == NULL)
        break;
      out = tab+1;
    }
    MessageBox(controlWindow, out, GetName(), MB_TASKMODAL);
    return;
  }


  if (!IsWindowVisible(controlWindow) && !debugHidden)
    ShowWindow(controlWindow, SW_SHOWDEFAULT);

  int len = strlen(out);
  int max = isWin95 ? 32000 : 128000;
  while (GetWindowTextLength(debugWindow)+len >= max) {
    SendMessage(debugWindow, WM_SETREDRAW, PFalse, 0);
    DWORD start, finish;
    SendMessage(debugWindow, EM_GETSEL, (WPARAM)&start, (LPARAM)&finish);
    SendMessage(debugWindow, EM_SETSEL, 0,
                SendMessage(debugWindow, EM_LINEINDEX, 1, 0));
    SendMessage(debugWindow, EM_REPLACESEL, PFalse, (DWORD)"");
    SendMessage(debugWindow, EM_SETSEL, start, finish);
    SendMessage(debugWindow, WM_SETREDRAW, PTrue, 0);
  }

  SendMessage(debugWindow, EM_SETSEL, max, max);
  char * lf;
  char * prev = (char *)out;
  while ((lf = strchr(prev, '\n')) != NULL) {
    if (*(lf-1) == '\r')
      prev = lf+1;
    else {
      *lf++ = '\0';
      SendMessage(debugWindow, EM_REPLACESEL, PFalse, (DWORD)out);
      SendMessage(debugWindow, EM_REPLACESEL, PFalse, (DWORD)"\r\n");
      out = (const char *)lf;
      prev = lf;
    }
  }
  if (*out != '\0')
    SendMessage(debugWindow, EM_REPLACESEL, PFalse, (DWORD)out);
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
  if (statusHandle == FALSE)
    return;

  // report the status to Service Control Manager.
  if (!ReportStatus(SERVICE_START_PENDING, NO_ERROR, 1, 20000))
    return;

  // create the stop event object. The control handler function signals
  // this event when it receives the "stop" control code.
  terminationEvent = CreateEvent(NULL, PTrue, PFalse, (const char *)GetName());
  if (terminationEvent == NULL)
    return;

  startedEvent = CreateEvent(NULL, PTrue, PFalse, NULL);
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
  ReportStatus(SERVICE_STOPPED, terminationValue);
}


void PServiceProcess::StaticThreadEntry(void * arg)
{
  ((PServiceProcess *)arg)->ThreadEntry();
}


void PServiceProcess::ThreadEntry()
{
  activeThreadMutex.Wait();
  threadId = GetCurrentThreadId();
  threadHandle = GetCurrentThread();
  activeThreads.SetAt(threadId, this);
  activeThreadMutex.Signal();

  SetTerminationValue(1);
  PBoolean ok = OnStart();

  // signal the above function to stop reporting the "start pending" status
  // and start waiting for the termination event
  if (!debugMode)
    SetEvent(startedEvent);

  // if the OnStart handler reported success, enter the main loop
  if (ok) {
    ReportStatus(SERVICE_RUNNING);
    SetTerminationValue(0);
    Main();
  }

  ReportStatus(SERVICE_STOP_PENDING, terminationValue, 1, 30000);
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


PBoolean PServiceProcess::ReportStatus(DWORD dwCurrentState,
                                   DWORD dwWin32ExitCode,
                                   DWORD dwCheckPoint,
                                   DWORD dwWaitHint)
{
  // Disable control requests until the service is started.
  if (dwCurrentState == SERVICE_START_PENDING)
    status.dwControlsAccepted = 0;
  else
    status.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_PAUSE_CONTINUE;

  // These SERVICE_STATUS members are set from parameters.
  status.dwCurrentState = dwCurrentState;
  status.dwWin32ExitCode = dwWin32ExitCode;
  status.dwCheckPoint = dwCheckPoint;
  status.dwWaitHint = dwWaitHint;

  if (debugMode || isWin95)
    return PTrue;

  // Report the status of the service to the service control manager.
  if (SetServiceStatus(statusHandle, &status))
    return PTrue;

  // If an error occurs, stop the service.
  PSystemLog::Output(PSystemLog::Error, "SetServiceStatus failed");
  return PFalse;
}


void PServiceProcess::OnStop()
{
}


PBoolean PServiceProcess::OnPause()
{
  SuspendThread(threadHandle);
  return PTrue;
}


void PServiceProcess::OnContinue()
{
  ResumeThread(threadHandle);
}


void PServiceProcess::OnControl()
{
}



class ServiceManager
{
  public:
    ServiceManager()  { error = 0; }

    virtual PBoolean Create(PServiceProcess * svc) = 0;
    virtual PBoolean Delete(PServiceProcess * svc) = 0;
    virtual PBoolean Start(PServiceProcess * svc) = 0;
    virtual PBoolean Stop(PServiceProcess * svc) = 0;
    virtual PBoolean Pause(PServiceProcess * svc) = 0;
    virtual PBoolean Resume(PServiceProcess * svc) = 0;

    DWORD GetError() const { return error; }

  protected:
    DWORD error;
};


class Win95_ServiceManager : public ServiceManager
{
  public:
    virtual PBoolean Create(PServiceProcess * svc);
    virtual PBoolean Delete(PServiceProcess * svc);
    virtual PBoolean Start(PServiceProcess * svc);
    virtual PBoolean Stop(PServiceProcess * svc);
    virtual PBoolean Pause(PServiceProcess * svc);
    virtual PBoolean Resume(PServiceProcess * svc);
};


PBoolean Win95_ServiceManager::Create(PServiceProcess * svc)
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
    return PFalse;

  PString cmd = "\"" + svc->GetFile() + "\"";
  error = RegSetValueEx(key, svc->GetName(), 0, REG_SZ,
                        (LPBYTE)(const char *)cmd, cmd.GetLength() + 1);

  RegCloseKey(key);

  return error == ERROR_SUCCESS;
}


PBoolean Win95_ServiceManager::Delete(PServiceProcess * svc)
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
    return PFalse;

  error = RegDeleteValue(key, (char *)(const char *)svc->GetName());

  RegCloseKey(key);

  return error == ERROR_SUCCESS;
}


PBoolean Win95_ServiceManager::Start(PServiceProcess * service)
{
  if (IsServiceRunning(service)) {
    PError << "Service already running" << endl;
    error = 1;
    return PFalse;
  }

  PBoolean ok = _spawnl(_P_DETACH, service->GetFile(), service->GetFile(), NULL) >= 0;
  error = errno;
  return ok;
}


PBoolean Win95_ServiceManager::Stop(PServiceProcess * service)
{
  HANDLE hEvent = OpenEvent(EVENT_MODIFY_STATE, PFalse, service->GetName());
  if (hEvent == NULL) {
    error = ::GetLastError();
    PError << "Service is not running" << endl;
    return PFalse;
  }

  SetEvent(hEvent);
  CloseHandle(hEvent);

  // Wait for process to go away.
  for (PINDEX i = 0; i < 20; i++) {
    hEvent = OpenEvent(EVENT_MODIFY_STATE, PFalse, service->GetName());
    if (hEvent == NULL)
      return PTrue;
    CloseHandle(hEvent);
    ::Sleep(500);
  }

  error = 0x10000000;
  return PFalse;
}


PBoolean Win95_ServiceManager::Pause(PServiceProcess *)
{
  PError << "Cannot pause service under Windows 95" << endl;
  error = 1;
  return PFalse;
}


PBoolean Win95_ServiceManager::Resume(PServiceProcess *)
{
  PError << "Cannot resume service under Windows 95" << endl;
  error = 1;
  return PFalse;
}



class NT_ServiceManager : public ServiceManager
{
  public:
    NT_ServiceManager()  { schSCManager = schService = NULL; }
    ~NT_ServiceManager();

    PBoolean Create(PServiceProcess * svc);
    PBoolean Delete(PServiceProcess * svc);
    PBoolean Start(PServiceProcess * svc);
    PBoolean Stop(PServiceProcess * svc)
      { return Control(svc, SERVICE_CONTROL_STOP); }
    PBoolean Pause(PServiceProcess * svc)
      { return Control(svc, SERVICE_CONTROL_PAUSE); }
    PBoolean Resume(PServiceProcess * svc)
      { return Control(svc, SERVICE_CONTROL_CONTINUE); }

    DWORD GetError() const { return error; }

  private:
    PBoolean OpenManager();
    PBoolean Open(PServiceProcess * svc);
    PBoolean Control(PServiceProcess * svc, DWORD command);

    SC_HANDLE schSCManager, schService;
};


NT_ServiceManager::~NT_ServiceManager()
{
  if (schService != NULL)
    CloseServiceHandle(schService);
  if (schSCManager != NULL)
    CloseServiceHandle(schSCManager);
}


PBoolean NT_ServiceManager::OpenManager()
{
  schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
  if (schSCManager != NULL)
    return PTrue;

  error = ::GetLastError();
  PError << "Could not open Service Manager." << endl;
  return PFalse;
}


PBoolean NT_ServiceManager::Open(PServiceProcess * svc)
{
  if (!OpenManager())
    return PFalse;

  schService = OpenService(schSCManager, svc->GetName(), SERVICE_ALL_ACCESS);
  if (schService != NULL)
    return PTrue;

  error = ::GetLastError();
  PError << "Service is not installed." << endl;
  return PFalse;
}


PBoolean NT_ServiceManager::Create(PServiceProcess * svc)
{
  if (!OpenManager())
    return PFalse;

  schService = OpenService(schSCManager, svc->GetName(), SERVICE_ALL_ACCESS);
  if (schService != NULL) {
    PError << "Service is already installed." << endl;
    return PFalse;
  }

  PString binaryFilename;
  GetShortPathName(svc->GetFile(), binaryFilename.GetPointer(_MAX_PATH), _MAX_PATH);
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
    return PFalse;
  }

  HKEY key;
  if ((error = RegCreateKey(HKEY_LOCAL_MACHINE,
             "SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application\\" +
                                       svc->GetName(), &key)) != ERROR_SUCCESS)
    return PFalse;

  LPBYTE fn = (LPBYTE)(const char *)binaryFilename;
  PINDEX fnlen = binaryFilename.GetLength()+1;
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


PBoolean NT_ServiceManager::Delete(PServiceProcess * svc)
{
  if (!Open(svc))
    return PFalse;

  PString name = "SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application\\" + svc->GetName();
  error = ::RegDeleteKey(HKEY_LOCAL_MACHINE, (char *)(const char *)name);

  if (!::DeleteService(schService))
    error = ::GetLastError();

  return error == ERROR_SUCCESS;
}


PBoolean NT_ServiceManager::Start(PServiceProcess * svc)
{
  if (!Open(svc))
    return PFalse;

  PBoolean ok = ::StartService(schService, 0, NULL);
  error = ::GetLastError();

  if (!ok)
    return PFalse;

  SERVICE_STATUS serviceStatus;

  // query the service status
  if (!QueryServiceStatus(schService, &serviceStatus))
    return PFalse;

  // if pending periodicaly re-query the status
  while (serviceStatus.dwCurrentState == SERVICE_START_PENDING) {
    DWORD waitTime = PMIN(serviceStatus.dwWaitHint / 10, 10000);
	Sleep(waitTime);

	if (! QueryServiceStatus(schService, &serviceStatus)) break;
  }

  if (serviceStatus.dwCurrentState == SERVICE_RUNNING) {
    return PTrue;
  } else {
    error = serviceStatus.dwWin32ExitCode;
    return PFalse;
  }
}


PBoolean NT_ServiceManager::Control(PServiceProcess * svc, DWORD command)
{
  if (!Open(svc))
    return PFalse;

  SERVICE_STATUS status;
  PBoolean ok = ::ControlService(schService, command, &status);
  error = ::GetLastError();
  return ok;
}


PBoolean PServiceProcess::ProcessCommand(const char * cmd)
{
  PINDEX cmdNum = 0;
  while (strcasecmp(cmd, ServiceCommandNames[cmdNum]) != 0) {
    if (++cmdNum >= NumSvcCmds) {
      if (!CreateControlWindow(PTrue))
        return PFalse;
      if (*cmd != '\0')
        PError << "Unknown command \"" << cmd << "\".\n";
      else
        PError << "Could not start service.\n";
      PError << "usage: " << GetName() << " [ ";
      for (cmdNum = 0; cmdNum < NumSvcCmds-1; cmdNum++)
        PError << ServiceCommandNames[cmdNum] << " | ";
      PError << ServiceCommandNames[cmdNum] << " ]" << endl;
      return PFalse;
    }
  }

  NT_ServiceManager nt;
  Win95_ServiceManager win95;

  ServiceManager * svcManager;
  if (isWin95)
    svcManager = &win95;
  else
    svcManager = &nt;

  PBoolean good = PFalse;
  switch (cmdNum) {
    case SvcCmdNoWindow :
      if (controlWindow == NULL)
        controlWindow = (HWND)-1;
      break;

    case SvcCmdTray :
      if (CreateControlWindow(PFalse)) {
        PNotifyIconData nid(controlWindow, NIF_MESSAGE|NIF_ICON, GetName());
        nid.hIcon = (HICON)LoadImage(hInstance, MAKEINTRESOURCE(ICON_RESID), IMAGE_ICON, // 16x16 icon
                               GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0);
        nid.uCallbackMessage = UWM_SYSTRAY; // message sent to nid.hWnd
        nid.Add();    // This adds the icon
        debugWindow = (HWND)-1;
        systemLogFileName = DebuggerLogOutput;
        return PTrue;
      }
      return PFalse;

    case SvcCmdNoTray :
      if (TrayIconRegistry(this, CheckTrayIcon)) {
        TrayIconRegistry(this, DelTrayIcon);
        PError << "Tray icon removed.";
      }
      else {
        TrayIconRegistry(this, AddTrayIcon);
        PError << "Tray icon installed.";
      }
      return PTrue;

    case SvcCmdVersion : // Version command
      ::SetLastError(0);
      PError << GetName() << " Version " << GetVersion(PTrue)
             << " by " << GetManufacturer()
             << " on " << GetOSClass()   << ' ' << GetOSName()
             << " ("   << GetOSVersion() << '-' << GetOSHardware() << ')' << endl;
      return PTrue;

    case SvcCmdInstall : // install
      good = svcManager->Create(this);
      TrayIconRegistry(this, AddTrayIcon);
      break;

    case SvcCmdRemove : // remove
      good = svcManager->Delete(this);
      TrayIconRegistry(this, DelTrayIcon);
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
      TrayIconRegistry(this, DelTrayIcon);
#if P_CONFIG_FILE
      PConfig cfg;
      PStringArray sections = cfg.GetSections();
      PINDEX i;
      for (i = 0; i < sections.GetSize(); i++)
        cfg.DeleteSection(sections[i]);
#endif // P_CONFIG_FILE
      good = PTrue;
      break;
  }

  SetLastError(0);

  PError << "Service command \"" << ServiceCommandNames[cmdNum] << "\" ";
  if (good)
    PError << "successful.";
  else {
    PError << "failed - ";
    switch (svcManager->GetError()) {
      case ERROR_ACCESS_DENIED :
        PError << "Access denied";
        break;
      case 0x10000000 :
        PError << "process still running.";
        break;
      default :
        PError << "error code = " << svcManager->GetError();
    }
  }
  PError << endl;

  return PTrue;
}


// End Of File ///////////////////////////////////////////////////////////////
